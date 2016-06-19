#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>

#include "gmm_vtk_data_array.h"
#include <core/ndarray.h>
#include <core/thrust_common.h>
using namespace std;
namespace edda{
using namespace dist;

GmmVtkDataArray::GmmVtkDataArray(vtkFieldData *fieldData, const char *arrayNamePrefix)  {
  char meanArrayName[1024];
  char stdevArrayName[1024];
  char varArrayName[1024];
  char weightArrayName[1024];
  for (int i=0; i<fieldData->GetNumberOfArrays(); i++)
  {
    sprintf(meanArrayName, "%smean%d", arrayNamePrefix, i );
    sprintf(stdevArrayName, "%sstdev%d", arrayNamePrefix, i );
    sprintf(varArrayName, "%svar%d", arrayNamePrefix, i );
    sprintf(weightArrayName, "%sweight%d", arrayNamePrefix, i );

    vtkSmartPointer<vtkDataArray> meanArray = fieldData->GetArray(meanArrayName);
    vtkSmartPointer<vtkDataArray> stdevArray = fieldData->GetArray(stdevArrayName);
    vtkSmartPointer<vtkDataArray> varArray = fieldData->GetArray(varArrayName);
    vtkSmartPointer<vtkDataArray> weightArray = fieldData->GetArray(weightArrayName);
    if (i==0 && meanArray && weightArray==0) {
      int c = meanArray->GetNumberOfComponents();
      int n = meanArray->GetNumberOfTuples();
      // allows when only one mean and one variance are provided
      // create weight Array
      weightArray = vtkSmartPointer<vtkFloatArray>::New();
      weightArray->SetNumberOfComponents(c);
      weightArray->SetNumberOfTuples(n);
      // asign 1's
      float *p = (float *)weightArray->GetVoidPointer(0);

      for (int j=0; j<n*c; j++)
        p[j] = 1.f;
    }
    if (meanArray && varArray && weightArray) {
      // get components
      int c = meanArray->GetNumberOfComponents();
      if (c != varArray->GetNumberOfComponents() || c!= weightArray->GetNumberOfComponents()) {
        printf("Warning: the number of array components do not match\n") ;
        c = 1;
      }
      this->components = c;

      // set arrays
      arrays.push_back(meanArray);
      arrays.push_back(varArray);
      arrays.push_back(weightArray);

    } else if (meanArray && stdevArray && weightArray) {
      // get components
      int c = meanArray->GetNumberOfComponents();
      if (c != stdevArray->GetNumberOfComponents() || c!= weightArray->GetNumberOfComponents()) {
        printf("Warning: the number of array components do not match\n") ;
        c = 1;
      }
      this->components = c;

      // convert stdev to variance
      arrays.push_back(meanArray);
      vsp_new(vtkFloatArray, varArray);
      int n = stdevArray->GetNumberOfTuples();
      varArray->SetNumberOfComponents(1);
      varArray->SetNumberOfTuples(n);
      for (int j=0; j<n; j++)
        *(float *)varArray->GetVoidPointer(j) = pow(stdevArray->GetTuple1(j), 2.) ;

      // set arrays
      arrays.push_back(varArray);
      arrays.push_back(weightArray);
    }
  }
  if (arrays.size() == 0) {
    length = 0;
    return;
  }
  length = arrays[0]->GetNumberOfTuples();
  for (size_t i=1; i<arrays.size(); i++)
  {
    length = std::min(length, (size_t)arrays[i]->GetNumberOfTuples());
  }
}

GmmVtkDataArray::GmmVtkDataArray(std::vector<vtkSmartPointer<vtkDataArray> > arrays_) {
  if (arrays_.size() == 0) {
    printf("Warning: no array assigned to GmmVtkArray\n");
    return;
  }
  if (arrays_.size() % 3 != 0) {
    printf("Warning: GmmVtkArray: some input arrays are truncated\n");
  }
  for (size_t i=0; i<arrays_.size()/3; i++) {
    this->arrays.push_back(arrays_[i]);
    this->arrays.push_back(arrays_[i+1]);
    this->arrays.push_back(arrays_[i+2]);
  }

  length = this->arrays[0]->GetNumberOfTuples();
  for (size_t i=0; i<this->arrays.size(); i++)
  {
    length = min(length, (size_t)this->arrays[i]->GetNumberOfTuples());
  }
}

boost::any GmmVtkDataArray::getItem(size_t idx) {
  return getVector(idx);
}

std::vector<dist::Variant> GmmVtkDataArray::getVector(size_t idx) {
  int components = this->getNumComponents();
  std::vector<dist::Variant> v( components );
  for (int c = 0; c < components; c++ )
  {
    std::vector<GMMTuple> models(arrays.size()/3) ;
    for (size_t i=0; i<arrays.size(); i++) {
      models[i/3].p[i%3] = arrays[i]->GetComponent(idx, c);
    }
    v[c] = GaussianMixture<MAX_GMMs>(models);
  }
  return v;
}

void GmmVtkDataArray::setItem(size_t idx, int component, const boost::any &item) {
  // not tested
  GaussianMixture<MAX_GMMs> gmm = boost::any_cast<GaussianMixture<MAX_GMMs> >( item );
  for (size_t i=0; i<arrays.size(); i++) {
    arrays[i]->SetComponent(idx, component, gmm.models[i/3].p[i%3]);
  }
}

dist::Variant GmmVtkDataArray::getScalar(size_t idx) {
  std::vector<GMMTuple> models ( arrays.size()/3 );
  for (size_t i=0; i<arrays.size(); i++) {
    models[i/3].p[i%3] = arrays[i]->GetComponent(idx, 0);
  }
  return GaussianMixture<MAX_GMMs>(models);
}


std::shared_ptr<GmmArray> GmmVtkDataArray::genNdArray() {
  //int n= arrays[i]->GetNumberOfTuples();
  //NdArray ndarray({length, arrays.size()});
  std::vector<NdArray<Real> > data(arrays.size());
  int n = length; // array length

  for (size_t i=0; i<arrays.size(); i++) {

    vtkFloatArray *farray = vtkFloatArray::SafeDownCast( arrays[i].Get() );
    vtkDoubleArray *darray = vtkDoubleArray::SafeDownCast( arrays[i].Get() );

    // Sometimes Real type that edda is using can be different from vtk array type
    // The following automatically converts vtk Float/Double Array to Real type

    if ((farray && sizeof(float) == sizeof(Real))   // alternative: typeid(Real) == typeid(float)
        || (darray && sizeof(double)==sizeof(Real)) ) {
      NdArray<Real> ndarray((Real *)arrays[i]->GetVoidPointer(0), {n} );

      data[i].take(ndarray);
    } else if ( sizeof(float) == sizeof(Real) ){
      // create a temp array in float
      vtkFloatArray *newArray = vtkFloatArray::New();
      newArray->DeepCopy(arrays[i]);
      NdArray<Real> ndarray((Real *)newArray->GetVoidPointer(0), {n} );
      data[i].take(ndarray);
      newArray->Delete();

    } else if ( sizeof(double) == sizeof(Real) ) {
      // create a temp array in double
      vtkDoubleArray *newArray = vtkDoubleArray::New();
      newArray->DeepCopy(arrays[i]);
      NdArray<Real> ndarray((Real *)newArray->GetVoidPointer(0), {n} );
      data[i].take(ndarray);
      newArray->Delete();

    } else {
      throw std::runtime_error("Real type not float or double.");
    }
  }
  return std::shared_ptr<GmmArray> ( new GmmArray(data) );

}
}; //edda
