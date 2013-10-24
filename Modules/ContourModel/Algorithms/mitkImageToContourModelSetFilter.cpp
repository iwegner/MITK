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

#include "mitkImageToContourModelSetFilter.h"
#include "mitkImageAccessByItk.h"
#include "mitkProgressBar.h"

#include "itkContourExtractor2DImageFilter.h"

mitk::ImageToContourModelSetFilter::ImageToContourModelSetFilter() :
m_UseProgressBar(false),
m_ProgressStepSize(1),
m_SliceGeometry(NULL)
{
}

mitk::ImageToContourModelSetFilter::~ImageToContourModelSetFilter()
{

}

void mitk::ImageToContourModelSetFilter::SetInput ( const mitk::ImageToContourModelSetFilter::InputType* input )
{
  this->ProcessObject::SetNthInput ( 0, const_cast<InputType*> ( input ) );
  this->Modified();
}

/*
void mitk::ImageToContourModelSetFilter::SetInput ( unsigned int idx, const mitk::ImageToContourModelSetFilter::InputType* input )
{
  if ( idx + 1 > this->GetNumberOfInputs() )
  {
    this->SetNumberOfRequiredInputs(idx + 1);
  }
  if ( input != static_cast<InputType*> ( ProcessObject::GetInput ( idx ) ) )
  {
    this->ProcessObject::SetNthInput ( idx, const_cast<InputType*> ( input ) );
    this->Modified();
  }
}
*/

const mitk::ImageToContourModelSetFilter::InputType* mitk::ImageToContourModelSetFilter::GetInput( void )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  return static_cast<const mitk::ImageToContourModelSetFilter::InputType*>(this->ProcessObject::GetInput(0));
}
/*
const mitk::ImageToContourModelSetFilter::InputType* mitk::ImageToContourModelSetFilter::GetInput( unsigned int idx )
{
  if (this->GetNumberOfInputs() < 1)
    return NULL;
  return static_cast<const mitk::ImageToContourModelSetFilter::InputType*>(this->ProcessObject::GetInput(idx));
}
*/
void mitk::ImageToContourModelSetFilter::GenerateData()
{
  mitk::Image::ConstPointer sliceImage = ImageToContourModelSetFilter::GetInput();

  if ( !sliceImage )
  {
    MITK_ERROR << "No input available. Please set the input!" << std::endl;
    itkExceptionMacro("No input available. Please set the input!");
    return;
  }

  if ( sliceImage->GetDimension() > 2 || sliceImage->GetDimension() < 2)
  {
    MITK_ERROR << "Please assure that your input image is 2D!" << std::endl;
    itkExceptionMacro("Please assure that your input image is 2D!");
    return;
  }

  m_SliceGeometry = sliceImage->GetGeometry();

  AccessFixedDimensionByItk(sliceImage, ExtractContoursITKProcessing, 2);
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::ImageToContourModelSetFilter::ExtractContoursITKProcessing (itk::Image<TPixel, VImageDimension>* sliceImage)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::ContourExtractor2DImageFilter<ImageType> ContourExtractorType;

  typename ContourExtractorType::Pointer contourExtractor = ContourExtractorType::New();
  contourExtractor->SetInput(sliceImage);
  contourExtractor->SetContourValue(0.5);
  contourExtractor->Update();

  unsigned int foundPaths = contourExtractor->GetNumberOfOutputs();

  //
  // set the number of outputs to the number of paths found
  // initialize the output contour models according to the available time steps
  //
  const mitk::Image* image = this->GetInput();
  unsigned int numberOfTimeSteps = image->GetTimeGeometry()->CountTimeSteps();

  mitk::ContourModelSet::Pointer output = this->GetOutput();
  assert ( output.IsNotNull() );
  output->Initialize();

  output->Expand( numberOfTimeSteps );

  for (unsigned int i = 0; i < foundPaths; i++)
  {
    if (!contourExtractor->GetOutput(i)) return;
    const ContourPath* currentPath = contourExtractor->GetOutput(i)->GetVertexList();
    mitk::ContourModel::Pointer contour = mitk::ContourModel::New();
    for (unsigned int j = 0; j < currentPath->Size(); j++)
    {
      mitk::Point3D currentWorldPoint;
      currentWorldPoint[0] = currentPath->ElementAt(j)[0];
      currentWorldPoint[1] = currentPath->ElementAt(j)[1];
      currentWorldPoint[2] = 0;
// fix me: either here or in the mapper this transformation should be applied
//      m_SliceGeometry->IndexToWorld(currentWorldPoint, currentWorldPoint);

      contour->AddVertex(currentWorldPoint);
    }
    output->AddContourModel(contour);
  }
}

void mitk::ImageToContourModelSetFilter::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();
}

void mitk::ImageToContourModelSetFilter::SetUseProgressBar(bool status)
{
  m_UseProgressBar = status;
}
