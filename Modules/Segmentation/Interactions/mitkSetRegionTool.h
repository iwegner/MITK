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

#ifndef mitkSetRegionTool_h_Included
#define mitkSetRegionTool_h_Included

#include "mitkCommon.h"
#include <MitkSegmentationExports.h>
#include "mitkFeedbackContourTool.h"

namespace mitk
{

class Image;
class StateMachineAction;
class InteractionEvent;
/**
  \brief Fills or erases a 2D region

  \sa FeedbackContourTool
  \sa ExtractImageFilter
  \sa OverwriteSliceImageFilter

  \ingroup Interaction
  \ingroup ToolManagerEtAl

  Finds the outer contour of a shape in 2D (possibly including holes) and sets all
  the inside pixels to a specified value. This might fill holes or erase segmentations.

  \warning Only to be instantiated by mitk::ToolManager.

  $Author$
*/
class MitkSegmentation_EXPORT SetRegionTool : public FeedbackContourTool
{
  public:

    mitkClassMacro(SetRegionTool, FeedbackContourTool);

  protected:

    SetRegionTool(); // purposely hidden
    virtual ~SetRegionTool();

    void ConnectActionsAndFunctions();

    virtual void Activated();
    virtual void Deactivated();

    virtual bool OnMousePressed ( StateMachineAction*, InteractionEvent* );
    virtual bool OnMouseReleased( StateMachineAction*, InteractionEvent* );

    bool m_LogicInverted;

    bool m_FillContour;
    bool m_StatusFillWholeSlice;
};

} // namespace

#endif


