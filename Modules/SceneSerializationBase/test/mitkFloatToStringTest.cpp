/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include "mitkFloatToString.h"

#include <limits>
#include <math.h>

class mitkFloatToStringTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkFloatToStringTestSuite);
    MITK_TEST(TestConversionsFloat);
    MITK_TEST(TestConversionsDouble);
  CPPUNIT_TEST_SUITE_END();

public:

  void setUp() override
  {
  }


  void tearDown() override
  {
  }

  void CheckRoundTrip_Float(float input)
  {
    std::string s = mitk::FloatToString(input);
    float result = mitk::StringToFloat(s);
    CPPUNIT_ASSERT_EQUAL(input, result);
  }

  void TestConversionsFloat()
  {
      float nan = mitk::StringToFloat("nan");
      float inf = mitk::StringToFloat("inf");
      float minus_inf = mitk::StringToFloat("-inf");

      float ref_inf = std::numeric_limits<float>::infinity();
      float ref_minus_inf = -std::numeric_limits<float>::infinity();
      CPPUNIT_ASSERT_MESSAGE("nan==nan must be false", !(nan==nan) );
      float not_parsed = mitk::StringToFloat("quark");
      CPPUNIT_ASSERT(!(not_parsed == not_parsed)); // we expect NaN
      CPPUNIT_ASSERT_EQUAL( ref_inf, inf);
      CPPUNIT_ASSERT_EQUAL( ref_minus_inf, minus_inf);

      // we cannot test the NaN roundtrip because nan == nan will never be true
      CheckRoundTrip_Float(inf);
      CheckRoundTrip_Float(minus_inf);

      CheckRoundTrip_Float(std::numeric_limits<float>::denorm_min());
      CheckRoundTrip_Float(std::numeric_limits<float>::epsilon());
      CheckRoundTrip_Float(std::numeric_limits<float>::lowest());
      CheckRoundTrip_Float(std::numeric_limits<float>::min());
      CheckRoundTrip_Float(std::numeric_limits<float>::max());
      CheckRoundTrip_Float(sqrt(2));
      CheckRoundTrip_Float(0.0);
      CheckRoundTrip_Float(-0.0);
  }

  void CheckRoundTrip_Double(float input)
  {
    std::string s = mitk::DoubleToString(input);
    float result = mitk::StringToDouble(s);
    CPPUNIT_ASSERT_EQUAL(input, result);
  }


  void TestConversionsDouble()
  {
      double nan = mitk::StringToDouble("nan");
      double inf = mitk::StringToDouble("inf");
      double minus_inf = mitk::StringToDouble("-inf");

      double ref_inf = std::numeric_limits<double>::infinity();
      double ref_minus_inf = -std::numeric_limits<double>::infinity();
      CPPUNIT_ASSERT_MESSAGE("nan==nan must be false", !(nan==nan) );
      CPPUNIT_ASSERT_EQUAL( std::string("nan"), mitk::DoubleToString(nan) );
      double not_parsed = mitk::StringToDouble("quark");
      CPPUNIT_ASSERT(!(not_parsed == not_parsed)); // we expect NaN
      CPPUNIT_ASSERT_EQUAL( ref_inf, inf);
      CPPUNIT_ASSERT_EQUAL( ref_minus_inf, minus_inf);

      // we cannot test the NaN roundtrip because nan == nan will never be true
      CheckRoundTrip_Double(inf);
      CheckRoundTrip_Double(minus_inf);

      CheckRoundTrip_Double(std::numeric_limits<double>::denorm_min());
      CheckRoundTrip_Double(std::numeric_limits<double>::epsilon());
      CheckRoundTrip_Double(std::numeric_limits<double>::lowest());
      CheckRoundTrip_Double(std::numeric_limits<double>::min());
      CheckRoundTrip_Double(std::numeric_limits<double>::max());
      CheckRoundTrip_Double(sqrt(2));
      CheckRoundTrip_Double(0.0);
      CheckRoundTrip_Double(-0.0);
  }
}; // class

MITK_TEST_SUITE_REGISTRATION(mitkFloatToString)
