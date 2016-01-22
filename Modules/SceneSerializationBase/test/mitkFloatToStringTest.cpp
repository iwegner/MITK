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

#include <math.h>
#include "mitkEqual.h"

#include "mitkLog.h"


#include <limits>
#include <functional>

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
    MITK_TEST(ConfirmStringValues);
    MITK_TEST(TestConversions);
    MITK_TEST(TestPrecisionParameter);
  CPPUNIT_TEST_SUITE_END();

public:

  template <typename DATATYPE>
  void ConfirmNumberToString(DATATYPE number,
                             const std::string& s,
                             std::function<std::string(DATATYPE)> to_string)

  {
      CPPUNIT_ASSERT_EQUAL( to_string(number), s );
  }


  template <typename DATATYPE>
  void ConfirmStringToNumber(const std::string& s,
                             DATATYPE number,
                             std::function<DATATYPE(const std::string&)> from_string)

  {
      CPPUNIT_ASSERT_EQUAL( number,  from_string(s) );
  }

  template <typename DATATYPE>
  void ConfirmStringValues(std::function<DATATYPE(const std::string&)> from_string,
                           std::function<std::string(DATATYPE)> to_string)
  {
    // we want to make sure that the following strings will be accepted and returned
    // by our conversion functions. This must not change in the future to ensure compatibility
    DATATYPE nan = from_string("nan");
    CPPUNIT_ASSERT_MESSAGE("nan==nan must be false", !(nan==nan) );
    nan = from_string("NAN");
    CPPUNIT_ASSERT_MESSAGE("NAN==NAN must be false", !(nan==nan) );

    std::string s_nan = to_string(nan);
    CPPUNIT_ASSERT_EQUAL(std::string("nan"), s_nan);

    ConfirmStringToNumber("inf", std::numeric_limits<DATATYPE>::infinity(), from_string);
    ConfirmStringToNumber("INF", std::numeric_limits<DATATYPE>::infinity(), from_string);
    ConfirmStringToNumber("infinity", std::numeric_limits<DATATYPE>::infinity(), from_string);
    ConfirmStringToNumber("INFINITY", std::numeric_limits<DATATYPE>::infinity(), from_string);

    ConfirmStringToNumber("-inf", -std::numeric_limits<DATATYPE>::infinity(), from_string);
    ConfirmStringToNumber("-INF", -std::numeric_limits<DATATYPE>::infinity(), from_string);
    ConfirmStringToNumber("-infinity", -std::numeric_limits<DATATYPE>::infinity(), from_string);
    ConfirmStringToNumber("-INFINITY", -std::numeric_limits<DATATYPE>::infinity(), from_string);

    ConfirmNumberToString(std::numeric_limits<DATATYPE>::infinity(), "inf", to_string);
    ConfirmNumberToString(-std::numeric_limits<DATATYPE>::infinity(), "-inf", to_string);
  }

  void ConfirmStringValues()
  {
    std::function<std::string(float)> float_to_string = std::bind(mitk::FloatToString, std::placeholders::_1, 16);
    ConfirmStringValues<float>(mitk::StringToFloat, float_to_string);

    std::function<std::string(double)> double_to_string = std::bind(mitk::DoubleToString, std::placeholders::_1, 16);
    ConfirmStringValues<double>(mitk::StringToDouble, double_to_string);
  }




  template <typename DATATYPE>
  void CheckRoundTrip(DATATYPE number,
                      std::function<DATATYPE(const std::string&)> from_string,
                      std::function<std::string(DATATYPE)> to_string)
  {
    std::string s = to_string(number);
    DATATYPE number2 = from_string(s);

    CPPUNIT_ASSERT_MESSAGE(std::string("Must not parse string ") + s + " as NaN", number2 == number2);

    CPPUNIT_ASSERT(mitk::Equal(number, number2));
  }

  template <typename DATATYPE>
  void TestConversions(std::function<DATATYPE(const std::string&)> from_string,
                       std::function<std::string(DATATYPE)> to_string)
  {
    auto check_roundtrip = std::bind(&mitkFloatToStringTestSuite::CheckRoundTrip<DATATYPE>, this,
                                    std::placeholders::_1, from_string, to_string);

    // we cannot test the NaN roundtrip because nan == nan will never be true
    check_roundtrip(std::numeric_limits<DATATYPE>::infinity());
    check_roundtrip(-std::numeric_limits<DATATYPE>::infinity());

    check_roundtrip(std::numeric_limits<DATATYPE>::denorm_min());
    check_roundtrip(std::numeric_limits<DATATYPE>::epsilon());
    //check_roundtrip(std::numeric_limits<DATATYPE>::lowest());
    //check_roundtrip(std::numeric_limits<DATATYPE>::min());
    //check_roundtrip(std::numeric_limits<DATATYPE>::max());
    check_roundtrip(sqrt(2));
    check_roundtrip(0.000000042);
    check_roundtrip(422345678.2345678);
    check_roundtrip(0.0);
    check_roundtrip(-0.0);
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

  //-------------------- double --------------------
  void CheckRoundTrip_Double(const std::string& input)
  {
    double d = mitk::StringToDouble(input);
    std::string result = mitk::DoubleToString(d);
    double d2 = mitk::StringToDouble(result);

    // There are normal imprecisions when converting to string
    // We do only compare if the numeric values match "close enough"
    CPPUNIT_ASSERT(mitk::Equal(d, d2));
  }

  void TestConversions()
  {
    std::function<std::string(float)> float_to_string = std::bind(mitk::FloatToString, std::placeholders::_1, 16);
    TestConversions<float>(mitk::StringToFloat, float_to_string);

    std::function<std::string(double)> double_to_string = std::bind(mitk::DoubleToString, std::placeholders::_1, 16);
    TestConversions<double>(mitk::StringToDouble, double_to_string);


    CheckRoundTrip_Float("1");
    CheckRoundTrip_Float("1.1");
    CheckRoundTrip_Float("1.12121212");
    CheckRoundTrip_Float("1.1e-2");

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
