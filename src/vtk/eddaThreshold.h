#ifndef eddaThreshold_h
#define eddaThreshold_h

#include "vtkDataSetAlgorithm.h"

class vtkDataArray;
class vtkIdList;

class eddaThreshold : public vtkDataSetAlgorithm
{
public:
  static eddaThreshold *New();
  vtkTypeMacro(eddaThreshold,vtkDataSetAlgorithm)
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Criterion is cells whose scalars are less or equal to lower threshold.
  void ThresholdByLower(double lower) {
    this->LowerThreshold = lower;
    this->Modified();
  }

  // Description:
  // Criterion is cells whose scalars are greater or equal to upper threshold.
  void ThresholdByUpper(double upper) {
    this->UpperThreshold = upper;
    this->Modified();
  }

  // Description:
  // Criterion is cells whose scalars are between lower and upper thresholds
  // (inclusive of the end values).
  void ThresholdBetween(double lower, double upper) {
    this->LowerThreshold = lower;
    this->UpperThreshold = upper;
    this->Modified();
  }

  // Description:
  // Get the Upper and Lower thresholds.
  vtkGetMacro(UpperThreshold,double)
  vtkSetMacro(UpperThreshold,double)

  vtkGetMacro(LowerThreshold,double)
  vtkSetMacro(LowerThreshold,double)

protected:
  eddaThreshold();
  ~eddaThreshold();

  // Usual data generation method
  virtual int RequestData(vtkInformation *, vtkInformationVector **, vtkInformationVector *) ;

  virtual int FillInputPortInformation(int port, vtkInformation *info) ;

  double LowerThreshold;
  double UpperThreshold;

private:
  eddaThreshold(const eddaThreshold&);  // Not implemented.
  void operator=(const eddaThreshold&);  // Not implemented.
};

#endif // eddaThreshold_h
