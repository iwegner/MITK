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

#include "mitkAutoSegmentationTool.h"
#include "mitkToolManager.h"
#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"

mitk::AutoSegmentationTool::AutoSegmentationTool()
:Tool("dummy"),
m_OverwriteExistingSegmentation (false)
{
}

mitk::AutoSegmentationTool::~AutoSegmentationTool()
{
}

const char* mitk::AutoSegmentationTool::GetGroup() const
{
  return "autoSegmentation";
}

void mitk::AutoSegmentationTool::SetOverwriteExistingSegmentation(bool overwrite)
{
  m_OverwriteExistingSegmentation = overwrite;
}

std::string mitk::AutoSegmentationTool::GetCurrentSegmentationName()
{
  if (m_ToolManager->GetWorkingData(0))
    return m_ToolManager->GetWorkingData(0)->GetName();
  else
    return "";
}

mitk::DataNode* mitk::AutoSegmentationTool::GetTargetSegmentationNode()
{
  mitk::DataNode::Pointer emptySegmentation;
  if (m_OverwriteExistingSegmentation)
  {
    emptySegmentation = m_ToolManager->GetWorkingData(0);
  }
  else
  {
    mitk::DataNode::Pointer refNode = m_ToolManager->GetReferenceData(0);
    if (refNode.IsNull())
    {
      //TODO create and use segmentation exceptions instead!!
      MITK_ERROR<<"No valid reference data!";
      return NULL;
    }
    std::string nodename = m_ToolManager->GetReferenceData(0)->GetName()+"_"+this->GetName();
    mitk::Color color;
    color.SetRed(1);
    color.SetBlue(0);
    color.SetGreen(0);
    emptySegmentation = CreateEmptySegmentationNode(dynamic_cast<mitk::Image*>(refNode->GetData()), nodename, color);
    m_ToolManager->GetDataStorage()->Add(emptySegmentation, refNode);

  }
  return emptySegmentation;
}

void mitk::AutoSegmentationTool::PasteSegmentation( Image* targetImage, Image* sourceImage, int pixelvalue, int timestep )
{
  if ((!targetImage)|| (!sourceImage)) return;

  try
  {
    AccessFixedDimensionByItk_2( targetImage, ItkPasteSegmentation, 3, sourceImage, pixelvalue );
  }
  catch( itk::ExceptionObject & e )
  {
    MITK_ERROR << "Could not paste segmentation " << e.GetDescription();
    m_ToolManager->ActivateTool(-1);
    return;
  }
  catch (...)
  {
    MITK_ERROR << "Could not generate segmentation.";
    m_ToolManager->ActivateTool(-1);
    return;
  }
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::AutoSegmentationTool::ItkPasteSegmentation(
       itk::Image<TPixel,VImageDimension>* targetImage, const mitk::Image* sourceImage, int overwritevalue )
{
  typedef itk::Image<TPixel,VImageDimension> ImageType;

  typename ImageType::Pointer sourceImageITK;
  CastToItkImage( sourceImage, sourceImageITK );

  typedef itk::ImageRegionConstIterator< ImageType > SourceIteratorType;
  typedef itk::ImageRegionIterator< ImageType >      TargetIteratorType;

  typename SourceIteratorType sourceIterator( sourceImageITK, sourceImageITK->GetLargestPossibleRegion() );
  typename TargetIteratorType targetIterator( targetImage, targetImage->GetLargestPossibleRegion() );

  sourceIterator.GoToBegin();
  targetIterator.GoToBegin();

  const int& activePixelValue = m_ToolManager->GetActiveLabel()->GetIndex();

  if (activePixelValue == 0) // if exterior is the active label
  {
    while ( !targetIterator.IsAtEnd() )
    {
      if (sourceIterator.Get() != 0)
      {
        targetIterator.Set( overwritevalue );
      }
      ++targetIterator;
      ++sourceIterator;
    }
  }
  else if (overwritevalue != 0) // if we are not erasing
  {
    while ( !targetIterator.IsAtEnd() )
    {
      const int targetValue = targetIterator.Get();
      if ( sourceIterator.Get() != 0 )
      {
        if (!m_ToolManager->GetLabelLocked(targetValue))
          targetIterator.Set( overwritevalue );
      }

      ++targetIterator;
      ++sourceIterator;
    }
  }
}
