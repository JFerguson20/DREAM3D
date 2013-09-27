/* ============================================================================
 * Copyright (c) 2011 Michael A. Jackson (BlueQuartz Software)
 * Copyright (c) 2011 Dr. Michael A. Groeber (US Air Force Research Laboratories)
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
 * Neither the name of Michael A. Groeber, Michael A. Jackson, the US Air Force,
 * BlueQuartz Software nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior written
 * permission.
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
 *
 *  This code was written under United States Air Force Contract number
 *                           FA8650-07-D-5800
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "INLWriter.h"

#include <QtCore/QtDebug>
#include <fstream>


#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QFile>

#include "EbsdLib/TSL/AngConstants.h"

#include "DREAM3DLib/DREAM3DVersion.h"

#include "DREAM3DLib/OrientationOps/CubicOps.h"
#include "DREAM3DLib/OrientationOps/CubicLowOps.h"
#include "DREAM3DLib/OrientationOps/HexagonalOps.h"
#include "DREAM3DLib/OrientationOps/TrigonalOps.h"
#include "DREAM3DLib/OrientationOps/TetragonalOps.h"
#include "DREAM3DLib/OrientationOps/OrthoRhombicOps.h"
#include "DREAM3DLib/OrientationOps/MonoclinicOps.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
INLWriter::INLWriter() :
  FileWriter(),
  m_DataContainerName(DREAM3D::HDF5::VolumeDataContainerName),
  m_GrainIdsArrayName(DREAM3D::CellData::GrainIds),
  m_CellPhasesArrayName(DREAM3D::CellData::Phases),
  m_CellEulerAnglesArrayName(DREAM3D::CellData::EulerAngles),
  m_CrystalStructuresArrayName(DREAM3D::EnsembleData::CrystalStructures),
  m_MaterialNamesArrayName(DREAM3D::EnsembleData::MaterialName),
  m_GrainIds(NULL),
  m_CellPhases(NULL),
  m_CellEulerAngles(NULL)

{
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
INLWriter::~INLWriter()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void INLWriter::setupFilterParameters()
{
  QVector<FilterParameter::Pointer> parameters;
  {
    FilterParameter::Pointer option = FilterParameter::New();
    option->setHumanLabel("Output File");
    option->setPropertyName("OutputFile");
    option->setWidgetType(FilterParameter::OutputFileWidget);
    option->setFileExtension("*.txt");
    option->setFileType("INL Format");
    option->setValueType("string");
    parameters.push_back(option);
  }
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void INLWriter::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  /* Code to read the values goes between these statements */
  /* FILTER_WIDGETCODEGEN_AUTO_GENERATED_CODE BEGIN*/
  setOutputFile( reader->readValue( "OutputFile", getOutputFile() ) );
  /* FILTER_WIDGETCODEGEN_AUTO_GENERATED_CODE END*/
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int INLWriter::writeFilterParameters(AbstractFilterParametersWriter* writer, int index)
{
  writer->openFilterGroup(this, index);
  writer->writeValue("OutputFile", getOutputFile());
  writer->closeFilterGroup();
  return ++index; // we want to return the next index that was just written to
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void INLWriter::dataCheck(bool preflight, size_t voxels, size_t fields, size_t ensembles)
{
  setErrorCondition(0);

  VolumeDataContainer* m = getDataContainerArray()->getDataContainerAs<VolumeDataContainer>(getDataContainerName());
  if(NULL == m)
  {
    setErrorCondition(-999);
    notifyErrorMessage("The DataContainer Object was NULL", -999);
    return;
  }
  if(getOutputFile().isEmpty() == true)
  {

    QString ss = QObject::tr("%1 needs the Output File Set and it was not.").arg(ClassName());
    addErrorMessage(getHumanLabel(), ss, -1);
    setErrorCondition(-387);
  }


  GET_PREREQ_DATA(m, DREAM3D, CellData, GrainIds, -300, int32_t, Int32ArrayType, voxels, 1)
  GET_PREREQ_DATA(m, DREAM3D, CellData, CellPhases, -302, int32_t, Int32ArrayType, voxels, 1)
  GET_PREREQ_DATA(m, DREAM3D, CellData, CellEulerAngles, -305, float, FloatArrayType, voxels, 3)

  typedef DataArray<unsigned int> XTalStructArrayType;
  GET_PREREQ_DATA(m, DREAM3D, CellEnsembleData, CrystalStructures, -304, unsigned int, XTalStructArrayType, ensembles, 1)

  IDataArray::Pointer materialNamePtr = m->getCellEnsembleData(DREAM3D::EnsembleData::MaterialName);
  StringDataArray* materialNames = StringDataArray::SafePointerDownCast(materialNamePtr.get());
  if (NULL == materialNames)
  {

    QString ss = QObject::tr("%1 requires the %2 Ensemble array to be present in the Data Container.").arg(ClassName()).arg(DREAM3D::EnsembleData::MaterialName);
    addErrorMessage(getHumanLabel(), ss, -1111);
    setErrorCondition(-1111);
  }
  //  else
  //  {
  //    addRequiredEnsembleData(DREAM3D::EnsembleData::MaterialName);
  //  }


}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void INLWriter::preflight()
{
  dataCheck(true, 1, 1, 1);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int INLWriter::writeHeader()
{
  return 0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int INLWriter::writeFile()
{
  VolumeDataContainer* m = getDataContainerArray()->getDataContainerAs<VolumeDataContainer>(getDataContainerName());
  if (NULL == m)
  {

    QString ss = QObject::tr("DataContainer Pointer was NULL and Must be valid.%1(%2)").arg(__LINE__).arg(__FILE__);
    addErrorMessage(getHumanLabel(), ss, -1);
    setErrorCondition(-1);
    return -1;
  }

  int64_t totalPoints = m->getTotalPoints();
  size_t numgrains = m->getNumCellFieldTuples();
  size_t numensembles = m->getNumCellEnsembleTuples();
  dataCheck(false, totalPoints, numgrains, numensembles);
  if(getErrorCondition() < 0)
  {
    return -40;
  }

  int err = 0;
  size_t dims[3] =
  { 0, 0, 0 };
  m->getDimensions(dims);

  float res[3];
  m->getResolution(res);

  float origin[3];
  m->getOrigin(origin);

  // Make sure any directory path is also available as the user may have just typed
  // in a path without actually creating the full path
  QFileInfo fi(getOutputFile());
  QDir dir(fi.path());
  if(!dir.mkpath("."))
  {

    QString ss = QObject::tr("Error creating parent path '%1'").arg(fi.path());
    notifyErrorMessage(ss, -1);
    setErrorCondition(-1);
    return -1;
  }

  FILE* f = fopen(getOutputFile().toLatin1().data(), "wb");
  if(NULL == f)
  {

    QString ss = QObject::tr("Error Opening File for writing '%1'").arg(getOutputFile());
    notifyErrorMessage(ss, -1);
    setErrorCondition(-1);
    return -1;
  }

  // Write the header, Each line starts with a "#" symbol
  fprintf(f, "# File written from %s\r\n", DREAM3DLib::Version::PackageComplete().toLatin1().data());
  fprintf(f, "# DateTime: %s\r\n", QDateTime::currentDateTime().toString().toLatin1().data());
  fprintf(f, "# X_STEP: %f\r\n", res[0]);
  fprintf(f, "# Y_STEP: %f\r\n", res[1]);
  fprintf(f, "# Z_STEP: %f\r\n", res[2]);
  fprintf(f, "#\r\n");
  fprintf(f, "# X_MIN: %f\r\n", origin[0]);
  fprintf(f, "# Y_MIN: %f\r\n", origin[1]);
  fprintf(f, "# Z_MIN: %f\r\n", origin[2]);
  fprintf(f, "#\r\n");
  fprintf(f, "# X_MAX: %f\r\n", origin[0]+(dims[0]*res[0]));
  fprintf(f, "# Y_MAX: %f\r\n", origin[1]+(dims[1]*res[1]));
  fprintf(f, "# Z_MAX: %f\r\n", origin[2]+(dims[0]*res[2]));
  fprintf(f, "#\r\n");
  fprintf(f, "# X_DIM: %llu\r\n", static_cast<unsigned long long int>(dims[0]));
  fprintf(f, "# Y_DIM: %llu\r\n", static_cast<unsigned long long int>(dims[1]));
  fprintf(f, "# Z_DIM: %llu\r\n", static_cast<unsigned long long int>(dims[2]));
  fprintf(f, "#\r\n");

 // IDataArray::Pointer pDataPtr = m->getCellEnsembleData(DREAM3D::EnsembleData::PhaseTypes);
  IDataArray::Pointer materialNamePtr = m->getCellEnsembleData(DREAM3D::EnsembleData::MaterialName);
  StringDataArray* materialNames = StringDataArray::SafePointerDownCast(materialNamePtr.get());
  if (NULL == materialNames)
  {
    fclose(f);

    QString ss = QObject::tr("The MaterialNames Ensemble Array was not in the Data Container");
    notifyErrorMessage(ss, -1111);
    setErrorCondition(-1111);
    return -1;
  }
  uint32_t symmetry = 0;
  for(size_t i = 1; i < materialNamePtr->getNumberOfTuples(); ++i)
  {
    symmetry = m_CrystalStructures[i];
    fprintf(f, "# Phase_%zu: %s\r\n", i, materialNames->GetValue(i).toLatin1().data());
    if(symmetry == Ebsd::CrystalStructure::Cubic_High)
    {
      symmetry = Ebsd::Ang::PhaseSymmetry::Cubic;
    }
    else if(symmetry == Ebsd::CrystalStructure::Hexagonal_High)
    {
      symmetry = Ebsd::Ang::PhaseSymmetry::DiHexagonal;
    }
    else
    {
      symmetry = Ebsd::Ang::PhaseSymmetry::UnknownSymmetry;
    }
    fprintf(f, "# Symmetry_%zu: %u\r\n", i, symmetry);
    fprintf(f, "#\r\n");
  }

  QSet<int32_t> uniqueGrainIds;
  for(int64_t i = 0; i < totalPoints; ++i)
  {
    uniqueGrainIds.insert(m_GrainIds[i]);
  }
  fprintf(f, "# Num_Grains: %d \r\n", uniqueGrainIds.size());
  fprintf(f, "#\r\n");

  //  fprintf(f, "# Column 1-3: phi1, PHI, phi2 (orientation of point in radians)\r\n");
  //  fprintf(f, "# Column 4-6: x, y, z (coordinates of point in microns)\r\n");
  //  fprintf(f, "# Column 7: Grain ID\r\n");
  //  fprintf(f, "# Column 8: Phase ID\r\n");


  fprintf(f,"# phi1 PHI phi2 x y z GrainId PhaseId Symmetry\r\n");

  float phi1, phi, phi2;
  float xPos, yPos, zPos;
  int32_t grainId;
  int32_t phaseId;

  // unsigned char rgba[4] = {0,0,0,255};
  //float refDir[3] = {0.0f, 0.0f, 1.0f};


  size_t index = 0;
  for(size_t z = 0; z < dims[2]; ++z)
  {
    for (size_t y = 0; y < dims[1]; ++y)
    {
      for (size_t x = 0; x < dims[0]; ++x)
      {
        index = (z*dims[0]*dims[1]) + (dims[0]*y) + x;
        phi1 = m_CellEulerAngles[index*3];
        phi = m_CellEulerAngles[index*3+1];
        phi2 = m_CellEulerAngles[index*3+2];
        xPos = origin[0] + (x * res[0]);
        yPos = origin[1] + (y * res[1]);
        zPos = origin[2] + (z * res[2]);
        grainId = m_GrainIds[index];
        phaseId = m_CellPhases[index];
        // rgba[0] = 0; rgba[1] = 0; rgba[2] = 0; // Reset the color to black
        symmetry = m_CrystalStructures[phaseId];
        if(phaseId > 0)
        {
          if(symmetry == Ebsd::CrystalStructure::Cubic_High)
          {
            symmetry = Ebsd::Ang::PhaseSymmetry::Cubic;
          }
          else if(symmetry == Ebsd::CrystalStructure::Hexagonal_High)
          {
            symmetry = Ebsd::Ang::PhaseSymmetry::DiHexagonal;
          }
          else
          {
            symmetry = Ebsd::Ang::PhaseSymmetry::UnknownSymmetry;
          }
        }
        else
        {
          symmetry = Ebsd::Ang::PhaseSymmetry::UnknownSymmetry;
        }


        fprintf(f, "%f %f %f %f %f %f %d %d %d\r\n",phi1, phi, phi2, xPos, yPos, zPos, grainId, phaseId, symmetry);
      }
    }
  }



  fclose(f);


  notifyStatusMessage("Complete");
  return err;
}
