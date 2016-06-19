#include <memory>
#include <vtkDataSet.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLStructuredGridReader.h>
#include <vtkImageData.h>

#include "io/path.h"
#include "dataset/abstract_sampling_array.h"

#include "io/gmm_vtk_data_array.h"
#include "vtk/vtk_common.h"
#include "io/edda_reader.h"

using namespace std;

namespace edda {

template <typename T>
shared_ptr<Dataset<T> > loadEddaRandomSamplingDataset(const string &edda_file, const string &array_name)
{
  string ext = getFileExtension(edda_file);
  if (ext.compare("vti")==0) {
     vsp_new(vtkXMLImageDataReader, reader);
     reader->SetFileName(edda_file.c_str());
     reader->Update();
     vtkImageData *vtkdata = reader->GetOutput();

     int *dim = vtkdata->GetDimensions();
     printf("dim: %d %d %d\n", dim[0], dim[1], dim[2]);

     // TODO : check if gmm dataset
     shared_ptr<Dataset<T> > dataset = make_Dataset<T>(
                                   new RegularCartesianGrid(dim[0], dim[1], dim[2]),
                                   new AbstractSamplingArray( new GmmVtkDataArray( vtkdata->GetPointData(), array_name.c_str() ) )
         );
     // or is histogram dataset

     return dataset;

  } else if (ext.compare("vts")==0){ // structured grids

    vsp_new(vtkXMLStructuredGridReader, reader);
    reader->SetFileName(edda_file.c_str());
    reader->Update();
    vtkStructuredGrid *vtkdata = reader->GetOutput();

    int *dim = vtkdata->GetDimensions();
    printf("dim: %d %d %d\n", dim[0], dim[1], dim[2]);

    // TODO : check if gmm dataset
    shared_ptr<Dataset<T> > dataset = make_Dataset<T>(
                                  new RegularCartesianGrid(dim[0], dim[1], dim[2]), // TODO: Curvilinear grids
                                  new AbstractSamplingArray( new GmmVtkDataArray( vtkdata->GetPointData(), array_name.c_str() ) )
        );
    // or is histogram dataset

    return dataset;
  } else {
    printf("File format of %s not supported\n", edda_file.c_str());
    exit(1);
  }
}

shared_ptr<Dataset<Real> > loadEddaScalarDataset(const string &edda_file, const string &array_name)
{
  return loadEddaRandomSamplingDataset<Real>(edda_file, array_name);
}

shared_ptr<Dataset<VECTOR3> > loadEddaVector3Dataset(const string &edda_file, const string &array_name)
{
  return loadEddaRandomSamplingDataset<VECTOR3>(edda_file, array_name);
}


} // edda
