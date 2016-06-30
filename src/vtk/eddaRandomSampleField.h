#ifndef eddaRandomSampleField_h
#define eddaRandomSampleField_h

#include <string>
#include "vtkFiltersCoreModule.h" // For export macro
#include "vtkDataSetAlgorithm.h"
#include "vtkDataSetAttributes.h" // needed for vtkDataSetAttributes::FieldList

#include "io/gmm_vtk_data_array.h"

class vtkIdTypeArray;
class vtkCharArray;
class vtkMaskPoints;

class eddaRandomSampleField : public vtkDataSetAlgorithm
{
public:
  static eddaRandomSampleField *New();
  vtkTypeMacro(eddaRandomSampleField,vtkDataSetAlgorithm)
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  double Isov;
  std::string ResultName;
  std::string Prefix;

  eddaRandomSampleField();
  ~eddaRandomSampleField();

  virtual int RequestData(vtkInformation *, vtkInformationVector **,
    vtkInformationVector *);
  virtual int RequestInformation(vtkInformation *, vtkInformationVector **,
    vtkInformationVector *);
  virtual int RequestUpdateExtent(vtkInformation *, vtkInformationVector **,
    vtkInformationVector *);

  // non-vtk functions
  virtual void InitializeData(vtkDataSet* input,
                              vtkDataSet* output);
  virtual void SampleDataArray(std::shared_ptr<edda::GmmVtkDataArray> dataArray, vtkSmartPointer<vtkFieldData> output);

private:
  eddaRandomSampleField(const eddaRandomSampleField&);  // Not implemented.
  void operator=(const eddaRandomSampleField&);  // Not implemented.

};

#endif // eddaRandomSampleField_h
