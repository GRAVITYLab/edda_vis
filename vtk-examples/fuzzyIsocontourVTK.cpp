// Copyright 2015 The Edda Authors. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

#include <string>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <memory>

#include <vtkPiecewiseFunction.h>
#include <vtkSmartPointer.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolumeProperty.h>
#include <vtkVolume.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkColorTransferFunction.h>
#include <vtkOutlineFilter.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLStructuredGridReader.h>
#include <vtkCellDataToPointData.h>
#include <vtkXMLStructuredGridWriter.h>

#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>

#include "distributions/gaussian.h"
#include "distributions/distribution.h"
#include "io/file_reader.h"
#include "io/file_writer.h"
#include "io/path.h"
#include "filters/level_crossing_prob.h"
#include "vtk/vtk_common.h"
#include "vtk/eddaUncertainIsocontour.h"
#include <io/gmm_vtk_data_array.h>
#include "core/gmm_array.h"
#include "filters/fuzzy_isosurface.h"


using namespace std;
using namespace edda;

vtkSmartPointer<vtkDataSet> process_vtk_file(string vtk_file, float isov)
{
  int *dim;
  vtkSmartPointer <vtkDataSet> dataset;
  if (getFileExtension(vtk_file).compare("vts")==0) {
    vsp_new(vtkXMLStructuredGridReader, reader);
    reader->SetFileName(vtk_file.c_str());
    reader->Update();
    dataset = reader->GetOutput();
    dim = vtkStructuredGrid::SafeDownCast(dataset)->GetDimensions();

  } else if (getFileExtension(vtk_file).compare("vti")==0) {
    vsp_new(vtkXMLImageDataReader, reader);
    reader->SetFileName(vtk_file.c_str());
    reader->Update();
    dataset = reader->GetOutput();
    dim = vtkImageData::SafeDownCast(dataset)->GetDimensions();

  } else {
    printf("File format not supported\n");
    exit(1);
  }


  // process point data
  shared_ptr<GmmVtkDataArray> dataArray(new GmmVtkDataArray(dataset->GetPointData()) );
  shared_ptr<GmmArray> gmmArray = dataArray->genNdArray();

  shared_ptr<NdArray<float> > out_ndarray;

  edda::FuzzyIsosurface(gmmArray->begin(), dim, isov, out_ndarray);

    // create output array
  vsp_new(vtkFloatArray, out_vtkArray);
  out_vtkArray->SetNumberOfComponents(1);
  out_vtkArray->SetNumberOfTuples(dim[0]*dim[1]*dim[2]);
  out_vtkArray->SetName("ProbField");
  // copy from device to host
  out_ndarray->copy_to_host((float *)out_vtkArray->GetVoidPointer(0));

  dataset->GetPointData()->AddArray(out_vtkArray);


  printf("Saving output file\n");
  if (getFileExtension(vtk_file).compare("vti")==0) {
    vsp_new(vtkXMLImageDataWriter, writer);
    writer->SetFileName("ProbField.vti");
    writer->SetInputData(dataset);
    writer->Write();
  } else {
    vsp_new(vtkXMLStructuredGridWriter, writer);
    writer->SetFileName("ProbField.vts");
    writer->SetInputData(dataset);
    writer->Write();
    exit(0);
  }

  return dataset;
}


int main(int argc, char **argv) {
  cout << "fuzzyIsocontour <vts/vti file> <iso-value>" << endl;
  if (argc<=2)
    return -1;
  string input_file = argv[1];
  float isov = atof(argv[2]);

  vtkSmartPointer<vtkDataSet> probField;
  if (getFileExtension(input_file).compare("vts")==0 || getFileExtension(input_file).compare("vti")==0) {
    probField = process_vtk_file(input_file, isov);
  }
  else {
    printf("File format Unsupported \n");
    //probField = process_info_file(input_file, isov);
  }


}
