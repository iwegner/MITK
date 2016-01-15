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

#ifndef mitkClippingPropertySerializer_h_included
#define mitkClippingPropertySerializer_h_included

#include "mitkBasePropertySerializer.h"

#include "mitkClippingProperty.h"
#include "mitkNumericTypes.h"
#include "mitkFloatToString.h"

namespace mitk
{
class ClippingPropertySerializer : public BasePropertySerializer
{
  public:
    mitkClassMacro( ClippingPropertySerializer, BasePropertySerializer );
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual TiXmlElement* Serialize() override
    {
      if (const ClippingProperty* prop = dynamic_cast<const ClippingProperty*>(m_Property.GetPointer()))
      {
        auto  element = new TiXmlElement("clipping");
        if (prop->GetClippingEnabled())
          element->SetAttribute("enabled", "true");
        else
          element->SetAttribute("enabled", "false");
        auto  originElement = new TiXmlElement("origin");
        const Point3D origin = prop->GetOrigin();
        originElement->SetAttribute("x", DoubleToString(origin[0]));
        originElement->SetAttribute("y", DoubleToString(origin[1]));
        originElement->SetAttribute("z", DoubleToString(origin[2]));
        element->LinkEndChild(originElement);
        auto  normalElement = new TiXmlElement("normal");
        const Vector3D normal = prop->GetNormal();
        normalElement->SetAttribute("x", DoubleToString(normal[0]));
        normalElement->SetAttribute("y", DoubleToString(normal[1]));
        normalElement->SetAttribute("z", DoubleToString(normal[2]));
        element->LinkEndChild(normalElement);
        return element;
      }
      else
        return nullptr;
    }

    virtual BaseProperty::Pointer Deserialize(TiXmlElement* element) override
    {
      if (!element)
        return nullptr;
      bool enabled = std::string(element->Attribute("enabled")) == "true";

      TiXmlElement* originElement = element->FirstChildElement("origin");
      if (originElement == nullptr)
        return nullptr;
      std::string origin_string[3];
      if ( originElement->QueryStringAttribute( "x", &origin_string[0] ) != TIXML_SUCCESS )
        return nullptr;
      if ( originElement->QueryStringAttribute( "y", &origin_string[1] ) != TIXML_SUCCESS )
        return nullptr;
      if ( originElement->QueryStringAttribute( "z", &origin_string[2] ) != TIXML_SUCCESS )
        return nullptr;
      Point3D origin;
      StringsToDoubles(3, origin_string, origin);
      TiXmlElement* normalElement = element->FirstChildElement("normal");
      if (normalElement == nullptr)
        return nullptr;
      std::string normal_string[3];
      if ( normalElement->QueryStringAttribute( "x", &normal_string[0] ) != TIXML_SUCCESS )
        return nullptr;
      if ( normalElement->QueryStringAttribute( "y", &normal_string[1] ) != TIXML_SUCCESS )
        return nullptr;
      if ( normalElement->QueryStringAttribute( "z", &normal_string[2] ) != TIXML_SUCCESS )
        return nullptr;
      Vector3D normal;
      StringsToDoubles(3, normal_string, normal);
      ClippingProperty::Pointer cp = ClippingProperty::New(origin, normal);
      cp->SetClippingEnabled(enabled);
     return cp.GetPointer();
    }
  protected:
    ClippingPropertySerializer() {}
    virtual ~ClippingPropertySerializer() {}
};
} // namespace
// important to put this into the GLOBAL namespace (because it starts with 'namespace mitk')
MITK_REGISTER_SERIALIZER(ClippingPropertySerializer);
#endif
