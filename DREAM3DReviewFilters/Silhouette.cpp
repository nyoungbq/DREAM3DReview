/* ============================================================================
* Copyright (c) 2009-2016 BlueQuartz Software, LLC
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
* Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
* contributors may be used to endorse or promote products derived from this software
* without specific prior written permission.
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
* The code contained herein was partially funded by the followig contracts:
*    United States Air Force Prime Contract FA8650-07-D-5800
*    United States Air Force Prime Contract FA8650-10-D-5210
*    United States Prime Contract Navy N00173-07-C-2068
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "Silhouette.h"

#include <unordered_set>

#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/Common/TemplateHelpers.hpp"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/AttributeMatrixSelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/ChoiceFilterParameter.h"
#include "SIMPLib/FilterParameters/DataArrayCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedBooleanFilterParameter.h"

#include "util/EvaluationAlgorithms/SilhouetteTemplate.hpp"

#include "DREAM3DReview/DREAM3DReviewConstants.h"
#include "DREAM3DReview/DREAM3DReviewVersion.h"

// Include the MOC generated file for this class
#include "moc_Silhouette.cpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
Silhouette::Silhouette()
: AbstractFilter()
, m_SelectedArrayPath("", "", "")
, m_UseMask(false)
, m_MaskArrayPath("", "", "Mask")
, m_FeatureIdsArrayPath("", "", "ClusterIds")
, m_SilhouetteArrayPath("", "", "Silhouette")
, m_DistanceMetric(0)
, m_InData(nullptr)
, m_Mask(nullptr)
, m_FeatureIds(nullptr)
, m_SilhouetteArray(nullptr)
{
  setupFilterParameters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
Silhouette::~Silhouette()
= default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void Silhouette::setupFilterParameters()
{
  FilterParameterVector parameters;
  {
    ChoiceFilterParameter::Pointer parameter = ChoiceFilterParameter::New();
    parameter->setHumanLabel("Distance Metric");
    parameter->setPropertyName("DistanceMetric");
    parameter->setSetterCallback(SIMPL_BIND_SETTER(Silhouette, this, DistanceMetric));
    parameter->setGetterCallback(SIMPL_BIND_GETTER(Silhouette, this, DistanceMetric));
    DistanceTemplate distOptions;
    QVector<QString> choices = DistanceTemplate::GetDistanceMetricsOptions();
    parameter->setChoices(choices);
    parameter->setCategory(FilterParameter::Parameter);
    parameters.push_back(parameter);
  }
  QStringList linkedProps("MaskArrayPath");
  parameters.push_back(SIMPL_NEW_LINKED_BOOL_FP("Use Mask", UseMask, FilterParameter::Parameter, Silhouette, linkedProps));
  DataArraySelectionFilterParameter::RequirementType dasReq =
      DataArraySelectionFilterParameter::CreateRequirement(SIMPL::Defaults::AnyPrimitive, SIMPL::Defaults::AnyComponentSize, AttributeMatrix::Type::Any, IGeometry::Type::Any);
  parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Attribute Array to Silhouette", SelectedArrayPath, FilterParameter::RequiredArray, Silhouette, dasReq));
  dasReq = DataArraySelectionFilterParameter::CreateRequirement(SIMPL::TypeNames::Bool, 1, AttributeMatrix::Type::Any, IGeometry::Type::Any);
  parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Mask", MaskArrayPath, FilterParameter::RequiredArray, Silhouette, dasReq));
  dasReq = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::UInt32, 1, AttributeMatrix::Category::Element);
  parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Cluster Ids", FeatureIdsArrayPath, FilterParameter::RequiredArray, Silhouette, dasReq));
  DataArrayCreationFilterParameter::RequirementType dacReq = DataArrayCreationFilterParameter::CreateRequirement(AttributeMatrix::Type::Any, IGeometry::Type::Any);
  AttributeMatrix::Types amTypes;
  amTypes.push_back(AttributeMatrix::Type::VertexFeature);
  amTypes.push_back(AttributeMatrix::Type::EdgeFeature);
  amTypes.push_back(AttributeMatrix::Type::FaceFeature);
  amTypes.push_back(AttributeMatrix::Type::CellFeature);
  amTypes.push_back(AttributeMatrix::Type::VertexEnsemble);
  amTypes.push_back(AttributeMatrix::Type::EdgeEnsemble);
  amTypes.push_back(AttributeMatrix::Type::FaceEnsemble);
  amTypes.push_back(AttributeMatrix::Type::CellEnsemble);
  dacReq.amTypes = amTypes;
  parameters.push_back(SIMPL_NEW_DA_CREATION_FP("Silhouette", SilhouetteArrayPath, FilterParameter::CreatedArray, Silhouette, dacReq));
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void Silhouette::readFilterParameters(AbstractFilterParametersReader* reader, int index)
{
  reader->openFilterGroup(this, index);
  setSelectedArrayPath(reader->readDataArrayPath("SelectedArrayPath", getSelectedArrayPath()));
  setUseMask(reader->readValue("UseMask", getUseMask()));
  setMaskArrayPath(reader->readDataArrayPath("MaskArrayPath", getMaskArrayPath()));
  setFeatureIdsArrayPath(reader->readDataArrayPath("FeatureIdsArrayPath", getFeatureIdsArrayPath()));
  setSilhouetteArrayPath(reader->readDataArrayPath("SilhouetteArrayName", getSilhouetteArrayPath()));
  setDistanceMetric(reader->readValue("DistanceMetric", getDistanceMetric()));
  reader->closeFilterGroup();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void Silhouette::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void Silhouette::dataCheck()
{
  setErrorCondition(0);
  setWarningCondition(0);

  QVector<DataArrayPath> dataArrayPaths;
  QVector<size_t> cDims(1, 1);

  m_SilhouetteArrayPtr = getDataContainerArray()->createNonPrereqArrayFromPath<DataArray<double>, AbstractFilter>(this, getSilhouetteArrayPath(), 0, cDims);
  if(m_SilhouetteArrayPtr.lock())
  {
    m_SilhouetteArray = m_SilhouetteArrayPtr.lock()->getPointer(0);
  }
  if(getErrorCondition() >= 0)
  {
    dataArrayPaths.push_back(getSilhouetteArrayPath());
  }

  m_FeatureIdsPtr = getDataContainerArray()->getPrereqArrayFromPath<DataArray<int32_t>, AbstractFilter>(this, getFeatureIdsArrayPath(), cDims);
  if(m_FeatureIdsPtr.lock())
  {
    m_FeatureIds = m_FeatureIdsPtr.lock()->getPointer(0);
  }
  if(getErrorCondition() >= 0)
  {
    dataArrayPaths.push_back(getFeatureIdsArrayPath());
  }

  m_InDataPtr = getDataContainerArray()->getPrereqIDataArrayFromPath<IDataArray, AbstractFilter>(this, getSelectedArrayPath());
  if(getErrorCondition() >= 0)
  {
    dataArrayPaths.push_back(getSelectedArrayPath());
  }

  if(m_UseMask)
  {
    m_MaskPtr = getDataContainerArray()->getPrereqArrayFromPath<BoolArrayType, AbstractFilter>(this, getMaskArrayPath(), cDims);
    if(m_MaskPtr.lock().get() != nullptr)
    {
      m_Mask = m_MaskPtr.lock()->getPointer(0);
    }
    if(getErrorCondition() >= 0)
    {
      dataArrayPaths.push_back(getMaskArrayPath());
    }
  }

  getDataContainerArray()->validateNumberOfTuples<AbstractFilter>(this, dataArrayPaths);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void Silhouette::preflight()
{
  setInPreflight(true);
  emit preflightAboutToExecute();
  emit updateFilterParameters(this);
  dataCheck();
  emit preflightExecuted();
  setInPreflight(false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void Silhouette::execute()
{
  setErrorCondition(0);
  setWarningCondition(0);
  dataCheck();
  if(getErrorCondition() < 0)
  {
    return;
  }

  size_t numTuples = m_FeatureIdsPtr.lock()->getNumberOfTuples();
  std::unordered_set<int32_t> uniqueIds;

  for(size_t i = 0; i < numTuples; i++)
  {
    uniqueIds.insert(m_FeatureIds[i]);
  }

  if(m_UseMask)
  {
    EXECUTE_TEMPLATE(this, SilhouetteTemplate, m_InDataPtr.lock(), this, m_InDataPtr.lock(), m_SilhouetteArrayPtr.lock(), m_MaskPtr.lock(), uniqueIds.size(), m_FeatureIdsPtr.lock(), m_DistanceMetric)
  }
  else
  {
    BoolArrayType::Pointer tmpMask = BoolArrayType::CreateArray(numTuples, "_INTERNAL_USE_ONLY_tmpMask");
    tmpMask->initializeWithValue(true);
    EXECUTE_TEMPLATE(this, SilhouetteTemplate, m_InDataPtr.lock(), this, m_InDataPtr.lock(), m_SilhouetteArrayPtr.lock(), tmpMask, uniqueIds.size(), m_FeatureIdsPtr.lock(), m_DistanceMetric)
  }

  notifyStatusMessage(getHumanLabel(), "Complete");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer Silhouette::newFilterInstance(bool copyFilterParameters)
{
  Silhouette::Pointer filter = Silhouette::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString Silhouette::getCompiledLibraryName()
{
  return DREAM3DReviewConstants::DREAM3DReviewBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString Silhouette::getBrandingString()
{
  return "DREAM3DReview";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString Silhouette::getFilterVersion()
{
  QString version;
  QTextStream vStream(&version);
  vStream << DREAM3DReview::Version::Major() << "." << DREAM3DReview::Version::Minor() << "." << DREAM3DReview::Version::Patch();
  return version;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString Silhouette::getGroupName()
{
  return DREAM3DReviewConstants::FilterGroups::DREAM3DReviewFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString Silhouette::getSubGroupName()
{
  return DREAM3DReviewConstants::FilterSubGroups::ClusteringFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const QString Silhouette::getHumanLabel()
{
  return "Silhouette";
}
