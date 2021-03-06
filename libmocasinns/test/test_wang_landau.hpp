#ifndef TEST_WANG_LANDAU_HPP
#define TEST_WANG_LANDAU_HPP

#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/Test.h>
#include <cppunit/extensions/HelperMacros.h>

#include <gespinst/spin_lattice.hpp>
#include <gespinst/spins/ising_spin.hpp>

#include <mocasinns/wang_landau.hpp>
#include <mocasinns/histograms/histocrete.hpp>
#include <mocasinns/random/boost_random.hpp>

using namespace Mocasinns;

typedef Gespinst::SpinLattice<1, Gespinst::IsingSpin> IsingConfiguration1d;
typedef Gespinst::SpinLatticeStep<1, Gespinst::IsingSpin> IsingStep1d;
typedef WangLandau<IsingConfiguration1d, IsingStep1d, int, Histograms::Histocrete, Random::Boost_MT19937> IsingSimulation1d;

typedef Gespinst::SpinLattice<2, Gespinst::IsingSpin> IsingConfiguration2d;
typedef Gespinst::SpinLatticeStep<2, Gespinst::IsingSpin> IsingStep2d;
typedef WangLandau<IsingConfiguration2d, IsingStep2d, int, Histograms::Histocrete, Random::Boost_MT19937> IsingSimulation2d;

class TestWangLandau : CppUnit::TestFixture
{
private:
  IsingConfiguration1d* test_ising_config_1d;
  IsingSimulation1d* test_ising_simulation_1d;
  IsingConfiguration2d* test_ising_config_2d;
  IsingSimulation2d* test_ising_simulation_2d;

  IsingSimulation1d::Parameters parameters_1d;
  IsingSimulation2d::Parameters parameters_2d;

public:
  static CppUnit::Test* suite();
  
  void setUp();
  void tearDown();

  void test_do_wang_landau_steps();
  void test_do_wang_landau_simulation();

  void test_serialize();
};

#endif
