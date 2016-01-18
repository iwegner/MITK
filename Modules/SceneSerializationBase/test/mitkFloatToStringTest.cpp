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
    MITK_TEST(ConfirmStringValuesFloat);
    MITK_TEST(ConfirmStringValuesDouble);
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

  void ConfirmStringValuesFloat()
  {
      float nan = mitk::StringToFloat("nan");
      float inf = mitk::StringToFloat("inf");
      float minus_inf = mitk::StringToFloat("-inf");

      CPPUNIT_ASSERT_MESSAGE("nan==nan must be false", !(nan==nan) );
      float ref_minus_inf = -std::numeric_limits<float>::infinity();
      CPPUNIT_ASSERT_EQUAL( std::numeric_limits<float>::infinity(), inf );
      CPPUNIT_ASSERT_EQUAL( -std::numeric_limits<float>::infinity(), minus_inf );

      std::string s_nan = mitk::FloatToString( std::numeric_limits<float>::quiet_NaN() );
      CPPUNIT_ASSERT_EQUAL( std::string("nan"), s_nan );
      std::string s_inf = mitk::FloatToString( std::numeric_limits<float>::infinity() );
      CPPUNIT_ASSERT_EQUAL( std::string("inf"), s_inf );
      std::string s_minus_inf = mitk::FloatToString( -std::numeric_limits<float>::infinity() );
      CPPUNIT_ASSERT_EQUAL( std::string("-inf"), s_minus_inf );
}

  void ConfirmStringValuesDouble()
  {
      // we want to make sure that the following strings will be accepted and returned
      // by our conversion functions. This must not change in the future to ensure compatibility
      double nan = mitk::StringToDouble("nan");
      double inf = mitk::StringToDouble("inf");
      double minus_inf = mitk::StringToDouble("-inf");

      CPPUNIT_ASSERT_MESSAGE("nan==nan must be false", !(nan==nan) );
      double ref_minus_inf = -std::numeric_limits<double>::infinity();
      CPPUNIT_ASSERT_EQUAL( std::numeric_limits<double>::infinity(), inf );
      CPPUNIT_ASSERT_EQUAL( -std::numeric_limits<double>::infinity(), minus_inf );

      std::string s_nan = mitk::DoubleToString( std::numeric_limits<double>::quiet_NaN() );
      CPPUNIT_ASSERT_EQUAL( std::string("nan"), s_nan );
      std::string s_inf = mitk::DoubleToString( std::numeric_limits<double>::infinity() );
      CPPUNIT_ASSERT_EQUAL( std::string("inf"), s_inf );
      std::string s_minus_inf = mitk::DoubleToString( -std::numeric_limits<double>::infinity() );
      CPPUNIT_ASSERT_EQUAL( std::string("-inf"), s_minus_inf );
  }


  void TestConversionsFloat()
  {
      // we cannot test the NaN roundtrip because nan == nan will never be true
      CheckRoundTrip_Float(std::numeric_limits<float>::infinity());
      CheckRoundTrip_Float(-std::numeric_limits<float>::infinity());

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
      // we cannot test the NaN roundtrip because nan == nan will never be true
      CheckRoundTrip_Double(std::numeric_limits<double>::infinity());
      CheckRoundTrip_Double(-std::numeric_limits<double>::infinity());

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
