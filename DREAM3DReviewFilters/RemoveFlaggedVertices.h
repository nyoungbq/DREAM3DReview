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
 * The code contained herein was partially funded by the following contracts:
 *    United States Air Force Prime Contract FA8650-07-D-5800
 *    United States Air Force Prime Contract FA8650-10-D-5210
 *    United States Prime Contract Navy N00173-07-C-2068
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#pragma once

#include <memory>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/DataArrays/DataArray.hpp"
#include "SIMPLib/Filtering/AbstractFilter.h"

#include "DREAM3DReview/DREAM3DReviewDLLExport.h"

/**
 * @brief The RemoveFlaggedVertices class. See [Filter documentation](@ref removeflaggedvertices) for details.
 */
class DREAM3DReview_EXPORT RemoveFlaggedVertices : public AbstractFilter
{
  Q_OBJECT

  // Start Python bindings declarations
  PYB11_BEGIN_BINDINGS(RemoveFlaggedVertices SUPERCLASS AbstractFilter)
  PYB11_FILTER()
  PYB11_SHARED_POINTERS(RemoveFlaggedVertices)
  PYB11_FILTER_NEW_MACRO(RemoveFlaggedVertices)
  PYB11_PROPERTY(DataArrayPath VertexGeometry READ getVertexGeometry WRITE setVertexGeometry)
  PYB11_PROPERTY(DataArrayPath MaskArrayPath READ getMaskArrayPath WRITE setMaskArrayPath)
  PYB11_PROPERTY(QString ReducedVertexGeometry READ getReducedVertexGeometry WRITE setReducedVertexGeometry)
  PYB11_END_BINDINGS()
  // End Python bindings declarations

public:
  using Self = RemoveFlaggedVertices;
  using Pointer = std::shared_ptr<Self>;
  using ConstPointer = std::shared_ptr<const Self>;
  using WeakPointer = std::weak_ptr<Self>;
  using ConstWeakPointer = std::weak_ptr<const Self>;
  static Pointer NullPointer();

  static std::shared_ptr<RemoveFlaggedVertices> New();

  /**
   * @brief Returns the name of the class for RemoveFlaggedVertices
   */
  QString getNameOfClass() const override;
  /**
   * @brief Returns the name of the class for RemoveFlaggedVertices
   */
  static QString ClassName();

  ~RemoveFlaggedVertices() override;

  /**
   * @brief Setter property for VertexGeometry
   */
  void setVertexGeometry(const DataArrayPath& value);
  /**
   * @brief Getter property for VertexGeometry
   * @return Value of VertexGeometry
   */
  DataArrayPath getVertexGeometry() const;
  Q_PROPERTY(DataArrayPath VertexGeometry READ getVertexGeometry WRITE setVertexGeometry)

  /**
   * @brief Setter property for MaskArrayPath
   */
  void setMaskArrayPath(const DataArrayPath& value);
  /**
   * @brief Getter property for MaskArrayPath
   * @return Value of MaskArrayPath
   */
  DataArrayPath getMaskArrayPath() const;
  Q_PROPERTY(DataArrayPath MaskArrayPath READ getMaskArrayPath WRITE setMaskArrayPath)

  /**
   * @brief Setter property for ReducedVertexGeometry
   */
  void setReducedVertexGeometry(const QString& value);
  /**
   * @brief Getter property for ReducedVertexGeometry
   * @return Value of ReducedVertexGeometry
   */
  QString getReducedVertexGeometry() const;
  Q_PROPERTY(QString ReducedVertexGeometry READ getReducedVertexGeometry WRITE setReducedVertexGeometry)

  /**
   * @brief getCompiledLibraryName Reimplemented from @see AbstractFilter class
   */
  QString getCompiledLibraryName() const override;

  /**
   * @brief getBrandingString Returns the branding string for the filter, which is a tag
   * used to denote the filter's association with specific plugins
   * @return Branding string
   */
  QString getBrandingString() const override;

  /**
   * @brief getFilterVersion Returns a version string for this filter. Default
   * value is an empty string.
   * @return
   */
  QString getFilterVersion() const override;

  /**
   * @brief newFilterInstance Reimplemented from @see AbstractFilter class
   */
  AbstractFilter::Pointer newFilterInstance(bool copyFilterParameters) const override;

  /**
   * @brief getGroupName Reimplemented from @see AbstractFilter class
   */
  QString getGroupName() const override;

  /**
   * @brief getSubGroupName Reimplemented from @see AbstractFilter class
   */
  QString getSubGroupName() const override;

  /**
   * @brief getUuid Return the unique identifier for this filter.
   * @return A QUuid object.
   */
  QUuid getUuid() const override;

  /**
   * @brief getHumanLabel Reimplemented from @see AbstractFilter class
   */
  QString getHumanLabel() const override;

  /**
   * @brief setupFilterParameters Reimplemented from @see AbstractFilter class
   */
  void setupFilterParameters() override;

  /**
   * @brief readFilterParameters Reimplemented from @see AbstractFilter class
   */
  void readFilterParameters(AbstractFilterParametersReader* reader, int index) override;

  /**
   * @brief execute Reimplemented from @see AbstractFilter class
   */
  void execute() override;

protected:
  RemoveFlaggedVertices();

  /**
   * @brief dataCheck Checks for the appropriate parameter values and availability of arrays
   */
  void dataCheck() override;

  /**
   * @brief Initializes all the private instance variables
   */
  void initialize();

private:
  std::weak_ptr<DataArray<bool>> m_MaskPtr;
  bool* m_Mask = nullptr;

  DataArrayPath m_VertexGeometry = {SIMPL::Defaults::VertexDataContainerName, "", ""};
  DataArrayPath m_MaskArrayPath = {SIMPL::Defaults::VertexDataContainerName, SIMPL::Defaults::VertexAttributeMatrixName, SIMPL::CellData::Mask};
  QString m_ReducedVertexGeometry = {"ReducedVertexDataContainer"};

  QList<QString> m_AttrMatList;

public:
  RemoveFlaggedVertices(const RemoveFlaggedVertices&) = delete;            // Copy Constructor Not Implemented
  RemoveFlaggedVertices(RemoveFlaggedVertices&&) = delete;                 // Move Constructor Not Implemented
  RemoveFlaggedVertices& operator=(const RemoveFlaggedVertices&) = delete; // Copy Assignment Not Implemented
  RemoveFlaggedVertices& operator=(RemoveFlaggedVertices&&) = delete;      // Move Assignment Not Implemented
};
