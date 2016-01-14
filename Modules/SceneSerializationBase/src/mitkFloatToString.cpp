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

#include <Poco/NumberFormatter.h>
#include <Poco/NumericString.h>

std::string mitk::FloatToString(float f)
{
  return Poco::NumberFormatter::format(f);
}

float mitk::StringToFloat(const std::string& s)
{
  return Poco::strToFloat(s.c_str());
}

std::string mitk::DoubleToString(double d)
{
  return Poco::NumberFormatter::format(d);
}

double mitk::StringToDouble(const std::string& s)
{
  return Poco::strToDouble(s.c_str());
}