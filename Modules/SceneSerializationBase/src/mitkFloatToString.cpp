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

#include "mitkFloatToString.h"

// number to string
#include <boost/lexical_cast.hpp>

std::string mitk::FloatToString(float f)
{
  return boost::lexical_cast<std::string, float>(f);
}

float mitk::StringToFloat(const std::string& s)
{
  try
  {
    return boost::lexical_cast<float>(s);
  }
  catch (boost::bad_lexical_cast&)
  {
    // boost accepts only "infinity"
    if (s == "-inf" || s == "-INF")
        return std::numeric_limits<float>::infinity();
    if (s == "inf" || s == "INF")
        return std::numeric_limits<float>::infinity();
    return std::numeric_limits<float>::quiet_NaN();
  }
}

std::string mitk::DoubleToString(double d)
{
  return boost::lexical_cast<std::string, double>(d);
}

double mitk::StringToDouble(const std::string& s)
{
  try
  {
    return boost::lexical_cast<double>(s);
  }
  catch (boost::bad_lexical_cast&)
  {
    // boost accepts only "infinity"
    if (s == "-inf" || s == "-INF")
        return std::numeric_limits<float>::infinity();
    if (s == "inf" || s == "INF")
        return std::numeric_limits<float>::infinity();
    return std::numeric_limits<float>::quiet_NaN();
  }
}