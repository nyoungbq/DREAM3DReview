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
 * Neither the name of Michael A. Groeber, Michael A. Jackson,
 * the US Air Force, BlueQuartz Software nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
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
 *
 *  This code was written under United States Air Force Contract number
 *                   FA8650-07-D-5800 and FA8650-10-D-5226
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "DiscretizeDDDomain.h"

#include <QtCore/QTextStream>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/DataContainers/DataContainer.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/DataContainerCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/DataContainerSelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/FloatVec3FilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/SeparatorFilterParameter.h"
#include "SIMPLib/FilterParameters/StringFilterParameter.h"
#include "SIMPLib/Geometry/EdgeGeom.h"
#include "SIMPLib/Geometry/ImageGeom.h"
#include "SIMPLib/Math/GeometryMath.h"
#include "SIMPLib/Math/SIMPLibMath.h"

#include "DREAM3DReview/DREAM3DReviewVersion.h"

enum createdPathID : RenameDataPath::DataID_t
{
  AttributeMatrixID21 = 21,

  DataArrayID31 = 31,

  DataContainerID = 1
};

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DiscretizeDDDomain::DiscretizeDDDomain()
{
  m_CellSize[0] = 2.0;
  m_CellSize[1] = 2.0;
  m_CellSize[2] = 2.0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
DiscretizeDDDomain::~DiscretizeDDDomain() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DiscretizeDDDomain::setupFilterParameters()
{
  FilterParameterVectorType parameters;

  parameters.push_back(SIMPL_NEW_FLOAT_VEC3_FP("Cell Size (Microns)", CellSize, FilterParameter::Category::Parameter, DiscretizeDDDomain));
  // parameters.push_back(SeparatorFilterParameter::Create("", FilterParameter::Category::Parameter));
  {
    DataContainerSelectionFilterParameter::RequirementType req;
    parameters.push_back(SIMPL_NEW_DC_SELECTION_FP("Edge Data Container", EdgeDataContainerName, FilterParameter::Category::RequiredArray, DiscretizeDDDomain, req));
  }
  // parameters.push_back(SeparatorFilterParameter::Create("", FilterParameter::Category::Uncategorized));
  parameters.push_back(SIMPL_NEW_DC_CREATION_FP("Volume Data Container", OutputDataContainerName, FilterParameter::Category::CreatedArray, DiscretizeDDDomain));
  parameters.push_back(SIMPL_NEW_AM_WITH_LINKED_DC_FP("Cell Attribute Matrix", OutputAttributeMatrixName, OutputDataContainerName, FilterParameter::Category::CreatedArray, DiscretizeDDDomain));
  parameters.push_back(SIMPL_NEW_DA_WITH_LINKED_AM_FP("Dislocation Line Density Array Name", OutputArrayName, OutputDataContainerName, OutputAttributeMatrixName,
                                                      FilterParameter::Category::CreatedArray, DiscretizeDDDomain));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DiscretizeDDDomain::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setEdgeDataContainerName(reader->readDataArrayPath("EdgeDataContainerName", getEdgeDataContainerName()));
  setOutputDataContainerName(reader->readDataArrayPath("OutputDataContainerName", getOutputDataContainerName()));
  setOutputAttributeMatrixName(reader->readString("OutputAttributeMatrixName", getOutputAttributeMatrixName()));
  setOutputArrayName(reader->readString("OutputArrayName", getOutputArrayName()));
  setCellSize(reader->readFloatVec3("CellSize", getCellSize()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DiscretizeDDDomain::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DiscretizeDDDomain::dataCheck()
{

  clearErrorCode();
  clearWarningCode();
  DataArrayPath tempPath;

  // First sanity check the inputs and output names. All must be filled in

  if(getOutputDataContainerName().isEmpty())
  {
    QString ss = QObject::tr("The output DataContainer name is empty. Please assign a name for the created DataContainer");
    setErrorCondition(-11001, ss);
  }

  if(getOutputAttributeMatrixName().isEmpty())
  {
    QString ss = QObject::tr("The output AttributeMatrix name is empty. Please assign a name for the created AttributeMatrix");
    setErrorCondition(-11002, ss);
  }

  if(getOutputArrayName().isEmpty())
  {
    QString ss = QObject::tr("The output array name is empty. Please assign a name for the created array");
    setErrorCondition(-11003, ss);
  }

  // we can not go any further until all of the names are set.
  if(getErrorCode() < 0)
  {
    return;
  }

  // Next check the existing DataContainer/AttributeMatrix
  DataContainer::Pointer m = getDataContainerArray()->getPrereqDataContainer(this, getEdgeDataContainerName());
  if(getErrorCode() < 0)
  {
    return;
  }

  EdgeGeom::Pointer edges = m->getPrereqGeometry<EdgeGeom>(this);
  if(getErrorCode() < 0)
  {
    return;
  }

  // We MUST have Vertices defined.
  if(edges->getVertices().get() == nullptr)
  {
    setErrorCondition(-384, "DataContainer geometry missing Vertices");
  }
  // We MUST have Edges defined also.
  if(edges->getEdges().get() == nullptr)
  {
    setErrorCondition(-384, "DataContainer geometry missing Edges");
  }

  // Create a new DataContainer
  DataContainer::Pointer m2 = getDataContainerArray()->createNonPrereqDataContainer(this, getOutputDataContainerName(), DataContainerID);
  if(getErrorCode() < 0)
  {
    return;
  }

  // Create the voxel geometry to hold the local densities
  ImageGeom::Pointer image = ImageGeom::CreateGeometry(SIMPL::Geometry::ImageGeometry);
  m2->setGeometry(image);

  // Create the cell attrMat in the new data container
  std::vector<size_t> tDims(3, 0);
  AttributeMatrix::Pointer newCellAttrMat = m2->createNonPrereqAttributeMatrix(this, getOutputAttributeMatrixName(), tDims, AttributeMatrix::Type::Cell, AttributeMatrixID21);
  if(getErrorCode() < 0)
  {
    return;
  }

  // Get the name and create the array in the new data attrMat
  std::vector<size_t> dims(1, 1);
  tempPath.update(getOutputDataContainerName().getDataContainerName(), getOutputAttributeMatrixName(), getOutputArrayName());
  m_OutputArrayPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<int32_t>>(this, tempPath, 0, dims, "", DataArrayID31);
  if(nullptr != m_OutputArrayPtr.lock())
  {
    m_OutputArray = m_OutputArrayPtr.lock()->getPointer(0);
  } /* Now assign the raw pointer to data from the DataArray<T> object */
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void DiscretizeDDDomain::execute()
{
  QString ss;
  clearErrorCode();
  clearWarningCode();
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  DataContainer::Pointer edc = getDataContainerArray()->getDataContainer(getEdgeDataContainerName());
  DataContainer::Pointer vdc = getDataContainerArray()->getDataContainer(getOutputDataContainerName());
  AttributeMatrix::Pointer cellAttrMat = vdc->getAttributeMatrix(getOutputAttributeMatrixName());
  EdgeGeom::Pointer edgeGeom = edc->getGeometryAs<EdgeGeom>();

  float* nodes = edgeGeom->getVertexPointer(0);
  MeshIndexType* edge = edgeGeom->getEdgePointer(0);
  MeshIndexType numNodes = edgeGeom->getNumberOfVertices();
  MeshIndexType numEdges = edgeGeom->getNumberOfEdges();

  float xMin = 1000000000.0;
  float yMin = 1000000000.0;
  float zMin = 1000000000.0;
  float xMax = 0.0;
  float yMax = 0.0;
  float zMax = 0.0;
  float x, y, z;
  for(size_t i = 0; i < numNodes; i++)
  {
    x = nodes[3 * i + 0];
    y = nodes[3 * i + 1];
    z = nodes[3 * i + 2];
    if(x < xMin)
    {
      xMin = x;
    }
    if(x > xMax)
    {
      xMax = x;
    }
    if(y < yMin)
    {
      yMin = y;
    }
    if(y > yMax)
    {
      yMax = y;
    }
    if(z < zMin)
    {
      zMin = z;
    }
    if(z > zMax)
    {
      zMax = z;
    }
  }

  FloatVec3Type halfCellSize;
  halfCellSize[0] = (m_CellSize[0] / 2.0);
  halfCellSize[1] = (m_CellSize[1] / 2.0);
  halfCellSize[2] = (m_CellSize[2] / 2.0);
  FloatVec3Type quarterCellSize;
  quarterCellSize[0] = (m_CellSize[0] / 4.0);
  quarterCellSize[1] = (m_CellSize[1] / 4.0);
  quarterCellSize[2] = (m_CellSize[2] / 4.0);

  vdc->getGeometryAs<ImageGeom>()->setOrigin(FloatVec3Type(xMin, yMin, zMin));
  size_t dcDims[3];
  dcDims[0] = size_t((xMax - xMin) / halfCellSize[0]);
  dcDims[1] = size_t((yMax - yMin) / halfCellSize[1]);
  dcDims[2] = size_t((zMax - zMin) / halfCellSize[2]);
  vdc->getGeometryAs<ImageGeom>()->setDimensions(SizeVec3Type(dcDims[0], dcDims[1], dcDims[2]));
  vdc->getGeometryAs<ImageGeom>()->setSpacing(FloatVec3Type(m_CellSize[0] / 2.0, m_CellSize[1] / 2.0, m_CellSize[2] / 2.0));

  std::vector<size_t> tDims(3, 0);
  tDims[0] = dcDims[0];
  tDims[1] = dcDims[1];
  tDims[2] = dcDims[2];
  cellAttrMat->resizeAttributeArrays(tDims);

  float point1[3], point2[3], corner1[3], corner2[3];
  size_t xCellMin, xCellMax;
  size_t yCellMin, yCellMax;
  size_t zCellMin, zCellMax;
  float x1, y1, z1, x2, y2, z2;
  size_t zStride, yStride, point;
  int system = 0;
  float length;
  for(size_t i = 0; i < numEdges; i++)
  {
    point1[0] = nodes[3 * edge[2 * i + 0] + 0];
    point1[1] = nodes[3 * edge[2 * i + 0] + 1];
    point1[2] = nodes[3 * edge[2 * i + 0] + 2];
    point2[0] = nodes[3 * edge[2 * i + 1] + 0];
    point2[1] = nodes[3 * edge[2 * i + 1] + 1];
    point2[2] = nodes[3 * edge[2 * i + 1] + 2];
    x1 = (point1[0] - xMin);
    y1 = (point1[1] - yMin);
    z1 = (point1[2] - zMin);
    x2 = (point2[0] - xMin);
    y2 = (point2[1] - yMin);
    z2 = (point2[2] - zMin);
    if(x1 > x2)
    {
      xCellMin = size_t(x2 / quarterCellSize[0]), xCellMax = size_t(x1 / quarterCellSize[0]);
    }
    else
    {
      xCellMin = size_t(x1 / quarterCellSize[0]), xCellMax = size_t(x2 / quarterCellSize[0]);
    }
    if(y1 > y2)
    {
      yCellMin = size_t(y2 / quarterCellSize[1]), yCellMax = size_t(y1 / quarterCellSize[1]);
    }
    else
    {
      yCellMin = size_t(y1 / quarterCellSize[1]), yCellMax = size_t(y2 / quarterCellSize[1]);
    }
    if(z1 > z2)
    {
      zCellMin = size_t(z2 / quarterCellSize[2]), zCellMax = size_t(z1 / quarterCellSize[2]);
    }
    else
    {
      zCellMin = size_t(z1 / quarterCellSize[2]), zCellMax = size_t(z2 / quarterCellSize[2]);
    }
    xCellMin = (xCellMin - 1) / 2;
    yCellMin = (yCellMin - 1) / 2;
    zCellMin = (zCellMin - 1) / 2;
    xCellMax = ((xCellMax - 1) / 2) + 1;
    yCellMax = ((yCellMax - 1) / 2) + 1;
    zCellMax = ((zCellMax - 1) / 2) + 1;
    if(xCellMax >= tDims[0])
    {
      xCellMax = tDims[0] - 1;
    }
    if(yCellMax >= tDims[1])
    {
      yCellMax = tDims[1] - 1;
    }
    if(zCellMax >= tDims[2])
    {
      zCellMax = tDims[2] - 1;
    }
    for(size_t j = zCellMin; j <= zCellMax; j++)
    {
      zStride = j * tDims[0] * tDims[1];
      corner1[2] = (j * halfCellSize[2]) - halfCellSize[2] + quarterCellSize[2] + zMin;
      corner2[2] = (j * halfCellSize[2]) + halfCellSize[2] + quarterCellSize[2] + zMin;
      for(size_t k = yCellMin; k <= yCellMax; k++)
      {
        yStride = k * tDims[0];
        corner1[1] = (k * halfCellSize[1]) - halfCellSize[1] + quarterCellSize[1] + yMin;
        corner2[1] = (k * halfCellSize[1]) + halfCellSize[1] + quarterCellSize[1] + yMin;
        for(size_t l = xCellMin; l <= xCellMax; l++)
        {
          corner1[0] = (l * halfCellSize[0]) - halfCellSize[0] + quarterCellSize[0] + xMin;
          corner2[0] = (l * halfCellSize[0]) + halfCellSize[0] + quarterCellSize[0] + xMin;
          length = GeometryMath::LengthOfRayInBox(point1, point2, corner1, corner2);
          point = (zStride + yStride + l);
          m_OutputArray[14 * point + 0] += length;
          m_OutputArray[14 * point + system] += length;
        }
      }
    }
  }

  float max = 0.0;
  float cellVolume = m_CellSize[0] * m_CellSize[1] * m_CellSize[2];
  for(size_t j = 0; j < tDims[2]; j++)
  {
    zStride = j * tDims[0] * tDims[1];
    for(size_t k = 0; k < tDims[1]; k++)
    {
      yStride = k * tDims[0];
      for(size_t l = 0; l < tDims[0]; l++)
      {
        point = (zStride + yStride + l);
        // take care of total density first before looping over all systems
        m_OutputArray[14 * point] /= cellVolume;
        // convert to m/mm^3 from um/um^3
        m_OutputArray[14 * point] *= 1.0E12f;
        max = 0.0;
        for(int iter = 1; iter < 14; iter++)
        {
          m_OutputArray[14 * point + iter] /= cellVolume;
          // convert to m/mm^3 from um/um^3
          m_OutputArray[14 * point + iter] *= 1.0E12f;
        }
      }
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer DiscretizeDDDomain::newFilterInstance(bool copyFilterParameters) const
{
  DiscretizeDDDomain::Pointer filter = DiscretizeDDDomain::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString DiscretizeDDDomain::getCompiledLibraryName() const
{
  return DREAM3DReviewConstants::DREAM3DReviewBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString DiscretizeDDDomain::getBrandingString() const
{
  return "DDDAnalysisToolbox";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString DiscretizeDDDomain::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << DREAM3DReview::Version::Major() << "." << DREAM3DReview::Version::Minor() << "." << DREAM3DReview::Version::Patch();
  return version;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString DiscretizeDDDomain::getGroupName() const
{
  return SIMPL::FilterGroups::Unsupported;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid DiscretizeDDDomain::getUuid() const
{
  return QUuid("{ebdfe707-0c9c-5552-89f6-6ee4a1e0891b}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString DiscretizeDDDomain::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::MiscFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString DiscretizeDDDomain::getHumanLabel() const
{
  return "Discretize DDD Domain";
}

// -----------------------------------------------------------------------------
DiscretizeDDDomain::Pointer DiscretizeDDDomain::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<DiscretizeDDDomain> DiscretizeDDDomain::New()
{
  struct make_shared_enabler : public DiscretizeDDDomain
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString DiscretizeDDDomain::getNameOfClass() const
{
  return QString("DiscretizeDDDomain");
}

// -----------------------------------------------------------------------------
QString DiscretizeDDDomain::ClassName()
{
  return QString("DiscretizeDDDomain");
}

// -----------------------------------------------------------------------------
void DiscretizeDDDomain::setEdgeDataContainerName(const DataArrayPath& value)
{
  m_EdgeDataContainerName = value;
}

// -----------------------------------------------------------------------------
DataArrayPath DiscretizeDDDomain::getEdgeDataContainerName() const
{
  return m_EdgeDataContainerName;
}

// -----------------------------------------------------------------------------
void DiscretizeDDDomain::setCellSize(const FloatVec3Type& value)
{
  m_CellSize = value;
}

// -----------------------------------------------------------------------------
FloatVec3Type DiscretizeDDDomain::getCellSize() const
{
  return m_CellSize;
}

// -----------------------------------------------------------------------------
void DiscretizeDDDomain::setOutputDataContainerName(const DataArrayPath& value)
{
  m_OutputDataContainerName = value;
}

// -----------------------------------------------------------------------------
DataArrayPath DiscretizeDDDomain::getOutputDataContainerName() const
{
  return m_OutputDataContainerName;
}

// -----------------------------------------------------------------------------
void DiscretizeDDDomain::setOutputAttributeMatrixName(const QString& value)
{
  m_OutputAttributeMatrixName = value;
}

// -----------------------------------------------------------------------------
QString DiscretizeDDDomain::getOutputAttributeMatrixName() const
{
  return m_OutputAttributeMatrixName;
}

// -----------------------------------------------------------------------------
void DiscretizeDDDomain::setOutputArrayName(const QString& value)
{
  m_OutputArrayName = value;
}

// -----------------------------------------------------------------------------
QString DiscretizeDDDomain::getOutputArrayName() const
{
  return m_OutputArrayName;
}
