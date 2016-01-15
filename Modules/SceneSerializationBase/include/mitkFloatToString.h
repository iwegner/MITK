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

//! Call mitk::StringToDouble for all count elements of something that can be accessed
//! via operator[], e.g. to fill a Point3D / Vector3D
//!
//! \param count the number of elements to convert from "strings" to "doubles"
//! \param double a container for at least "count" double values with indices starting from 0
//! \param strings a container for at least three string values with indices starting from 0
//!
//! \warning This method has absolutely no means of verifying that your containers
//!          are big enough. It is the caller's responsibility to make sure that
//!          both the input and the output container can be addressed via [0], [1], [2].
//!
//! \code
//! std::vector<std::string> serialized_double_values = ... read from some file ...
//! mitk:;Point3D point;
//! mitk::StringToDouble(3, serialized_double_values, point);
//! \endcode
template <typename STRING_ARRAY, typename DOUBLE_ARRAY>
void StringsToDoubles(unsigned int count, const STRING_ARRAY& strings, DOUBLE_ARRAY& doubles)
{
  for (unsigned int i = 0; i < count ; ++i)
  {
    doubles[i] = StringToDouble(strings[i]);
  }
}

//! Same as StringsToDoubles for the float type
template <typename STRING_ARRAY, typename FLOAT_ARRAY>
void StringsToFloats(unsigned int count, const STRING_ARRAY& strings, FLOAT_ARRAY& floats)
{
  for ( unsigned int i = 0; i < count; ++i )
  {
    floats[i] = StringToFloat(strings[i]);
  }
}

} // namespace

#endif

