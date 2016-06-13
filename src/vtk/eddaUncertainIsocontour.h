#ifndef eddaUncertainIsocontour_h
#define eddaUncertainIsocontour_h

#include "vtkFiltersCoreModule.h" // For export macro
#include "vtkDataSetAlgorithm.h"
#include "vtkDataSetAttributes.h" // needed for vtkDataSetAttributes::FieldList

class vtkIdTypeArray;
class vtkCharArray;
class vtkMaskPoints;

class VTKFILTERSCORE_EXPORT eddaUncertainIsocontour : public vtkDataSetAlgorithm
{
public:
  static eddaUncertainIsocontour *New();
  vtkTypeMacro(eddaUncertainIsocontour,vtkDataSetAlgorithm)
  void PrintSelf(ostream& os, vtkIndent indent);

  ///
  /// \brief Get isovalue
  ///
  vtkGetMacro(Isov, double)

  ///
  /// \brief Set isovalue
  ///
  vtkSetMacro(Isov, double)

protected:
  double Isov;

  eddaUncertainIsocontour();
  ~eddaUncertainIsocontour();

  virtual int RequestData(vtkInformation *, vtkInformationVector **,
    vtkInformationVector *);
  virtual int RequestInformation(vtkInformation *, vtkInformationVector **,
    vtkInformationVector *);
  virtual int RequestUpdateExtent(vtkInformation *, vtkInformationVector **,
    vtkInformationVector *);

  virtual void InitializeData(vtkDataSet* input,
                              vtkDataSet* output);

  virtual void Compute(vtkDataSet* input, int *dim,
                              vtkDataSet* output);

private:
  eddaUncertainIsocontour(const eddaUncertainIsocontour&);  // Not implemented.
  void operator=(const eddaUncertainIsocontour&);  // Not implemented.

};

#endif // eddaUncertainIsocontour_h
