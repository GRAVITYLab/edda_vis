// cuda related codes only
#include <iostream>

#include "eddaRandomSampleField.h"

#include "common.h"
#include "vtk_common.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include "io/gmm_vtk_data_array.h"
#include "filters/random_sample_field.h"

using namespace std;
using namespace edda;


void eddaRandomSampleField::SampleDataArray(shared_ptr<GmmVtkDataArray> dataArray, vtkSmartPointer<vtkFieldData> output_field)
{
  // create output array
  vsp_new(vtkFloatArray, out_vtkArray);
  out_vtkArray->SetNumberOfComponents(dataArray->getNumComponents());
  out_vtkArray->SetNumberOfTuples(dataArray->getLength());
  out_vtkArray->SetName(ResultName.c_str());

#if 1 // thrust
  shared_ptr<GmmArray> gmmArray = dataArray->genNdArray();
  thrust::device_vector<Real> out(dataArray->getLength());

  randomSampleField(gmmArray->begin(), gmmArray->end(), out.begin());

  thrust::copy(out.begin(), out.end(), (float *)out_vtkArray->GetVoidPointer(0));

#else // sequential
  float *p = (float *)out_vtkArray->GetVoidPointer(0);
  int nc = dataArray->getNumComponents();
#pragma omp parallel for
  for (size_t i=0; i<dataArray->getLength(); i++)
  {
    if (nc==1) {
      p[i*nc] = dist::getSample( dataArray->getScalar(i) );
    } else {
      std::vector< dist::Variant > v = dataArray->getVector(i);
      for (int c=0; c<nc; c++)
        p[i*nc+c] = dist::getSample( v[c] );
    }
  }
#endif

  output_field->AddArray(out_vtkArray);
}
