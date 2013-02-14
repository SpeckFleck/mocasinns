/*!
 * \file metropolis.cpp
 * \brief Implementation of the libMoCaSinns template interface
 * 
 * Usage examples are found in the test cases.
 * 
 * \author Johannes F. Knauf
 * \author Benedikt Krüger
 */

#ifdef MOCASINNS_METROPOLIS_HPP

#include <cmath>

// Includes for boost accumulators
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/bind.hpp>
#include <boost/ref.hpp>

namespace ba = boost::accumulators;

#include "details/metropolis/vector_accumulator.hpp"

namespace Mocasinns
{

/*!
 * \param num_steps Number of Metropolis steps that will be performed
 * \param beta Inverse temperature that will be used for calculation of the acceptance probability of the Metropolis steps.
 */
template <class ConfigurationType, class Step, class RandomNumberGenerator>
template <class TemperatureType>
void Metropolis<ConfigurationType, Step, RandomNumberGenerator>::do_metropolis_steps(const uint32_t& num_steps, const TemperatureType& beta = 0)
{
  for (uint32_t i = 0; i < num_steps; i++)
  {
    Step next_step = this->configuration_space->propose_step(this->rng);

    if (next_step.is_executable())
    {
      // One does need to define this in advance, because the Temperature Type need not to be comparable to an double 0.0
      double beta_times_delta_E = beta*next_step.delta_E();
      double selection_probability_factor = next_step.selection_probability_factor();
      double random_accept = this->rng->random_double();
      if (beta_times_delta_E <= -log(selection_probability_factor) || random_accept < (1.0/selection_probability_factor)*exp(-beta_times_delta_E))
      {
	next_step.execute();
      }
    }
  }
}

/*!
  \tparam Observable Class with static function Observable::observe(ConfigurationType*) taking a pointer to the simulation and returning the value of a arbitrary observable. The class must contain a typedef ::observable_type classifying the return type of the functor.
  \tparam TemperatureType Type of the inverse temperature, there must be an operator* defined this class and the energy type of the configuration.
  \param beta Inverse temperature at which the simulation is performed.
  \returns Vector containing the single measurements performed
*/
template<class ConfigurationType, class Step, class RandomNumberGenerator>
template<class Observable, class TemperatureType>
std::vector<typename Observable::observable_type> Metropolis<ConfigurationType, Step, RandomNumberGenerator>::do_metropolis_simulation(const TemperatureType& beta)
{
  // Call the accumulator function using the VectorAccumulator
  Details::Metropolis::VectorAccumulator<typename Observable::observable_type> measurements_accumulator;
  do_metropolis_simulation<Observable>(beta, measurements_accumulator);

  // Return the plain data
  return measurements_accumulator.internal_vector;
}

/*!  
  \tparam Observable Class with static function Observable::observe(ConfigurationType*) taking a pointer to the simulation and returning the value of a arbitrary observable. The class must contain a typedef ::observable_type classifying the return type of the functor.
  \tparam InputIterator Type of the iterator that iterates the different temperatures that will be considered
  \tparam TemperatureType Type of the inverse temperature, there must be an operator* defined this class and the energy type of the configuration.
  \param beta_begin Iterator pointing to the first inverse temperature that is calculated
  \param beta_end Iterator pointing on position after the last inverse temperature that is calculated
  \returns Vector containing the vectors of measurments performed for each temperature. (First index: inverse temperature, second index: measurment number)
*/
template<class ConfigurationType, class Step, class RandomNumberGenerator>
template<class Observable, class InputIterator>
std::vector<std::vector<typename Observable::observable_type> > Metropolis<ConfigurationType, Step, RandomNumberGenerator>::do_metropolis_simulation(InputIterator first_beta, InputIterator last_beta)
{
  std::vector<std::vector<typename Observable::observable_type> > results;
  for (InputIterator beta = first_beta; beta != last_beta; ++beta)
  {
    results.push_back(do_metropolis_simulation(*beta));
    if (this->is_terminating) break;
  }
}  

/*!
 \tparam Observable Class with static function Observable::observe(ConfigurationType*) taking a pointer to the simulation and returning the value of an arbitrary observable. The class must contain a typedef ::observable_type classifying the return type of the functor.
 \tparam Accumulator Class that accepts the observable in operator() and gathers the required informations about the observables (e.g. boost::accumulator)
 \tparam TemperatureType Type of the inverse temperature, there must be an operator* defined this class and the energy type of the configuration.
 \param beta Inverse temperature at which the simulation is performed
 \param measurement_accumulator Reference to the accumulator that stores the simulation results
*/
template<class ConfigurationType, class Step, class RandomNumberGenerator>
template<class Observable, class Accumulator, class TemperatureType>
void Metropolis<ConfigurationType,Step,RandomNumberGenerator>::do_metropolis_simulation(const TemperatureType& beta, Accumulator& measurement_accumulator)
{
 // Perform the relaxation steps
  do_metropolis_steps(simulation_parameters.relaxation_steps, beta);
  
  // For each measurement, perform the steps, invoke the signal handler, take the measurement and check for posix signals
  for (unsigned int m = 0; m < simulation_parameters.measurement_number; ++m)
  {
    do_metropolis_steps(simulation_parameters.steps_between_measurement, beta);
    signal_handler_measurement(this);
    measurement_accumulator(Observable::observe(this->configuration_space));
    if (this->check_for_posix_signal()) return;
  }
}

/*!
 \tparam Observable Class with static function Observable::observe(ConfigurationType*) taking a pointer to the simulation and returning the value of an arbitrary observable. The class must contain a typedef ::observable_type classifying the return type of the functor.
 \tparam AccumulatorIterator Iterator of a container of a class that accepts the observable in operator() and gathers the required informations about the observables (e.g. boost::accumulator)
 \tparam InverseTemperatureIterator  Iterator of a container of a type of the inverse temperature, there must be an operator* defined this class and the energy type of the configuration.
 \param beta_begin Iterator pointing to the first inverse temperature that is calculated
 \param beta_end Iterator pointing on position after the last inverse temperature that is calculated
 \param measurement_accumulator_begin Iterator pointing to the first accumulator that calculates the data for the first inverse temperature.
 \param measurement_accumulator_end Iterator pointing one position after the last accumulator that calculates the data for the last inverse temperature
*/
template<class ConfigurationType, class Step, class RandomNumberGenerator>
template<class Observable, class AccumulatorIterator, class InverseTemperatureIterator>
void Metropolis<ConfigurationType,Step,RandomNumberGenerator>::do_metropolis_simulation(InverseTemperatureIterator beta_begin, InverseTemperatureIterator beta_end, AccumulatorIterator measurement_accumulator_begin, AccumulatorIterator measurement_accumulator_end)
{  
  InverseTemperatureIterator beta_iterator = beta_begin;
  AccumulatorIterator measurement_accumulator_iterator = measurement_accumulator_begin;
  for (; beta_iterator != beta_end; ++beta_iterator, ++measurement_accumulator_iterator)
  {
    do_metropolis_simulation(*beta_iterator, *measurement_accumulator_iterator);
    if (this->is_terminating) break;
  }
}

/*!
  \details Calculates the autocorrelation function C(t) of the observable f using the formula
  \f[
  C_f(t) = \left\langle f_0 f_t \right\rangle - \langle f \rangle^2
  \f]
  where \f$ f_t \f$ is the value of the observable at time \f$ t \f$. The second average is taken over all measurements done in this simulation. The first average is calculated as following:
  \f[
  \left\langle f_0 f_t \right\rangle = \frac{1}{N} \sum_{i=0}^{N-1} f_{i\cdot s} f_{i\cdot s + t}
  \f]
  where \f$ N \f$ is the parameters simulation_time_factor and s is the parameter maximal_time (the time indices are allways measured in units of the system size). Before taking the measurements the number of relaxation steps set in the parameters are taken.

  \tparam Observable Class with static function Observable::observe(ConfigurationType*) taking a pointer to the simulation and returning the value of an arbitrary observable. The class must contain a typedef ::observable_type classifying the return type of the functor.
  \tparam TemperatureType Type of the inverse temperature, there must be an operator* defined this class and the energy type of the configuration.
  \param beta Inverse temperature at which the simulation is performed
  \param maximal_time Maximal time for the correlation function and size of the returned vector, in units of ConfigurationType::system_size
  \param simulation_time_factor Run time of the simulation in units of the maximal time (also gives the number of measurements taken for averaging the value of the autocorrelation function at each time). The default value is 5.
*/									
template<class ConfigurationType, class Step, class RandomNumberGenerator>
template<class Observable, class TemperatureType>
std::vector<typename Observable::observable_type> Metropolis<ConfigurationType, Step, RandomNumberGenerator>::autocorrelation_function(const TemperatureType& beta, unsigned int maximal_time, unsigned int simulation_time_factor)
{
  // Do the relaxation steps
  do_metropolis_steps(simulation_parameters.relaxation_steps, beta);

  // Define the vector with the results
  std::vector<typename Observable::observable_type> results;

  // Define the vector with the measurments of the observable and take the measurements
  std::vector<typename Observable::observable_type> observable_measurements;
  for (unsigned int i = 0; i <= maximal_time*simulation_time_factor; ++i)
  {
    do_metropolis_steps(this->get_config_space()->system_size(), beta);
    observable_measurements.push_back(Observable::observe(this->get_config_space()));
  }

  // Define an accumulator and calculate the mean value of the observable
  ba::accumulator_set<typename Observable::observable_type, ba::stats<ba::tag::mean> > acc_measured_mean(observable_measurements[0]);
  for (unsigned int i = 0; i <= maximal_time*simulation_time_factor; ++i)
  {
    acc_measured_mean(observable_measurements[i]);
  }
  typename Observable::observable_type measured_mean = ba::mean(acc_measured_mean);

  // Calculate the autocorrelation function using an accumulator
  for (unsigned int time = 0; time <= maximal_time; ++time)
  {
    // Calculate the mean autocorrelation function for time
    ba::accumulator_set<typename Observable::observable_type, ba::stats<ba::tag::mean> > acc_autocorrelation_function_time(observable_measurements[0]);
    for (unsigned int sweep = 0; sweep < simulation_time_factor; ++sweep)
    {
      unsigned int start_time = sweep*maximal_time;
      acc_autocorrelation_function_time(observable_measurements[start_time]*observable_measurements[start_time + time]);
    }
    
    // Add to the result vector
    results.push_back(ba::mean(acc_autocorrelation_function_time) - measured_mean*measured_mean);
  }

  // Return the result vector
  return results;
}

/*!
  \details Calculates the integrated autocorrelation time \f$ \tau_\mathrm{int} \f$ based on the autocorrelation function \f$ C(t) \f$ using this formula:
  \f[
  \tau_\mathrm{int} = \left[1 + 2\sum_{t=1}^{N-1} \left( 1 - \frac{t}{N}\right) \frac{C(t)}{C(0)} \right]
  \f]
  where \f$ N \f$ denotes the maximal considered time.

  \tparam Observable Class with static function Observable::observe(ConfigurationType*) taking a pointer to the simulation and returning the value of an arbitrary observable. The class must contain a typedef ::observable_type classifying the return type of the functor.
  \tparam TemperatureType Type of the inverse temperature, there must be an operator* defined this class and the energy type of the configuration.
  \param beta Inverse temperature at which the simulation is performed
  \param maximal_time Maximal time for the correlation function and size of the returned vector, in units of ConfigurationType::system_size
  \param simulation_time_factor Run time of the simulation in units of the maximal time (also gives the number of measurements taken for averaging the value of the autocorrelation function at each time). The default value is 5.
 */
template<class ConfigurationType, class Step, class RandomNumberGenerator>
template<class Observable, class TemperatureType>
typename Observable::observable_type Metropolis<ConfigurationType, Step, RandomNumberGenerator>::integrated_autocorrelation_time(const TemperatureType& beta, unsigned int maximal_time, unsigned int considered_time_factor)
{
  // Calculate the autocorrelation function
  std::vector<typename Observable::observable_type> vec_autocorrelation_function = autocorrelation_function<Observable>(beta, maximal_time, considered_time_factor);

  // Calculate the integrated autocorrelation time
  typename Observable::observable_type result = vec_autocorrelation_function[0]/vec_autocorrelation_function[0]; // Use this to initialise a one in each component even if using an VectorObservable or ArrayObservable
  for (unsigned int t = 1; t < maximal_time; ++t)
  {
    result += 2.0*(1.0 - static_cast<double>(t)/static_cast<double>(maximal_time))*(vec_autocorrelation_function[t]/vec_autocorrelation_function[0]);
  }

  return result;
}

template <class ConfigurationType, class Step, class RandomNumberGenerator>
void Metropolis<ConfigurationType, Step, RandomNumberGenerator>::load_serialize(std::istream& input_stream)
{
  boost::archive::text_iarchive input_archive(input_stream);
  input_archive >> (*this);
}
template <class ConfigurationType, class Step, class RandomNumberGenerator>
void Metropolis<ConfigurationType, Step, RandomNumberGenerator>::load_serialize(const char* filename)
{
  std::ifstream input_filestream(filename);
  load_serialize(input_filestream);
  input_filestream.close();
}
template <class ConfigurationType, class Step, class RandomNumberGenerator>
void Metropolis<ConfigurationType, Step, RandomNumberGenerator>::save_serialize(std::ostream& output_stream) const
{
  boost::archive::text_oarchive output_archive(output_stream);
  output_archive << (*this);
}
template <class ConfigurationType, class Step, class RandomNumberGenerator>
void Metropolis<ConfigurationType, Step, RandomNumberGenerator>::save_serialize(const char* filename) const
{
  std::ofstream output_filestream(filename);
  save_serialize(output_filestream);
  output_filestream.close();
}

} // of namespace Mocasinns

#endif
