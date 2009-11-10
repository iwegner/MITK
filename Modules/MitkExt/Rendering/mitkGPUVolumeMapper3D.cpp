/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-06-18 15:59:04 +0200 (Thu, 18 Jun 2009) $
Version:   $Revision: 17786 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#define GPU_LOG LOG_INFO(false)("VR")

#include "mitkGPUVolumeMapper3D.h"

#include "mitkDataTreeNode.h"

#include "mitkProperties.h"
#include "mitkLevelWindow.h"
#include "mitkColorProperty.h"
#include "mitkLevelWindowProperty.h"
#include "mitkLookupTableProperty.h"
#include "mitkTransferFunctionProperty.h"
#include "mitkColorProperty.h"
#include "mitkVtkPropRenderer.h"
#include "mitkRenderingManager.h"

#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkVolumeRayCastMapper.h>

#include <vtkVolumeTextureMapper2D.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkVolumeRayCastMIPFunction.h>
#include <vtkFiniteDifferenceGradientEstimator.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageShiftScale.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageWriter.h>
#include <vtkImageData.h>
#include <vtkLODProp3D.h>
#include <vtkImageResample.h>
#include <vtkPlane.h>
#include <vtkImplicitPlaneWidget.h>
#include <vtkAssembly.h>
#include <vtkFixedPointVolumeRayCastMapper.h>

#include <vtkCubeSource.h>
#include <vtkPolyDataMapper.h>
#include "mitkVtkVolumeRenderingProperty.h"


#include <itkMultiThreader.h>

#include "vtkMitkOpenGLVolumeTextureMapper3D.h"



const mitk::Image* mitk::GPUVolumeMapper3D::GetInput()
{
  return static_cast<const mitk::Image*> ( GetData() );
}

void mitk::GPUVolumeMapper3D::MitkRenderVolumetricGeometry(mitk::BaseRenderer* renderer)
{
  BaseVtkMapper3D::MitkRenderVolumetricGeometry(renderer);
  if(m_T2DMapper)
    m_T2DMapper->UpdateMTime();
}

mitk::GPUVolumeMapper3D::GPUVolumeMapper3D()
{
  GPU_LOG << "Instantiating GPUVolumeMapper3D";
  
  m_T2DMapper = vtkMitkOpenGLVolumeTextureMapper3D::New();
  m_T2DMapper->SetUseCompressedTexture(true);
  m_T2DMapper->SetPreferredMethodToFragmentProgram();
  m_T2DMapper->SetSampleDistance(1.0); 
 
  m_MapperCPU = vtkFixedPointVolumeRayCastMapper::New();
  m_MapperCPU->SetSampleDistance(0.5); // 4 rays for every pixel
  m_MapperCPU->IntermixIntersectingGeometryOn();
  m_MapperCPU->SetNumberOfThreads( itk::MultiThreader::GetGlobalDefaultNumberOfThreads() );
 
  m_VolumePropertyLow = vtkVolumeProperty::New();

  m_VolumePropertyLow->ShadeOn();
  m_VolumePropertyLow->SetAmbient (0.10f); //0.05f
  m_VolumePropertyLow->SetDiffuse (0.50f); //0.45f
  m_VolumePropertyLow->SetSpecular(0.40f); //0.50f
  m_VolumePropertyLow->SetSpecularPower(6.0f);
  m_VolumePropertyLow->SetInterpolationTypeToLinear();
    
  m_VolumeLOD = vtkVolume::New();
  m_VolumeLOD->SetMapper(   m_T2DMapper );
  m_VolumeLOD->SetProperty(m_VolumePropertyLow );
  
  m_VolumeCPU = vtkVolume::New();
  m_VolumeCPU->SetMapper(   m_MapperCPU );
  m_VolumeCPU->SetProperty(m_VolumePropertyLow );
                    
  m_UnitSpacingImageFilter = vtkImageChangeInformation::New();
  m_UnitSpacingImageFilter->SetOutputSpacing( 1.0, 1.0, 1.0 );

  m_T2DMapper->SetInput( this->m_UnitSpacingImageFilter->GetOutput() );//m_Resampler->GetOutput());
  m_MapperCPU->SetInput( this->m_UnitSpacingImageFilter->GetOutput() );//m_Resampler->GetOutput());

  CreateDefaultTransferFunctions();
}


mitk::GPUVolumeMapper3D::~GPUVolumeMapper3D()
{
  GPU_LOG << "Destroying GPUVolumeMapper3D";

  m_UnitSpacingImageFilter->Delete();
  m_T2DMapper->Delete();
  m_VolumePropertyLow->Delete();
  m_VolumeLOD->Delete();
  m_DefaultColorTransferFunction->Delete();
  m_DefaultOpacityTransferFunction->Delete();
  m_DefaultGradientTransferFunction->Delete();
  m_BinaryColorTransferFunction->Delete();
  m_BinaryOpacityTransferFunction->Delete();
  m_BinaryGradientTransferFunction->Delete();
}

vtkProp *mitk::GPUVolumeMapper3D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  return IsGPUEnabled( renderer ) ? m_VolumeLOD : m_VolumeCPU;
}


void mitk::GPUVolumeMapper3D::GenerateData( mitk::BaseRenderer *renderer )
{
  if(IsGPUEnabled(renderer))
    GenerateDataGPU(renderer);
  else
    GenerateDataCPU(renderer);
}

void mitk::GPUVolumeMapper3D::GenerateDataGPU( mitk::BaseRenderer *renderer )
{
  GPU_LOG << "GenerateDataGPU";
  
  mitk::Image *input = const_cast< mitk::Image * >( this->GetInput() );
  
  GPU_LOG << "mitk image mtime: " << input->GetMTime();
  
  if ( !input || !input->IsInitialized() )
    return;

  vtkRenderWindow* renderWindow = renderer->GetRenderWindow();

  if (this->IsVisible(renderer)==false ||
      this->GetDataTreeNode() == NULL ||
      dynamic_cast<mitk::BoolProperty*>(GetDataTreeNode()->GetProperty("volumerendering",renderer))==NULL ||
      dynamic_cast<mitk::BoolProperty*>(GetDataTreeNode()->GetProperty("volumerendering",renderer))->GetValue() == false
    )
  {
    m_VolumeLOD->VisibilityOff();
    return;
  }

  m_VolumeLOD->VisibilityOn();
  
  const TimeSlicedGeometry* inputtimegeometry = input->GetTimeSlicedGeometry();
  assert(inputtimegeometry!=NULL);

  const Geometry3D* worldgeometry = renderer->GetCurrentWorldGeometry();
  if(worldgeometry==NULL)
  {
    LOG_WARN << "no world geometry found, turning off volumerendering for data node " << GetDataTreeNode()->GetName();
    GetDataTreeNode()->SetProperty("volumerendering",mitk::BoolProperty::New(false));
    m_VolumeLOD->VisibilityOff();
    return;
  }

  if(IsLODEnabled(renderer))
  {
    switch ( mitk::RenderingManager::GetInstance()->GetNextLOD( renderer ) )
    {
      case 0:
        m_T2DMapper->SetSampleDistance(2.0); 
        break;

      default: case 1:
        m_T2DMapper->SetSampleDistance(1.0); 
        break;
    }
  }
  else
  {
    m_T2DMapper->SetSampleDistance(1.0); 
  }

  int timestep=0;
  ScalarType time = worldgeometry->GetTimeBounds()[0];
  if (time> ScalarTypeNumericTraits::NonpositiveMin())
    timestep = inputtimegeometry->MSToTimeStep(time);

  if (inputtimegeometry->IsValidTime(timestep)==false)
    return;

  vtkImageData *inputData = input->GetVtkImageData(timestep);
  if(inputData==NULL)
    return;

   GPU_LOG << "input data mtime1: " << inputData->GetMTime();

   m_UnitSpacingImageFilter->SetInput( inputData );
              
   UpdateTransferFunctions( renderer );
}


void mitk::GPUVolumeMapper3D::GenerateDataCPU( mitk::BaseRenderer *renderer )
{
  GPU_LOG << "GenerateDataCPU";
  
  mitk::Image *input = const_cast< mitk::Image * >( this->GetInput() );
  
  if ( !input || !input->IsInitialized() )
    return;

  vtkRenderWindow* renderWindow = renderer->GetRenderWindow();

  if (this->IsVisible(renderer)==false ||
      this->GetDataTreeNode() == NULL ||
      dynamic_cast<mitk::BoolProperty*>(GetDataTreeNode()->GetProperty("volumerendering",renderer))==NULL ||
      dynamic_cast<mitk::BoolProperty*>(GetDataTreeNode()->GetProperty("volumerendering",renderer))->GetValue() == false
    )
  {
    m_VolumeCPU->VisibilityOff();
    return;
  }

  m_VolumeCPU->VisibilityOn();
  
  const TimeSlicedGeometry* inputtimegeometry = input->GetTimeSlicedGeometry();
  assert(inputtimegeometry!=NULL);

  const Geometry3D* worldgeometry = renderer->GetCurrentWorldGeometry();
  if(worldgeometry==NULL)
  {
    LOG_WARN << "no world geometry found, turning off volumerendering for data node " << GetDataTreeNode()->GetName();
    GetDataTreeNode()->SetProperty("volumerendering",mitk::BoolProperty::New(false));
    m_VolumeCPU->VisibilityOff();
    return;
  }

  if(IsLODEnabled(renderer))
  {
    switch ( mitk::RenderingManager::GetInstance()->GetNextLOD( renderer ) )
    {
      case 0:
        m_MapperCPU->SetSampleDistance(2.0); 
        break;

      default: case 1:
        m_MapperCPU->SetSampleDistance(1.0); 
        break;
    }
  }
  else
  {
    m_MapperCPU->SetSampleDistance(1.0); 
  }

  int timestep=0;
  ScalarType time = worldgeometry->GetTimeBounds()[0];
  if (time> ScalarTypeNumericTraits::NonpositiveMin())
    timestep = inputtimegeometry->MSToTimeStep(time);

  if (inputtimegeometry->IsValidTime(timestep)==false)
    return;

  vtkImageData *inputData = input->GetVtkImageData(timestep);
  if(inputData==NULL)
    return;

  m_UnitSpacingImageFilter->SetInput( inputData );
              
  UpdateTransferFunctions( renderer );
}



void mitk::GPUVolumeMapper3D::CreateDefaultTransferFunctions()
{
  GPU_LOG << "CreateDefaultTransferFunctions";

  m_DefaultOpacityTransferFunction = vtkPiecewiseFunction::New();
  m_DefaultOpacityTransferFunction->AddPoint( 0.0, 0.0 );
  m_DefaultOpacityTransferFunction->AddPoint( 255.0, 0.8 );
  m_DefaultOpacityTransferFunction->ClampingOn();

  m_DefaultGradientTransferFunction = vtkPiecewiseFunction::New();
  m_DefaultGradientTransferFunction->AddPoint( 0.0, 0.0 );
  m_DefaultGradientTransferFunction->AddPoint( 255.0, 0.8 );
  m_DefaultGradientTransferFunction->ClampingOn();

  m_DefaultColorTransferFunction = vtkColorTransferFunction::New();
  m_DefaultColorTransferFunction->AddRGBPoint( 0.0, 0.0, 0.0, 0.0 );
  m_DefaultColorTransferFunction->AddRGBPoint( 127.5, 1, 1, 0.0 );
  m_DefaultColorTransferFunction->AddRGBPoint( 255.0, 0.8, 0.2, 0 );
  m_DefaultColorTransferFunction->ClampingOn();

  m_BinaryOpacityTransferFunction = vtkPiecewiseFunction::New();
  m_BinaryOpacityTransferFunction->AddPoint( 0, 0.0 );
  m_BinaryOpacityTransferFunction->AddPoint( 1, 1.0 );

  m_BinaryGradientTransferFunction = vtkPiecewiseFunction::New();
  m_BinaryGradientTransferFunction->AddPoint( 0.0, 1.0 );

  m_BinaryColorTransferFunction = vtkColorTransferFunction::New();
}

void mitk::GPUVolumeMapper3D::UpdateTransferFunctions( mitk::BaseRenderer * renderer )
{
  GPU_LOG << "UpdateTransferFunctions";

  vtkPiecewiseFunction *opacityTransferFunction = m_DefaultOpacityTransferFunction;
  vtkPiecewiseFunction *gradientTransferFunction = m_DefaultGradientTransferFunction;
  vtkColorTransferFunction *colorTransferFunction = m_DefaultColorTransferFunction;

  bool isBinary = false;

  GetDataTreeNode()->GetBoolProperty("binary", isBinary, renderer);

  if(isBinary)
  {
    opacityTransferFunction = m_BinaryOpacityTransferFunction;
    gradientTransferFunction = m_BinaryGradientTransferFunction;
    colorTransferFunction = m_BinaryColorTransferFunction;

    colorTransferFunction->RemoveAllPoints();
    float rgb[3];
    if( !GetDataTreeNode()->GetColor( rgb,renderer ) )
       rgb[0]=rgb[1]=rgb[2]=1;
    colorTransferFunction->AddRGBPoint( 0,rgb[0],rgb[1],rgb[2] );
    colorTransferFunction->Modified();
  }
  else
  {
    mitk::TransferFunctionProperty::Pointer transferFunctionProp = 
      dynamic_cast<mitk::TransferFunctionProperty*>(this->GetDataTreeNode()->GetProperty("TransferFunction",renderer));

    if( transferFunctionProp.IsNotNull() )   
    {
      opacityTransferFunction = transferFunctionProp->GetValue()->GetScalarOpacityFunction();
      gradientTransferFunction = transferFunctionProp->GetValue()->GetGradientOpacityFunction();
      colorTransferFunction = transferFunctionProp->GetValue()->GetColorTransferFunction();
    }
  }

  m_VolumePropertyLow->SetColor( colorTransferFunction );
  m_VolumePropertyLow->SetScalarOpacity( opacityTransferFunction );
  m_VolumePropertyLow->SetGradientOpacity( gradientTransferFunction );
}

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>



void mitk::GPUVolumeMapper3D::ApplyProperties(vtkActor* /*actor*/, mitk::BaseRenderer* /*renderer*/)
{
  GPU_LOG << "ApplyProperties";
}

void mitk::GPUVolumeMapper3D::SetDefaultProperties(mitk::DataTreeNode* node, mitk::BaseRenderer* renderer, bool overwrite)
{
  GPU_LOG << "SetDefaultProperties";

  node->AddProperty( "volumerendering", mitk::BoolProperty::New( false ), renderer, overwrite );
  node->AddProperty( "volumerendering.uselod", mitk::BoolProperty::New( true ), renderer, overwrite );
  node->AddProperty( "volumerendering.usegpu", mitk::BoolProperty::New( true ), renderer, overwrite );
  node->AddProperty( "binary", mitk::BoolProperty::New( false ), renderer, overwrite );
 
  mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
  if(image.IsNotNull() && image->IsInitialized())
  {
    if((overwrite) || (node->GetProperty("levelwindow", renderer)==NULL))
    {
      mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
      mitk::LevelWindow levelwindow;
      levelwindow.SetAuto( image );
      levWinProp->SetLevelWindow( levelwindow );
      node->SetProperty( "levelwindow", levWinProp, renderer );
    }
    
    if((overwrite) || (node->GetProperty("TransferFunction", renderer)==NULL))
    {
      // add a default transfer function
      mitk::TransferFunction::Pointer tf = mitk::TransferFunction::New();
      tf->SetTransferFunctionMode(0);
      node->SetProperty ( "TransferFunction", mitk::TransferFunctionProperty::New ( tf.GetPointer() ) );
    }
  }

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}


bool mitk::GPUVolumeMapper3D::IsLODEnabled( mitk::BaseRenderer * renderer ) const
{
  return
    dynamic_cast<mitk::BoolProperty*>(GetDataTreeNode()->GetProperty("volumerendering.uselod",renderer)) != NULL &&
    dynamic_cast<mitk::BoolProperty*>(GetDataTreeNode()->GetProperty("volumerendering.uselod",renderer))->GetValue() == true;
}


bool mitk::GPUVolumeMapper3D::IsGPUEnabled( mitk::BaseRenderer * renderer ) const
{
  return
    dynamic_cast<mitk::BoolProperty*>(GetDataTreeNode()->GetProperty("volumerendering.usegpu",renderer)) != NULL &&
    dynamic_cast<mitk::BoolProperty*>(GetDataTreeNode()->GetProperty("volumerendering.usegpu",renderer))->GetValue() == true;
}



