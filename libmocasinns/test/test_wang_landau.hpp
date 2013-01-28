#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/Test.h>
#include <cppunit/extensions/HelperMacros.h>

#include <spinlattice.hpp>
#include <spin_ising.hpp>

#include <wang_landau.hpp>
#include <histocrete.hpp>
#include <random_boost_mt19937.hpp>

#ifndef TEST_WANG_LANDAU_HPP
#define TEST_WANG_LANDAU_HPP

typedef SpinLattice<1, SpinIsing> IsingConfiguration1d;
typedef Step<1, SpinIsing> IsingStep1d;
typedef WangLandau<IsingConfiguration1d, IsingStep1d, int, Histocrete, Boost_MT19937> IsingSimulation1d;

typedef SpinLattice<2, SpinIsing> IsingConfiguration2d;
typedef Step<2, SpinIsing> IsingStep2d;
typedef WangLandau<IsingConfiguration2d, IsingStep2d, int, Histocrete, Boost_MT19937> IsingSimulation2d;

class TestWangLandau : CppUnit::TestFixture
{
private:
  IsingConfiguration1d* test_ising_config_1d;
  IsingSimulation1d* test_ising_simulation_1d;
  IsingConfiguration2d* test_ising_config_2d;
  IsingSimulation2d* test_ising_simulation_2d;

  IsingSimulation1d::Parameters<int> parameters_1d;
  IsingSimulation2d::Parameters<int> parameters_2d;

public:
  static CppUnit::Test* suite();
  
  void setUp();
  void tearDown();

  void test_do_wang_landau_steps();
  void test_do_wang_landau_simulation();

  void test_serialize();
};

#endif