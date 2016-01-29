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

#ifndef mitkColorPropertySerializer_h_included
#define mitkColorPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkColorProperty.h"
#include "mitkFloatToString.h"

namespace mitk
{

class ColorPropertySerializer : public BasePropertySerializer
{
  public:

    mitkClassMacro( ColorPropertySerializer, BasePropertySerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual TiXmlElement* Serialize() override
    {
      if (const ColorProperty* prop = dynamic_cast<const ColorProperty*>(m_Property.GetPointer()))
      {
        auto  element = new TiXmlElement("color");
        Color color = prop->GetValue();
        element->SetAttribute("r", FloatToString(color[0]));
        element->SetAttribute("g", FloatToString(color[1]));
        element->SetAttribute("b", FloatToString(color[2]));
        return element;
      }
      else return nullptr;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) override
    {
      if (!element) return nullptr;

      std::string c_string[3];
      if ( element->QueryStringAttribute( "r", &c_string[0] ) != TIXML_SUCCESS ) return nullptr;
      if ( element->QueryStringAttribute( "g", &c_string[1] ) != TIXML_SUCCESS ) return nullptr;
      if ( element->QueryStringAttribute( "b", &c_string[2] ) != TIXML_SUCCESS ) return nullptr;
      Color c;
      StringsToFloats(3, c_string, c);

      return ColorProperty::New( c ).GetPointer();
    }

  protected:

    ColorPropertySerializer() {}
    virtual ~ColorPropertySerializer() {}
};

} // namespace

// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(ColorPropertySerializer);

#endif

