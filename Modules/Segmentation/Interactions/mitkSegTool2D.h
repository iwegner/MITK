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

#ifndef mitkSegTool2D_h_Included
#define mitkSegTool2D_h_Included

#include "mitkCommon.h"
#include "SegmentationExports.h"
#include "mitkTool.h"
#include "mitkImage.h"
#include "mitkStateEvent.h"
#include "mitkPositionEvent.h"
#include "mitkPlanePositionManager.h"
#include "mitkRestorePlanePositionOperation.h"
#include "mitkInteractionConst.h"
#include "mitkToolCommand.h"
#include <mitkDiffSliceOperation.h>


namespace mitk
{

class BaseRenderer;

/**
  \brief Abstract base class for segmentation tools.

  \sa Tool

  \ingroup Interaction
  \ingroup ToolManagerEtAl

  Implements 2D segmentation specific helper methods, that might be of use to
  all kind of 2D segmentation tools. At the moment these are:
   - Determination of the slice where the user paints upon (DetermineAffectedImageSlice)
   - Projection of a 3D contour onto a 2D plane/slice

   SegTool2D tries to structure the interaction a bit. If you pass "PressMoveRelease" as the interaction type
   of your derived tool, you might implement the methods OnMousePressed, OnMouseMoved, and OnMouseReleased.
   Yes, your guess about when they are called is correct.

  \warning Only to be instantiated by mitk::ToolManager.

  $Author$
*/
class Segmentation_EXPORT SegTool2D : public Tool
{
  public:

    mitkClassMacro(SegTool2D, Tool);

    /**
      \brief Calculates for a given Image and PlaneGeometry, which slice of the image (in index corrdinates) is meant by the plane.

      \return false, if no slice direction seems right (e.g. rotated planes)
      \param affectedDimension The image dimension, which is constant for all points in the plane, e.g. Axial --> 2
      \param affectedSlice The index of the image slice
    */
    static bool DetermineAffectedImageSlice( const Image* image, const PlaneGeometry* plane, int& affectedDimension, int& affectedSlice );

    void SetShowMarkerNodes(bool);

    /**
     * \brief Enables or disables the 3D interpolation after writing back the 2D segmentation result, and defaults to false.
     */
    void SetEnable3DInterpolation(bool);

    /**
     * \brief Enables or disables the 2D interpolation after writing back the 2D segmentation result, and defaults to false.
     */
    void SetEnable2DInterpolation(bool);

  protected:

    SegTool2D(); // purposely hidden
    SegTool2D(const char*); // purposely hidden
    virtual ~SegTool2D();

    /**
    * \brief Calculates how good the data, this statemachine handles, is hit by the event.
    *
    */
    virtual float CanHandleEvent( StateEvent const *stateEvent) const;

    /**
      \brief Extract the slice of an image that the user just scribbles on.
      \return NULL if SegTool2D is either unable to determine which slice was affected, or if there was some problem getting the image data at that position.
    */
    Image::Pointer GetAffectedImageSliceAs2DImage(const PositionEvent*, const Image* image);

    /**
      \brief Extract the slice of an image cut by given plane.
      \return NULL if SegTool2D is either unable to determine which slice was affected, or if there was some problem getting the image data at that position.
    */
    Image::Pointer GetAffectedImageSliceAs2DImage(const PlaneGeometry* planeGeometry, const Image* image, unsigned int timeStep);

    /**
      \brief Extract the slice of the currently selected working image that the user just scribbles on.
      \return NULL if SegTool2D is either unable to determine which slice was affected, or if there was some problem getting the image data at that position,
                   or just no working image is selected.
    */
    Image::Pointer GetAffectedWorkingSlice(const PositionEvent*);

    /**
      \brief Extract the slice of the currently selected reference image that the user just scribbles on.
      \return NULL if SegTool2D is either unable to determine which slice was affected, or if there was some problem getting the image data at that position,
                   or just no reference image is selected.
    */
    Image::Pointer GetAffectedReferenceSlice(const PositionEvent*);

    void PasteSegmentationOnWorkingImage( Image* targetSlice, Image* sourceSlice, int paintingPixelValue, int timestep );

    void WriteBackSegmentationResult (const PositionEvent*, Image*);

    void WriteBackSegmentationResult (const PlaneGeometry* planeGeometry, Image*, unsigned int timeStep);

    /**
      \brief Adds a new node called Contourmarker to the datastorage which holds a mitk::PlanarFigure.
             By selecting this node the slicestack will be reoriented according to the PlanarFigure's Geometry
    */

    unsigned int AddContourmarker ( const PositionEvent* );

    BaseRenderer*  m_LastEventSender;
    int   m_LastEventSlice;
    bool m_3DInterpolationEnabled;
    bool m_2DInterpolationEnabled;

    ToolCommand::Pointer m_ProgressCommand;

  private:
    //The prefix of the contourmarkername. Suffix is a consecutive number
    const std::string     m_Contourmarkername;

    bool m_ShowMarkerNodes;

    DiffSliceOperation* m_doOperation;
    DiffSliceOperation* m_undoOperation;

    template<typename TPixel, unsigned int VImageDimension>
    void ItkPasteSegmentationOnWorkingImage( itk::Image<TPixel,VImageDimension>* targetSlice, const mitk::Image* sourceSlice, int pixelvalue );
};

} // namespace

#endif
