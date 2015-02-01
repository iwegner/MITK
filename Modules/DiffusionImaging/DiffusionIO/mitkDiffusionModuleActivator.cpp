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
#include <usModuleActivator.h>
#include <usModuleContext.h>

#include <mitkDiffusionImageNrrdReaderService.h>
#include <mitkDiffusionImageNiftiReaderService.h>
#include <mitkNrrdTensorImageReader.h>
#include <mitkNrrdQBallImageReader.h>
#include <mitkFiberBundleXReader.h>
#include <mitkConnectomicsNetworkReader.h>

#include <mitkDiffusionImageNrrdWriterService.h>
#include <mitkDiffusionImageNiftiWriterService.h>
#include <mitkNrrdTensorImageWriter.h>
#include <mitkNrrdQBallImageWriter.h>
#include <mitkFiberBundleXWriter.h>
#include <mitkConnectomicsNetworkWriter.h>

#include "mitkDiffusionIOMimeTypes.h"

namespace mitk
{
  /**
  \brief Registers services for segmentation module.
  */
  class DiffusionModuleActivator : public us::ModuleActivator
  {
  public:

    void Load(us::ModuleContext* context)
    {
      us::ServiceProperties props;
      props[ us::ServiceConstants::SERVICE_RANKING() ] = 10;

      m_MimeTypes = mitk::DiffusionIOMimeTypes::Get();
      for (std::vector<mitk::CustomMimeType*>::const_iterator mimeTypeIter = m_MimeTypes.begin(),
        iterEnd = m_MimeTypes.end(); mimeTypeIter != iterEnd; ++mimeTypeIter)
      {
        context->RegisterService(*mimeTypeIter, props);
      }

      m_DiffusionImageNrrdReaderService = new DiffusionImageNrrdReaderService();
      m_DiffusionImageNiftiReaderService = new DiffusionImageNiftiReaderService();
      m_NrrdTensorImageReader = new NrrdTensorImageReader();
      m_NrrdQBallImageReader = new NrrdQBallImageReader();
      m_FiberBundleXReader = new FiberBundleXReader();
      m_ConnectomicsNetworkReader = new ConnectomicsNetworkReader();

      m_DiffusionImageNrrdWriterService = new DiffusionImageNrrdWriterService();
      m_DiffusionImageNiftiWriterService = new DiffusionImageNiftiWriterService();
      m_NrrdTensorImageWriter = new NrrdTensorImageWriter();
      m_NrrdQBallImageWriter = new NrrdQBallImageWriter();
      m_FiberBundleXWriter = new FiberBundleXWriter();
      m_ConnectomicsNetworkWriter = new ConnectomicsNetworkWriter();
    }

    void Unload(us::ModuleContext*)
    {
      for (unsigned int loop(0); loop < m_MimeTypes.size(); ++loop)
      {
        delete m_MimeTypes.at(loop);
      }

      delete m_DiffusionImageNrrdReaderService;
      delete m_DiffusionImageNiftiReaderService;
      delete m_NrrdTensorImageReader;
      delete m_NrrdQBallImageReader;
      delete m_FiberBundleXReader;
      delete m_ConnectomicsNetworkReader;

      delete m_DiffusionImageNrrdWriterService;
      delete m_DiffusionImageNiftiWriterService;
      delete m_NrrdTensorImageWriter;
      delete m_NrrdQBallImageWriter;
      delete m_FiberBundleXWriter;
      delete m_ConnectomicsNetworkWriter;
    }

  private:

    DiffusionImageNrrdReaderService * m_DiffusionImageNrrdReaderService;
    DiffusionImageNiftiReaderService * m_DiffusionImageNiftiReaderService;
    NrrdTensorImageReader * m_NrrdTensorImageReader;
    NrrdQBallImageReader * m_NrrdQBallImageReader;
    FiberBundleXReader * m_FiberBundleXReader;
    ConnectomicsNetworkReader * m_ConnectomicsNetworkReader;

    DiffusionImageNrrdWriterService * m_DiffusionImageNrrdWriterService;
    DiffusionImageNiftiWriterService * m_DiffusionImageNiftiWriterService;
    NrrdTensorImageWriter * m_NrrdTensorImageWriter;
    NrrdQBallImageWriter * m_NrrdQBallImageWriter;
    FiberBundleXWriter * m_FiberBundleXWriter;
    ConnectomicsNetworkWriter * m_ConnectomicsNetworkWriter;

    std::vector<mitk::CustomMimeType*> m_MimeTypes;

  };
}

US_EXPORT_MODULE_ACTIVATOR(mitk::DiffusionModuleActivator)
