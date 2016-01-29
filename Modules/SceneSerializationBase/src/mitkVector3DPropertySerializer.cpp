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

#ifndef mitkVector3DPropertySerializer_h_included
#define mitkVector3DPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkProperties.h"
#include "mitkFloatToString.h"

namespace mitk
{

class Vector3DPropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( Vector3DPropertySerializer, BasePropertySerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual TiXmlElement* Serialize() override
    {
      if (const Vector3DProperty* prop = dynamic_cast<const Vector3DProperty*>(m_Property.GetPointer()))
      {
        auto  element = new TiXmlElement("vector");
        Vector3D point = prop->GetValue();
        element->SetAttribute("x", DoubleToString(point[0]));
        element->SetAttribute("y", DoubleToString(point[1]));
        element->SetAttribute("z", DoubleToString(point[2]));
        return element;
      }
      else return nullptr;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) override
    {
      if (!element) return nullptr;

      std::string v_str[3];
      if ( element->QueryStringAttribute( "x", &v_str[0] ) != TIXML_SUCCESS ) return nullptr;
      if ( element->QueryStringAttribute( "y", &v_str[1] ) != TIXML_SUCCESS ) return nullptr;
      if ( element->QueryStringAttribute( "z", &v_str[2] ) != TIXML_SUCCESS ) return nullptr;
      Vector3D v;
      StringsToDoubles(3, v_str, v);

     return Vector3DProperty::New( v ).GetPointer();
    }

  protected:

    Vector3DPropertySerializer() {}
    virtual ~Vector3DPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(Vector3DPropertySerializer);

#endif

