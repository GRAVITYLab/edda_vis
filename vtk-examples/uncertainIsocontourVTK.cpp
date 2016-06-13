// Copyright 2015 The Edda Authors. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

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

using namespace std;
using namespace edda;

vtkSmartPointer<vtkDataSet> process_vtk_file(string vtk_file, float isov)
{
  vtkSmartPointer <vtkDataSet> dataset;
  if (getFileExtension(vtk_file).compare("vts")==0) {
    vsp_new(vtkXMLStructuredGridReader, reader);
    reader->SetFileName(vtk_file.c_str());
    reader->Update();
    dataset = reader->GetOutput();

  } else if (getFileExtension(vtk_file).compare("vti")==0) {
    vsp_new(vtkXMLImageDataReader, reader);
    reader->SetFileName(vtk_file.c_str());
    reader->Update();
    dataset = reader->GetOutput();
  } else {
    printf("File format not supported\n");
    exit(1);
  }

  // Edda filter
  vsp_new(eddaUncertainIsocontour, isocontour);
  isocontour->SetInputData(dataset);
  isocontour->SetIsov(isov);
  isocontour->Update();

  vsp_new(vtkCellDataToPointData, cell2point);
  cell2point->SetInputData(isocontour->GetOutput());
  cell2point->Update();

  cell2point->GetOutput()->PrintSelf(cout, vtkIndent(0));

  printf("Saving output file\n");
  if (getFileExtension(vtk_file).compare("vti")==0) {
    vsp_new(vtkXMLImageDataWriter, writer);
    writer->SetFileName("ProbField.vti");
    writer->SetInputData(isocontour->GetOutput());
    writer->Write();
  } else {
    vsp_new(vtkXMLStructuredGridWriter, writer);
    writer->SetFileName("ProbField.vts");
    writer->SetInputData(isocontour->GetOutput());
    writer->Write();
    exit(0);
  }

  return cell2point->GetOutput();
}

#if 0 // removed
vtkSmartPointer<vtkDataSet> process_info_file(string info_file, float isov)
{
  // load data
  shared_ptr<Dataset<dist::Gaussian> > dataset = loadData<dist::Gaussian>(info_file);

  // uncertain isocontour
  shared_ptr<Dataset<Real> > output = uncertainIsocontour(dataset, isov);

  // Convert to vtk image data
  vsp_new(vtkImageData, image);
  image->SetDimensions(output->getDimension());
  image->AllocateScalars(VTK_FLOAT,1);

  shared_ary<float> array = boost::any_cast<shared_ary<float> >( output->getArray()->getRawArray() );
  std::copy( &array[0], &array[0]+output->getArray()->getLength(),
            (float *)image->GetScalarPointer(0,0,0));

  return image;
}
#endif

int main(int argc, char **argv) {
  cout << "isoProbField <vts/vti file> <iso-value>" << endl;
  if (argc<=2)
    return -1;
  string input_file = argv[1];
  float isov = atof(argv[2]);

  vtkSmartPointer<vtkDataSet> probField;
  if (getFileExtension(input_file).compare("vts")==0 || getFileExtension(input_file).compare("vti")==0) {
    probField = process_vtk_file(input_file, isov);
  }
  else {
    //probField = process_info_file(input_file, isov);
  }


#if 0 // not working in new vtk version
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
