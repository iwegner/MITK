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
m_ActiveLabel(-1),
m_WorkingImage(NULL)
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
      delete(m_MapOfContourLists[(*it).first].at(j).position);
    }
    m_MapOfContourLists.erase(it);
  }

/*
  //Removing all observers
  std::map<mitk::Image*, unsigned long>::iterator dataIter = m_SegmentationObserverTags.begin();
  for (; dataIter != m_SegmentationObserverTags.end(); ++dataIter )
  {
    (*dataIter).first->GetProperty("visible")->RemoveObserver( (*dataIter).second );
  }
  m_SegmentationObserverTags.clear();
*/
}

void mitk::SurfaceInterpolationController::Initialize()
{
  m_ReduceFilter = ReduceContourSetFilter::New();
  m_NormalsFilter = ComputeContourSetNormalsFilter::New();
  m_InterpolateSurfaceFilter = CreateDistanceImageFromSurfaceFilter::New();

  m_ReduceFilter->SetUseProgressBar(false);
  m_NormalsFilter->SetUseProgressBar(false);
  m_InterpolateSurfaceFilter->SetUseProgressBar(false);

  m_Contours = Surface::New();

  m_InterpolationResult = 0;
  m_CurrentNumberOfReducedContours = 0;
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

void mitk::SurfaceInterpolationController::AddNewContour(mitk::ContourModel::Pointer newContour, RestorePlanePositionOperation* op, int activeLabel)
{
  MITK_INFO << "AddNewContour 1";

  AffineTransform3D::Pointer transform = AffineTransform3D::New();
  transform = op->GetTransform();

  mitk::Vector3D direction = op->GetDirectionVector();
  int pos(-1);

  MITK_INFO << "AddNewContour 2";

  for (unsigned int i=0; i < m_MapOfContourLists[activeLabel].size(); i++)
  {
    itk::Matrix<float> diffM = transform->GetMatrix()-m_MapOfContourLists[activeLabel].at(i).position->GetTransform()->GetMatrix();
    bool isSameMatrix(true);
    for (unsigned int j = 0; j < 3; j++)
    {
      if (fabs(diffM[j][0]) > 0.0001 && fabs(diffM[j][1]) > 0.0001 && fabs(diffM[j][2]) > 0.0001)
      {
        isSameMatrix = false;
        break;
      }
    }
    itk::Vector<float> diffV = m_MapOfContourLists[activeLabel].at(i).position->GetTransform()->GetOffset()-transform->GetOffset();
    if ( isSameMatrix && m_MapOfContourLists[activeLabel].at(i).position->GetPos() == op->GetPos() && (fabs(diffV[0]) < 0.0001 && fabs(diffV[1]) < 0.0001 && fabs(diffV[2]) < 0.0001) )
    {
      pos = i;
      break;
    }
  }

  MITK_INFO << "AddNewContour 3";

  mitk::ContourModelToSurfaceFilter::Pointer converter = mitk::ContourModelToSurfaceFilter::New();
  converter->SetInput(newContour);
  converter->Update();

  mitk::Surface::Pointer surface = converter->GetOutput();

  MITK_INFO << "AddNewContour 4 pos: " << pos << " num vertices: " << newContour->GetNumberOfVertices();

  //Don't save a new empty contour
  if (pos == -1 && newContour->GetNumberOfVertices() > 0)
  {
    mitk::RestorePlanePositionOperation* newOp = new mitk::RestorePlanePositionOperation (OpRESTOREPLANEPOSITION, op->GetWidth(),
      op->GetHeight(), op->GetSpacing(), op->GetPos(), direction, transform);
    ContourPositionPair newData;
    newData.contour = newContour;
    newData.position = newOp;

    MITK_INFO << "AddNewContour 4.5";

    m_ReduceFilter->SetInput(m_MapOfContourLists[activeLabel].size(), surface);

    MITK_INFO << "AddNewContour 4.6";
    m_MapOfContourLists[activeLabel].push_back(newData);
  }
  //Edit an existing contour. If the contour is empty, edit it anyway so that the interpolation will always be consistent
  else if (pos != -1)
  {
    m_MapOfContourLists[activeLabel].at(pos).contour = newContour;
    m_ReduceFilter->SetInput(pos, surface);
  }

  MITK_INFO << "AddNewContour 5";

  m_ReduceFilter->Update();
  m_CurrentNumberOfReducedContours = m_ReduceFilter->GetNumberOfOutputs();

  MITK_INFO << "AddNewContour 6";

  for (unsigned int i=0; i < m_CurrentNumberOfReducedContours; i++)
  {
    m_NormalsFilter->SetInput(i, m_ReduceFilter->GetOutput(i));
    m_InterpolateSurfaceFilter->SetInput(i, m_NormalsFilter->GetOutput(i));
  }

  MITK_INFO << "AddNewContour 7";

  this->Modified();
}

void mitk::SurfaceInterpolationController::Interpolate()
{
  if (m_CurrentNumberOfReducedContours < 2)
  {
    //If no interpolation is possible reset the interpolation result
    m_InterpolationResult = 0;
    return;
  }

  MITK_INFO << "Interpolate 1";

/*
  ContourListMap::iterator it = m_MapOfContourLists.find(m_ActiveLabel);

  if (it == m_MapOfContourLists.end())
  {
    ContourPositionPairList newList;
    m_MapOfContourLists.insert(std::pair<unsigned int, ContourPositionPairList>(m_ActiveLabel, newList));
    m_InterpolationResult = 0;
    m_CurrentNumberOfReducedContours = 0;
  }
  else
  {
    MITK_INFO << "Interpolate() 3";

    for (unsigned int i = 0; i < m_MapOfContourLists[m_ActiveLabel].size(); i++)
    {
      MITK_INFO << "Interpolate() 4 " << i << " " << m_MapOfContourLists[m_ActiveLabel].at(i).contour->GetVtkPolyData()->GetNumberOfPoints();
      m_ReduceFilter->SetInput(i, m_MapOfContourLists[m_ActiveLabel].at(i).contour);
    }

    m_ReduceFilter->Update();

    m_CurrentNumberOfReducedContours = m_ReduceFilter->GetNumberOfOutputs();

    for (unsigned int i = 0; i < m_CurrentNumberOfReducedContours; i++)
    {
      m_NormalsFilter->SetInput(i, m_ReduceFilter->GetOutput(i));
      m_InterpolateSurfaceFilter->SetInput(i, m_NormalsFilter->GetOutput(i));
    }
  }
  */

  // update the filter and get the resulting distance-image
  m_InterpolateSurfaceFilter->SetDistanceImageVolume(m_DistanceImageVolume);

  itk::ImageBase<3>::Pointer itkImage = itk::ImageBase<3>::New();
  AccessFixedDimensionByItk_1( m_WorkingImage, GetImageBase, 3, itkImage );
  m_InterpolateSurfaceFilter->SetReferenceImage( itkImage.GetPointer() );
  m_NormalsFilter->SetWorkingImage(m_WorkingImage, m_ActiveLabel);

  m_InterpolateSurfaceFilter->Update();
  Image::Pointer distanceImage = m_InterpolateSurfaceFilter->GetOutput();

  MITK_INFO << "Interpolate 2";

  // create a surface from the distance-image
  mitk::ImageToSurfaceFilter::Pointer imageToSurfaceFilter = mitk::ImageToSurfaceFilter::New();
  imageToSurfaceFilter->SetInput( distanceImage );
  imageToSurfaceFilter->SetThreshold( 0 );
  imageToSurfaceFilter->Update();
  m_InterpolationResult = imageToSurfaceFilter->GetOutput();
  m_InterpolationResult->DisconnectPipeline();

  MITK_INFO << "Interpolate 3";

  vtkSmartPointer<vtkAppendPolyData> polyDataAppender = vtkSmartPointer<vtkAppendPolyData>::New();
  for (unsigned int i = 0; i < m_ReduceFilter->GetNumberOfOutputs(); i++)
  {
    polyDataAppender->AddInput(m_ReduceFilter->GetOutput(i)->GetVtkPolyData());
  }
  polyDataAppender->Update();
  m_Contours->SetVtkPolyData(polyDataAppender->GetOutput());

  MITK_INFO << "Interpolate 4";
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
  m_ReduceFilter->SetMinSpacing(minSpacing);
}

void mitk::SurfaceInterpolationController::SetMaxSpacing(double maxSpacing)
{
  m_ReduceFilter->SetMaxSpacing(maxSpacing);
  m_NormalsFilter->SetMaxSpacing(maxSpacing);
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
  double numberOfPointsAfterReduction = m_ReduceFilter->GetNumberOfPointsAfterReduction()*3;
  double sizeOfPoints = pow(numberOfPointsAfterReduction,2)*sizeof(double);
  double totalMem = mitk::MemoryUtilities::GetTotalSizeOfPhysicalRam();
  double percentage = sizeOfPoints/totalMem;
  return percentage;
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::SurfaceInterpolationController::GetImageBase(itk::Image<TPixel, VImageDimension>* input, itk::ImageBase<3>::Pointer& result)
{
  result->Graft(input);
}

void mitk::SurfaceInterpolationController::SetActiveLabel(int activeLabel)
{
  if (m_ActiveLabel == activeLabel)
    return;

  MITK_INFO << "SetActiveLabel 1";

  m_ActiveLabel = activeLabel;

  m_ReduceFilter->Reset();
  m_NormalsFilter->Reset();
  m_InterpolateSurfaceFilter->Reset();

  m_InterpolationResult = NULL;

  m_Contours->SetVtkPolyData(NULL);

  MITK_INFO << "SetActiveLabel 2";

  m_CurrentNumberOfReducedContours = m_MapOfContourLists[m_ActiveLabel].size();

  for (unsigned int i=0; i < m_CurrentNumberOfReducedContours; i++)
  {
    mitk::ContourModelToSurfaceFilter::Pointer converter = mitk::ContourModelToSurfaceFilter::New();
    converter->SetInput( m_MapOfContourLists[m_ActiveLabel].at(i).contour );
    converter->Update();
    mitk::Surface::Pointer surface = converter->GetOutput();
    MITK_INFO << "re adding: " << i;
    m_ReduceFilter->SetInput(i, surface);
    m_NormalsFilter->SetInput(i, m_ReduceFilter->GetOutput(i));
    m_InterpolateSurfaceFilter->SetInput(i, m_NormalsFilter->GetOutput(i));
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
