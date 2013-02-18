/**
 * \file histogram.hpp
 * \brief Histogram = Histogram class storing binned values, derived from HistoBase
 * 
 * The Histogram is basically a wrapper for a std::map and is used for storing a histogram with binned x values.
 * 
 * \author Benedikt Krüger
 */

#ifndef MOCASINNS_HISTOGRAMS_HISTOGRAM_HPP
#define MOCASINNS_HISTOGRAMS_HISTOGRAM_HPP

#include "histobase.hpp"
#include "binnings.hpp"

// Boost serialization for derived classes
#include <boost/serialization/base_object.hpp>

namespace Mocasinns
{
namespace Histograms
{

//! Class for a binned histogram
template <class x_value_type, class y_value_type, class BinningFunctor> 
class Histogram : public HistoBase<x_value_type, y_value_type>
{
private:
  BinningFunctor binning;

  // Serialization stuff
  //! Member variable for boost serialization
  friend class boost::serialization::access;
  //! Method to serialize this class (omitted version name to avoid unused parameter warnings)
  template<class Archive> void serialize(Archive & ar, const unsigned int)
  {
    // serialize base class information
    ar & boost::serialization::base_object<HistoBase<x_value_type, y_value_type> >(*this);
    ar & binning;
  }

public:
  // Typedefs for iterator
  // Necessary because this is a class template
  typedef typename HistoBase<x_value_type, y_value_type>::iterator iterator;
  typedef typename HistoBase<x_value_type, y_value_type>::const_iterator const_iterator;
  typedef typename HistoBase<x_value_type, y_value_type>::reverse_iterator reverse_iterator;
  typedef typename HistoBase<x_value_type, y_value_type>::const_reverse_iterator const_reverse_iterator;
  typedef typename HistoBase<x_value_type, y_value_type>::value_type value_type;
  typedef typename HistoBase<x_value_type, y_value_type>::size_type size_type;

  //! Standard constructor
  Histogram() : binning(1.0, 0.0) {}
  //! Constructor taking a binning functor
  /*!
   * \param binning_functor Class that maps a arbitrary x-value to another x-value that corresponds to a specific binning encoded in this class using the binning_functor::operator()
   */
  Histogram(BinningFunctor binning_functor) : binning(binning_functor) {}
  //! Constructor taking the binning width and the binning reference
  Histogram(x_value_type binning_width, x_value_type binning_reference) : binning(binning_width, binning_reference) {}
  //! Copy constructor
  Histogram(HistoBase<x_value_type, y_value_type>& other) : HistoBase<x_value_type,y_value_type>(other), binning(other.binning) {}

  //! Get-accessor for the binning functor
  BinningFunctor get_binning() const { return binning; }
  //! Set-accessor for the binning functor
  void set_binning(BinningFunctor value) { binning = value; }
  //! Get-accessor for the width of the binning
  x_value_type get_binning_width() const { return binning.get_binning_width(); }
  //! Set-accessor for the width of the binning
  void set_binning_width(x_value_type value) { binning.set_binning_width(value); }
  //! Get-accessor for the reference point of the binning
  x_value_type get_binning_reference() const { return binning.get_binning_reference(); }
  //! Set-accessor for the reference point of the binning
  void set_binning_reference(x_value_type value) { binning.set_binning_reference(value); }

  // Operators
  //! Increment the y-value of the given Histogram bin by one
  void operator<< (const x_value_type & bin) { this->values[binning(bin)] += 1; }
  //! Increment the y-value of the given Histocrete bin by the given y-value
  void operator<< (const value_type & xy_pair) { this->values[binning(xy_pair.first)] += xy_pair.second; }
  //! Value of the Histogram at given bin, takes binning into account
  y_value_type& operator[] (const x_value_type & bin) { return this->values[binning(bin)]; }
  //! Value of the Histogram at given bin, takes binning into account
  const y_value_type& operator[] (const x_value_type & bin) const { return this->values[binning(bin)]; }
  //! Adds a given HistoBase (using the binning function) to this Histogram
  Histogram<x_value_type, y_value_type, BinningFunctor>& operator+= (HistoBase<x_value_type, y_value_type>& other_histobase);
  //! Adds a given value to all bins of this Histogram
  Histogram<x_value_type, y_value_type, BinningFunctor>& operator+= (const y_value_type& const_value);
  //! Devides this Histogram binwise through a given HistoBase
  Histogram<x_value_type, y_value_type, BinningFunctor>& operator/= (HistoBase<x_value_type, y_value_type>& other_histobase);
  //! Devides this Histogram binwise through a given value
  Histogram<x_value_type, y_value_type, BinningFunctor>& operator/= (const y_value_type& const_value); 

  //! Bin a calue
  virtual x_value_type bin_value(x_value_type value) { return binning(value); }

  //! Initialise the Histogram with all necessary data of another Histogram, but sets all y-values to 0
  template <class other_y_value_type>
  void initialise_empty(const Histogram<x_value_type, other_y_value_type, BinningFunctor>& other);

  //! Insert element, take binning into account
  std::pair<iterator, bool> insert(const value_type& x) { return this->values.insert(value_type(binning(x.first), x.second)); }
  //! Insert element, take binning into account
  iterator insert(iterator position, const value_type& x) { return this->values.insert(position, value_type(binning(x.first), x.second)); }
  //! Insert elements, take binning into account
  template <class InputIterator> void insert(InputIterator first, InputIterator last) 
  { 
    for (InputIterator it = first; it != last; ++it)
      this->insert(*it); 
  }

  //! Load the data of the histogram from a serialization stream
  virtual void load_serialize(std::istream& input_stream);
  //! Load the data of the histogram from a serialization file
  virtual void load_serialize(const char* filename);
  //! Save the data of the histogram to a serialization stream
  virtual void save_serialize(std::ostream& output_stream) const;
  //! Save the data of the histogram to a serialization file
  virtual void save_serialize(const char* filename) const;
};

//! Class implementing a histogram for number x_value_types
template<class x_value_type, class y_value_type> 
class HistogramNumber : public Histogram<x_value_type, y_value_type, BinningNumber<x_value_type> >
{
public:
  typedef Histogram<x_value_type, y_value_type, BinningNumber<x_value_type> > Base;
  
  //! Standard constructor
  HistogramNumber() : Base() {}
  //! Constructor taking a binning functor
  /*!
   * \param binning_functor Class that maps a arbitrary x-value to another x-value that corresponds to a specific binning encoded in this class using the binning_functor::operator()
   */
  HistogramNumber(BinningNumber<x_value_type> binning_functor) : Base(binning_functor) {}
  //! Constructor taking the binning width and the binning reference
  HistogramNumber(x_value_type binning_width, x_value_type binning_reference) : Base(binning_width, binning_reference) {}
  //! Copy constructor
  HistogramNumber(const HistoBase<x_value_type, y_value_type>& other) : Base(other) {}  
};

/*!
  \brief Class implementing a histogram for vector of numbers as x_value_types

  \details It is not possible to give directly the value_type of the vector, because this would break the notion that this is the x-value type
  \tparam x_value_type Type of the x-values of the histogram, must be a std::vector<T> of a type T
  \tparam y_value_type Type of the y-values of the histogram

  \bug The class does not work, because std::vector does not support an operator<< for csv output. This has to be implemented manually.
 */
template<class x_value_type, class y_value_type>
class HistogramVector : public Histogram<x_value_type, y_value_type, BinningNumberVector<typename x_value_type::value_type> >
{
public:
  typedef Histogram<x_value_type, y_value_type, BinningNumberVector<typename x_value_type::value_type> > Base;
  
  //! Standard constructor
  HistogramVector() : Base() {}
  //! Constructor taking a binning functor
  /*!
   * \param binning_functor Class that maps a arbitrary x-value to another x-value that corresponds to a specific binning encoded in this class using the binning_functor::operator()
   */
  HistogramVector(BinningNumberVector<x_value_type> binning_functor) : Base(binning_functor) {}
  //! Constructor taking the binning width and the binning reference
  HistogramVector(x_value_type binning_width, x_value_type binning_reference) : Base(binning_width, binning_reference) {}
  //! Copy constructor
  HistogramVector(const HistoBase<x_value_type, y_value_type>& other) : Base(other) {}  

  //! Load the data of the HistogramVector from a serialization stream
  //! \todo Needs to be implemented
  void load_serialize(std::istream& input_stream);
  //! Load the data of the HistogramVector from a serialization file
  //! \todo Needs to be implemented
  void load_serialize(const char* filename);
  //! Save the data of the HistogramVector to a serialization stream
  //! \todo Needs to be implemented
  void save_serialize(std::ostream& output_stream) const;
  //! Save the data of the HistogramVector to a serialization file
  //! \todo Needs to be implemented
  void save_serialize(const char* filename) const;

};

} // of namespace Histogram
} // of namespace Mocasinns

#include "histogram.cpp"

#endif
