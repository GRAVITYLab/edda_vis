// Copyright 2015 The Edda Authors. All rights reserved.
// Use of this source code is governed by a MIT-style license that can be
// found in the LICENSE file.

/// Read a distribution data set and generate a new field of random samples.

#include <string>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <algorithm>

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

#include <vtkPointData.h>

#include "distributions/gaussian.h"
#include "distributions/distribution.h"
#include "io/file_reader.h"
#include "io/file_writer.h"
#include "io/path.h"
#include "vtk/vtk_common.h"
#include "vtk/gmm_vtk_data_array.h"
#include "dataset/dataset.h"
#include "dataset/data_array.h"

using namespace std;
using namespace edda;

void process_vtk_file(string &vtk_file, int res)
{
  vsp_new(vtkStructuredGrid, sgrid1);
  vsp_new(vtkImageData, image1);
  vtkDataSet *dataset;
  if (getFileExtension(vtk_file).compare("vts")==0) {
    vsp_new(vtkXMLStructuredGridReader, reader);
    reader->SetFileName(vtk_file.c_str());
    reader->Update();
    sgrid1->DeepCopy(reader->GetOutput());
    dataset = sgrid1.Get();

    int *dim = sgrid1->GetDimensions();
    int newdim[3] = {(dim[0]-1)*res+1, (dim[1]-1)*res+1, (dim[2]-1)*res+1};
    printf("dim: %d %d %d\n", dim[0], dim[1], dim[2]);

    //vtkFloatArray *floatArray = vtkFloatArray::New();
    //floatArray->DeepCopy(sgrid1->GetPoints());
    shared_ary<VECTOR3> array(sgrid1->GetPoints()->GetNumberOfPoints());
    VECTOR3 *pf = (VECTOR3 *)sgrid1->GetPoints()->GetVoidPointer(0);
    std::copy(pf , pf+array.getLength(), array.get());
    Dataset<VECTOR3> pointsDataset(new RegularCartesianGrid(dim[0], dim[1], dim[2]),
                                   new VectorArray<float, 3>(array) );

    Dataset<Real> valuesDataset(new RegularCartesianGrid(dim[0], dim[1], dim[2]),
                                  new DataSamplingArray(new GmmVtkDataArray(sgrid1->GetPointData() ))
        );

    int newsize = newdim[0]*newdim[1]*newdim[2];
    vsp_new(vtkPoints, points);
    points->SetNumberOfPoints(newsize);
    vsp_new(vtkFloatArray, sampledArray);
    sampledArray->SetNumberOfComponents(1);
    sampledArray->SetNumberOfTuples(newsize);
    sampledArray->SetName("RandomSample");
    int i,j,k;
    int count = 0;
    for (k=0; k<newdim[2]; k++)
      for (j=0; j<newdim[1]; j++)
        for (i=0; i<newdim[0]; i++)
        {
          VECTOR3 p(i/(float)res, j/(float)res, k/(float)res),
                  v;
          ReturnStatus r = pointsDataset.at_phys(p, v);
          if (r!=SUCCESS) {
            printf("Error in interpolating point [%d %d %d]\n", i,j,k);
          }
          if(v[0]==0 && v[1]==0 && v[2]==0) {
            printf("%d %d %d\n", i,j,k);
          }
          points->SetPoint(count, v[0], v[1], v[2]);

          Real val;
          r = valuesDataset.at_phys(p, val);
          if (r!=SUCCESS) {
            printf("Error in interpolating value [%d %d %d]\n", i,j,k);
          }
          sampledArray->SetTuple1(count, val);
          count++;
        }

    vsp_new(vtkStructuredGrid, output);
    output->SetDimensions(newdim);
    output->SetPoints(points);
    output->GetPointData()->AddArray(sampledArray);


    printf("Saving output file: sampling.vts\n");
    vsp_new(vtkXMLStructuredGridWriter, writer);
    writer->SetFileName("sampling.vts");
    writer->SetInputData(output);
    writer->Write();
  } else if (getFileExtension(vtk_file).compare("vti")==0) {
    vsp_new(vtkXMLImageDataReader, reader);
    reader->SetFileName(vtk_file.c_str());
    reader->Update();
    image1->DeepCopy( reader->GetOutput() );
    dataset = image1.Get();

    vsp_new(vtkImageData, output);

    printf("Saving output file: sampling.vti\n");
    vsp_new(vtkXMLImageDataWriter, writer);
    writer->SetFileName("sampling.vti");
    writer->SetInputData(output);
    writer->Write();


  } else {
    printf("File format not supported\n");
    exit(1);
  }


  // create denser grids


}


int main(int argc, char **argv) {
  srand(time(NULL));

  cout << "denseSamplingField <file> <resolution> " << endl;
  if (argc<=2)
    return -1;
  string input_file = argv[1];

  process_vtk_file(input_file, atoi(argv[2]));

}
