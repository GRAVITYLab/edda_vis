#include <iostream>
#include <cstring>

#include "eddaUncertainIsocontour.h"

#include "vtk_common.h"
#include "vtkCell.h"
#include "vtkCellData.h"
#include "vtkCharArray.h"
#include "vtkIdTypeArray.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkStreamingDemandDrivenPipeline.h"

#include "io/gmm_vtk_data_array.h"

// Compile with C++ compiler if Cuda is not used
#include "thrust/device_vector.h"
#if THRUST_DEVICE_SYSTEM!=THRUST_DEVICE_SYSTEM_CUDA
#include "eddaUncertainIsocontour.cu"
#endif



using namespace std;
using namespace edda;

vtkStandardNewMacro(eddaUncertainIsocontour)

//----------------------------------------------------------------------------
eddaUncertainIsocontour::eddaUncertainIsocontour()
{
  this->SetNumberOfInputPorts(1);
  this->SetNumberOfOutputPorts(1);
}

//----------------------------------------------------------------------------
eddaUncertainIsocontour::~eddaUncertainIsocontour()
{
}


void eddaUncertainIsocontour::InitializeData(vtkDataSet* input,
  vtkDataSet* output)
{
  // First, copy the input to the output as a starting point
  output->CopyStructure(input);
}

//----------------------------------------------------------------------------
int eddaUncertainIsocontour::RequestData(
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

  // check whether input is structured grids or image data
  int *dim = NULL;
  vtkImageData *image = vtkImageData::SafeDownCast(input);
  vtkStructuredGrid *sgrid = vtkStructuredGrid::SafeDownCast(input);
  if (image) {
    dim = image->GetDimensions();
  }
  else if (sgrid) {
    dim = sgrid->GetDimensions();
  }
  else {
    return 0;
  }

  this->InitializeData(input, output);
  this->Compute(input, dim, output);

  //output->PrintSelf(cout, vtkIndent(0));

  return 1;
}

//----------------------------------------------------------------------------
int eddaUncertainIsocontour::RequestInformation(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(),
               inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT()),
               6);

  return 1;
}

//----------------------------------------------------------------------------
int eddaUncertainIsocontour::RequestUpdateExtent(
  vtkInformation *vtkNotUsed(request),
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  // get the info objects
  vtkInformation *inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation *outInfo = outputVector->GetInformationObject(0);

  int usePiece = 0;

  // What ever happened to CopyUpdateExtent in vtkDataObject?
  // Copying both piece and extent could be bad.  Setting the piece
  // of a structured data set will affect the extent.
  vtkDataObject* output = outInfo->Get(vtkDataObject::DATA_OBJECT());
  if (output &&
      (!strcmp(output->GetClassName(), "vtkUnstructuredGrid") ||
       !strcmp(output->GetClassName(), "vtkPolyData")))
    {
    usePiece = 1;
    }

  inInfo->Set(vtkStreamingDemandDrivenPipeline::EXACT_EXTENT(), 1);

    inInfo->Set(
      vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(),
      outInfo->Get(vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT()), 6);

  return 1;
}

//----------------------------------------------------------------------------
void eddaUncertainIsocontour::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

