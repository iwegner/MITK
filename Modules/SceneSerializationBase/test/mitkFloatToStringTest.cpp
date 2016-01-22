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
#include "mitkEqual.h"

#include "mitkLog.h"

//!
//! Verifies functions FloatToString, DoubleToString and inverse
//! functions StringToFloat, StringToDouble.
//!
//! Verifies:
//! - special numbers behavior:
//!   - infinity converted to "inf", parsed from "inf" or "infinity" (case-independent)
//!   - not-a-number converted to "nan", parsed from "nan" (case-independent)
//! - Default round-trip conversion double-to-string-to-double compares equal given
//!   the MITK default epsilon value (mitk::Equal)
//!
class mitkFloatToStringTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkFloatToStringTestSuite);
    MITK_TEST(ConfirmStringValuesFloat);
    MITK_TEST(TestConversionsFloat);
    MITK_TEST(ConfirmStringValuesDouble);
    MITK_TEST(TestConversionsDouble);
    MITK_TEST(TestPrecisionParameter);
  CPPUNIT_TEST_SUITE_END();

public:

  //-------------------- float --------------------

  void ConfirmStringToFloat(const std::string& s, float number)
  {
      CPPUNIT_ASSERT_EQUAL( number,  mitk::StringToFloat(s) );
  }

  void ConfirmFloatToString(float number, const std::string& s)
  {
      CPPUNIT_ASSERT_EQUAL( mitk::StringToFloat(s), number );
  }

  void ConfirmStringValuesFloat()
  {
      // we want to make sure that the following strings will be accepted and returned
      // by our conversion functions. This must not change in the future to ensure compatibility
      float nan = mitk::StringToFloat("nan");
      CPPUNIT_ASSERT_MESSAGE("nan==nan must be false", !(nan==nan) );
      nan = mitk::StringToFloat("NAN");
      CPPUNIT_ASSERT_MESSAGE("NAN==NAN must be false", !(nan==nan) );

      std::string s_nan = mitk::FloatToString(nan);
      CPPUNIT_ASSERT_EQUAL(std::string("nan"), s_nan);


      ConfirmStringToFloat("inf", std::numeric_limits<float>::infinity());
      ConfirmStringToFloat("INF", std::numeric_limits<float>::infinity());
      ConfirmStringToFloat("infinity", std::numeric_limits<float>::infinity());
      ConfirmStringToFloat("INFINITY", std::numeric_limits<float>::infinity());

      ConfirmStringToFloat("-inf", -std::numeric_limits<float>::infinity());
      ConfirmStringToFloat("-INF", -std::numeric_limits<float>::infinity());
      ConfirmStringToFloat("-infinity", -std::numeric_limits<float>::infinity());
      ConfirmStringToFloat("-INFINITY", -std::numeric_limits<float>::infinity());

      ConfirmFloatToString(std::numeric_limits<float>::infinity(), "inf");
      ConfirmFloatToString(-std::numeric_limits<float>::infinity(), "-inf");
  }

  void CheckRoundTrip_Float(float input)
  {
    std::string s = mitk::FloatToString(input);
    float result = mitk::StringToFloat(s);
    CPPUNIT_ASSERT_EQUAL(input, result);
  }

  void CheckRoundTrip_Float(const std::string& input)
  {
    float f = mitk::StringToFloat(input);
    std::string result = mitk::FloatToString(f);

    // There are normal imprecisions when converting to string
    // We do only compare if the numeric values match "close enough"
    double f2 = mitk::StringToFloat(result);
    CPPUNIT_ASSERT(mitk::Equal(f, f2));
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
      CheckRoundTrip_Float(0.000000042);
      CheckRoundTrip_Float(422345678.2345678);
      CheckRoundTrip_Float(0.0);
      CheckRoundTrip_Float(-0.0);

      CheckRoundTrip_Float("1");
      CheckRoundTrip_Float("1.1");
      CheckRoundTrip_Float("1.12121212");
      CheckRoundTrip_Float("1.1e-2");
  }

  //-------------------- double --------------------

  void ConfirmStringToDouble(const std::string& s, double number)
  {
      CPPUNIT_ASSERT_EQUAL( number,  mitk::StringToDouble(s) );
  }

  void ConfirmDoubleToString(double number, const std::string& s)
  {
      CPPUNIT_ASSERT_EQUAL( mitk::StringToDouble(s), number );
  }

  void ConfirmStringValuesDouble()
  {
      // we want to make sure that the following strings will be accepted and returned
      // by our conversion functions. This must not change in the future to ensure compatibility
      double nan = mitk::StringToDouble("nan");
      CPPUNIT_ASSERT_MESSAGE("nan==nan must be false", !(nan==nan) );
      nan = mitk::StringToDouble("NAN");
      CPPUNIT_ASSERT_MESSAGE("NAN==NAN must be false", !(nan==nan) );

      std::string s_nan = mitk::DoubleToString(nan);
      CPPUNIT_ASSERT_EQUAL(std::string("nan"), s_nan);

      ConfirmStringToDouble("inf", std::numeric_limits<double>::infinity());
      ConfirmStringToDouble("INF", std::numeric_limits<double>::infinity());
      ConfirmStringToDouble("infinity", std::numeric_limits<double>::infinity());
      ConfirmStringToDouble("INFINITY", std::numeric_limits<double>::infinity());

      ConfirmStringToDouble("-inf", -std::numeric_limits<double>::infinity());
      ConfirmStringToDouble("-INF", -std::numeric_limits<double>::infinity());
      ConfirmStringToDouble("-infinity", -std::numeric_limits<double>::infinity());
      ConfirmStringToDouble("-INFINITY", -std::numeric_limits<double>::infinity());

      ConfirmDoubleToString(std::numeric_limits<double>::infinity(), "inf");
      ConfirmDoubleToString(-std::numeric_limits<double>::infinity(), "-inf");
  }

  void CheckRoundTrip_Double(double input)
  {
    std::string s = mitk::DoubleToString(input);
    double result = mitk::StringToDouble(s);
    CPPUNIT_ASSERT(mitk::Equal(input, result));
  }

  void CheckRoundTrip_Double(const std::string& input)
  {
    double d = mitk::StringToDouble(input);
    std::string result = mitk::DoubleToString(d);
    double d2 = mitk::StringToDouble(result);

    // There are normal imprecisions when converting to string
    // We do only compare if the numeric values match "close enough"
    CPPUNIT_ASSERT(mitk::Equal(d, d2));
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
      CheckRoundTrip_Double(0.000000042);
      CheckRoundTrip_Double(422345678.2345678);
      CheckRoundTrip_Double(0.0);
      CheckRoundTrip_Double(-0.0);

      CheckRoundTrip_Double("1");
      CheckRoundTrip_Double("1.1");
      CheckRoundTrip_Double("1.12121212");
      CheckRoundTrip_Double("1.1e-2");
  }

  void TestPrecisionParameter()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("5.213"), mitk::DoubleToString(5.213));
    CPPUNIT_ASSERT_EQUAL(std::string("5"), mitk::DoubleToString(5.213, 1));
    CPPUNIT_ASSERT_EQUAL(std::string("5.2"), mitk::DoubleToString(5.213, 2));
  }

}; // class

MITK_TEST_SUITE_REGISTRATION(mitkFloatToString)
