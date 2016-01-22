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
#include <boost/format.hpp>

namespace
{

template <typename DATATYPE>
std::string NumberToString(DATATYPE number, unsigned int precision)
{
  if (!(number == number)) // NaN
  {
      return "nan";
  }
  else if (number == std::numeric_limits<double>::infinity())
  {
      return "inf";
  }
  else if (number == -std::numeric_limits<double>::infinity())
  {
      return "-inf";
  }

  std::stringstream converter;
  converter.precision(precision);
  converter << number;
  return converter.str();}
}


std::string mitk::FloatToString(float f, unsigned int precision)
{
    return NumberToString(f,precision);
}

float mitk::StringToFloat(const std::string& s)
{
  try
  {
    return boost::lexical_cast<float>(s);
  }
  catch (boost::bad_lexical_cast&)
  {
    return std::numeric_limits<float>::quiet_NaN();
  }
}

std::string mitk::DoubleToString(double d, unsigned int precision)
{
    return NumberToString(d,precision);
}

double mitk::StringToDouble(const std::string& s)
{
  try
  {
    return boost::lexical_cast<double>(s);
  }
  catch (boost::bad_lexical_cast&)
  {
    return std::numeric_limits<float>::quiet_NaN();
  }
}