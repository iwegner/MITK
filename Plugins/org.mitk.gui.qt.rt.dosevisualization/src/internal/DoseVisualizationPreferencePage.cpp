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


#include "DoseVisualizationPreferencePage.h"
#include "mitkRTUIConstants.h"

#include <QWidget>
#include <QMessageBox>
#include <QInputDialog>
#include <QMenu>

#include <berryIPreferencesService.h>
#include <berryPlatform.h>

#include <QmitkDoseColorDelegate.h>
#include <QmitkDoseValueDelegate.h>
#include <QmitkDoseVisualStyleDelegate.h>
#include <QmitkIsoDoseLevelSetModel.h>

#include "mitkIsoLevelsGenerator.h"

DoseVisualizationPreferencePage::DoseVisualizationPreferencePage()
: m_MainControl(0), m_Controls(0)
{

}

DoseVisualizationPreferencePage::~DoseVisualizationPreferencePage()
{
  delete m_LevelSetModel;
  delete m_DoseColorDelegate;
  delete m_DoseValueDelegate;
  delete m_DoseVisualDelegate;
  delete m_Controls;
}

void DoseVisualizationPreferencePage::Init(berry::IWorkbench::Pointer )
{

}

void DoseVisualizationPreferencePage::CreateQtControl(QWidget* parent)
{
  berry::IPreferencesService::Pointer prefService
    = berry::Platform::GetServiceRegistry()
    .GetServiceById<berry::IPreferencesService>(berry::IPreferencesService::ID);

  m_DoseVisNode = prefService->GetSystemPreferences()->Node(mitk::rt::UIConstants::ROOT_DOSE_VIS_PREFERENCE_NODE_ID);

  m_MainControl = new QWidget(parent);
  m_Controls = new Ui::DoseVisualizationPreferencePageControls;
  m_Controls->setupUi( m_MainControl );

  m_LevelSetModel = new QmitkIsoDoseLevelSetModel(this);
  m_DoseColorDelegate = new QmitkDoseColorDelegate(this);
  m_DoseValueDelegate = new QmitkDoseValueDelegate(this);
  m_DoseVisualDelegate = new QmitkDoseVisualStyleDelegate(this);

  this->m_Controls->isoLevelSetView->setModel(m_LevelSetModel);
  this->m_Controls->isoLevelSetView->setItemDelegateForColumn(0,m_DoseColorDelegate);
  this->m_Controls->isoLevelSetView->setItemDelegateForColumn(1,m_DoseValueDelegate);
  this->m_Controls->isoLevelSetView->setItemDelegateForColumn(2,m_DoseVisualDelegate);
  this->m_Controls->isoLevelSetView->setItemDelegateForColumn(3,m_DoseVisualDelegate);
  this->m_Controls->isoLevelSetView->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(m_Controls->spinReferenceDose, SIGNAL(valueChanged(double)), m_LevelSetModel, SLOT(setReferenceDose(double)));
  connect(m_Controls->checkGlobalSync, SIGNAL(toggled(bool)), m_Controls->spinReferenceDose, SLOT(setEnabled(bool)));
  connect(m_Controls->radioAbsDose, SIGNAL(toggled(bool)), m_LevelSetModel, SLOT(setShowAbsoluteDose(bool)));
  connect(m_Controls->isoLevelSetView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnShowContextMenuIsoSet(const QPoint&)));
  connect(m_Controls->listPresets, SIGNAL(currentItemChanged ( QListWidgetItem *, QListWidgetItem *)), this, SLOT(OnCurrentItemChanged ( QListWidgetItem *, QListWidgetItem *)));
  connect(m_Controls->btnAddPreset, SIGNAL(clicked(bool)), this, SLOT(OnAddPresetClicked(bool)));
  connect(m_Controls->btnDelPreset, SIGNAL(clicked(bool)), this, SLOT(OnDelPresetClicked(bool)));
  connect(m_Controls->btnResetPreset, SIGNAL(clicked(bool)), this, SLOT(OnResetPresetClicked(bool)));
  connect(m_Controls->btnDelLevel, SIGNAL(clicked(bool)), this, SLOT(OnDelLevelClicked(bool)));
  connect(m_Controls->btnAddLevel, SIGNAL(clicked(bool)), this, SLOT(OnAddLevelClicked(bool)));

  this->Update();
}

QWidget* DoseVisualizationPreferencePage::GetQtControl() const
{
  return m_MainControl;
}

bool DoseVisualizationPreferencePage::PerformOk()
{
  m_DoseVisNode->PutBool(mitk::rt::UIConstants::DOSE_DISPLAY_ABSOLUTE_ID,m_Controls->radioAbsDose->isChecked());
  m_DoseVisNode->PutBool(mitk::rt::UIConstants::GLOBAL_VISIBILITY_COLORWASH_ID,m_Controls->checkGlobalVisColorWash->isChecked());
  m_DoseVisNode->PutBool(mitk::rt::UIConstants::GLOBAL_VISIBILITY_ISOLINES_ID,m_Controls->checkGlobalVisIsoLine->isChecked());
  m_DoseVisNode->PutDouble(mitk::rt::UIConstants::REFERENCE_DOSE_ID,m_Controls->spinReferenceDose->value());
  m_DoseVisNode->PutBool(mitk::rt::UIConstants::GLOBAL_REFERENCE_DOSE_SYNC_ID, m_Controls->checkGlobalSync->isChecked());

  mitk::rt::StorePresetsMap(this->m_Presets);

  return true;
}

void DoseVisualizationPreferencePage::PerformCancel()
{
}

void DoseVisualizationPreferencePage::Update()
{
  m_Controls->checkGlobalVisColorWash->setChecked(m_DoseVisNode->GetBool(mitk::rt::UIConstants::GLOBAL_VISIBILITY_COLORWASH_ID, true));
  m_Controls->checkGlobalVisIsoLine->setChecked(m_DoseVisNode->GetBool(mitk::rt::UIConstants::GLOBAL_VISIBILITY_ISOLINES_ID, true));
  m_Controls->radioAbsDose->setChecked(m_DoseVisNode->GetBool(mitk::rt::UIConstants::DOSE_DISPLAY_ABSOLUTE_ID, true));
  m_Controls->radioRelDose->setChecked(!(m_DoseVisNode->GetBool(mitk::rt::UIConstants::DOSE_DISPLAY_ABSOLUTE_ID, false)));
  m_Controls->spinReferenceDose->setValue(m_DoseVisNode->GetDouble(mitk::rt::UIConstants::REFERENCE_DOSE_ID, 40.0));
  m_Controls->checkGlobalSync->setChecked(m_DoseVisNode->GetBool(mitk::rt::UIConstants::GLOBAL_REFERENCE_DOSE_SYNC_ID, true));

  berry::IPreferences::Pointer presetsNode = m_DoseVisNode->Node(mitk::rt::UIConstants::ROOT_ISO_PRESETS_PREFERENCE_NODE_ID);
  this->m_Presets = mitk::rt::LoadPresetsMap();
  UpdatePresetsWidgets();
}


mitk::IsoDoseLevelSet* DoseVisualizationPreferencePage::GetSelectedIsoLevelSet()
{
  QListWidgetItem* selectedItem = m_Controls->listPresets->currentItem();

  mitk::IsoDoseLevelSet::Pointer result;

  if (selectedItem)
  {
    result = m_Presets[selectedItem->text().toStdString()];
  }

  return result;
}

void DoseVisualizationPreferencePage::UpdateLevelSetWidgets()
{
  this->m_Controls->btnAddLevel->setEnabled(this->GetSelectedIsoLevelSet()!=NULL);

    QModelIndex selectedIndex = m_Controls->isoLevelSetView->currentIndex();
  this->m_Controls->btnDelLevel->setEnabled(this->GetSelectedIsoLevelSet()!=NULL && selectedIndex.isValid());
}

void DoseVisualizationPreferencePage::UpdatePresetsWidgets()
{
  m_Controls->listPresets->clear();

  QListWidgetItem* selectedItem = NULL;
  for (PresetMapType::iterator pos = m_Presets.begin(); pos != m_Presets.end(); ++pos)
  {
    QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(pos->first));
    if (!selectedItem)
    {
      selectedItem = item;
    }

    m_Controls->listPresets->addItem(item);
  }

  if (selectedItem)
    {
      m_Controls->listPresets->setCurrentItem(selectedItem);
  }

  this->m_LevelSetModel->setIsoDoseLevelSet(this->GetSelectedIsoLevelSet());

   m_Controls->btnDelPreset->setEnabled((m_Controls->listPresets->currentItem() != NULL) && (m_Controls->listPresets->count()>1));
}

void DoseVisualizationPreferencePage::OnCurrentItemChanged ( QListWidgetItem * currentItem, QListWidgetItem * previousItem)
{
  this->m_LevelSetModel->setIsoDoseLevelSet(this->GetSelectedIsoLevelSet());
}

void DoseVisualizationPreferencePage::OnShowContextMenuIsoSet(const QPoint& pos)
{
  QPoint globalPos = m_Controls->isoLevelSetView->viewport()->mapToGlobal(pos);

  QModelIndex selectedIndex = m_Controls->isoLevelSetView->currentIndex();

  QMenu viewMenu;
  QAction* addLevelAct = viewMenu.addAction("Add new level");
  QAction* delLevelAct = viewMenu.addAction("Delete selected level");
  delLevelAct->setEnabled(selectedIndex.isValid());
  viewMenu.addSeparator();
  QAction* invertIsoLineAct = viewMenu.addAction("Invert iso line visibility");
  QAction* activateIsoLineAct = viewMenu.addAction("Activate all iso lines");
  QAction* deactivateIsoLineAct = viewMenu.addAction("Deactivate all iso lines");
  viewMenu.addSeparator();
  QAction* invertColorWashAct = viewMenu.addAction("Invert color wash visibility");
  QAction* activateColorWashAct = viewMenu.addAction("Activate all color wash levels");
  QAction* deactivateColorWashAct = viewMenu.addAction("Deactivate all color wash levels");
  viewMenu.addSeparator();
  QAction* swapAct = viewMenu.addAction("Swap iso line/color wash visibility");

  QAction* selectedItem = viewMenu.exec(globalPos);
  if (selectedItem == invertIsoLineAct)
  {
    this->m_LevelSetModel->invertVisibilityIsoLines();
  }
  else if (selectedItem == activateIsoLineAct)
  {
    this->m_LevelSetModel->switchVisibilityIsoLines(true);
  }
  else if (selectedItem == deactivateIsoLineAct)
  {
    this->m_LevelSetModel->switchVisibilityIsoLines(false);
  }
  else if (selectedItem == invertColorWashAct)
  {
    this->m_LevelSetModel->invertVisibilityColorWash();
  }
  else if (selectedItem == activateColorWashAct)
  {
    this->m_LevelSetModel->switchVisibilityColorWash(true);
  }
  else if (selectedItem == deactivateColorWashAct)
  {
    this->m_LevelSetModel->switchVisibilityColorWash(false);
  }
  else if (selectedItem == swapAct)
  {
    this->m_LevelSetModel->swapVisibility();
  }
  else if (selectedItem == addLevelAct)
  {
    this->m_LevelSetModel->addLevel();
  }
  else if (selectedItem == delLevelAct)
  {
    this->m_LevelSetModel->deleteLevel(selectedIndex);
  }
}

void DoseVisualizationPreferencePage::OnAddPresetClicked(bool checked)
{
  bool done = false;
  QString name = tr("new_preset");
  while (!done)
  {
   bool ok;
     name = QInputDialog::getText(m_MainControl, tr("Define name of new preset."), tr("Preset name:"), QLineEdit::Normal, name, &ok);

   if (!ok)
   {
     return; //cancled by user;
   }

   bool uniqueName = m_Presets.find(name.toStdString()) == m_Presets.end();
   if (!uniqueName)
   {
     QMessageBox box;
     box.setText(tr("New preset name is not unique. Please, choose another one."));
     box.exec();
   }

   bool validName = name.indexOf(tr("/")) ==-1;
   if (!validName)
   {
     QMessageBox box;
     box.setText(tr("New preset name is not valid. Please don't use \"/\"."));
     box.exec();
   }

     done = uniqueName && validName;
  }

  mitk::IsoDoseLevelSet::Pointer newSet = mitk::rt::GeneratIsoLevels_Virtuos();
  m_Presets.insert(std::make_pair(name.toStdString(),newSet));

  UpdatePresetsWidgets();
}

void DoseVisualizationPreferencePage::OnDelPresetClicked(bool checked)
{
  QListWidgetItem* selectedItem = m_Controls->listPresets->currentItem();

  if (selectedItem)
  {
    if (m_Controls->listPresets->count() > 1)
    {
      m_Presets.erase(selectedItem->text().toStdString());
      this->UpdatePresetsWidgets();
    }
  }
}


void DoseVisualizationPreferencePage::OnResetPresetClicked(bool checked)
{
  QMessageBox box;
  box.setText("Do you want to reset the presets?");
  box.setInformativeText("If you reset the presets. All user defined presets will be removed and the default presets will be loaded.");
  box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
  box.setDefaultButton(QMessageBox::No);
  int ret = box.exec();

  if (ret == QMessageBox::Yes)
  {
  mitk::IsoDoseLevelSet::Pointer newSet = mitk::rt::GeneratIsoLevels_Virtuos();
  m_Presets.clear();
  m_Presets.insert(std::make_pair("Virtuos",newSet));

  UpdatePresetsWidgets();
  }
}


void DoseVisualizationPreferencePage::OnAddLevelClicked(bool checked)
{
  this->m_LevelSetModel->addLevel();
}

void DoseVisualizationPreferencePage::OnDelLevelClicked(bool checked)
{
  QModelIndex selectedIndex = m_Controls->isoLevelSetView->currentIndex();
  this->m_LevelSetModel->deleteLevel(selectedIndex);
}
