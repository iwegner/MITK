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
#include <itkNumberToString.h>

// string to number
#include <cstdlib>

std::string mitk::FloatToString(float f)
{
  itk::NumberToString<float> converter;
  return converter(f);
}

float mitk::StringToFloat(const std::string& s)
{
  return std::atof(s.c_str());
}

std::string mitk::DoubleToString(double d)
{
  itk::NumberToString<double> converter;
  return converter(d);
}

double mitk::StringToDouble(const std::string& s)
{
  return std::strtod(s.c_str(),nullptr);
}