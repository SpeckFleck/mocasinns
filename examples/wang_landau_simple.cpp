// Includes from libising
#include <spinlattice.hpp>
#include <spin_ising.hpp>
// Includes from libmocasinns
#include <mocasinns/wang_landau.hpp>
#include <mocasinns/histograms/histocrete.hpp>
// Includes from librandom
#include <random_boost_mt19937.hpp>

// Typedef for the configuration and the step
typedef Ising::SpinLattice<2, Ising::SpinIsing> ConfigurationType;
typedef Ising::Step<2, Ising::SpinIsing> StepType;

// Typedef for the Simulation
typedef Mocasinns::WangLandau<ConfigurationType, StepType, int, Mocasinns::Histograms::Histocrete, Boost_MT19937> Simulation;

int main()
{
  // Create a configuration of size 10x10
  std::vector<unsigned int> size(2, 10);
  ConfigurationType lattice(size);

  // Set the parameters of the simulation
  Simulation::Parameters<int> simulation_parameters;
  simulation_parameters.modification_factor_final = 1e-6;
  simulation_parameters.modification_factor_multiplier = 0.9;
  simulation_parameters.flatness = 0.8;

  // Create the simulation
  Simulation sim(simulation_parameters, &lattice);
  sim.set_random_seed(0);

  // Do the Wang-Landau-simulation
  sim.do_wang_landau_simulation();

  // Extract the density of states
  Mocasinns::Histograms::Histocrete<int, double> dos_log = sim.get_density_of_states();
  std::cout << "E\tg(E)" << std::endl;
  for (Mocasinns::Histograms::Histocrete<int, double>::const_iterator it = dos_log.begin(); it != dos_log.end(); ++it)
  {
    std::cout << it->first << "\t" << exp(it->second) << std::endl;
  }
}