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

#include "QmitkLabelSetWidget.h"

#include <berryPlatform.h>

#include <mitkIDataStorageService.h>
#include <mitkLabelSetImage.h>
#include <mitkProperties.h>
#include <mitkToolManagerProvider.h>
#include <mitkLabelSetImageToSurfaceThreadedFilter.h>
#include <mitkNrrdLabelSetImageReader.h>
#include <mitkNrrdLabelSetImageWriter.h>
#include <mitkRenderingManager.h>
#include <mitkImageWriteAccessor.h>

#include <QmitkDataStorageComboBox.h>
#include <QmitkNewSegmentationDialog.h>

#include <algorithm>
#include <cassert>
#include <iterator>

#include <QCompleter>
#include <QStringListModel>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QDateTime>


QmitkLabelSetWidget::QmitkLabelSetWidget(QWidget* parent): QWidget(parent),
m_ReferenceNode(0),
m_WorkingNode(0)
{
  m_Controls.setupUi(this);

  m_Controls.m_LabelSetTableWidget->Init();

  connect( m_Controls.m_cbWorkingNodeSelector, SIGNAL( OnSelectionChanged( const mitk::DataNode* ) ),
       this, SLOT( OnSegmentationSelectionChanged( const mitk::DataNode* ) ) );

  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(newLabel()), this, SLOT(OnNewLabel()) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(renameLabel(int, const mitk::Color&, const std::string&)), this, SLOT(OnRenameLabel(int, const mitk::Color&, const std::string&)) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(createSurface(int)), this, SLOT(OnCreateSurface(int)) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(goToLabel(const mitk::Point3D&)), this, SIGNAL(goToLabel(const mitk::Point3D&)) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(combineAndCreateSurface( const QList<QTableWidgetSelectionRange>& )),
      this, SLOT(OnCombineAndCreateSurface( const QList<QTableWidgetSelectionRange>&)) );

  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(createMask(int)), this, SLOT(OnCreateMask(int)) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(combineAndCreateMask( const QList<QTableWidgetSelectionRange>& )),
      this, SLOT(OnCombineAndCreateMask( const QList<QTableWidgetSelectionRange>&)) );

  connect( m_Controls.m_btNewLabelSet, SIGNAL(clicked()), this, SLOT(OnNewSegmentation()) );
  connect( m_Controls.m_btSaveLabelSet, SIGNAL(clicked()), this, SLOT(OnSaveSegmentation()) );
  connect( m_Controls.m_btLoadLabelSet, SIGNAL(clicked()), this, SLOT(OnLoadSegmentation()) );
  connect( m_Controls.m_btImportLabelSet, SIGNAL(clicked()), this, SLOT(OnImportSegmentation()) );
  connect( m_Controls.m_btNewLabel, SIGNAL(clicked()), this, SLOT(OnNewLabel()) );

  m_Controls.m_LabelSearchBox->setAlwaysShowClearIcon(true);
  m_Controls.m_LabelSearchBox->setShowSearchIcon(true);

  QStringList completionList;
  completionList << "";
  m_Completer = new QCompleter(completionList,this);
  m_Completer->setCaseSensitivity(Qt::CaseInsensitive);
  m_Controls.m_LabelSearchBox->setCompleter(m_Completer);

  connect( m_Controls.m_LabelSearchBox, SIGNAL(returnPressed()), this, SLOT(OnSearchLabel()) );
  connect( m_Controls.m_LabelSetTableWidget, SIGNAL(labelListModified(const QStringList&)), this, SLOT( OnLabelListModified(const QStringList&)) );

  QStringListModel* completeModel = static_cast<QStringListModel*> (m_Completer->model());
  completeModel->setStringList(m_Controls.m_LabelSetTableWidget->GetLabelList());

  // react whenever the set of selected segmentation changes
  mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  toolManager->ReferenceDataChanged += mitk::MessageDelegate<QmitkLabelSetWidget>( this, &QmitkLabelSetWidget::OnToolManagerReferenceDataModified );

  m_Controls.m_LabelSetTableWidget->setEnabled(false);
  m_Controls.m_LabelSearchBox->setEnabled(false);
  m_Controls.m_btNewLabelSet->setEnabled(false);
  m_Controls.m_btLoadLabelSet->setEnabled(false);
  m_Controls.m_btSaveLabelSet->setEnabled(false);
  m_Controls.m_btNewLabel->setEnabled(false);
  m_Controls.m_btImportLabelSet->setEnabled(false);
}

QmitkLabelSetWidget::~QmitkLabelSetWidget()
{
  mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  toolManager->ReferenceDataChanged -= mitk::MessageDelegate<QmitkLabelSetWidget>( this, &QmitkLabelSetWidget::OnToolManagerReferenceDataModified );
  m_WorkingNode = NULL;
  m_ReferenceNode = NULL;
}

void QmitkLabelSetWidget::ReInit()
{
  this->OnToolManagerReferenceDataModified();
  this->OnSegmentationSelectionChanged(m_Controls.m_cbWorkingNodeSelector->GetSelectedNode());
}

void QmitkLabelSetWidget::SetDataStorage( mitk::DataStorage& storage )
{
  m_DataStorage = &storage;
  m_Controls.m_cbWorkingNodeSelector->SetDataStorage(m_DataStorage);
}

void QmitkLabelSetWidget::SetPreferences( berry::IPreferences::Pointer prefs )
{
  m_Preferences = prefs;
}

void QmitkLabelSetWidget::SetPredicate( mitk::NodePredicateBase* predicate )
{
  m_Predicate = predicate;
  m_Controls.m_cbWorkingNodeSelector->SetPredicate(m_Predicate);
  m_Controls.m_cbWorkingNodeSelector->SetAutoSelectNewItems(true);
}

void QmitkLabelSetWidget::OnSegmentationSelectionChanged(const mitk::DataNode *node)
{
  if (m_WorkingNode == node) return;
  m_WorkingNode = const_cast<mitk::DataNode*>(node);
  m_Controls.m_LabelSetTableWidget->setEnabled(m_WorkingNode.IsNotNull());
  m_Controls.m_LabelSearchBox->setEnabled(m_WorkingNode.IsNotNull());
  m_Controls.m_btSaveLabelSet->setEnabled(m_WorkingNode.IsNotNull());
  m_Controls.m_btNewLabel->setEnabled(m_WorkingNode.IsNotNull());
  m_Controls.m_btImportLabelSet->setEnabled(m_WorkingNode.IsNotNull());

  if (m_WorkingNode.IsNotNull())
  {
    mitk::LabelSetImage* lsImage = dynamic_cast<mitk::LabelSetImage*>( m_WorkingNode->GetData() );
    m_Controls.m_LabelSetTableWidget->SetActiveLabelSetImage(lsImage);
  }
  else
  {
    m_Controls.m_LabelSetTableWidget->SetActiveLabelSetImage(NULL);
    return;
  }

  m_WorkingNode->SetVisibility(true);

  mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  toolManager->SetWorkingData(m_WorkingNode);
//      m_Controls.m_SlicesInterpolator->setEnabled( true );
  mitk::DataStorage::SetOfObjects::ConstPointer others = m_DataStorage->GetSubset(m_Predicate);
  for(mitk::DataStorage::SetOfObjects::const_iterator iter = others->begin(); iter != others->end(); ++iter)
  {
    mitk::DataNode* _other = *iter;
    if (_other != m_WorkingNode)
      _other->SetVisibility(false);
  }
}

void QmitkLabelSetWidget::OnToolManagerReferenceDataModified()
{
  mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  mitk::DataNode* node = toolManager->GetReferenceData(0);
  if (m_ReferenceNode == node) return;
  m_ReferenceNode = node;
  m_Controls.m_btNewLabelSet->setEnabled(m_ReferenceNode.IsNotNull());
  m_Controls.m_btLoadLabelSet->setEnabled(m_ReferenceNode.IsNotNull());
}

void QmitkLabelSetWidget::OnSearchLabel()
{
  m_Controls.m_LabelSetTableWidget->SetActiveLabel(m_Controls.m_LabelSearchBox->text().toStdString());
}

void QmitkLabelSetWidget::OnLabelListModified(const QStringList& list)
{
  QStringListModel* completeModel = static_cast<QStringListModel*> (m_Completer->model());
  completeModel->setStringList(list);
}

void QmitkLabelSetWidget::OnRenameLabel(int index, const mitk::Color& color, const std::string& name)
{
  mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  toolManager->ActivateTool(-1);

  // ask about the name and organ type of the new segmentation
  QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog( this );

  QString storedList = QString::fromStdString( m_Preferences->GetByteArray("Organ-Color-List","") );
  QStringList organColors;
  if (storedList.isEmpty())
  {
    organColors = GetDefaultOrganColorString();
  }
  else
  {
    // recover string list from BlueBerry view's preferences
    QString storedString = QString::fromStdString( m_Preferences->GetByteArray("Organ-Color-List","") );
    // match a string consisting of any number of repetitions of either "anything but ;" or "\;". This matches everything until the next unescaped ';'
    QRegExp onePart("(?:[^;]|\\\\;)*");
    int count = 0;
    int pos = 0;
    while( (pos = onePart.indexIn( storedString, pos )) != -1 )
    {
      ++count;
      int length = onePart.matchedLength();
      if (length == 0) break;
      QString matchedString = storedString.mid(pos, length);
      MITK_DEBUG << "   Captured length " << length << ": " << matchedString.toStdString();
      pos += length + 1; // skip separating ';'

      // unescape possible occurrences of '\;' in the string
      matchedString.replace("\\;", ";");

      // add matched string part to output list
      organColors << matchedString;
    }
  }

  dialog->setWindowTitle("Rename Label");
  dialog->SetSuggestionList( organColors );
  dialog->SetColor(color);
  dialog->SetSegmentationName(name);

  int dialogReturnValue = dialog->exec();

  if ( dialogReturnValue == QDialog::Rejected ) return; // user clicked cancel or pressed Esc or something similar

  mitk::DataNode* workingNode = mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetWorkingData(0);
  if (!workingNode) return;

  mitk::LabelSetImage* lsImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  if ( !lsImage ) return;

  lsImage->RenameLabel(index, dialog->GetSegmentationName().toStdString(), dialog->GetColor());
}

void QmitkLabelSetWidget::OnNewLabel()
{
  mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  toolManager->ActivateTool(-1);

  // ask about the name and organ type of the new segmentation
  QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog( this );

  QString storedList = QString::fromStdString( m_Preferences->GetByteArray("Organ-Color-List","") );
  QStringList organColors;
  if (storedList.isEmpty())
  {
    organColors = GetDefaultOrganColorString();
  }
  else
  {
    /*
    a couple of examples of how organ names are stored:

    a simple item is built up like 'name#AABBCC' where #AABBCC is the hexadecimal notation of a color as known from HTML

    items are stored separated by ';'
    this makes it necessary to escape occurrences of ';' in name.
    otherwise the string "hugo;ypsilon#AABBCC;eugen#AABBCC" could not be parsed as two organs
    but we would get "hugo" and "ypsilon#AABBCC" and "eugen#AABBCC"

    so the organ name "hugo;ypsilon" is stored as "hugo\;ypsilon"
    and must be unescaped after loading

    the following lines could be one split with Perl's negative lookbehind
    */

    // recover string list from BlueBerry view's preferences
    QString storedString = QString::fromStdString( m_Preferences->GetByteArray("Organ-Color-List","") );
    MITK_DEBUG << "storedString: " << storedString.toStdString();
    // match a string consisting of any number of repetitions of either "anything but ;" or "\;". This matches everything until the next unescaped ';'
    QRegExp onePart("(?:[^;]|\\\\;)*");
    MITK_DEBUG << "matching " << onePart.pattern().toStdString();
    int count = 0;
    int pos = 0;
    while( (pos = onePart.indexIn( storedString, pos )) != -1 )
    {
      ++count;
      int length = onePart.matchedLength();
      if (length == 0) break;
      QString matchedString = storedString.mid(pos, length);
      MITK_DEBUG << "   Captured length " << length << ": " << matchedString.toStdString();
      pos += length + 1; // skip separating ';'

      // unescape possible occurrences of '\;' in the string
      matchedString.replace("\\;", ";");

      // add matched string part to output list
      organColors << matchedString;
    }
    MITK_DEBUG << "Captured " << count << " organ name/colors";
  }

  dialog->SetSuggestionList( organColors );

  dialog->setWindowTitle("New Label");

  int dialogReturnValue = dialog->exec();

  if ( dialogReturnValue == QDialog::Rejected ) return; // user clicked cancel or pressed Esc or something similar

  mitk::Color color = dialog->GetColor();

  mitk::DataNode* workingNode = mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetWorkingData(0);
  if (!workingNode) return;

  mitk::LabelSetImage* lsImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  if ( !lsImage ) return;

  lsImage->AddLabel(dialog->GetSegmentationName().toStdString(), color);
}

void QmitkLabelSetWidget::OnCreateMask(int index)
{
  mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  toolManager->ActivateTool(-1);

  mitk::DataNode* segNode = toolManager->GetWorkingData(0);
  if (!segNode) return;

  mitk::LabelSetImage* segImage = dynamic_cast<mitk::LabelSetImage*>( segNode->GetData() );
  if (!segImage) return;

  // continue
}

void QmitkLabelSetWidget::OnCreateSurface(int index)
{
  mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  toolManager->ActivateTool(-1);

  mitk::DataNode* segNode = toolManager->GetWorkingData(0);
  if (!segNode) return;

  mitk::LabelSetImage* segImage = dynamic_cast<mitk::LabelSetImage*>( segNode->GetData() );
  if (!segImage) return;

  mitk::LabelSetImageToSurfaceThreadedFilter::Pointer filter =
     mitk::LabelSetImageToSurfaceThreadedFilter::New();

  filter->SetPointerParameter("Input", segImage);
  filter->SetParameter("RequestedLabel", index);
  filter->SetDataStorage( *m_DataStorage );

  filter->StartAlgorithm();
}

void QmitkLabelSetWidget::OnImportSegmentation()
{
  mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  toolManager->ActivateTool(-1);

  mitk::DataNode* segNode = toolManager->GetWorkingData(0);
  if (!segNode) return;

  mitk::LabelSetImage* segImage = dynamic_cast<mitk::LabelSetImage*>( segNode->GetData() );
  if (!segImage) return;

  std::string fileExtensions("Segmentation files (*.lset);;");
  QString qfileName = QFileDialog::getOpenFileName(this, "Import Segmentation", "", fileExtensions.c_str() );
  if (qfileName.isEmpty() ) return;

  mitk::NrrdLabelSetImageReader<unsigned char>::Pointer reader = mitk::NrrdLabelSetImageReader<unsigned char>::New();
  reader->SetFileName(qfileName.toLatin1());

  this->WaitCursorOn();

  try
  {
    reader->Update();
  }
  catch ( itk::ExceptionObject & e )
  {
      MITK_ERROR << "Exception caught: " << e.GetDescription();
      QMessageBox::information(this, "Import Segmentation", "Could not load the selected segmentation. See error log for details.\n");
      this->WaitCursorOff();
  }

  this->WaitCursorOff();

  mitk::LabelSetImage::Pointer lsImage = reader->GetOutput();

  if (!segImage->Concatenate(lsImage)) {
      QMessageBox::information(this,
      "Import segmentation...",
      "Could not import the selected segmentation session (dimensions must match).\n");
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetWidget::OnLoadSegmentation()
{
    mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    toolManager->ActivateTool(-1);

    mitk::DataNode* refNode = mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetReferenceData(0);
    if (!refNode) return;

    mitk::Image* refImage = dynamic_cast<mitk::Image*>( refNode->GetData() );
    if (!refImage) return;

    std::string fileExtensions("Segmentation files (*.lset);;");
    QString qfileName = QFileDialog::getOpenFileName(this, "Load Segmentation", "", fileExtensions.c_str() );
    if (qfileName.isEmpty() ) return;

    mitk::NrrdLabelSetImageReader<unsigned char>::Pointer reader = mitk::NrrdLabelSetImageReader<unsigned char>::New();
    reader->SetFileName(qfileName.toLatin1());

    this->WaitCursorOn();

    try
    {
        reader->Update();
    }
    catch ( itk::ExceptionObject & e )
    {
        MITK_ERROR << "Exception caught: " << e.GetDescription();
        QMessageBox::information(this, "Load Segmentation", "Could not load the selected segmentation. See error log for details.\n");
        this->WaitCursorOff();
    }

    this->WaitCursorOff();

    mitk::LabelSetImage::Pointer lsImage = reader->GetOutput();

    mitk::DataNode::Pointer newNode = mitk::DataNode::New();
    newNode->SetData(lsImage);
    newNode->SetName( lsImage->GetLabelSetName() );

    m_DataStorage->Add(newNode,refNode);

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetWidget::OnSaveSegmentation()
{
  mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  toolManager->ActivateTool(-1);

  mitk::DataNode* workingNode = toolManager->GetWorkingData(0);
  if (!workingNode) return;

  mitk::LabelSetImage* lsImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  if ( !lsImage ) return;

  // update the name in case has changed
  lsImage->SetName( workingNode->GetName() );

  //set last modification date and time
  QDateTime cTime = QDateTime::currentDateTime ();
  lsImage->SetLabelSetLastModified( cTime.toString().toStdString() );

  QString proposedFileName = QString::fromStdString(workingNode->GetName());
  QString selected_suffix("segmentation files (*.lset)");
  QString qfileName = QFileDialog::getSaveFileName(this, "Save Segmentation", proposedFileName, selected_suffix);
  if (qfileName.isEmpty() ) return;

  mitk::NrrdLabelSetImageWriter<unsigned char>::Pointer writer = mitk::NrrdLabelSetImageWriter<unsigned char>::New();
  writer->SetFileName(qfileName.toStdString());
  std::string newName = itksys::SystemTools::GetFilenameWithoutExtension(qfileName.toStdString());
  lsImage->SetName(newName);
  workingNode->SetName(newName);
  writer->SetInput(lsImage);

  this->WaitCursorOn();

  try
  {
      writer->Update();
  }
  catch ( itk::ExceptionObject & excep )
  {
    MITK_ERROR << "Exception caught: " << excep.GetDescription();
    QMessageBox::information(this, "Save Segmenation", "Could not save active segmentation. See error log for details.\n\n");
    this->WaitCursorOff();
  }

  this->WaitCursorOff();
}

void QmitkLabelSetWidget::OnNewSegmentation()
{
  mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  toolManager->ActivateTool(-1);

  mitk::DataNode::Pointer refNode = toolManager->GetReferenceData(0);
  if (refNode.IsNull()) return;

  mitk::Image::Pointer refImage = dynamic_cast<mitk::Image*>( refNode->GetData() );
  if (refImage.IsNull()) return;

  bool ok = false;
  QString refNodeName = QString::fromStdString(refNode->GetName());
  refNodeName.append("-");
  QString newName = QInputDialog::getText(this, "New Segmentation", "Set a name:", QLineEdit::Normal, refNodeName, &ok);
  if (!ok) return;

  mitk::PixelType pixelType(mitk::MakeScalarPixelType<unsigned char>() );
  mitk::LabelSetImage::Pointer labelSetImage = mitk::LabelSetImage::New();

  if (refImage->GetDimension() == 2)
  {
    const unsigned int dimensions[] = { refImage->GetDimension(0), refImage->GetDimension(1), 1 };
    labelSetImage->Initialize(pixelType, 3, dimensions);
  }
  else
  {
    labelSetImage->Initialize(pixelType, refImage->GetDimension(), refImage->GetDimensions());
  }

  unsigned int byteSize = sizeof(unsigned char);
  for (unsigned int dim = 0; dim < labelSetImage->GetDimension(); ++dim)
  {
    byteSize *= labelSetImage->GetDimension(dim);
  }

  mitk::ImageWriteAccessor accessor(static_cast<mitk::Image*>(labelSetImage));
  memset( accessor.GetData(), 0, byteSize );

  mitk::TimeSlicedGeometry::Pointer originalGeometry = refImage->GetTimeSlicedGeometry()->Clone();
  labelSetImage->SetGeometry( originalGeometry );

  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  newNode->SetData(labelSetImage);
  newNode->SetName(newName.toStdString());
  labelSetImage->SetName(newName.toStdString());

  m_DataStorage->Add(newNode,refNode);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  this->OnNewLabel();
}

void QmitkLabelSetWidget::OnCombineAndCreateSurface( const QList<QTableWidgetSelectionRange>& ranges )
{

}

void QmitkLabelSetWidget::OnCombineAndCreateMask( const QList<QTableWidgetSelectionRange>& ranges )
{

}

void QmitkLabelSetWidget::WaitCursorOn()
{
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
}

void QmitkLabelSetWidget::BusyCursorOn()
{
  QApplication::setOverrideCursor( QCursor(Qt::BusyCursor) );
}

void QmitkLabelSetWidget::WaitCursorOff()
{
  this->RestoreOverrideCursor();
}

void QmitkLabelSetWidget::BusyCursorOff()
{
  this->RestoreOverrideCursor();
}

void QmitkLabelSetWidget::RestoreOverrideCursor()
{
  QApplication::restoreOverrideCursor();
}
