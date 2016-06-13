// Copyright 2015 The Edda Authors. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <iterator>

#include "vtkDataSet.h"
#include "vtkSmartPointer.h"
#include "vtkLineSource.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "vtkRenderWindow.h"
#include "vtkRenderer.h"
#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkOutlineFilter.h"
#include "vtkProperty.h"
#include "vtkLineWidget.h"
#include "vtkCommand.h"
#include "vtkCallbackCommand.h"
#include "vtkPointData.h"
#include "vtkFloatArray.h"
#include "vtkSphereSource.h"

#include "vtkCubeSource.h"
#include "vtkLine.h"
#include "vtkIdList.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"

#include "vtkAppendPolyData.h"
#include "vtkGlyph3D.h"
#include "vtkGlyph3DMapper.h"

#include <edda.h>
#include <distributions/gaussian.h>
#include <distributions/distribution.h>
#include <io/file_reader.h>
#include <filters/stream_tracer.h>
#include <dataset/dataset.h>

#include "vtkTransformFilter.h"
#include "vtkTransform.h"

using namespace std;
using namespace edda;

#define vsp_new(cls,x) vtkSmartPointer<cls> x = vtkSmartPointer<cls>::New()

vtkSmartPointer<vtkLineWidget> lineWidget;
vtkSmartPointer<vtkRenderWindow> renWin;
vtkSmartPointer<vtkPolyData> streamlines;

shared_ptr<StreamTracer<Gaussian3, GetPositionFromDistributionMean> > streamTracer;

void computeStreamlines(vtkSmartPointer<vtkPolyData> vtk_seeds)
{
  int i;
  // convert seeds from vtk
  list<Gaussian3> seeds;
  for (i=0; i<vtk_seeds->GetNumberOfPoints(); i++)
  {
    double p[3];
    vtk_seeds->GetPoint(i, p);
    // add a particle location with zero mean
    seeds.push_back(Gaussian3(
                      dist::Gaussian(p[0], 0),
                      dist::Gaussian(p[1], 0),
                      dist::Gaussian(p[2], 0) )
        );
  }

  list<list<Gaussian3> >traces;
  streamTracer->step_size = 0.05;
  streamTracer->max_steps = 500;
  streamTracer->compute(seeds, traces);

  // convert traces to vtk
  vtkPoints *points = vtkPoints::New();
  vtkCellArray *cells = vtkCellArray::New();
  vtkFloatArray *fieldData = vtkFloatArray::New();
  fieldData->SetNumberOfComponents(1);
  vtkFloatArray *uncertainty = vtkFloatArray::New();
  uncertainty->SetName("uncertainty");
  uncertainty->SetNumberOfComponents(3);
  int count = 0;
  for (auto traceItr = traces.begin(); traceItr != traces.end(); ++traceItr)
  {
    auto &singleTrace = *traceItr;
    vtkIdList *ids = vtkIdList::New();
    for (auto posItr = singleTrace.begin(); posItr!=singleTrace.end(); ++posItr)
    {
      Gaussian3 &p = *posItr;

      int id = points->InsertNextPoint(p[0].mean, p[1].mean, p[2].mean);
      ids->InsertNextId(id);

      // get 97.5% percentile
      uncertainty->InsertNextTuple3(
          sqrt(p[0].var)*1.96,
          sqrt(p[1].var)*1.96,
          sqrt(p[2].var)*1.96 );

      fieldData->InsertNextTuple1(count);
    }
    cells->InsertNextCell(ids);
    count ++;
  }
  streamlines->SetPoints(points);
  streamlines->SetLines(cells);
  streamlines->GetPointData()->SetScalars(fieldData);
  streamlines->GetPointData()->SetVectors(uncertainty);
}

void updateSeeds(vtkObject* caller, unsigned long eventId, void *clientdata, void *calldata)
{
  vsp_new(vtkPolyData, seeds) ;
  lineWidget->GetPolyData(seeds);
  computeStreamlines(seeds);
  renWin->Render();
}


int main(int argc, char **argv)
{
  cout << "Input arguments: <info file>" << endl;
  if (argc<2)
      return -1;
  string filename;
  filename = argv[1];

  cout << "Press 'i' to change the rake\n";

  // load data with random sampling
  shared_ptr<Dataset<Gaussian3> > dataset = loadVectorData<Gaussian3>(filename);

  VECTOR3 minB, maxB;
  dataset->getGrid()->boundary(minB, maxB);

  // create stream tracer
  new StreamTracer<Gaussian3, GetPositionFromDistributionMean>(dataset);
  streamTracer = shared_ptr<StreamTracer<Gaussian3, GetPositionFromDistributionMean> >
      (new StreamTracer<Gaussian3, GetPositionFromDistributionMean>(dataset) );

  // create a dummy vtk bounding box
  vsp_new(vtkCubeSource, cube);
  cube->SetBounds(minB[0], maxB[0], minB[1], maxB[1], minB[2], maxB[2]);
  cube->Update();

  // user can change the seeding rake
  lineWidget = vtkSmartPointer<vtkLineWidget>::New();
  lineWidget->SetInputData(cube->GetOutput());
  lineWidget->SetResolution(19); // 20 seeds along the line
  lineWidget->SetAlignToYAxis();
  lineWidget->PlaceWidget();
  lineWidget->ClampToBoundsOn();

  vsp_new(vtkPolyData, seeds);
  lineWidget->GetPolyData(seeds);

  streamlines = vtkSmartPointer<vtkPolyData>::New();

  computeStreamlines(seeds);

  //
  // render Streamlines
  //
  vsp_new(vtkPolyDataMapper, mapper);
  mapper->SetScalarRange(0, 20); // for 20 seeds
  mapper->SetInputData(streamlines);
  mapper->SetColorModeToMapScalars();
  vsp_new(vtkActor, actor);
  actor->SetMapper(mapper);

  //
  // glyph for uncertainty range
  //
  vsp_new(vtkGlyph3DMapper, uncertaintyMapper);
  uncertaintyMapper->SetInputData(streamlines);
  uncertaintyMapper->SetColorModeToMapScalars();
  uncertaintyMapper->SetScaleModeToScaleByVectorComponents();
  uncertaintyMapper->OrientOff();
  uncertaintyMapper->SetScaleArray("uncertainty");
  uncertaintyMapper->SetScaleFactor(1.);
  uncertaintyMapper->SetScalarRange(0, 20);

  vsp_new(vtkSphereSource, sphere);
  sphere->SetPhiResolution(8);
  sphere->SetThetaResolution(8);
  uncertaintyMapper->SetSourceConnection(sphere->GetOutputPort());

  vsp_new(vtkActor, uncertaintyActor);
  uncertaintyActor->SetMapper(uncertaintyMapper);

  //
  // outline
  //
  vsp_new(vtkOutlineFilter, outline);
  outline->SetInputData(cube->GetOutput());

  vtkPolyDataMapper *outlineMapper = vtkPolyDataMapper::New();
  outlineMapper->SetInputConnection(outline->GetOutputPort());

  vtkActor *outlineActor = vtkActor::New();
  outlineActor->SetMapper(outlineMapper);
  outlineActor->GetProperty()->SetColor(.5,.5,.5);

  //
  // renderer
  //
  vsp_new(vtkRenderer, ren);
  renWin = vtkSmartPointer<vtkRenderWindow>::New();
  renWin->AddRenderer(ren);
  vsp_new(vtkRenderWindowInteractor, iren);
  iren->SetRenderWindow(renWin);
  vsp_new(vtkInteractorStyleTrackballCamera, style);
  iren->SetInteractorStyle(style);

  // line widget interactor
  lineWidget->SetInteractor(iren);
  lineWidget->SetDefaultRenderer(ren);
  vsp_new(vtkCallbackCommand, callback );
  callback->SetCallback(updateSeeds);
  lineWidget->AddObserver(vtkCommand::EndInteractionEvent, callback);

  ren->AddActor(actor);
  ren->AddActor(uncertaintyActor);
  ren->AddActor(outlineActor);
  ren->SetBackground(0,0,0);
  ren->ResetCamera();

  renWin->SetSize(500,500);

  iren->Initialize();
  renWin->Render();
  iren->Start();

  return 0;
}



