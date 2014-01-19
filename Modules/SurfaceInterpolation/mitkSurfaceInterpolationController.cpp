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

#include "mitkSurfaceInterpolationController.h"

#include "mitkMemoryUtilities.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include "mitkImageToSurfaceFilter.h"
#include "mitkRestorePlanePositionOperation.h"
#include "mitkContourModelToSurfaceFilter.h"
#include "mitkInteractionConst.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkIOUtil.h"

#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkAppendPolyData.h"
#include "mitkVtkRepresentationProperty.h"
#include "vtkProperty.h"

#include <itkCommand.h>

mitk::SurfaceInterpolationController::SurfaceInterpolationController() :
m_ActiveLabel(0),
m_WorkingImage(NULL),
m_MinSpacing(1.0),
m_MaxSpacing(1.0)
{
  this->Initialize();
}

mitk::SurfaceInterpolationController::~SurfaceInterpolationController()
{
  ContourListMap::iterator it = m_MapOfContourLists.begin();
  for (; it != m_MapOfContourLists.end(); it++)
  {
    for (unsigned int j = 0; j < m_MapOfContourLists[(*it).first].size(); ++j)
    {
      delete(m_MapOfContourLists[(*it).first].at(j).second);
    }
    m_MapOfContourLists.erase(it);
  }
}

void mitk::SurfaceInterpolationController::Initialize()
{
  m_InterpolateSurfaceFilter = CreateDistanceImageFromSurfaceFilter::New();
  m_InterpolateSurfaceFilter->SetUseProgressBar(false);

  m_Contours = Surface::New();

  m_InterpolationResult = NULL;
}

mitk::SurfaceInterpolationController* mitk::SurfaceInterpolationController::GetInstance()
{
  static mitk::SurfaceInterpolationController* m_Instance;

  if ( m_Instance == 0)
  {
    m_Instance = new SurfaceInterpolationController();
  }

  return m_Instance;
}

void mitk::SurfaceInterpolationController::AddNewContour(mitk::ContourModel::Pointer newContour, RestorePlanePositionOperation* op)
{
  if (m_ActiveLabel == 0)
    return;

  AffineTransform3D::Pointer transform = AffineTransform3D::New();
  transform = op->GetTransform();

  mitk::Vector3D direction = op->GetDirectionVector();
  int pos(-1);

  for (unsigned int i=0; i < m_MapOfContourLists[m_ActiveLabel].size(); i++)
  {
    itk::Matrix<ScalarType> diffM = transform->GetMatrix()-m_MapOfContourLists[m_ActiveLabel].at(i).second->GetTransform()->GetMatrix();
    bool isSameMatrix(true);
    for (unsigned int j = 0; j < 3; j++)
    {
      if (fabs(diffM[j][0]) > 0.0001 && fabs(diffM[j][1]) > 0.0001 && fabs(diffM[j][2]) > 0.0001)
      {
        isSameMatrix = false;
        break;
      }
    }
    itk::Vector<float> diffV = m_MapOfContourLists[m_ActiveLabel].at(i).second->GetTransform()->GetOffset()-transform->GetOffset();
    if ( isSameMatrix && m_MapOfContourLists[m_ActiveLabel].at(i).second->GetPos() == op->GetPos() && (fabs(diffV[0]) < 0.0001 && fabs(diffV[1]) < 0.0001 && fabs(diffV[2]) < 0.0001) )
    {
      pos = i;
      break;
    }
  }

  if (pos == -1 && newContour->GetNumberOfVertices() > 0) // add a new contour
  {
    mitk::RestorePlanePositionOperation* newOp = new mitk::RestorePlanePositionOperation (OpRESTOREPLANEPOSITION, op->GetWidth(),
      op->GetHeight(), op->GetSpacing(), op->GetPos(), direction, transform);
    ContourPositionPair newData = std::make_pair(newContour,newOp);
    m_MapOfContourLists[m_ActiveLabel].push_back(newData);
  }
  else if (pos != -1) // replace existing contour
  {
    m_MapOfContourLists[m_ActiveLabel].at(pos).first = newContour;
  }

  this->Modified();
}

void mitk::SurfaceInterpolationController::Interpolate()
{
  if (m_MapOfContourLists[m_ActiveLabel].size() < 2)
  {
    //If no interpolation is possible reset the interpolation result
    m_InterpolationResult = NULL;
    return;
  }

  m_InterpolateSurfaceFilter->Reset();

  m_InterpolateSurfaceFilter->SetReferenceImage( m_WorkingImage );

  //CreateDistanceImageFromSurfaceFilter::Pointer interpolateSurfaceFilter = CreateDistanceImageFromSurfaceFilter::New();
  vtkSmartPointer<vtkAppendPolyData> polyDataAppender = vtkSmartPointer<vtkAppendPolyData>::New();

  for (unsigned int i=0; i < m_MapOfContourLists[m_ActiveLabel].size(); i++)
  {
    mitk::ContourModelToSurfaceFilter::Pointer converter = mitk::ContourModelToSurfaceFilter::New();
    converter->SetInput(m_MapOfContourLists[m_ActiveLabel].at(i).first);
    converter->Update();

    mitk::Surface::Pointer surface = converter->GetOutput();
    surface->DisconnectPipeline();

    ReduceContourSetFilter::Pointer reduceFilter = ReduceContourSetFilter::New();
    reduceFilter->SetUseProgressBar(false);
    reduceFilter->SetInput(surface);
    reduceFilter->SetMinSpacing(m_MinSpacing);
    reduceFilter->SetMaxSpacing(m_MaxSpacing);
    reduceFilter->Update();

    ComputeContourSetNormalsFilter::Pointer normalsFilter = ComputeContourSetNormalsFilter::New();
    normalsFilter->SetUseProgressBar(false);
    normalsFilter->SetInput(reduceFilter->GetOutput());
    normalsFilter->SetMaxSpacing(m_MaxSpacing);
    normalsFilter->SetWorkingImage(m_WorkingImage, m_ActiveLabel);
    normalsFilter->Update();

    m_InterpolateSurfaceFilter->SetInput(i, normalsFilter->GetOutput());

    polyDataAppender->AddInput(reduceFilter->GetOutput()->GetVtkPolyData());
  }

  polyDataAppender->Update();
  m_Contours->SetVtkPolyData(polyDataAppender->GetOutput());

  // update the filter and get the resulting distance-image
  m_InterpolateSurfaceFilter->Update();
  Image::Pointer distanceImage = m_InterpolateSurfaceFilter->GetOutput();
  if (distanceImage.IsNull())
  {
    //If no interpolation is possible reset the interpolation result
    m_InterpolationResult = NULL;
    return;
  }

  // create a surface from the distance-image
  mitk::ImageToSurfaceFilter::Pointer imageToSurfaceFilter = mitk::ImageToSurfaceFilter::New();
  imageToSurfaceFilter->SetInput( distanceImage );
  imageToSurfaceFilter->SetThreshold( 0 );
  imageToSurfaceFilter->SetSmooth(true);
  imageToSurfaceFilter->SetSmoothIteration(20);
  imageToSurfaceFilter->Update();
  m_InterpolationResult = imageToSurfaceFilter->GetOutput();

  m_InterpolationResult->DisconnectPipeline();
}

mitk::Surface::Pointer mitk::SurfaceInterpolationController::GetInterpolationResult()
{
  return m_InterpolationResult;
}

mitk::Surface* mitk::SurfaceInterpolationController::GetContoursAsSurface()
{
  return m_Contours;
}

void mitk::SurfaceInterpolationController::SetMinSpacing(double minSpacing)
{
  m_MinSpacing = minSpacing;
}

void mitk::SurfaceInterpolationController::SetMaxSpacing(double maxSpacing)
{
  m_MaxSpacing = maxSpacing;
}

void mitk::SurfaceInterpolationController::SetDistanceImageVolume(unsigned int value)
{
  m_DistanceImageVolume = value;
}

void mitk::SurfaceInterpolationController::SetWorkingImage(Image* workingImage)
{
  m_WorkingImage = workingImage;
}

mitk::Image* mitk::SurfaceInterpolationController::GetImage()
{
  return m_InterpolateSurfaceFilter->GetOutput();
}

double mitk::SurfaceInterpolationController::EstimatePortionOfNeededMemory()
{
  double numberOfPoints = 0.0;
  for (unsigned int i = 0; i < m_MapOfContourLists[m_ActiveLabel].size(); i++)
  {
    numberOfPoints += m_MapOfContourLists[m_ActiveLabel].at(i).first->GetNumberOfVertices()*3;
  }

  double sizeOfPoints = pow(numberOfPoints,2)*sizeof(double);
  double totalMem = mitk::MemoryUtilities::GetTotalSizeOfPhysicalRam();
  double percentage = sizeOfPoints/totalMem;
  return percentage;
}

void mitk::SurfaceInterpolationController::SetActiveLabel(int activeLabel)
{
  if (m_ActiveLabel == activeLabel)
    return;

  if (activeLabel == 0)
    return;

  m_ActiveLabel = activeLabel;

  ContourListMap::iterator it = m_MapOfContourLists.find(m_ActiveLabel);

  if (it == m_MapOfContourLists.end())
  {
    ContourPositionPairList newList;
    m_MapOfContourLists.insert(std::pair<unsigned int, ContourPositionPairList>(m_ActiveLabel, newList));
    m_InterpolationResult = NULL;
  }

  this->Modified();
}

/*
void mitk::SurfaceInterpolationController::RemoveSegmentationFromContourList(mitk::Image *segmentation)
{
  if (segmentation != 0)
  {
    m_MapOfContourLists.erase(segmentation);
    if (m_SelectedSegmentation == segmentation)
    {
      SetSegmentationImage(NULL);
      m_SelectedSegmentation = 0;
    }
  }
}
*/

/*
void mitk::SurfaceInterpolationController::OnSegmentationDeleted(const itk::Object *caller, const itk::EventObject &event)
{
  mitk::Image* tempImage = dynamic_cast<mitk::Image*>(const_cast<itk::Object*>(caller));
  if (tempImage)
  {
    RemoveSegmentationFromContourList(tempImage);
    if (tempImage == m_SelectedSegmentation)
    {
      SetSegmentationImage(NULL);
      m_SelectedSegmentation = 0;
    }
  }
}
*/
