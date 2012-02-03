/* ============================================================================
 * Copyright (c) 2011, Michael A. Jackson (BlueQuartz Software)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of Michael A. Jackson nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "PipelineBuilderWidget.h"


//-- Qt Includes
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QThread>
#include <QtCore/QFileInfoList>
#include <QtGui/QFileDialog>
#include <QtGui/QCloseEvent>
#include <QtGui/QMessageBox>
#include <QtGui/QListWidget>
#include <QtGui/QListWidgetItem>
#include <QtGui/QTreeWidgetItem>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>

#include "DREAM3DLib/DREAM3DLib.h"
#include "DREAM3DLib/Common/DREAM3DSetGetMacros.h"
#include "DREAM3DLib/DREAM3DFilters.h"

#include "PipelineBuilderPlugin.h"
#include "PipelineBuilder/FilterWidgets/QFilterWidget.h"
#include "PipelineBuilder/FilterWidgets/QFilterWidgetManager.h"




// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineBuilderWidget::PipelineBuilderWidget(QWidget *parent) :
DREAM3DPluginFrame(parent),
m_FilterPipeline(NULL),
m_WorkerThread(NULL),
m_SelectedFilterWidget(NULL),
m_FilterWidgetLayout(NULL),
#if defined(Q_WS_WIN)
m_OpenDialogLastDirectory("C:\\")
#else
m_OpenDialogLastDirectory("~/")
#endif
{
  setupUi(this);
  setupGui();
  checkIOFiles();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineBuilderWidget::~PipelineBuilderWidget()
{
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineBuilderWidget::readSettings(QSettings &prefs)
{
  prefs.beginGroup("PipelineBuilder");

  prefs.endGroup();

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineBuilderWidget::writeSettings(QSettings &prefs)
{
  prefs.beginGroup("PipelineBuilder");

  prefs.endGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineBuilderWidget::setWidgetListEnabled(bool b)
{
  foreach (QWidget* w, m_WidgetList) {
    w->setEnabled(b);
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineBuilderWidget::setupGui()
{

  m_FilterWidgetLayout = new QVBoxLayout(scrollAreaWidgetContents);
  m_FilterWidgetLayout->setContentsMargins(0, 0, 4, 4);
  m_FilterWidgetLayout->setSpacing(15);
  m_FilterWidgetLayout->setObjectName(QString::fromUtf8("verticalLayout"));

  // Get the QFilterWidget Mangager Instance
  QFilterWidgetManager::Pointer fm = QFilterWidgetManager::Instance();

  std::set<std::string> groupNames = fm->getGroupNames();
  QMap<QString, QTreeWidgetItem*> groupToItem;

  QTreeWidgetItem* library = new QTreeWidgetItem(filterLibraryTree);
  library->setText(0, "Library");

  for(std::set<std::string>::iterator iter = groupNames.begin(); iter != groupNames.end(); ++iter)
  {
    QTreeWidgetItem* filterGroup = new QTreeWidgetItem(library);
    filterGroup->setText(0, QString::fromStdString(*iter));

    groupToItem.insert(QString::fromStdString(*iter), filterGroup);
  }
  library->setExpanded(true);

  toggleDocs->setChecked(true);
  on_toggleDocs_clicked();

  connect(scrollArea, SIGNAL(filterDropped(QString)),
          this, SLOT(addFilter(QString)));
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineBuilderWidget::on_filterLibraryTree_itemClicked( QTreeWidgetItem* item, int column )
{
  // Clear all the current items from the list
  filterList->clear();

  // Get the QFilterWidget Mangager Instance
  QFilterWidgetManager::Pointer fm = QFilterWidgetManager::Instance();
  QFilterWidgetManager::Collection factories;
  if (item->parent() == NULL)
  {
    factories = fm->getFactories();
  }
  else
  {
    factories = fm->getFactories(item->text(0).toStdString());
  }

  for (QFilterWidgetManager::Collection::iterator factory = factories.begin(); factory != factories.end(); ++factory)
  {
    QListWidgetItem* fitlerItem = new QListWidgetItem(filterList);
    fitlerItem->setText(QString::fromStdString((*factory).first));
  }


}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineBuilderWidget::on_filterList_itemDoubleClicked( QListWidgetItem* item )
{
  addFilter(item->text());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineBuilderWidget::addFilter(QString filterName)
{
  QFilterWidgetManager::Pointer wm = QFilterWidgetManager::Instance();
  IFilterWidgetFactory::Pointer wf = wm->getFactoryForFilter(filterName.toStdString());
  QFilterWidget* w = wf->createWidget();

  scrollAreaWidgetContents->layout()->addWidget(w);
  w->setParent(scrollAreaWidgetContents);
  connect(w, SIGNAL(clicked(bool)),
          this, SLOT(removeFilterWidget()) );
  connect(w, SIGNAL(widgetSelected(QFilterWidget*)),
          this, SLOT(setSelectedFilterWidget(QFilterWidget*)) );

  setSelectedFilterWidget(w);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineBuilderWidget::removeFilterWidget()
{
  QObject* whoSent = sender();
  if (whoSent)
  {
    QWidget* w = qobject_cast<QWidget*>(whoSent);

    scrollAreaWidgetContents->layout()->removeWidget(w);
    w->deleteLater();
  }
  m_SelectedFilterWidget = NULL;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineBuilderWidget::setSelectedFilterWidget(QFilterWidget* w)
{
  if(NULL != m_SelectedFilterWidget && w != m_SelectedFilterWidget)
  {
    m_SelectedFilterWidget->changeStyle(false);
  }
  m_SelectedFilterWidget = w;

  if(NULL != m_SelectedFilterWidget)
  {
    m_SelectedFilterWidget->changeStyle(true);

    qint32 selectedIndex = m_FilterWidgetLayout->indexOf(m_SelectedFilterWidget);
    qint32 count = m_FilterWidgetLayout->count();
    if(count > 1)
    {
      filterUp->setEnabled(true);
      filterDown->setEnabled(true);
    }
    else
    {
      filterUp->setEnabled(false);
      filterDown->setEnabled(false);
    }

    if(selectedIndex == 0)
    {
      filterUp->setEnabled(false);
    }
    else if(selectedIndex == count - 1)
    {
      filterDown->setEnabled(false);
    }

  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineBuilderWidget::on_filterDown_clicked()
{
  std::cout << "on_filterDown_clicked" << std::endl;
  if (NULL != m_SelectedFilterWidget)
  {
    qint32 selectedIndex = m_FilterWidgetLayout->indexOf(m_SelectedFilterWidget);
    qint32 count = m_FilterWidgetLayout->count();
    if (selectedIndex >= 0 && selectedIndex < count - 1)
    {
      m_FilterWidgetLayout->removeWidget(m_SelectedFilterWidget);
      m_FilterWidgetLayout->insertWidget(selectedIndex + 1, m_SelectedFilterWidget);
      setSelectedFilterWidget(m_SelectedFilterWidget);
    }

  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineBuilderWidget::on_filterUp_clicked()
{
  std::cout << "on_filterUp_clicked" << std::endl;
  if(NULL != m_SelectedFilterWidget)
  {
    qint32 selectedIndex = m_FilterWidgetLayout->indexOf(m_SelectedFilterWidget);
    if(selectedIndex > 0)
    {
      //  qint32  count = m_FilterWidgetLayout->count();
      m_FilterWidgetLayout->removeWidget(m_SelectedFilterWidget);
      m_FilterWidgetLayout->insertWidget(selectedIndex - 1, m_SelectedFilterWidget);
      setSelectedFilterWidget(m_SelectedFilterWidget);
    }
  }
}



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineBuilderWidget::on_toggleDocs_clicked()
{
  if (toggleDocs->isChecked())
  {
    helpTextEdit->hide();
  }
  else
  {
    helpTextEdit->show();
  }

}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineBuilderWidget::on_m_LoadSettingsBtn_clicked()
{
  QString file = QFileDialog::getOpenFileName(this, tr("Select Settings File"),
                                                 m_OpenDialogLastDirectory,
                                                 tr("Settings File (*.txt)") );
  if ( true == file.isEmpty() ){  return;  }
  QSettings prefs(file, QSettings::IniFormat, this);
  readSettings(prefs);
  // Do any additional validation necessary

}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineBuilderWidget::on_m_SaveSettingsBtn_clicked()
{
  QString proposedFile = m_OpenDialogLastDirectory + QDir::separator() + "PipelineBuilderSettings.txt";
  QString file = QFileDialog::getSaveFileName(this, tr("Save PipelineBuilder Settings"),
                                              proposedFile,
                                              tr("*.txt") );
  if ( true == file.isEmpty() ){ return;  }

  QSettings prefs(file, QSettings::IniFormat, this);
  writeSettings(prefs);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineBuilderWidget::checkIOFiles()
{
#if 0
  // Use this code as an example of using some macros to make the validation of
  // input/output files easier
  CHECK_QLABEL_OUTPUT_FILE_EXISTS(AIM::SyntheticBuilder, m_, CrystallographicErrorFile)

  CHECK_QCHECKBOX_OUTPUT_FILE_EXISTS(AIM::SyntheticBuilder, m_ , IPFVizFile)
#endif
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineBuilderWidget::on_m_GoBtn_clicked()
{

  if (m_GoBtn->text().compare("Cancel") == 0)
  {
    if(m_FilterPipeline!= NULL)
    {
      //std::cout << "canceling from GUI...." << std::endl;
      emit cancelPipeline();
    }
    return;
  }

#if 0
  if (false == sanityCheckOutputDirectory(m_OutputDir, QString("PipelineBuilder")) )
  {
    return;
  }
  SANITY_CHECK_INPUT(m_, OutputDir)
#endif


  if (m_WorkerThread != NULL)
  {
    m_WorkerThread->wait(); // Wait until the thread is complete
    delete m_WorkerThread; // Kill the thread
    m_WorkerThread = NULL;
  }
  m_WorkerThread = new QThread(); // Create a new Thread Resource

  m_FilterPipeline = new QFilterPipeline(NULL);

  // Move the PipelineBuilder object into the thread that we just created.
  m_FilterPipeline->moveToThread(m_WorkerThread);

  //
  qint32 count = m_FilterWidgetLayout->count();
  for(qint32 i = 0; i < count; ++i)
  {
    QWidget* w = m_FilterWidgetLayout->itemAt(i)->widget();
    QFilterWidget* fw = qobject_cast<QFilterWidget*>(w);
    if (fw)
    {
      m_FilterPipeline->pushBack(fw->getFilter());
    }
  }

  /* Connect the signal 'started()' from the QThread to the 'run' slot of the
   * PipelineBuilder object. Since the PipelineBuilder object has been moved to another
   * thread of execution and the actual QThread lives in *this* thread then the
   * type of connection will be a Queued connection.
   */
  // When the thread starts its event loop, start the PipelineBuilder going
  connect(m_WorkerThread, SIGNAL(started()),
          m_FilterPipeline, SLOT(run()));

  // When the PipelineBuilder ends then tell the QThread to stop its event loop
  connect(m_FilterPipeline, SIGNAL(finished() ),
          m_WorkerThread, SLOT(quit()) );

  // When the QThread finishes, tell this object that it has finished.
  connect(m_WorkerThread, SIGNAL(finished()),
          this, SLOT( pipelineComplete() ) );

  // If the use clicks on the "Cancel" button send a message to the PipelineBuilder object
  // We need a Direct Connection so the
  connect(this, SIGNAL(cancelPipeline() ),
          m_FilterPipeline, SLOT (on_CancelWorker() ) , Qt::DirectConnection);

  // Send Progress from the PipelineBuilder to this object for display
  connect(m_FilterPipeline, SIGNAL (updateProgress(int)),
          this, SLOT(pipelineProgress(int) ) );

  // Send progress messages from PipelineBuilder to this object for display
  connect(m_FilterPipeline, SIGNAL (progressMessage(QString)),
          this, SLOT(addProgressMessage(QString) ));

  // Send progress messages from PipelineBuilder to this object for display
  connect(m_FilterPipeline, SIGNAL (warningMessage(QString)),
          this, SLOT(addWarningMessage(QString) ));

  // Send progress messages from PipelineBuilder to this object for display
  connect(m_FilterPipeline, SIGNAL (errorMessage(QString)),
          this, SLOT(addErrorMessage(QString) ));



  setWidgetListEnabled(false);
  emit pipelineStarted();
  m_WorkerThread->start();
  m_GoBtn->setText("Cancel");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineBuilderWidget::pipelineComplete()
{
 // std::cout << "PipelineBuilderWidget::PipelineBuilder_Finished()" << std::endl;
  m_GoBtn->setText("Go");
  setWidgetListEnabled(true);
  this->m_progressBar->setValue(0);
  emit pipelineEnded();
  checkIOFiles();
  m_FilterPipeline->deleteLater();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineBuilderWidget::pipelineProgress(int val)
{
  this->m_progressBar->setValue( val );
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineBuilderWidget::addErrorMessage(QString message)
{
  QString title = QString::fromStdString(DREAM3D::UIPlugins::PipelineBuilderDisplayName).append(" Error");
  displayDialogBox(title, message, QMessageBox::Critical);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineBuilderWidget::addWarningMessage(QString message)
{
  QString title = QString::fromStdString(DREAM3D::UIPlugins::PipelineBuilderDisplayName).append(" Warning");
  displayDialogBox(title, message, QMessageBox::Warning);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineBuilderWidget::addProgressMessage(QString message)
{
  if (NULL != this->statusBar()) {
    this->statusBar()->showMessage(message);
  }
}
