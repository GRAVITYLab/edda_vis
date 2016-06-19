// Copyright 2015 The Edda Authors. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#ifndef GMM_VTK_ARRAY_H
#define GMM_VTK_ARRAY_H

#include <vector>
#include <stdexcept>
#include <string>

#include "vtk/vtk_common.h"

#include <distributions/gaussian_mixture.h>
#include <dataset/abstract_data_array.h>
#include <core/gmm_array.h>

namespace edda {

//---------------------------------------------------------------------------------------

/// \brief GmmVtkDataArray implements AbstractDataArray.  It holds vtkDataArrys and returns GMMs with interleaved memory accessing.
class GmmVtkDataArray: public AbstractDataArray
{
protected:
  std::vector<vtkSmartPointer<vtkDataArray> > arrays;

  size_t length = 0;
  int components = 1;
public:
  GmmVtkDataArray(vtkFieldData *fieldData, const char *arrayNamePrefix="")  ;

  /// \brief Based on the input array order, assign mean0, var0, weight0, mean1, var1, weight1,...
  /// The number of arrays should be multiples of 3
  GmmVtkDataArray(std::vector<vtkSmartPointer<vtkDataArray> > arrays_);

  typedef std::vector<vtkSmartPointer<vtkDataArray> > RawArrayType;

  virtual ~GmmVtkDataArray() { }

  virtual size_t getLength() { return length; }

  virtual int getNumComponents() { return this->components; }

  virtual dist::Variant getScalar(size_t idx);

  virtual std::vector<dist::Variant> getVector(size_t idx);

  virtual boost::any getItem(size_t idx);

  virtual void setItem(size_t idx, int component, const boost::any &item);

  virtual boost::any getRawArray() { return boost::any(arrays); }

  std::shared_ptr<GmmArray> genNdArray() ;
};

} // namespace edda

#endif // GMM_VTK_ARRAY_H
