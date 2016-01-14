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

#ifndef mitkFloatToString_h_included
#define mitkFloatToString_h_included

#include <MitkSceneSerializationBaseExports.h>

#include <string>

namespace mitk
{

//! Converts the number to a string
//! Will care to convert infinity to "inf" and not-a-number to "nan"
MITKSCENESERIALIZATIONBASE_EXPORT std::string FloatToString(float f);

//! Converts the string to a float number.
//! Interpret "inf" to infinity and "nan" to not-a-number
//! \return NaN if string is not a valid float
MITKSCENESERIALIZATIONBASE_EXPORT float StringToFloat(const std::string& s);

//! Converts the number to a string
//! Will care to convert infinity to "inf" and not-a-number to "nan"
MITKSCENESERIALIZATIONBASE_EXPORT std::string DoubleToString(double f);

//! Converts the string to a float number.
//! Interpret "inf" to infinity and "nan" to not-a-number
//! \return NaN if string is not a valid double
MITKSCENESERIALIZATIONBASE_EXPORT double StringToDouble(const std::string& s);

} // namespace

#endif

