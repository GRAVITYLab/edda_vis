#include <math.h>

#include "vtkDataSet.h"
#include "vtkUnstructuredGrid.h"
#include "vtkStreamingDemandDrivenPipeline.h"
#include "vtkImageData.h"
#include "vtkStructuredGrid.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"

#include "io/gmm_vtk_data_array.h"

#include "eddaThreshold.h"

using namespace edda;
using namespace std;

vtkStandardNewMacro(eddaThreshold)

eddaThreshold::eddaThreshold()
{
  this->UpperThreshold = INFINITY;
  this->LowerThreshold = -INFINITY;
}

eddaThreshold::~eddaThreshold()
{

}


// Usual data generation method
int eddaThreshold::RequestData(
    vtkInformation *vtkNotUsed(request),
    vtkInformationVector **inputVector,
    vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  // get the input and output
  vtkDataSet *input = vtkDataSet::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkDataSet *output = vtkDataSet::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  // First, copy the input to the output as a starting point
  output->CopyStructure(input);

  // process point data
  shared_ptr<GmmVtkDataArray> dataArray(new GmmVtkDataArray(input->GetPointData()) );
  //shared_ptr<GmmNdArray> gmmNdArray = dataArray->genNdArray();

  // has point data?
  if (dataArray->getLength() > 0) {
    int out_length = dataArray->getLength();

    // create output array
    vsp_new(vtkFloatArray, out_vtkArray);
    out_vtkArray->SetNumberOfComponents(1);
    out_vtkArray->SetNumberOfTuples(out_length);
    out_vtkArray->SetName("ThresholdProbability");
    // copy from device to host
    for (int i=0; i<out_length; i++)
    {
      out_vtkArray->SetTuple1(i, dist::getCdf(dataArray->getScalar(i), this->UpperThreshold)
                                -dist::getCdf(dataArray->getScalar(i), this->LowerThreshold));
    }

    output->GetPointData()->AddArray(out_vtkArray);

    this->Modified();
  }

  return 1;

}


int eddaThreshold::FillInputPortInformation(int port, vtkInformation *info) {
  info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkDataSet");
  return 1;
}

//----------------------------------------------------------------------------
void eddaThreshold::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
