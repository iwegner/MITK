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

#include "QmitkLabelSetTableWidget.h"

#include <mitkProperties.h>
#include <mitkRenderingManager.h>
#include <mitkToolManagerProvider.h>

#include <QHeaderView>
#include <QAction>
#include <QMenu>
#include <QEvent>
#include <QKeyEvent>
#include <QPushButton>
#include <QColorDialog>
#include <QMessageBox>
#include <QWidgetAction>
#include <QLabel>
#include <QTableWidgetSelectionRange>
#include <QVBoxLayout>
#include <QStringList>
#include <QApplication>

#include <itkCommand.h>

QmitkLabelSetTableWidget::QmitkLabelSetTableWidget( QWidget* parent ) : QTableWidget(parent),
m_BlockEvents(false),
m_AutoSelectNewLabels(true),
m_AllOutlined(false),
m_ColorSequenceRainbow( new mitk::ColorSequenceRainbow() )
{
  m_ColorSequenceRainbow->GoToBegin();

  m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  assert(m_ToolManager);

  m_ToolManager->WorkingDataChanged += mitk::MessageDelegate<QmitkLabelSetTableWidget>( this, &QmitkLabelSetTableWidget::OnToolManagerWorkingDataModified );

  this->Initialize();
}

QmitkLabelSetTableWidget::~QmitkLabelSetTableWidget()
{
  m_ToolManager->WorkingDataChanged -= mitk::MessageDelegate<QmitkLabelSetTableWidget>( this, &QmitkLabelSetTableWidget::OnToolManagerWorkingDataModified );

  // remove listeners
  if(m_LabelSetImage.IsNotNull())
  {
    m_LabelSetImage->AddLabelEvent.RemoveListener( mitk::MessageDelegate<QmitkLabelSetTableWidget>(
        this, &QmitkLabelSetTableWidget::LabelAdded ) );

    m_LabelSetImage->RemoveLabelEvent.RemoveListener( mitk::MessageDelegate<QmitkLabelSetTableWidget>(
        this, &QmitkLabelSetTableWidget::LabelRemoved ) );

    m_LabelSetImage->ModifyLabelEvent.RemoveListener( mitk::MessageDelegate1<QmitkLabelSetTableWidget
      , int>( this, &QmitkLabelSetTableWidget::LabelModified ) );

    m_LabelSetImage->ActiveLabelEvent.RemoveListener( mitk::MessageDelegate1<QmitkLabelSetTableWidget
      , int>( this, &QmitkLabelSetTableWidget::ActiveLabelChanged ) );

    m_LabelSetImage->AllLabelsModifiedEvent.RemoveListener( mitk::MessageDelegate<QmitkLabelSetTableWidget>(
        this, &QmitkLabelSetTableWidget::AllLabelsModified ) );
  }
}

void QmitkLabelSetTableWidget::OnToolManagerWorkingDataModified()
{
  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);

  if (workingNode)
    this->SetActiveLabelSetImage( dynamic_cast<mitk::LabelSetImage*>( workingNode->GetData() ));
  else
    this->SetActiveLabelSetImage(NULL);
}

void QmitkLabelSetTableWidget::setEnabled(bool enabled)
{
  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);

  if (workingNode)
    this->SetActiveLabelSetImage( dynamic_cast<mitk::LabelSetImage*>( workingNode->GetData() ));
  else
    this->SetActiveLabelSetImage(NULL);

  QWidget::setEnabled(enabled);
}

void QmitkLabelSetTableWidget::Initialize()
{
  connect(this, SIGNAL(itemClicked(QTableWidgetItem *)), this, SLOT(OnItemClicked(QTableWidgetItem *)));
  connect(this, SIGNAL(itemDoubleClicked(QTableWidgetItem *)), this, SLOT(OnItemDoubleClicked(QTableWidgetItem *)));
//  connect(this, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(OnItemNameChanged(QTableWidgetItem *)));
  this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
  this->setTabKeyNavigation(false);
  this->setAlternatingRowColors(false);
  this->setFocusPolicy(Qt::NoFocus);
  this->setColumnCount(4);
  this->resizeColumnToContents(NAME_COL);
  this->setColumnWidth(LOCKED_COL,25);
  this->setColumnWidth(COLOR_COL,25);
  this->setColumnWidth(VISIBLE_COL,25);
  this->horizontalHeader()->setResizeMode( 0, QHeaderView::Stretch );
//  this->setStyleSheet("QTableView {selection-background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #3257bf, stop: 1 white);}");
//  this->setStyleSheet("QTableView {selection-background-color: #5151cd; selection-color: #ffffff;}");
 // this->setStyleSheet("background: rgb(255,255,255);color:rgb(0,0,0); font-family:Arial Narrow;font-size:18px;"
//"selection-background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #3257bf, stop: 1 white);");

  this->setContextMenuPolicy(Qt::CustomContextMenu);

  connect( this, SIGNAL(customContextMenuRequested(const QPoint&))
    , this, SLOT(NodeTableViewContextMenuRequested(const QPoint&)) );

  this->horizontalHeader()->hide();
  this->setSortingEnabled ( false );
  //this->horizontalHeader()->hide();
  this->verticalHeader()->hide();
  this->setEditTriggers(QAbstractItemView::NoEditTriggers);

  this->setSelectionMode(QAbstractItemView::ExtendedSelection);
  this->setSelectionBehavior(QAbstractItemView::SelectRows);
}

//QStringList& QmitkLabelSetTableWidget::GetLabelStringList()
//{
//  return m_LabelStringList;
//}

bool QmitkLabelSetTableWidget::GetAutoSelectNewLabels()
{
  return m_AutoSelectNewLabels;
}

void QmitkLabelSetTableWidget::SetActiveLabelSetImage(mitk::LabelSetImage* _image)
{
  // if there is an old labelset, remove listeners
  if(m_LabelSetImage.IsNotNull())
  {
    m_LabelSetImage->AddLabelEvent.RemoveListener( mitk::MessageDelegate<QmitkLabelSetTableWidget>(
        this, &QmitkLabelSetTableWidget::LabelAdded ) );

    m_LabelSetImage->RemoveLabelEvent.RemoveListener( mitk::MessageDelegate<QmitkLabelSetTableWidget>(
        this, &QmitkLabelSetTableWidget::LabelRemoved ) );

    m_LabelSetImage->ModifyLabelEvent.RemoveListener( mitk::MessageDelegate1<QmitkLabelSetTableWidget
      , int>( this, &QmitkLabelSetTableWidget::LabelModified ) );

    m_LabelSetImage->ActiveLabelEvent.RemoveListener( mitk::MessageDelegate1<QmitkLabelSetTableWidget
      , int>( this, &QmitkLabelSetTableWidget::ActiveLabelChanged ) );

    m_LabelSetImage->AllLabelsModifiedEvent.RemoveListener( mitk::MessageDelegate<QmitkLabelSetTableWidget>(
        this, &QmitkLabelSetTableWidget::AllLabelsModified ) );
  }

  m_LabelSetImage = _image;

  // add listeners to the new labelset
  if(m_LabelSetImage.IsNotNull())
  {
    m_LabelSetImage->AddLabelEvent.AddListener( mitk::MessageDelegate<QmitkLabelSetTableWidget>(
        this, &QmitkLabelSetTableWidget::LabelAdded ) );

    m_LabelSetImage->RemoveLabelEvent.AddListener( mitk::MessageDelegate<QmitkLabelSetTableWidget>(
        this, &QmitkLabelSetTableWidget::LabelRemoved ) );

    m_LabelSetImage->ModifyLabelEvent.AddListener( mitk::MessageDelegate1<QmitkLabelSetTableWidget
      , int>( this, &QmitkLabelSetTableWidget::LabelModified ) );

    m_LabelSetImage->ActiveLabelEvent.AddListener( mitk::MessageDelegate1<QmitkLabelSetTableWidget
      , int>( this, &QmitkLabelSetTableWidget::ActiveLabelChanged ) );

    m_LabelSetImage->AllLabelsModifiedEvent.AddListener( mitk::MessageDelegate<QmitkLabelSetTableWidget>(
        this, &QmitkLabelSetTableWidget::AllLabelsModified ) );
  }

  this->Reset();
}

void QmitkLabelSetTableWidget::SetActiveLabel(const std::string& text)
{

  int pixelValue = -1;
  int row = -1;
  for(int i = 0; i < this->rowCount(); i++){
    if( this->item(i,0)->text().toStdString().compare(text) == 0)
    {
      pixelValue = this->item(i,0)->data(Qt::UserRole).toInt();
      row = i;
      break;
    }
  }
  if(pixelValue == -1) return;
  m_LabelSetImage->SetActiveLabel(pixelValue);

  QTableWidgetItem* nameItem = this->item(row,NAME_COL);
  if (!nameItem) return;

  this->clearSelection();
  this->setSelectionMode(QAbstractItemView::SingleSelection);
  this->selectRow(row);
  this->scrollToItem(nameItem);
  this->setSelectionMode(QAbstractItemView::ExtendedSelection);

  this->WaitCursorOn();
  const mitk::Point3D& pos = m_LabelSetImage->GetLabelCenterOfMassCoordinates(pixelValue, true);
  this->WaitCursorOff();

  m_ToolManager->WorkingDataModified.Send();

  if (pos.GetVnlVector().max_value() > 0.0) emit goToLabel(pos);

  emit activeLabelChanged(pixelValue);

}

void QmitkLabelSetTableWidget::LabelAdded()
{
  // this is an event function, avoid calling ourself
  if (!m_BlockEvents)
  {
    m_BlockEvents = true;
    this->Reset();
    m_BlockEvents = false;
  }
}

void QmitkLabelSetTableWidget::LabelRemoved( )
{
  // this is an event function, avoid calling ourself
  if (!m_BlockEvents)
  {
    m_BlockEvents = true;
    this->Reset();
    m_BlockEvents = false;
  }
}

void QmitkLabelSetTableWidget::AllLabelsModified( )
{
  // this is an event function, avoid calling ourself
  for(int i=0; i<this->rowCount(); i++)
    this->LabelModified(i);
}

void QmitkLabelSetTableWidget::ActiveLabelChanged(int rowIndex)
{
  if (!m_BlockEvents)
  {
    QTableWidgetItem* nameItem = this->item(rowIndex, NAME_COL);
    if (!nameItem) return;
    this->clearSelection();
    this->setSelectionMode(QAbstractItemView::SingleSelection);
    this->selectRow(rowIndex);
    this->scrollToItem(nameItem);
    this->setSelectionMode(QAbstractItemView::ExtendedSelection);
    emit activeLabelChanged(rowIndex);
  }
}

void QmitkLabelSetTableWidget::LabelModified( int rowIndex )
{
  QTableWidgetItem* nameItem = this->item(rowIndex,NAME_COL);
  if (!nameItem) return;

  int pixelValue = nameItem->data(Qt::UserRole).toInt();
  int colWidth = (this->columnWidth(NAME_COL) < 180) ? 180 : this->columnWidth(NAME_COL)-2;
  QString text = fontMetrics().elidedText(m_LabelSetImage->GetLabelName(pixelValue).c_str(), Qt::ElideMiddle, colWidth);
//  QString text = fontMetrics().elidedText(m_LabelSetImage->GetLabelName(index).c_str(), Qt::ElideMiddle, this->columnWidth(NAME_COL)-2);
  nameItem->setText(text);

  QPushButton* button;
  button = (QPushButton*) this->cellWidget(rowIndex,LOCKED_COL);
  if (!button) return;

  button->setChecked(!m_LabelSetImage->GetLabelLocked(pixelValue));

  button = (QPushButton*) this->cellWidget(rowIndex,COLOR_COL);
  if (!button) return;

  const mitk::Color& color = m_LabelSetImage->GetLabelColor(pixelValue);
  button->setAutoFillBackground(true);
  QColor qcolor(color[0]*255,color[1]*255,color[2]*255);
  QString styleSheet = "background-color:rgb(";
  styleSheet.append(QString::number(qcolor.red()));
  styleSheet.append(",");
  styleSheet.append(QString::number(qcolor.green()));
  styleSheet.append(",");
  styleSheet.append(QString::number(qcolor.blue()));
  styleSheet.append(")");
  button->setStyleSheet(styleSheet);

  button = (QPushButton*) this->cellWidget(rowIndex,VISIBLE_COL);
  if (!button) return;

  button->setChecked(!m_LabelSetImage->GetLabelVisible(pixelValue));
  //m_LabelStringList.replace(index, QString::fromStdString( m_LabelSetImage->GetLabelName(pixelValue) ) );
/*
  this->clearSelection();
  this->setSelectionMode(QAbstractItemView::SingleSelection);
  this->selectRow(index);
 // this->scrollToItem(nameItem);
  this->setSelectionMode(QAbstractItemView::ExtendedSelection);
*/
}

void QmitkLabelSetTableWidget::SetAutoSelectNewItems( bool value )
{
  m_AutoSelectNewLabels = value;
}

void QmitkLabelSetTableWidget::OnColorButtonClicked()
{
  int row;
  for(int i=0; i<this->rowCount(); i++)
  {
    if (sender() == this->cellWidget(i,COLOR_COL))
    {
      row = i;
    }
  }

  if (row >= 0 && row < this->rowCount())
  {
    const mitk::Color& color = m_LabelSetImage->GetLabelColor(row);
    QColor initial(color.GetRed()*255,color.GetGreen()*255,color.GetBlue()*255);
    QColor qcolor = QColorDialog::getColor(initial,0,QString("Change color"));
    if (!qcolor.isValid())
    return;

    QPushButton* button = (QPushButton*) this->cellWidget(row,COLOR_COL);
    if (!button) return;

    button->setAutoFillBackground(true);

    QString styleSheet = "background-color:rgb(";
    styleSheet.append(QString::number(qcolor.red()));
    styleSheet.append(",");
    styleSheet.append(QString::number(qcolor.green()));
    styleSheet.append(",");
    styleSheet.append(QString::number(qcolor.blue()));
    styleSheet.append(")");
    button->setStyleSheet(styleSheet);

    mitk::Color newColor;
    newColor.SetRed(qcolor.red()/255.0);
    newColor.SetGreen(qcolor.green()/255.0);
    newColor.SetBlue(qcolor.blue()/255.0);

    m_LabelSetImage->SetLabelColor(row,newColor);
  }
}

void QmitkLabelSetTableWidget::OnLockedButtonClicked()
{
  int row;
  for(int i=0; i<this->rowCount(); i++)
  {
    if (sender() == this->cellWidget(i,LOCKED_COL))
    {
      row = i;
    }
  }
  if (row >= 0 && row < this->rowCount())
  {
    m_LabelSetImage->SetLabelLocked(row, !m_LabelSetImage->GetLabelLocked(row) );
  }
}

void QmitkLabelSetTableWidget::OnVisibleButtonClicked()
{
  int row;
  for(int i=0; i<this->rowCount(); i++)
  {
    if (sender() == this->cellWidget(i,VISIBLE_COL))
    {
      row = i;
    }
  }
  if (row >= 0 && row < this->rowCount())
  {
   m_LabelSetImage->SetLabelVisible(row, !m_LabelSetImage->GetLabelVisible(row) );
   mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkLabelSetTableWidget::OnItemClicked(QTableWidgetItem *item)
{
  if (!item) return;
  int row = item->row();
  if (row >= 0 && row < this->rowCount())
  {
    int pixelValue = item->data(Qt::UserRole).toInt();
    m_BlockEvents = true;
    m_LabelSetImage->SetActiveLabel(pixelValue);
    m_ToolManager->WorkingDataModified.Send();
    emit activeLabelChanged(pixelValue);
    m_BlockEvents = false;
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetTableWidget::OnItemDoubleClicked(QTableWidgetItem *item)
{
  if (!item) return;
  int row = item->row();
  if (row >= 0 && row < this->rowCount())
  {
    int pixelValue = this->item(row,0)->data(Qt::UserRole).toInt();
    m_LabelSetImage->SetActiveLabel(pixelValue);
    m_ToolManager->WorkingDataModified.Send();
    emit activeLabelChanged(pixelValue);
    this->WaitCursorOn();
    const mitk::Point3D& pos = m_LabelSetImage->GetLabelCenterOfMassCoordinates(pixelValue, true);
    this->WaitCursorOff();
    if (pos.GetVnlVector().max_value() > 0.0)
        emit goToLabel(pos);
  }
}

void QmitkLabelSetTableWidget::InsertItem(const mitk::Label * label)
{
  //m_LabelStringList.append( QString::fromStdString(label->GetName()) );

  const mitk::Color& color = label->GetColor();

  QString styleSheet = "background-color:rgb(";
  styleSheet.append(QString::number(color[0]*255));
  styleSheet.append(",");
  styleSheet.append(QString::number(color[1]*255));
  styleSheet.append(",");
  styleSheet.append(QString::number(color[2]*255));
  styleSheet.append(")");

  int colWidth = (this->columnWidth(NAME_COL) < 180) ? 180 : this->columnWidth(NAME_COL)-2;
  QString text = fontMetrics().elidedText(label->GetName(), Qt::ElideMiddle, colWidth);
  QTableWidgetItem *nameItem = new QTableWidgetItem(text);
  nameItem->setTextAlignment(Qt::AlignCenter | Qt::AlignLeft);
  nameItem->setData(Qt::UserRole,QVariant(label->GetPixelValue()));

  QPushButton * pbColor = new QPushButton(this);
  pbColor->setFixedSize(24,24);
//  pbColor->setFlat(true);
  pbColor->setCheckable(false);
  pbColor->setAutoFillBackground(true);
  pbColor->setToolTip("Change label color");
  pbColor->setStyleSheet(styleSheet);
//  pbColor->setEnabled(!isBackground);

  connect( pbColor, SIGNAL(clicked()), this, SLOT(OnColorButtonClicked()) );

  QPushButton * pbLocked = new QPushButton(this);
  pbLocked->setFixedSize(24,24);
//  pbLocked->setFlat(true);
  QIcon * iconLocked = new QIcon();
  iconLocked->addFile(QString::fromUtf8(":/Qmitk/lock.png"), QSize(), QIcon::Normal, QIcon::Off);
  iconLocked->addFile(QString::fromUtf8(":/Qmitk/unlock.png"), QSize(), QIcon::Normal, QIcon::On);
  pbLocked->setIcon(*iconLocked);
  pbLocked->setIconSize(QSize(24,24));
  pbLocked->setCheckable(true);
  pbLocked->setToolTip("Lock/unlock label");
  pbLocked->setChecked(!label->GetLocked());
  //pbLocked->setEnabled(!isBackground);
  connect( pbLocked, SIGNAL(clicked()), this, SLOT(OnLockedButtonClicked()) );

  QPushButton * pbVisible = new QPushButton(this);
  pbVisible->setFixedSize(24,24);
//  pbVisible->setFlat(true);
  pbVisible->setAutoRepeat(false);
  QIcon * iconVisible = new QIcon();
  iconVisible->addFile(QString::fromUtf8(":/Qmitk/visible.png"), QSize(), QIcon::Normal, QIcon::Off);
  iconVisible->addFile(QString::fromUtf8(":/Qmitk/invisible.png"), QSize(), QIcon::Normal, QIcon::On);
  pbVisible->setIcon(*iconVisible);
  pbVisible->setIconSize(QSize(24,24));
  pbVisible->setCheckable(true);
  pbVisible->setToolTip("Show/hide label");
//  pbVisible->setEnabled(!isBackground);
  pbVisible->setChecked(!label->GetVisible());

  connect( pbVisible, SIGNAL(clicked()), this, SLOT(OnVisibleButtonClicked()) );

  int row = this->rowCount();
  this->insertRow(row);
  this->setRowHeight(row,24);
//  this->setItem(row, 0, idItem );
  this->setItem(row, 0, nameItem );
  this->setCellWidget(row, 1, pbLocked);
  this->setCellWidget(row, 2, pbColor);
  this->setCellWidget(row, 3, pbVisible);
  this->selectRow(row);

  m_LabelSetImage->SetActiveLabel(label->GetPixelValue());
  m_ToolManager->WorkingDataModified.Send();
  emit activeLabelChanged(row);

  if (row == 0)
  {
    this->hideRow(row); // hide exterior label
    return;
  }
}

void QmitkLabelSetTableWidget::NodeTableViewContextMenuRequested( const QPoint & pos )
{
  QTableWidgetItem *itemAt = this->itemAt(pos);
  if (!itemAt) return;
  int pixelValue = itemAt->data(Qt::UserRole).toInt();
  QMenu* menu = new QMenu(this);

  if (this->selectedItems().size()>1)
  {
    QAction* mergeAction = new QAction(QIcon(":/Qmitk/MergeLabels.png"), "Merge selection on current label", this );
    mergeAction->setEnabled(true);
    QObject::connect( mergeAction, SIGNAL( triggered(bool) ), this, SLOT( OnMergeLabels(bool) ) );
    menu->addAction(mergeAction);

    QAction* removeLabelsAction = new QAction(QIcon(":/Qmitk/RemoveLabel.png"), "Remove selected labels", this );
    removeLabelsAction->setEnabled(true);
    QObject::connect( removeLabelsAction, SIGNAL( triggered(bool) ), this, SLOT( OnRemoveLabels(bool) ) );
    menu->addAction(removeLabelsAction);

    QAction* eraseLabelsAction = new QAction(QIcon(":/Qmitk/EraseLabel.png"), "Erase selected labels", this );
    eraseLabelsAction->setEnabled(true);
    QObject::connect( eraseLabelsAction, SIGNAL( triggered(bool) ), this, SLOT( OnEraseLabels(bool) ) );
    menu->addAction(eraseLabelsAction);

    QAction* combineAndCreateSurfaceAction = new QAction(QIcon(":/Qmitk/CreateSurface.png"), "Combine and create a surface", this );
    combineAndCreateSurfaceAction->setEnabled(true);
    QObject::connect( combineAndCreateSurfaceAction, SIGNAL( triggered(bool) ), this, SLOT( OnCombineAndCreateSurface(bool) ) );
    menu->addAction(combineAndCreateSurfaceAction);

    QAction* createMasksAction = new QAction(QIcon(":/Qmitk/CreateMask.png"), "Create a mask for each selected label", this );
    createMasksAction->setEnabled(true);
    QObject::connect( createMasksAction, SIGNAL( triggered(bool) ), this, SLOT( OnCreateMasks(bool) ) );
    menu->addAction(createMasksAction);

    QAction* combineAndCreateMaskAction = new QAction(QIcon(":/Qmitk/CreateMask.png"), "Combine and create a mask", this );
    combineAndCreateMaskAction->setEnabled(true);
    QObject::connect( combineAndCreateMaskAction, SIGNAL( triggered(bool) ), this, SLOT( OnCombineAndCreateMask(bool) ) );
    menu->addAction(combineAndCreateMaskAction);
  }
  else
  {
/*
    QAction* toggleOutlineAction = new QAction(QIcon(":/QmitkExt/outline.png"), "Outline/Filled all", this );
    toggleOutlineAction->setEnabled(true);
    QObject::connect( toggleOutlineAction, SIGNAL( triggered(bool) ), this, SLOT( OnToggleOutline(bool) ) );
    menu->addAction(toggleOutlineAction);
*/
    QAction* renameAction = new QAction(QIcon(":/Qmitk/RenameLabel.png"), "Rename...", this );
    renameAction->setEnabled(true);
    QObject::connect( renameAction, SIGNAL( triggered(bool) ), this, SLOT( OnRenameLabel(bool) ) );
    menu->addAction(renameAction);

    QAction* removeAction = new QAction(QIcon(":/Qmitk/RemoveLabel.png"), "Remove...", this );
    removeAction->setEnabled(true);
    QObject::connect( removeAction, SIGNAL( triggered(bool) ), this, SLOT( OnRemoveLabel(bool) ) );
    menu->addAction(removeAction);

    QAction* eraseAction = new QAction(QIcon(":/Qmitk/EraseLabel.png"), "Erase...", this );
    eraseAction->setEnabled(true);
    QObject::connect( eraseAction, SIGNAL( triggered(bool) ), this, SLOT( OnEraseLabel(bool) ) );
    menu->addAction(eraseAction);

    QAction* mergeAction = new QAction(QIcon(":/Qmitk/MergeLabels.png"), "Merge...", this );
    mergeAction->setEnabled(true);
    QObject::connect( mergeAction, SIGNAL( triggered(bool) ), this, SLOT( OnMergeLabel(bool) ) );
    menu->addAction(mergeAction);

    QAction* randomColorAction = new QAction(QIcon(":/Qmitk/RandomColor.png"), "Random color", this );
    randomColorAction->setEnabled(true);
    QObject::connect( randomColorAction, SIGNAL( triggered(bool) ), this, SLOT( OnRandomColor(bool) ) );
    menu->addAction(randomColorAction);

    QAction* viewOnlyAction = new QAction(QIcon(":/Qmitk/visible.png"), "View only", this );
    viewOnlyAction->setEnabled(true);
    QObject::connect( viewOnlyAction, SIGNAL( triggered(bool) ), this, SLOT( OnSetOnlyActiveLabelVisible(bool) ) );
    menu->addAction(viewOnlyAction);

    QAction* viewAllAction = new QAction(QIcon(":/Qmitk/visible.png"), "View all", this );
    viewAllAction->setEnabled(true);
    QObject::connect( viewAllAction, SIGNAL( triggered(bool) ), this, SLOT( OnSetAllLabelsVisible(bool) ) );
    menu->addAction(viewAllAction);

    QAction* hideAllAction = new QAction(QIcon(":/Qmitk/invisible.png"), "Hide all", this );
    hideAllAction->setEnabled(true);
    QObject::connect( hideAllAction, SIGNAL( triggered(bool) ), this, SLOT( OnSetAllLabelsInvisible(bool) ) );
    menu->addAction(hideAllAction);

    QAction* lockAllAction = new QAction(QIcon(":/Qmitk/lock.png"), "Lock all", this );
    lockAllAction->setEnabled(true);
    QObject::connect( lockAllAction, SIGNAL( triggered(bool) ), this, SLOT( OnLockAllLabels(bool) ) );
    menu->addAction(lockAllAction);

    QAction* unlockAllAction = new QAction(QIcon(":/Qmitk/unlock.png"), "Unlock all", this );
    unlockAllAction->setEnabled(true);
    QObject::connect( unlockAllAction, SIGNAL( triggered(bool) ), this, SLOT( OnUnlockAllLabels(bool) ) );
    menu->addAction(unlockAllAction);

    QAction* createSurfaceAction = new QAction(QIcon(":/Qmitk/CreateSurface.png"), "Create surface", this );
    createSurfaceAction->setEnabled(true);
    createSurfaceAction->setMenu(new QMenu());

    QAction* tmp1 = createSurfaceAction->menu()->addAction(QString("Detailed"));
    QAction* tmp2 = createSurfaceAction->menu()->addAction(QString("Smoothed"));

    QObject::connect( tmp1, SIGNAL( triggered(bool) ), this, SLOT( OnCreateDetailedSurface(bool) ) );
    QObject::connect( tmp2, SIGNAL( triggered(bool) ), this, SLOT( OnCreateSmoothedSurface(bool) ) );

    menu->addAction(createSurfaceAction);

    QAction* createMaskAction = new QAction(QIcon(":/Qmitk/CreateMask.png"), "Create mask", this );
    createMaskAction->setEnabled(true);
    QObject::connect( createMaskAction, SIGNAL( triggered(bool) ), this, SLOT( OnCreateMask(bool) ) );

    menu->addAction(createMaskAction);

    QAction* createCroppedMaskAction = new QAction(QIcon(":/Qmitk/CreateMask.png"), "Create cropped mask", this );
    createCroppedMaskAction->setEnabled(true);
    QObject::connect( createCroppedMaskAction, SIGNAL( triggered(bool) ), this, SLOT( OnCreateCroppedMask(bool) ) );

    QAction* importAction = new QAction(QIcon(":/Qmitk/RenameLabel.png"), "Import...", this );
    importAction->setEnabled(true);
    QObject::connect( importAction, SIGNAL( triggered(bool) ), this, SLOT( OnImportSegmentationSession(bool) ) );
    menu->addAction(importAction);

    menu->addAction(createCroppedMaskAction);

    m_OpacitySlider = new QSlider;
    m_OpacitySlider->setMinimum(0);
    m_OpacitySlider->setMaximum(100);
    m_OpacitySlider->setOrientation(Qt::Horizontal);
    QObject::connect( m_OpacitySlider, SIGNAL( valueChanged(int) ), this, SLOT( OpacityChanged(int) ) );

    QLabel* _OpacityLabel = new QLabel("Opacity: ");
    QVBoxLayout* _OpacityWidgetLayout = new QVBoxLayout;
    _OpacityWidgetLayout->setContentsMargins(4,4,4,4);
    _OpacityWidgetLayout->addWidget(_OpacityLabel);
    _OpacityWidgetLayout->addWidget(m_OpacitySlider);
    QWidget* _OpacityWidget = new QWidget;
    _OpacityWidget->setLayout(_OpacityWidgetLayout);

    m_OpacityAction = new QWidgetAction(this);
    m_OpacityAction->setDefaultWidget(_OpacityWidget);
  //  QObject::connect( m_OpacityAction, SIGNAL( changed() ), this, SLOT( OpacityActionChanged() ) );
    m_OpacitySlider->setValue(static_cast<int>(m_LabelSetImage->GetLabelOpacity(pixelValue)*100));

    menu->addAction(m_OpacityAction);
  }
  menu->popup(QCursor::pos());
}

void QmitkLabelSetTableWidget::OpacityChanged(int value)
{
  float opacity = static_cast<float>(value)/100.0f;
  m_LabelSetImage->SetLabelOpacity(this->currentRow(), opacity);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetTableWidget::OnRemoveLabel(bool value)
{
  QString question = "Do you really want to remove label \"";
  question.append(QString::fromStdString(m_LabelSetImage->GetLabelName(this->currentRow())));
  question.append("\"?");

  QMessageBox::StandardButton answerButton = QMessageBox::question( this, "Remove label",
     question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    this->WaitCursorOn();
    m_LabelSetImage->RemoveLabel(this->currentRow());
    this->WaitCursorOff();
  }
}

void QmitkLabelSetTableWidget::OnToggleOutline(bool value)
{
  m_AllOutlined = !m_AllOutlined;
  emit toggleOutline(m_AllOutlined);
}

void QmitkLabelSetTableWidget::OnMergeLabel(bool value)
{
  emit mergeLabel( this->currentRow() );
}

void QmitkLabelSetTableWidget::OnMergeLabels(bool value)
{
  QString question = "Do you really want to merge selected labels into \"";
  question.append(QString::fromStdString(m_LabelSetImage->GetLabelName(this->currentRow())));
  question.append("\"?");

  QMessageBox::StandardButton answerButton = QMessageBox::question( this, "Merge selected label",
     question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    QList<QTableWidgetSelectionRange> ranges = this->selectedRanges();
    if ( ranges.isEmpty() )
      return;

    std::vector<int> VectorOfLablePixelValues;
    foreach (QTableWidgetSelectionRange a, ranges)
      for(int i = a.topRow(); i <= a.bottomRow(); i++)
        VectorOfLablePixelValues.push_back(this->item(i,0)->data(Qt::UserRole).toInt());

    this->WaitCursorOn();
    m_LabelSetImage->MergeLabels(VectorOfLablePixelValues,this->currentRow());
    this->WaitCursorOff();

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkLabelSetTableWidget::OnRemoveLabels(bool value)
{
  QString question = "Do you really want to remove selected labels?";
  QMessageBox::StandardButton answerButton = QMessageBox::question( this, "Remove selected labels",
     question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    QList<QTableWidgetSelectionRange> ranges = this->selectedRanges();
    if ( ranges.isEmpty() )
      return;

    std::vector<int> VectorOfLablePixelValues;
    foreach (QTableWidgetSelectionRange a, ranges)
      for(int i = a.topRow(); i <= a.bottomRow(); i++)
        VectorOfLablePixelValues.push_back(this->item(i,0)->data(Qt::UserRole).toInt());

    this->WaitCursorOn();
    m_LabelSetImage->RemoveLabels(VectorOfLablePixelValues);
    this->WaitCursorOff();
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetTableWidget::OnEraseLabels(bool value)
{
  QString question = "Do you really want to erase the selected labels?";

  QMessageBox::StandardButton answerButton = QMessageBox::question( this, "Erase selected labels",
     question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    QList<QTableWidgetSelectionRange> ranges = this->selectedRanges();
    if ( ranges.isEmpty() )
    return;

    std::vector<int> VectorOfLablePixelValues;
    foreach (QTableWidgetSelectionRange a, ranges)
      for(int i = a.topRow(); i <= a.bottomRow(); i++)
        VectorOfLablePixelValues.push_back(this->item(i,0)->data(Qt::UserRole).toInt());

    this->WaitCursorOn();
    m_LabelSetImage->EraseLabels(VectorOfLablePixelValues);
    this->WaitCursorOff();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkLabelSetTableWidget::OnEraseLabel(bool value)
{
  QString question = "Do you really want to erase the contents of label \"";
  question.append(QString::fromStdString(m_LabelSetImage->GetLabelName(this->currentRow())));
  question.append("\"?");

  QMessageBox::StandardButton answerButton = QMessageBox::question( this, "Erase label",
     question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    this->WaitCursorOn();
    m_LabelSetImage->EraseLabel(this->currentRow(), false );
    this->WaitCursorOff();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkLabelSetTableWidget::OnNewLabel(bool value)
{
  emit newLabel();
}

void QmitkLabelSetTableWidget::OnCreateSmoothedSurface(bool value)
{
  emit createSurface( this->currentRow(), true );
}

void QmitkLabelSetTableWidget::OnCreateDetailedSurface(bool value)
{
  emit createSurface( this->currentRow(), false );
}

void QmitkLabelSetTableWidget::OnCombineAndCreateSurface(bool value)
{
  emit combineAndCreateSurface( this->selectedRanges() );
}

void QmitkLabelSetTableWidget::OnCreateMask(bool value)
{
  emit createMask( this->currentRow() );
}

void QmitkLabelSetTableWidget::OnCreateCroppedMask(bool value)
{
  emit createCroppedMask( this->currentRow() );
}

void QmitkLabelSetTableWidget::OnImportSegmentationSession(bool value)
{
  emit importSegmentation();
}

void QmitkLabelSetTableWidget::OnCreateMasks(bool value)
{
//   emit createMasks( this->selectedRanges() );
}

void QmitkLabelSetTableWidget::OnCombineAndCreateMask(bool value)
{
  emit combineAndCreateMask( this->selectedRanges() );
}

void QmitkLabelSetTableWidget::OnLockAllLabels(bool value)
{
  m_LabelSetImage->SetAllLabelsLocked(true);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetTableWidget::OnUnlockAllLabels(bool value)
{
  m_LabelSetImage->SetAllLabelsLocked(false);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetTableWidget::OnSetAllLabelsVisible(bool value)
{
  m_LabelSetImage->SetAllLabelsVisible(true);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetTableWidget::OnSetAllLabelsInvisible(bool value)
{
  m_LabelSetImage->SetAllLabelsVisible(false);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetTableWidget::OnRemoveAllLabels(bool value)
{
  QString question = "Do you really want to remove all labels?";
  QMessageBox::StandardButton answerButton = QMessageBox::question( this, "Remove all labels",
     question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);

  if (answerButton == QMessageBox::Yes)
  {
    m_LabelSetImage->RemoveAllLabels();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkLabelSetTableWidget::OnSetOnlyActiveLabelVisible(bool value)
{
  m_LabelSetImage->SetAllLabelsVisible(false);
  m_LabelSetImage->SetLabelVisible(this->currentRow(), true);
  this->WaitCursorOn();
  const mitk::Point3D& pos = m_LabelSetImage->GetLabelCenterOfMassCoordinates(this->currentRow(), true);
  this->WaitCursorOff();
  if (pos.GetVnlVector().max_value() > 0.0)
    emit goToLabel(pos);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetTableWidget::OnRenameLabel(bool value)
{
  int rowIndex = this->currentRow();
  int pixelValue = this->item(rowIndex,0)->data(Qt::UserRole).toInt();
  emit renameLabel( pixelValue,
                    m_LabelSetImage->GetLabelColor(pixelValue),
                    m_LabelSetImage->GetLabelName(pixelValue) );
}

void QmitkLabelSetTableWidget::OnRandomColor(bool value)
{
  m_LabelSetImage->SetLabelColor( this->currentRow(), m_ColorSequenceRainbow->GetNextColor() );
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkLabelSetTableWidget::Reset()
{
  // remove all rows
  while (this->rowCount())
    this->removeRow( 0 );

  //m_LabelStringList.clear();

  if (m_LabelSetImage.IsNotNull()) // sometimes Reset() is correctly called without a labelset image
  {
    // add all labels
    mitk::LabelSet::LabelContainerConstIteratorType it = m_LabelSetImage->GetLabelSet()->IteratorBegin();
    mitk::LabelSet::LabelContainerConstIteratorType end = m_LabelSetImage->GetLabelSet()->IteratorEnd();
    while (it != end)
    {
      this->InsertItem(it->second);
      it++;
    }
  }
}

QStringList QmitkLabelSetTableWidget::GetLabelStringList()
{
  QStringList list;
  for(int i = 0 ; i < this->rowCount(); i++)
    list.append(this->item(i,0)->text());
  return list;
}

void QmitkLabelSetTableWidget::WaitCursorOn()
{
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
}

void QmitkLabelSetTableWidget::WaitCursorOff()
{
  this->RestoreOverrideCursor();
}

void QmitkLabelSetTableWidget::RestoreOverrideCursor()
{
  QApplication::restoreOverrideCursor();
}