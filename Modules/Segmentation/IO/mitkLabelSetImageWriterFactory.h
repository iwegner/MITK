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
#ifndef LabelSetImageWriterFactory_H_HEADER_INCLUDED
#define LabelSetImageWriterFactory_H_HEADER_INCLUDED

#include "MitkSegmentationExports.h"
#include "itkObjectFactoryBase.h"
#include "mitkBaseData.h"

namespace mitk
{

  class MitkSegmentation_EXPORT LabelSetImageWriterFactory : public itk::ObjectFactoryBase
  {
  public:

    mitkClassMacro( mitk::LabelSetImageWriterFactory, itk::ObjectFactoryBase )

      /** Class methods used to interface with the registered factories. */
      virtual const char* GetITKSourceVersion(void) const;
    virtual const char* GetDescription(void) const;

    /** Method for class instantiation. */
    itkFactorylessNewMacro(Self);

    /**
     * Register one factory of this type
     * \deprecatedSince{2013_09}
     */
    DEPRECATED(static void RegisterOneFactory(void));

    /**
     * UnRegister one factory of this type
     * \deprecatedSince{2013_09}
     */
    DEPRECATED(static void UnRegisterOneFactory(void));


  protected:
    LabelSetImageWriterFactory();
    ~LabelSetImageWriterFactory();

  private:
    LabelSetImageWriterFactory(const Self&); //purposely not implemented
    void operator=(const Self&); //purposely not implemented

    static itk::ObjectFactoryBase::Pointer GetInstance();
  };
}
#endif // LabelSetImageWriterFactory_H_HEADER_INCLUDED
