// Copyright 2015 The Edda Authors. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

/// Read a distribution data set and generate a new field of random samples.

#include <string>
#include <cstdlib>
#include <cstdio>
#include <iostream>

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
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridWriter.h>

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
#include "vtk/vtk_common.h"
#include "vtk/eddaRandomProbeFilter.h"
#include "vtk/eddaRandomSampleField.h"

using namespace std;
using namespace edda;

vtkSmartPointer<vtkDataSet> process_vtk_file(string &vtk_file, string &source_file)
{
  vsp_new(vtkStructuredGrid, sgrid1);
  vsp_new(vtkImageData, image1);
  vsp_new(vtkUnstructuredGrid, ugrid1);
  vtkDataSet *dataset;
  if (getFileExtension(vtk_file).compare("vts")==0) {
    vsp_new(vtkXMLStructuredGridReader, reader);
    reader->SetFileName(vtk_file.c_str());
    reader->Update();
    sgrid1->DeepCopy(reader->GetOutput());
    dataset = sgrid1.Get();

  } else if (getFileExtension(vtk_file).compare("vti")==0) {
    vsp_new(vtkXMLImageDataReader, reader);
    reader->SetFileName(vtk_file.c_str());
    reader->Update();
    image1->DeepCopy( reader->GetOutput() );
    dataset = image1.Get();
  }  else if (getFileExtension(vtk_file).compare("vtu")==0) {
    vsp_new(vtkXMLUnstructuredGridReader, reader);
    reader->SetFileName(vtk_file.c_str());
    reader->Update();
    ugrid1->DeepCopy( reader->GetOutput() );
    dataset = ugrid1.Get();
  } else {
    printf("File format not supported\n");
    exit(1);
  }


  vsp_new(vtkStructuredGrid, sgrid2);
  vsp_new(vtkImageData, image2);
  vtkDataSet * dataset2 = NULL;
  if (getFileExtension(source_file).compare("vts")==0) {
    vsp_new(vtkXMLStructuredGridReader, reader);
    reader->SetFileName(source_file.c_str());
    reader->Update();
    sgrid2->DeepCopy(reader->GetOutput());
    dataset2 = sgrid2.Get();

  } else if (getFileExtension(source_file).compare("vti")==0) {
    vsp_new(vtkXMLImageDataReader, reader);
    reader->SetFileName(source_file.c_str());
    reader->Update();
    image2->DeepCopy( reader->GetOutput() );
    dataset2 = image2.Get();

  }

  vsp_new(eddaRandomSampleField , randomSample);
  vsp_new(eddaRandomProbeFilter, randomProbe);
  vtkDataSet * output;
  if (dataset2==NULL)  {// test vtkRandomSampleField
    // Edda filter: random sample field
    randomSample->SetInputData(dataset);
    randomSample->Update();
    output = randomSample->GetOutput() ;

  } else { // test vtkRandomProbeFilter

    randomProbe->SetTolerance(1e-10);
    randomProbe->ComputeToleranceOff();
    randomProbe->SetInputData(dataset);
    randomProbe->SetSourceData(dataset2);
    randomProbe->Update();
    output = randomProbe->GetOutput();
  }

  // cell data to point data
  vsp_new(vtkCellDataToPointData, cell2point);
  cell2point->SetInputData(output);
  cell2point->Update();

  cell2point->GetOutput()->PrintSelf(cout, vtkIndent(0));

  if (getFileExtension(vtk_file).compare("vti")==0) {
    printf("Saving output file: sampling.vti\n");
    vsp_new(vtkXMLImageDataWriter, writer);
    writer->SetFileName("sampling.vti");
    writer->SetInputData(output);
    writer->Write();
  } else if (getFileExtension(vtk_file).compare("vts")==0) {
    printf("Saving output file: sampling.vts\n");
    vsp_new(vtkXMLStructuredGridWriter, writer);
    writer->SetFileName("sampling.vts");
    writer->SetInputData(output);
    writer->Write();
    exit(0);
  } else if (getFileExtension(vtk_file).compare("vtu")==0) {
    printf("Saving output file: sampling.vtu\n");
    vsp_new(vtkXMLUnstructuredGridWriter, writer);
    writer->SetFileName("sampling.vtu");
    writer->SetInputData(output);
    writer->Write();
    exit(0);

  }

  return cell2point->GetOutput();
}


int main(int argc, char **argv) {
  cout << "randomSample <vtk file> [<optional sampling geometry in vtk>]" << endl;
  if (argc<=1)
    return -1;
  string input_file = argv[1];
  string source_file;
  if (argc>2)
    source_file = argv[2];


  vtkSmartPointer<vtkDataSet> probField;
  probField = process_vtk_file(input_file, source_file);

#if 0 // Following codes not working after new vtk5
  // Volume render
  vsp_new(vtkPiecewiseFunction, alphaChannelFunc);
  alphaChannelFunc->AddPoint(0, 0);
  alphaChannelFunc->AddPoint(1., 1.);

  vsp_new(vtkColorTransferFunction, colorFunc);
  colorFunc->AddRGBPoint(0, 0.0, 0.0, 1.0);
  colorFunc->AddRGBPoint(.5, 1.0, 1.0, 1.0);
  colorFunc->AddRGBPoint(1., 1.0, 0.0, 0.0);

  vsp_new(vtkVolumeProperty, volumeProperty);
  volumeProperty->ShadeOff();
  volumeProperty->SetInterpolationType(VTK_LINEAR_INTERPOLATION);
  volumeProperty->SetColor(colorFunc);
  volumeProperty->SetScalarOpacity(alphaChannelFunc);

  vsp_new(vtkSmartVolumeMapper, volumeMapper);
  volumeMapper->SetBlendModeToComposite(); // composite first
  volumeMapper->SetInputData(probField);

  vsp_new(vtkVolume, volume);
  volume->SetMapper(volumeMapper);
  volume->SetProperty(volumeProperty);

  // outline
  vsp_new(vtkOutlineFilter, outline);
  outline->SetInputData(probField);
  vsp_new(vtkPolyDataMapper, outlineMapper);
  outlineMapper->SetInputConnection(outline->GetOutputPort());
  vsp_new(vtkActor, outlineActor);
  outlineActor->SetMapper(outlineMapper);
  outlineActor->GetProperty()->SetColor(.5,.5,.5);

  // Render window
  vsp_new(vtkRenderer, renderer);
  renderer->AddViewProp(volume);
  renderer->AddActor(outlineActor);
  renderer->SetBackground(0, 0, 0);
  renderer->ResetCamera();

  vsp_new(vtkRenderWindow, renderWin);
  renderWin->AddRenderer(renderer);
  vsp_new(vtkRenderWindowInteractor, renderInteractor);
  renderInteractor->SetRenderWindow(renderWin);

  vsp_new(vtkInteractorStyleTrackballCamera, style);
  renderInteractor->SetInteractorStyle(style);

  renderWin->SetSize(400, 400);
  renderInteractor->Initialize();

  renderWin->Render();

  // 3D texture mode. For coverage.
  //volumeMapper->SetRequestedRenderModeToRayCastAndTexture();
  //renderWin->Render();

  // Software mode, for coverage. It also makes sure we will get the same
  // regression image on all platforms.
  //volumeMapper->SetRequestedRenderModeToRayCast();
  //renderWin->Render();

  renderInteractor->Start();
#endif
}
