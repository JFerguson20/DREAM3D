///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2009, Michael A. Jackson. BlueQuartz Software
//  Copyright (c) 2009, Michael Groeber, US Air Force Research Laboratory
//  All rights reserved.
//  BSD License: http://www.opensource.org/licenses/bsd-license.html
//
// This code was partly written under US Air Force Contract FA8650-07-D-5800
//
///////////////////////////////////////////////////////////////////////////////
#include "RepresentationUI.h"

#include <AIM/Common/Constants.h>
#include <AIM/Common/Qt/AIMAboutBox.h>
#include <AIM/Common/Qt/QRecentFileList.h>
#include <AIM/Common/Qt/QR3DFileCompleter.h>
#include <AIM/Threads/AIMThread.h>


//-- Qt Includes
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QString>
#include <QtCore/QSettings>
#include <QtCore/QUrl>
#include <QtCore/QThread>
#include <QtCore/QFileInfoList>
#include <QtGui/QFileDialog>
#include <QtGui/QCloseEvent>
#include <QtGui/QMessageBox>
#include <QtGui/QListWidget>



#define READ_STRING_SETTING(prefs, var, emptyValue)\
  var->setText( prefs.value(#var).toString() );\
  if (var->text().isEmpty() == true) { var->setText(emptyValue); }

#define READ_FILEPATH_SETTING(prefs, var, emptyValue)\
  var->setText( prefs.value(#var).toString() );\
  if (var->text().isEmpty() == true) { var->setText(emptyValue); }

#define READ_SETTING(prefs, var, ok, temp, default, type)\
  ok = false;\
  temp = prefs.value(#var).to##type(&ok);\
  if (false == ok) {temp = default;}\
  var->setValue(temp);

#define READ_BOOL_SETTING(prefs, var, emptyValue)\
  { QString s = prefs.value(#var).toString();\
  if (s.isEmpty() == false) {\
    bool bb = prefs.value(#var).toBool();\
  var->setChecked(bb); } else { var->setChecked(emptyValue); } }

#define WRITE_BOOL_SETTING(prefs, var, b)\
    prefs.setValue(#var, (b) );

#define WRITE_STRING_SETTING(prefs, var)\
  prefs.setValue(#var , this->var->text());

#define WRITE_SETTING(prefs, var)\
  prefs.setValue(#var, this->var->value());


#define READ_COMBO_BOX(prefs, combobox)\
    { bool ok = false;\
    int i = prefs.value(#combobox).toInt(&ok);\
    if (false == ok) {i=0;}\
    combobox->setCurrentIndex(i); }

#define WRITE_COMBO_BOX(prefs, combobox)\
    prefs.setValue(#combobox, this->combobox->currentIndex());

#define SET_TEXT_ICON(name, icon)\
  { \
  QString iconFile = QString(":/") + QString(#icon) + QString("-16x16.png");\
  name->setText(AIM::Representation::name.c_str());\
  name##Icon->setPixmap(QPixmap(iconFile));\
  }


#define CHECK_OUTPUT_FILE_EXISTS(name) \
  { \
  QString absPath = outputDir->text() + QDir::separator() + AIM::Representation::name.c_str();\
  absPath = QDir::toNativeSeparators(absPath);\
  QFileInfo fi ( absPath );\
  if ( fi.exists() )  {\
  SET_TEXT_ICON(name, Check)\
  } else {\
  SET_TEXT_ICON(name, Delete) }  }


#define GG_CHECK_OUTPUT_FILE_EXISTS(name) \
  { \
  QString absPath = gg_OutputDir->text() + QDir::separator() + AIM::Representation::name.c_str();\
  absPath = QDir::toNativeSeparators(absPath);\
  QFileInfo fi ( absPath );\
  if ( fi.exists() )  {\
    QString iconFile = QString(":/") + QString("Check") + QString("-16x16.png");\
      name##_3->setText(AIM::Representation::name.c_str());\
      name##Icon##_3->setPixmap(QPixmap(iconFile));\
  } else {\
    QString iconFile = QString(":/") + QString("Delete") + QString("-16x16.png");\
    name##_3->setText(AIM::Representation::name.c_str());\
    name##Icon##_3->setPixmap(QPixmap(iconFile));\
  }\
}


#define GG_CHECK_INPUT_FILE_EXISTS(name) \
  { \
  QString absPath = gg_InputDir->text() + QDir::separator() + AIM::Representation::name.c_str();\
  absPath = QDir::toNativeSeparators(absPath);\
  QFileInfo fi ( absPath );\
  if ( fi.exists() )  {\
    QString iconFile = QString(":/") + QString("Check") + QString("-16x16.png");\
      name##_2->setText(AIM::Representation::name.c_str());\
      name##Icon##_2->setPixmap(QPixmap(iconFile));\
  } else {\
    QString iconFile = QString(":/") + QString("Delete") + QString("-16x16.png");\
    name##_2->setText(AIM::Representation::name.c_str());\
    name##Icon##_2->setPixmap(QPixmap(iconFile));\
  }\
 }

/*
#define SM_CS(a,b) a##b

#define SM_CHECK_INPUT_FILE_EXISTS(name, Icon) \
  { \
  QString absPath = QDir::toNativeSeparators(name->text());\
  QFileInfo fi ( absPath );\
  QString iconFile;\
  if ( fi.exists() && fi.isFile() )  {\
    iconFile = QString(":/") + QString("Check") + QString("-16x16.png");\
  } else {\
    iconFile = QString(":/") + QString("Delete") + QString("-16x16.png");\
  }\
  Icon->setPixmap(QPixmap(iconFile));\
 }

#define SM_CHECK_OUTPUT_FILE_EXISTS(name, File) \
  { \
  QString absPath = sm_OutputDir->text() + QDir::separator() + AIM::Representation::name.c_str();\
  absPath = QDir::toNativeSeparators(absPath);\
  QFileInfo fi ( absPath );\
  QString iconFile;\
  if ( fi.exists() )  {\
    iconFile = QString(":/") + QString("Check") + QString("-16x16.png");\
  } else {\
    iconFile = QString(":/") + QString("Delete") + QString("-16x16.png");\
  }\
  File->setText(AIM::Representation::name.c_str());\
  File##Icon->setPixmap(QPixmap(iconFile)); \
}
*/

#define VM_CS(a,b) a##b

#define CHECK_QLINEEDIT_FILE_EXISTS(prefix, name) \
  { \
  QString absPath = QDir::toNativeSeparators(prefix##name->text());\
  QFileInfo fi ( absPath );\
  QString iconFile;\
  if ( fi.exists() && fi.isFile() )  {\
    iconFile = QString(":/") + QString("Check") + QString("-16x16.png");\
  } else {\
    iconFile = QString(":/") + QString("Delete") + QString("-16x16.png");\
  }\
  VM_CS(prefix, name)Icon->setPixmap(QPixmap(iconFile));\
 }


#define CHECK_QLABEL_FILE_EXISTS(prefix, name) \
{ \
  QString absPath = prefix##OutputDir->text() + QDir::separator() + AIM::Representation::name.c_str();\
  absPath = QDir::toNativeSeparators(absPath);\
  QFileInfo fi ( absPath );\
  QString iconFile;\
  if ( fi.exists() )  {\
  iconFile = QString(":/") + QString("Check") + QString("-16x16.png");\
  } else {\
  iconFile = QString(":/") + QString("Delete") + QString("-16x16.png");\
  }\
  VM_CS(prefix, name)->setText(AIM::Representation::name.c_str());\
  VM_CS(prefix, name)Icon->setPixmap(QPixmap(iconFile));\
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
RepresentationUI::RepresentationUI(QWidget *parent) :
  QMainWindow(parent),
#if defined(Q_WS_WIN)
m_OpenDialogLastDirectory("C:\\")
#else
m_OpenDialogLastDirectory("~/")
#endif
{
  setupUi(this);
  readSettings();
  setupGui();

   QRecentFileList* recentFileList = QRecentFileList::instance();
   connect(recentFileList, SIGNAL (fileListChanged(const QString &)),
           this, SLOT(updateRecentFileList(const QString &)) );
   // Get out initial Recent File List
   this->updateRecentFileList(QString::null);


   this->setAcceptDrops(true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
RepresentationUI::~RepresentationUI()
{
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::resizeEvent ( QResizeEvent * event )
{
 // std::cout << "RepresentationUI::resizeEvent" << std::endl;
 // std::cout << "   oldSize: " << event->oldSize().width() << " x " << event->oldSize().height() << std::endl;
 // std::cout << "   newSize: " << event->size().width() << " x " << event->size().height() << std::endl;
  emit parentResized();
 // std::cout << "RepresentationUI::resizeEvent --- Done" << std::endl;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_actionExit_triggered()
{
  this->close();
}

// -----------------------------------------------------------------------------
//  Called when the main window is closed.
// -----------------------------------------------------------------------------
void RepresentationUI::closeEvent(QCloseEvent *event)
{
  qint32 err = _checkDirtyDocument();
  if (err < 0)
  {
    event->ignore();
  }
  else
  {
    writeSettings();
    event->accept();
  }
}


// -----------------------------------------------------------------------------
//  Read the prefs from the local storage file
// -----------------------------------------------------------------------------
void RepresentationUI::readSettings()
{
  // std::cout << "Read Settings" << std::endl;
  QSettings prefs;
  QString val;
  bool ok;
  qint32 i;
  double d;

  /* ******** This Section is for the Reconstruction Tab ************ */
  READ_FILEPATH_SETTING(prefs, angDir, "");
  READ_FILEPATH_SETTING(prefs, outputDir, "");
  READ_SETTING(prefs, angMaxSlice, ok, i, 300 , Int);
  READ_STRING_SETTING(prefs, angFilePrefix, "");
  READ_SETTING(prefs, zStartIndex, ok, i, 1 , Int);
  READ_SETTING(prefs, zEndIndex, ok, i, 10 , Int);
  READ_STRING_SETTING(prefs, zSpacing, "0.25");
  READ_BOOL_SETTING(prefs, mergeTwins, true);
  alreadyFormed->blockSignals(true);
  READ_BOOL_SETTING(prefs, alreadyFormed, false);
  alreadyFormed->blockSignals(false);
  READ_SETTING(prefs, minAllowedGrainSize, ok, i, 8 , Int);
  READ_SETTING(prefs, minConfidence, ok, d, 0.1 , Double);
  READ_SETTING(prefs, misOrientationTolerance, ok, d, 5.0 , Double);
  READ_COMBO_BOX(prefs, crystalStructure)


  /* ******** This Section is for the Grain Generator Tab ************ */
  READ_FILEPATH_SETTING(prefs, gg_InputDir, "");
  READ_FILEPATH_SETTING(prefs, gg_OutputDir, "");
  READ_SETTING(prefs, gg_XResolution, ok, d, 0.25 , Double);
  READ_SETTING(prefs, gg_YResolution, ok, d, 0.25 , Double);
  READ_SETTING(prefs, gg_ZResolution, ok, d, 0.25 , Double);
  READ_SETTING(prefs, gg_NumGrains, ok, i, 1000 , Int);

  READ_SETTING(prefs, gg_OverlapAllowed, ok, d, 0.00 , Double);
  READ_COMBO_BOX(prefs, gg_CrystalStructure)
  READ_COMBO_BOX(prefs, gg_ShapeClass)
  READ_COMBO_BOX(prefs, gg_OverlapAssignment)

  /* ******** This Section is for the Surface Meshing Tab ************ */
  READ_FILEPATH_SETTING(prefs, sm_DxFile, "");
  READ_FILEPATH_SETTING(prefs, sm_EdgeTableFile, "");
  READ_FILEPATH_SETTING(prefs, sm_NeighSpinTableFile, "");
  READ_FILEPATH_SETTING(prefs, sm_OutputDir, "");
  READ_SETTING(prefs, sm_XDim, ok, i, 100 , Int);
  READ_SETTING(prefs, sm_YDim, ok, i, 100 , Int);
  READ_SETTING(prefs, sm_ZDim, ok, i, 100 , Int);

  READ_BOOL_SETTING(prefs, sm_SmoothMesh, false);
  READ_BOOL_SETTING(prefs, sm_LockQuadPoints, false);
  READ_SETTING(prefs, sm_SmoothIterations, ok, i, 1 , Int);
  READ_SETTING(prefs, sm_WriteOutputFileIncrement, ok, i, 10 , Int);


}

// -----------------------------------------------------------------------------
//  Write our Prefs to file
// -----------------------------------------------------------------------------
void RepresentationUI::writeSettings()
{
  // std::cout << "writeSettings" << std::endl;
  QSettings prefs;
//  bool ok = false;
//  qint32 i = 0;
  /* ******** This Section is for the Reconstruction Tab ************ */
  WRITE_STRING_SETTING(prefs, angDir)
  WRITE_STRING_SETTING(prefs, outputDir)
  WRITE_STRING_SETTING(prefs, angFilePrefix)
  WRITE_STRING_SETTING(prefs, angMaxSlice)
  WRITE_STRING_SETTING(prefs, zStartIndex)
  WRITE_STRING_SETTING(prefs, zEndIndex)
  WRITE_STRING_SETTING(prefs, zSpacing)

  WRITE_BOOL_SETTING(prefs, mergeTwins, mergeTwins->isChecked())
  WRITE_BOOL_SETTING(prefs, alreadyFormed, alreadyFormed->isChecked())

  WRITE_SETTING(prefs, minAllowedGrainSize)
  WRITE_SETTING(prefs, minConfidence)
  WRITE_SETTING(prefs, misOrientationTolerance)
  WRITE_COMBO_BOX(prefs, crystalStructure)

  /* ******** This Section is for the Grain Generator Tab ************ */
  WRITE_STRING_SETTING(prefs, gg_InputDir)
  WRITE_STRING_SETTING(prefs, gg_OutputDir)
  WRITE_SETTING(prefs, gg_XResolution )
  WRITE_SETTING(prefs, gg_YResolution )
  WRITE_SETTING(prefs, gg_ZResolution )
  WRITE_SETTING(prefs, gg_NumGrains )

  WRITE_SETTING(prefs, gg_OverlapAllowed )
  WRITE_COMBO_BOX(prefs, gg_CrystalStructure)
  WRITE_COMBO_BOX(prefs, gg_ShapeClass)
  WRITE_COMBO_BOX(prefs, gg_OverlapAssignment)


  /* ******** This Section is for the Surface Meshing Tab ************ */
  WRITE_STRING_SETTING(prefs, sm_DxFile);
  WRITE_STRING_SETTING(prefs, sm_EdgeTableFile);
  WRITE_STRING_SETTING(prefs, sm_NeighSpinTableFile);
  WRITE_SETTING(prefs, sm_XDim );
  WRITE_SETTING(prefs, sm_YDim );
  WRITE_SETTING(prefs, sm_ZDim );
  WRITE_STRING_SETTING(prefs, sm_OutputDir);
  WRITE_BOOL_SETTING(prefs, sm_SmoothMesh, sm_SmoothMesh->isChecked() );
  WRITE_BOOL_SETTING(prefs, sm_LockQuadPoints, sm_LockQuadPoints->isChecked() );
  WRITE_SETTING(prefs, sm_SmoothIterations );
  WRITE_SETTING(prefs, sm_WriteOutputFileIncrement );
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::setupGui()
{
  // Setup the Reconstruction Tab GUI
  rec_SetupGui();
  rec_CheckIOFiles();

  // Setup the Grain Generator Tab Gui
  gg_SetupGui();
  gg_CheckIOFiles();

  // Setup the SurfaceMeshing Tab Gui
  sm_SetupGui();
  sm_CheckIOFiles();

  // Setup the Volume Meshing Tab Gui
  vm_SetupGui();
  vm_CheckIOFiles();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::setWidgetListEnabled(bool b)
{
  foreach (QWidget* w, m_WidgetList) {
    w->setEnabled(b);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::dragEnterEvent(QDragEnterEvent* e)
{
  const QMimeData* dat = e->mimeData();
  QList<QUrl> urls = dat->urls();
  QString file = urls.count() ? urls[0].toLocalFile() : QString();
  QDir parent(file);
  this->m_OpenDialogLastDirectory = parent.dirName();
  QFileInfo fi(file );
  QString ext = fi.suffix();
  if (fi.exists() && fi.isFile() && ( ext.compare("mxa") || ext.compare("h5") || ext.compare("hdf5") ) )
  {
    e->accept();
  }
  else
  {
    e->ignore();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::dropEvent(QDropEvent* e)
{
  const QMimeData* dat = e->mimeData();
  QList<QUrl> urls = dat->urls();
  QString file = urls.count() ? urls[0].toLocalFile() : QString();
  QDir parent(file);
  this->m_OpenDialogLastDirectory = parent.dirName();
  QFileInfo fi(file );
  QString ext = fi.suffix();
  file = QDir::toNativeSeparators(file);
  if (fi.exists() && fi.isFile() && ( ext.compare("tif") || ext.compare("tiff")  ) )
  {
    //TODO: INSERT Drop Event CODE HERE
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool RepresentationUI::_verifyOutputPathParentExists(QString outFilePath, QLineEdit* lineEdit)
{
  QFileInfo fileinfo(outFilePath);
  QDir parent (fileinfo.dir() );
//  if (false == parent.exists() )
//  {
//    lineEdit->setStyleSheet("border: 1px solid red;");
//  }
//  else
//  {
//    lineEdit->setStyleSheet("");
//  }
  return parent.exists();
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool RepresentationUI::_verifyPathExists(QString outFilePath, QLineEdit* lineEdit)
{
  QFileInfo fileinfo(outFilePath);
  if (false == fileinfo.exists() )
  {
    lineEdit->setStyleSheet("border: 1px solid red;");
  }
  else
  {
    lineEdit->setStyleSheet("");
  }
  return fileinfo.exists();
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
qint32 RepresentationUI::_checkDirtyDocument()
{
  qint32 err = -1;

  if (this->isWindowModified() == true)
  {
    int r = QMessageBox::warning(this, tr("AIM Mount Maker"),
                            tr("The Image has been modified.\nDo you want to save your changes?"),
                            QMessageBox::Save | QMessageBox::Default,
                            QMessageBox::Discard,
                            QMessageBox::Cancel | QMessageBox::Escape);
    if (r == QMessageBox::Save)
    {
      //TODO: Save the current document or otherwise save the state.
    }
    else if (r == QMessageBox::Discard)
    {
      err = 1;
    }
    else if (r == QMessageBox::Cancel)
    {
      err = -1;
    }
  }
  else
  {
    err = 1;
  }

  return err;
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_actionClose_triggered() {
 // std::cout << "RepresentationUI::on_actionClose_triggered" << std::endl;
  qint32 err = -1;
  err = _checkDirtyDocument();
  if (err >= 0)
  {
    // Close the window. Files have been saved if needed
    if (QApplication::activeWindow() == this)
    {
      this->close();
    }
    else
    {
      QApplication::activeWindow()->close();
    }
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::updateRecentFileList(const QString &file)
{
 // std::cout << "RepresentationUI::updateRecentFileList" << std::endl;

  // Clear the Recent Items Menu
  this->menu_RecentFiles->clear();

  // Get the list from the static object
  QStringList files = QRecentFileList::instance()->fileList();
  foreach (QString file, files)
    {
      QAction* action = new QAction(this->menu_RecentFiles);
      action->setText(QRecentFileList::instance()->parentAndFileName(file));
      action->setData(file);
      action->setVisible(true);
      this->menu_RecentFiles->addAction(action);
      connect(action, SIGNAL(triggered()), this, SLOT(openRecentFile()));
    }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::openRecentFile()
{
  //std::cout << "QRecentFileList::openRecentFile()" << std::endl;

  QAction *action = qobject_cast<QAction *>(sender());
  if (action)
  {
    //std::cout << "Opening Recent file: " << action->data().toString().toStdString() << std::endl;
    QString file = action->data().toString();
    //TODO: use the 'file' object to figure out what to open
  }

}

//TODO: Reconstruction Methods
/* *****************************************************************************
 *
 * Reconstruction Methods
 *
 ***************************************************************************** */


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::rec_SetupGui()
{
  QR3DFileCompleter* com = new QR3DFileCompleter(this, true);
  angDir->setCompleter(com);
  QObject::connect( com, SIGNAL(activated(const QString &)),
           this, SLOT(on_angDir_textChanged(const QString &)));

  QR3DFileCompleter* com2 = new QR3DFileCompleter(this, true);
  outputDir->setCompleter(com2);
  QObject::connect( com2, SIGNAL(activated(const QString &)),
           this, SLOT(on_outputDir_textChanged(const QString &)));


  QString msg ("All files will be over written that appear in the output directory.");

  QFileInfo fi (outputDir->text() + QDir::separator() +  AIM::Representation::ReconstructedDataFile.c_str() );
  if (alreadyFormed->isChecked() == true && fi.exists() == false)
  {
    alreadyFormed->setChecked(false);
  }

  if (alreadyFormed->isChecked())
  {
    msg += QString("\nThe 'reconstructed_data.txt' file will be used as an import and NOT over written with new data");
  }
  messageLabel->setText(msg);


  m_WidgetList << angDir << angDirBtn << outputDir << outputDirBtn;
  m_WidgetList << angFilePrefix << angMaxSlice << zStartIndex << zEndIndex << zSpacing;
  m_WidgetList << mergeTwins << alreadyFormed << minAllowedGrainSize << minConfidence << misOrientationTolerance;
  m_WidgetList << crystalStructure;
}

// -----------------------------------------------------------------------------
//
// ----------------------------------------------------------------------------_
void RepresentationUI::rec_CheckIOFiles()
{
  if (true == this->_verifyPathExists(angDir->text(), this->angDir) )
   {
     findAngMaxSliceAndPrefix();
   }

   if (this->_verifyPathExists(outputDir->text(), this->outputDir) )
   {
     findReconstructionOutputFiles();
   }
   else
   {
     SET_TEXT_ICON(StatsFile, Delete)
     SET_TEXT_ICON(VolBinFile, Delete)
     SET_TEXT_ICON(BOverABinsFile, Delete)
     SET_TEXT_ICON(COverABinsFile, Delete)
     SET_TEXT_ICON(COverBBinsFile, Delete)
     SET_TEXT_ICON(SVNFile, Delete)
     SET_TEXT_ICON(SVSFile, Delete)
     SET_TEXT_ICON(MisorientationBinsFile, Delete)
     SET_TEXT_ICON(MicroBinsFile, Delete)
     SET_TEXT_ICON(ReconstructedDataFile, Delete)
     SET_TEXT_ICON(ReconstructedVisualizationFile, Delete)
     SET_TEXT_ICON(GrainsFile, Delete)
     SET_TEXT_ICON(BoundaryCentersFile, Delete)
     SET_TEXT_ICON(AxisOrientationsFile, Delete)
     SET_TEXT_ICON(EulerAnglesFile, Delete)
   }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_angDirBtn_clicked()
{
  std::cout << "on_angDirBtn_clicked" << std::endl;
  QString outputFile = this->m_OpenDialogLastDirectory + QDir::separator();
  outputFile = QFileDialog::getExistingDirectory(this, tr("Select Ang Directory"), outputFile);
  if (!outputFile.isNull())
  {
    this->angDir->setText(outputFile);
    findAngMaxSliceAndPrefix();
    _verifyPathExists(outputFile, angDir);
  }

}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::findReconstructionOutputFiles()
{
  CHECK_OUTPUT_FILE_EXISTS(StatsFile)
  CHECK_OUTPUT_FILE_EXISTS(VolBinFile)
  CHECK_OUTPUT_FILE_EXISTS(BOverABinsFile)
  CHECK_OUTPUT_FILE_EXISTS(COverABinsFile)
  CHECK_OUTPUT_FILE_EXISTS(COverBBinsFile)
  CHECK_OUTPUT_FILE_EXISTS(SVNFile)
  CHECK_OUTPUT_FILE_EXISTS(SVSFile)
  CHECK_OUTPUT_FILE_EXISTS(MisorientationBinsFile)
  CHECK_OUTPUT_FILE_EXISTS(MicroBinsFile)
  CHECK_OUTPUT_FILE_EXISTS(ReconstructedDataFile)
  CHECK_OUTPUT_FILE_EXISTS(ReconstructedVisualizationFile)
  CHECK_OUTPUT_FILE_EXISTS(GrainsFile)
  CHECK_OUTPUT_FILE_EXISTS(BoundaryCentersFile)
  CHECK_OUTPUT_FILE_EXISTS(AxisOrientationsFile)
  CHECK_OUTPUT_FILE_EXISTS(EulerAnglesFile)
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_alreadyFormed_stateChanged(int currentState)
{
  QString absPath = outputDir->text() + QDir::separator() + AIM::Representation::ReconstructedDataFile.c_str();
  absPath = QDir::toNativeSeparators(absPath);
  QFileInfo fi (absPath);
  QString msg ("All files will be over written that appear in the output directory.");
  if (alreadyFormed->isChecked() == true && fi.exists() == false)
  {
    QMessageBox::critical(this, tr("AIM Representation"),
      tr("You have selected the 'Already Formed' check box \nbut the correct output file does not exist.\n"
      "The checkbox will revert to an unchecked state.?"),
      QMessageBox::Ok,
      QMessageBox::Ok);
      alreadyFormed->setChecked(false);
      CHECK_OUTPUT_FILE_EXISTS(ReconstructedDataFile)
  }

  if (alreadyFormed->isChecked())
  {
    msg += QString("\nThe 'reconstructed_data.txt' file will be used as an import and NOT over written with new data");
  }
  messageLabel->setText(msg);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::findAngMaxSliceAndPrefix()
{
  if (angDir->text().length() == 0) { return; }
  QDir dir(angDir->text());
  QStringList filters;
  filters << "*.ang";
  dir.setNameFilters(filters);
  QFileInfoList angList = dir.entryInfoList();
  int maxSlice = 0;
  int currValue =0;
  bool ok;
  QString fPrefix;
  QRegExp rx("(\\d+)");
  QStringList list;
  int pos = 0;
  foreach(QFileInfo fi, angList)
  {
    if (fi.suffix().compare(".ang") && fi.isFile() == true)
    {
      pos = 0;
      list.clear();
      QString fn = fi.baseName();
      while ((pos = rx.indexIn(fn, pos)) != -1)
      {
        list << rx.cap(0);
        fPrefix = fn.left(pos);
        pos += rx.matchedLength();
      }
      if (list.size() > 0) {
        currValue = list.front().toInt(&ok);
        if (currValue > maxSlice) { maxSlice = currValue; }
      }
    }
  }
  this->angMaxSlice->setValue(maxSlice);
  this->angFilePrefix->setText(fPrefix);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_outputDirBtn_clicked()
{
  QString outputFile = this->m_OpenDialogLastDirectory + QDir::separator();
  outputFile = QFileDialog::getExistingDirectory(this, tr("Select Output Directory"), outputFile);
  if (!outputFile.isNull())
  {
    this->outputDir->setText(outputFile);
    if (_verifyPathExists(outputFile, outputDir) == true )
    {
      findReconstructionOutputFiles();
      QFileInfo fi (outputDir->text() + QDir::separator() +  AIM::Representation::ReconstructedDataFile.c_str() );
      if (alreadyFormed->isChecked() == true && fi.exists() == false)
      {
        alreadyFormed->setChecked(false);
      }
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_outputDir_textChanged(const QString & text)
{
  _verifyPathExists(outputDir->text(), outputDir);
  rec_CheckIOFiles();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_angDir_textChanged(const QString & text)
{
  _verifyPathExists(angDir->text(), angDir);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_reconstructBtn_clicked()
{
  bool ok = false;
  if (reconstructBtn->text().compare("Cancel") == 0)
  {
    if(m_Reconstruction.get() != NULL)
    {
      //std::cout << "canceling from GUI...." << std::endl;
      emit sig_CancelWorker();
    }
    return;
  }

  //TODO: Add sanity checks for input files
  //TODO: Add warning for existing output files that will be over written

  m_Reconstruction = Reconstruction::New();
  m_Reconstruction->setInputDirectory(angDir->text().toStdString() );
  m_Reconstruction->setOutputDirectory(outputDir->text().toStdString());
  m_Reconstruction->setAngFilePrefix(angFilePrefix->text().toStdString());
  m_Reconstruction->setAngSeriesMaxSlice(angMaxSlice->value());
  m_Reconstruction->setZStartIndex(zStartIndex->value());
  m_Reconstruction->setZEndIndex(zEndIndex->value() + 1);
  m_Reconstruction->setZResolution(zSpacing->text().toDouble(&ok));
  m_Reconstruction->setMergeTwins(mergeTwins->isChecked() );
  m_Reconstruction->setMinAllowedGrainSize(minAllowedGrainSize->value());
  m_Reconstruction->setMinSeedConfidence(minConfidence->value());
  m_Reconstruction->setMisorientationTolerance(misOrientationTolerance->value());

  AIM::Representation::CrystalStructure crystruct = static_cast<AIM::Representation::CrystalStructure>(crystalStructure->currentIndex());
  if (crystruct == 0) { crystruct = AIM::Representation::Cubic; }
  else if (crystruct == 1) { crystruct = AIM::Representation::Hexagonal; }

  m_Reconstruction->setCrystalStructure(crystruct);
  m_Reconstruction->setAlreadyFormed(alreadyFormed->isChecked());

  connect(m_Reconstruction.get(), SIGNAL(finished()),
          this, SLOT( rec_ThreadFinished() ) );
  connect(m_Reconstruction.get(), SIGNAL (updateProgress(int)),
    this, SLOT(rec_ThreadProgressed(int) ) );
  connect(m_Reconstruction.get(), SIGNAL (updateMessage(QString)),
          this, SLOT(threadHasMessage(QString) ) );

  setWidgetListEnabled(false);
  m_Reconstruction->start();
  reconstructBtn->setText("Cancel");
  grainGeneratorTab->setEnabled(false);
  surfaceMeshingTab->setEnabled(false);
  volumeMeshingTab->setEnabled(false);

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::threadHasMessage(QString message)
{
 // std::cout << "RepresentationUI::threadHasMessage()" << message.toStdString() << std::endl;
  this->statusBar()->showMessage(message);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::rec_ThreadFinished()
{
  std::cout << "RepresentationUI::reconstruction_Finished()" << std::endl;
  reconstructBtn->setText("Go");
  setWidgetListEnabled(true);
  this->progressBar->setValue(0);
  grainGeneratorTab->setEnabled(true);
  surfaceMeshingTab->setEnabled(true);
  volumeMeshingTab->setEnabled(true);
  rec_CheckIOFiles();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::rec_ThreadProgressed(int val)
{
  this->progressBar->setValue( val );
}


//TODO: Grain Generator Methods
/* *****************************************************************************
 *
 * Grain Generator Methods
 *
 ***************************************************************************** */
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::gg_SetupGui()
{
  m_WidgetList << gg_InputDir << gg_InputDirBtn << gg_OutputDir << gg_OutputDirBtn;
  m_WidgetList << gg_CrystalStructure << gg_NumGrains << gg_XResolution << gg_YResolution << gg_ZResolution;
  m_WidgetList << gg_OverlapAllowed << gg_OverlapAssignment << gg_ShapeClass;

  if (NULL == gg_InputDir->completer()) {
    QR3DFileCompleter* com = new QR3DFileCompleter(this, true);
    gg_InputDir->setCompleter(com);
    QObject::connect( com, SIGNAL(activated(const QString &)),
             this, SLOT(on_gg_InputDir_textChanged(const QString &)));
  }

  if (NULL == gg_OutputDir->completer()) {
    QR3DFileCompleter* com2 = new QR3DFileCompleter(this, true);
    gg_OutputDir->setCompleter(com2);
    QObject::connect( com2, SIGNAL(activated(const QString &)),
             this, SLOT(on_gg_OutputDir_textChanged(const QString &)));
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::gg_CheckIOFiles()
{
  GG_CHECK_INPUT_FILE_EXISTS(VolBinFile)
  GG_CHECK_INPUT_FILE_EXISTS(BOverABinsFile)
  GG_CHECK_INPUT_FILE_EXISTS(COverABinsFile)
  GG_CHECK_INPUT_FILE_EXISTS(COverBBinsFile)
  GG_CHECK_INPUT_FILE_EXISTS(SeNBinsFile)
  GG_CHECK_INPUT_FILE_EXISTS(AxisOrientationsFile)
  GG_CHECK_INPUT_FILE_EXISTS(EulerAnglesFile)
  GG_CHECK_INPUT_FILE_EXISTS(SVNFile)
  GG_CHECK_INPUT_FILE_EXISTS(SVSFile)
  GG_CHECK_INPUT_FILE_EXISTS(MisorientationBinsFile)
  GG_CHECK_INPUT_FILE_EXISTS(MicroBinsFile)

  GG_CHECK_OUTPUT_FILE_EXISTS(CubeFile)
  GG_CHECK_OUTPUT_FILE_EXISTS(AnalysisFile)
  GG_CHECK_OUTPUT_FILE_EXISTS(BoundaryCentersFile)
  GG_CHECK_OUTPUT_FILE_EXISTS(GrainsFile)
  GG_CHECK_OUTPUT_FILE_EXISTS(VolumeFile)
}



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_gg_InputDirBtn_clicked()
{
  std::cout << "on_gg_InputDirBtn_clicked" << std::endl;
  QString outputFile = this->m_OpenDialogLastDirectory + QDir::separator();
  outputFile = QFileDialog::getExistingDirectory(this, tr("Select Input Directory"), outputFile);
  if (!outputFile.isNull())
  {
    this->gg_InputDir->setText(outputFile);
    if (_verifyPathExists(outputFile, gg_InputDir) == true)
    {
      gg_CheckIOFiles(); // Rescan for files in the input and output directory
    }
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_gg_OutputDirBtn_clicked()
{
  QString outputFile = this->m_OpenDialogLastDirectory + QDir::separator();
  outputFile = QFileDialog::getExistingDirectory(this, tr("Select Grain Generator Output Directory"), outputFile);
  if (!outputFile.isNull())
  {
    this->gg_OutputDir->setText(outputFile);
    if (_verifyPathExists(outputFile, gg_OutputDir) == true )
    {
      gg_CheckIOFiles();
    }
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_gg_InputDir_textChanged(const QString & text)
{
  if (_verifyPathExists(gg_InputDir->text(), gg_InputDir) )
  {
    gg_CheckIOFiles();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_gg_OutputDir_textChanged(const QString & text)
{
  if (_verifyPathExists(gg_OutputDir->text(), gg_OutputDir) )
  {
    gg_CheckIOFiles();
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_gg_GoBtn_clicked()
{

  if (gg_GoBtn->text().compare("Cancel") == 0)
  {
    if(m_GrainGenerator.get() != NULL)
    {
      //std::cout << "canceling from GUI...." << std::endl;
      emit sig_CancelWorker();
    }
    return;
  }

  m_GrainGenerator = GrainGenerator::New(NULL);
  m_GrainGenerator->setInputDirectory(gg_InputDir->text().toStdString() );
  m_GrainGenerator->setOutputDirectory(gg_OutputDir->text().toStdString());
  m_GrainGenerator->setNumGrains(gg_NumGrains->value());

  int shapeclass = gg_ShapeClass->currentIndex() + 1;
  m_GrainGenerator->setShapeClass(shapeclass);

  m_GrainGenerator->setXResolution(gg_XResolution->value());
  m_GrainGenerator->setYResolution(gg_YResolution->value());
  m_GrainGenerator->setZResolution(gg_ZResolution->value());


  m_GrainGenerator->setOverlapAllowed(gg_OverlapAllowed->value());
  int overlapassignment = gg_OverlapAssignment->currentIndex() + 1;
  m_GrainGenerator->setOverlapAssignment(overlapassignment);

  int crystruct =  gg_CrystalStructure->currentIndex() + 1;
  m_GrainGenerator->setCrystalStructure(crystruct);


  connect(m_GrainGenerator.get(), SIGNAL(finished()),
          this, SLOT( gg_ThreadFinished() ) );
  connect(m_GrainGenerator.get(), SIGNAL (updateProgress(int)),
    this, SLOT(gg_ThreadProgressed(int) ) , Qt::DirectConnection);
  connect(m_GrainGenerator.get(), SIGNAL (updateMessage(QString)),
          this, SLOT(threadHasMessage(QString) ) );

  setWidgetListEnabled(false);
  reconstructionTab->setEnabled(false);
  surfaceMeshingTab->setEnabled(false);
  volumeMeshingTab->setEnabled(false);
  m_GrainGenerator->start();
  gg_GoBtn->setText("Cancel");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::gg_ThreadFinished()
{
  std::cout << "RepresentationUI::grainGenerator_Finished()" << std::endl;
  gg_GoBtn->setText("Go");
  setWidgetListEnabled(true);
  this->gg_progressBar->setValue(0);
  reconstructionTab->setEnabled(true);
  surfaceMeshingTab->setEnabled(true);
  volumeMeshingTab->setEnabled(true);
  gg_CheckIOFiles();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::gg_ThreadProgressed(int val)
{
  this->gg_progressBar->setValue( val );
}

//TODO: Surface Meshing Methods
/* *****************************************************************************
 *
 * Surface Meshing Methods
 *
 ***************************************************************************** */

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::sm_SetupGui()
{

  if (NULL == sm_DxFile->completer()){
    QR3DFileCompleter* com = new QR3DFileCompleter(this, false);
    sm_DxFile->setCompleter(com);
    QObject::connect( com, SIGNAL(activated(const QString &)),
             this, SLOT(on_sm_DxFile_textChanged(const QString &)));
  }

  if (NULL == sm_EdgeTableFile->completer()) {
    QR3DFileCompleter* com2 = new QR3DFileCompleter(this, false);
    sm_EdgeTableFile->setCompleter(com2);
    QObject::connect( com2, SIGNAL(activated(const QString &)),
             this, SLOT(on_sm_EdgeTableFile_textChanged(const QString &)));
  }

  if (NULL == sm_NeighSpinTableFile->completer()) {
    QR3DFileCompleter* com3 = new QR3DFileCompleter(this, false);
    sm_NeighSpinTableFile->setCompleter(com3);
    QObject::connect( com3, SIGNAL(activated(const QString &)),
             this, SLOT(on_sm_NeighSpinTableFile_textChanged(const QString &)));
  }

  if (NULL == sm_OutputDir->completer()) {
    QR3DFileCompleter* com4 = new QR3DFileCompleter(this, true);
    sm_OutputDir->setCompleter(com4);
    QObject::connect( com4, SIGNAL(activated(const QString &)),
             this, SLOT(on_sm_OutputDir_textChanged(const QString &)));
  }

  sm_Message->setText("");
  m_WidgetList << sm_DxFile << sm_EdgeTableFile << sm_NeighSpinTableFile;
  m_WidgetList << sm_XDim << sm_YDim << sm_ZDim << sm_DxFileBtn << sm_OutputDir << sm_OutputDirBtn;
  m_WidgetList << sm_Message << sm_LockQuadPoints << sm_SmoothIterations << sm_SmoothMesh;
  m_WidgetList << sm_WriteOutputFileIncrement;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::sm_CheckIOFiles()
{

  if ( _verifyPathExists(sm_DxFile->text(), sm_DxFile) == true )
  {
    QFileInfo fi (sm_DxFile->text() );
    QString ext = fi.suffix();
    if (ext.compare(AIM::Representation::VTKExt.c_str() ) == 0)
    {
      sm_Message->setText("You have selected a VTK file which will need to be converted first to a 'dx' based file. This will be done for you.");
    }
    else
    {
      sm_Message->setText("You have selected a 'dx' file which can be used directly by the surface meshing code.");
    }
  }

  _verifyPathExists(sm_EdgeTableFile->text(), sm_EdgeTableFile);
  _verifyPathExists(sm_NeighSpinTableFile->text(), sm_NeighSpinTableFile);

  CHECK_QLINEEDIT_FILE_EXISTS(sm_, DxFile);
  CHECK_QLINEEDIT_FILE_EXISTS(sm_, EdgeTableFile);
  CHECK_QLINEEDIT_FILE_EXISTS(sm_, NeighSpinTableFile);

  _verifyPathExists(sm_OutputDir->text(), sm_OutputDir);
  CHECK_QLABEL_FILE_EXISTS(sm_, NodesFile);
  CHECK_QLABEL_FILE_EXISTS(sm_, TrianglesFile);
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_sm_DxFileBtn_clicked()
{
  QString file = QFileDialog::getOpenFileName(this, tr("Select Input File"),
                                                 m_OpenDialogLastDirectory,
                                                 tr("Viz Files (*.dx *.vtk)") );
  if ( true == file.isEmpty() ){return;  }
  QFileInfo fi (file);
  QString ext = fi.suffix();
  if (ext.compare(AIM::Representation::VTKExt.c_str() ) == 0)
  {
    sm_Message->setText("You have selected a VTK file which will need to be converted first to a 'dx' based file. This will be done for you.");
  }
  else
  {
    sm_Message->setText("You have selected a 'dx' file which can be used directly by the surface meshing code.");
  }
  sm_DxFile->setText(fi.absoluteFilePath());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_sm_EdgeTableFileBtn_clicked()
{
  QString file = QFileDialog::getOpenFileName(this, tr("Select EdgeTable File"),
                                                 m_OpenDialogLastDirectory,
                                                 tr("Text Files (*.txt)") );
  if ( true == file.isEmpty() ){return;  }
  sm_EdgeTableFile->setText(file);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_sm_NeighSpinTableFileBtn_clicked()
{
  QString file = QFileDialog::getOpenFileName(this, tr("Select NeighSpintTable File"),
                                                 m_OpenDialogLastDirectory,
                                                 tr("Text Files (*.txt)") );
  if ( true == file.isEmpty() ){return;  }
  sm_NeighSpinTableFile->setText(file);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_sm_OutputDirBtn_clicked()
{
  QString outputFile = this->m_OpenDialogLastDirectory + QDir::separator();
  outputFile = QFileDialog::getExistingDirectory(this, tr("Select Surface Meshing Output Directory"), outputFile);
  if (!outputFile.isNull())
  {
    this->sm_OutputDir->setText(outputFile);
    if (_verifyPathExists(outputFile, sm_OutputDir) == true )
    {
      sm_CheckIOFiles();
      sm_OutputDir->setText(outputFile);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_sm_DxFile_textChanged(const QString & text)
{
  sm_CheckIOFiles();
  _verifyPathExists(sm_DxFile->text(), sm_DxFile);
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_sm_EdgeTableFile_textChanged(const QString & text)
{
  sm_CheckIOFiles();
  _verifyPathExists(sm_EdgeTableFile->text(), sm_EdgeTableFile);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_sm_NeighSpinTableFile_textChanged(const QString & text)
{
  sm_CheckIOFiles();
  _verifyPathExists(sm_EdgeTableFile->text(), sm_EdgeTableFile);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_sm_OutputDir_textChanged(const QString & text)
{
  if (_verifyPathExists(sm_OutputDir->text(), sm_OutputDir) )
  {
    sm_CheckIOFiles();
  }
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_sm_GoBtn_clicked()
{
  if (sm_GoBtn->text().compare("Cancel") == 0)
  {
    if(m_SurfaceMesh.get() != NULL)
    {
      //std::cout << "canceling from GUI...." << std::endl;
      emit sig_CancelWorker();
    }
    return;
  }

  m_SurfaceMesh = SurfaceMesh::New(NULL);
  m_SurfaceMesh->setDXFile(sm_DxFile->text().toStdString() );
  m_SurfaceMesh->setEdgeTableFile(sm_EdgeTableFile->text().toStdString() );
  m_SurfaceMesh->setNeighSpinTableFile(sm_NeighSpinTableFile->text().toStdString() );
  m_SurfaceMesh->setOutputDirectory(sm_OutputDir->text().toStdString());
  m_SurfaceMesh->setXDim(sm_XDim->value());
  m_SurfaceMesh->setYDim(sm_YDim->value());
  m_SurfaceMesh->setZDim(sm_ZDim->value());
  m_SurfaceMesh->setSmoothMesh(sm_SmoothMesh->isChecked());
  m_SurfaceMesh->setSmoothIterations(sm_SmoothIterations->value());
  m_SurfaceMesh->setSmoothFileOutputIncrement(sm_WriteOutputFileIncrement->value());
  m_SurfaceMesh->setSmoothLockQuadPoints(sm_LockQuadPoints->isChecked());


  connect(m_SurfaceMesh.get(), SIGNAL(finished()),
          this, SLOT( sm_ThreadFinished() ) );
  connect(m_SurfaceMesh.get(), SIGNAL (updateProgress(int)),
    this, SLOT(sm_ThreadProgressed(int) ) , Qt::DirectConnection);
  connect(m_SurfaceMesh.get(), SIGNAL (updateMessage(QString)),
          this, SLOT(threadHasMessage(QString) ) );

  setWidgetListEnabled(false);
  reconstructionTab->setEnabled(false);
  grainGeneratorTab->setEnabled(false);
  volumeMeshingTab->setEnabled(false);
  m_SurfaceMesh->start();
  sm_GoBtn->setText("Cancel");
}



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::sm_ThreadFinished()
{
  std::cout << "RepresentationUI::surface_meshing()" << std::endl;
  sm_GoBtn->setText("Go");
  setWidgetListEnabled(true);
  this->sm_progressBar->setValue(0);
  reconstructionTab->setEnabled(true);
  surfaceMeshingTab->setEnabled(true);
  volumeMeshingTab->setEnabled(true);
  sm_CheckIOFiles();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::sm_ThreadProgressed(int value)
{
  sm_progressBar->setValue(value);
}


//TODO: Volume Meshing Methods
/* *****************************************************************************
 *
 * Volume Meshing Methods
 *
 ***************************************************************************** */

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::vm_SetupGui()
{
  if (NULL == vm_NodesFile->completer()) {
    QR3DFileCompleter* com4 = new QR3DFileCompleter(this, true);
    vm_NodesFile->setCompleter(com4);
    QObject::connect( com4, SIGNAL(activated(const QString &)),
      this, SLOT(on_vm_NodesFile_textChanged(const QString &)));
  }

  if (NULL == vm_TrianglesFile->completer()) {
    QR3DFileCompleter* com4 = new QR3DFileCompleter(this, true);
    vm_TrianglesFile->setCompleter(com4);
    QObject::connect( com4, SIGNAL(activated(const QString &)),
      this, SLOT(on_vm_TrianglesFile_textChanged(const QString &)));
  }

  if (NULL == vm_OutputDir->completer()) {
    QR3DFileCompleter* com4 = new QR3DFileCompleter(this, true);
    vm_OutputDir->setCompleter(com4);
    QObject::connect( com4, SIGNAL(activated(const QString &)),
      this, SLOT(on_vm_OutputDir_textChanged(const QString &)));
  }


  m_WidgetList << vm_NodesFile << vm_NodesFileBtn << vm_TrianglesFile << vm_TrianglesFileBtn;
  m_WidgetList << vm_XDim << vm_XRes << vm_YDim << vm_YRes << vm_ZDim << vm_ZRes;
  m_WidgetList << vm_NumGrains << vm_OutputDir << vm_OutputDirBtn;

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::vm_CheckIOFiles()
{

  _verifyPathExists(vm_NodesFile->text(), vm_NodesFile);
  _verifyPathExists(vm_TrianglesFile->text(), vm_TrianglesFile);
  _verifyPathExists(vm_OutputDir->text(), vm_OutputDir );

  CHECK_QLINEEDIT_FILE_EXISTS(vm_, NodesFile);
  CHECK_QLINEEDIT_FILE_EXISTS(vm_, TrianglesFile);

  CHECK_QLABEL_FILE_EXISTS(vm_, MeshFile);
  CHECK_QLABEL_FILE_EXISTS(vm_, MeshFile2);
  CHECK_QLABEL_FILE_EXISTS(vm_, ElementQualityFile);
  CHECK_QLABEL_FILE_EXISTS(vm_, VoxelsFile);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_vm_NodesFileBtn_clicked()
{

}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_vm_TrianglesFileBtn_clicked()
{

}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_vm_OutputDirBtn_clicked()
{

}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_vm_GoBtn_clicked()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_vm_NodesFile_textChanged(const QString & text)
{
  vm_CheckIOFiles();
  _verifyPathExists(vm_NodesFile->text(), vm_NodesFile);
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_vm_TrianglesFile_textChanged(const QString & text)
{
  vm_CheckIOFiles();
  _verifyPathExists(vm_TrianglesFile->text(), vm_TrianglesFile);
}
// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::on_vm_OutputDir_textChanged(const QString & text)
{
  vm_CheckIOFiles();
  _verifyPathExists(vm_OutputDir->text(), vm_OutputDir);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::vm_ThreadFinished()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RepresentationUI::vm_ThreadProgressed(int value)
{

}


