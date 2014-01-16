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

#ifndef QmitkSurfaceStampWidget_h_Included
#define QmitkSurfaceStampWidget_h_Included

#include "SegmentationUIExports.h"

#include <QWidget>

#include "ui_QmitkSurfaceStampWidgetControls.h"

namespace mitk {
  class ToolManager;
}

/**
  \brief GUI for surface-based interpolation.

  \ingroup ToolManagerEtAl
  \ingroup Widgets
*/

class SegmentationUI_EXPORT QmitkSurfaceStampWidget : public QWidget
{
  Q_OBJECT

  public:

    QmitkSurfaceStampWidget(QWidget* parent = 0, const char* name = 0);
    virtual ~QmitkSurfaceStampWidget();

    void OnToolManagerWorkingDataModified();

    void SetDataStorage( mitk::DataStorage& storage );

  protected slots:

    void OnShowInformation(bool);

    void OnStamp();

private:

    mitk::DataNode::Pointer m_WorkingNode;

    mitk::ToolManager* m_ToolManager;

    mitk::WeakPointer<mitk::DataStorage> m_DataStorage;

    Ui::QmitkSurfaceStampWidgetControls m_Controls;
};

#endif
