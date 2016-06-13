#include <iostream>
#include <vtkSmartPointer.h>
#include <vtkXMLStructuredGridReader.h>
#include <vtkStructuredGrid.h>

#include "vtk/eddaThreshold.h"

#include "edda.h"

using namespace std;

#define vsp_new(cls,x) vtkSmartPointer<cls> x = vtkSmartPointer<cls>::New()


void test_threshold()
{
  vsp_new(vtkXMLStructuredGridReader, reader);
  reader->SetFileName(SAMPLE_DATA_PATH "/out_92651_0.vts");
  reader->Update();

  vtkStructuredGrid *sgrid = reader->GetOutput();

  vsp_new(eddaThreshold, thresholding);
  thresholding->SetInputData(sgrid);
  thresholding->ThresholdBetween(0, 1.);
  thresholding->Update();
}

int main(int argc, char *argv[])
{

  test_threshold();

  return 0;
}
