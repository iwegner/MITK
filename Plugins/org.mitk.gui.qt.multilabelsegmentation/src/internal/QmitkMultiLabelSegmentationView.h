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

#ifndef QmitkMultiLabelSegmentationView_h
#define QmitkMultiLabelSegmentationView_h

#include <berryISizeProvider.h>
#include <berryIBerryPreferences.h>

#include <QmitkAbstractView.h>
#include <mitkIRenderWindowPartListener.h>
#include "mitkSegmentationInteractor.h"

#include "ui_QmitkMultiLabelSegmentationControls.h"

class QmitkRenderWindow;

/**
 * \ingroup ToolManagerEtAl
 * \ingroup org_mitk_gui_qt_multilabelsegmentation_internal
 */
class QmitkMultiLabelSegmentationView : public QmitkAbstractView,
  public mitk::IRenderWindowPartListener, public berry::ISizeProvider
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

  QmitkMultiLabelSegmentationView();
  virtual ~QmitkMultiLabelSegmentationView();

  typedef std::map<mitk::DataNode*, unsigned long> NodeTagMapType;

  // GUI setup
  void CreateQtPartControl(QWidget* parent);

  virtual int GetSizeFlags(bool width);
  virtual int ComputePreferredSize(bool width, int /*availableParallel*/, int /*availablePerpendicular*/, int preferredResult);

protected slots:

  /// \brief reaction to the selection of a new patient (reference) image in the DataStorage combobox
  void OnReferenceSelectionChanged(const mitk::DataNode* node);

  /// \brief reaction to the selection of a new Segmentation (working) image in the DataStorage combobox
  void OnSegmentationSelectionChanged(const mitk::DataNode *node);

  /// \brief reaction to the selection of any 2D segmentation tool
  void OnManualTool2DSelected(int id);

  /// \brief reaction to button "Load Segmentation"
  void OnLoadSegmentation();

  /// \brief reaction to button "Save Segmentation"
  void OnSaveSegmentation();

  /// \brief reaction to button "Delete Segmentation"
  void OnDeleteSegmentation();

  /// \brief reaction to button "New Label"
  void OnNewLabel();

  /// \brief reaction to button "New Segmentation"
  void OnNewSegmentation();

  /// \brief reaction to button "Surface Stamp"
  void OnSurfaceStamp();

  /// \brief reaction to button "Mask Stamp"
  void OnMaskStamp();

  /// \brief reaction to signal "goToLabel" from labelset widget
  void OnGoToLabel(const mitk::Point3D& pos);

protected:

  void SetFocus();

  void UpdateControls();

  void RenderWindowPartActivated(mitk::IRenderWindowPart *renderWindowPart);

  void RenderWindowPartDeactivated(mitk::IRenderWindowPart *renderWindowPart);

  void ResetMouseCursor();

  void SetMouseCursor(const us::ModuleResource, int hotspotX, int hotspotY );

  void InitializeListeners();

  /// \brief Checks if two images have the same size and geometry
  bool CheckForSameGeometry(const mitk::Image *image1, const mitk::Image *image2) const;

  /// \brief Reimplemented from QmitkAbstractView
  virtual void NodeAdded(const mitk::DataNode* node);

  /// \brief Reimplemented from QmitkAbstractView
  virtual void NodeRemoved(const mitk::DataNode* node);

  QString GetLastFileOpenPath();

  void SetLastFileOpenPath(const QString& path);

  // handling of a list of known (organ name, organ color) combination
  // ATTENTION these methods are defined in QmitkSegmentationOrganNamesHandling.cpp
  QStringList GetDefaultOrganColorString();
  void UpdateOrganList(QStringList& organColors, const QString& organname, mitk::Color colorname);
  void AppendToOrganList(QStringList& organColors, const QString& organname, int r, int g, int b);

  // the Qt parent of our GUI (NOT of this object)
  QWidget* m_Parent;

  // our GUI
  Ui::QmitkMultiLabelSegmentationControls m_Controls;

  mitk::IRenderWindowPart* m_IRenderWindowPart;

  mitk::ToolManager* m_ToolManager;

  mitk::DataNode::Pointer m_ReferenceNode;
  mitk::DataNode::Pointer m_WorkingNode;

  mitk::NodePredicateAnd::Pointer m_ReferencePredicate;
  mitk::NodePredicateAnd::Pointer m_SegmentationPredicate;
  mitk::NodePredicateAnd::Pointer m_SurfacePredicate;
  mitk::NodePredicateAnd::Pointer m_MaskPredicate;

  bool m_MouseCursorSet;

  mitk::SegmentationInteractor::Pointer m_Interactor;

  /**
    * Reference to the service registration of the observer,
    * it is needed to unregister the observer on unload.
  */
 // us::ServiceRegistration<mitk::InteractionEventObserver> m_ServiceRegistration;

};

#endif // QmitkMultiLabelSegmentationView_h
