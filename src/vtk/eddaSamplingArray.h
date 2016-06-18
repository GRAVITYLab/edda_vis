#ifndef eddaSamplingArray_h
#define eddaSamplingArray_h
#include <cstdio>
#include <vtkDataArray.h>

#include <vtkIdList.h>

#include <dataset/abstract_data_array.h>
#include <dataset/abstract_sampling_array.h>
#include <core/vector_matrix.h>
using namespace edda;

///
/// \brief The eddaSamplingArray class is a vtkDataArray that returns a random sample from the distribution array.
/// This is still experimental and unsafe to use because the grandparent class vtkAbstractArray allows GetVoidPointer() to bypass GetTuple().
///
class eddaSamplingArray : public vtkDataArray
{
  edda::AbstractDataArray *pArray;
public:
  vtkTypeMacro(eddaSamplingArray,vtkDataArray)
  static eddaSamplingArray *New();

  void SetEddaArray(edda::AbstractDataArray *array) {
    pArray = new AbstractSamplingArray( array );
    this->SetNumberOfComponents( array->getNumComponents() );
    this->SetNumberOfTuples(array->getLength());
  }

  // Description:
  // Set the tuple at the ith location using the jth tuple in the source array.
  // This method assumes that the two arrays have the same type
  // and structure. Note that range checking and memory allocation is not
  // performed; use in conjunction with SetNumberOfTuples() to allocate space.
  virtual void SetTuple(vtkIdType i, vtkIdType j, vtkAbstractArray* source)
  {
    printf("SetTuple\n");
  }

  // Description:
  // Insert the jth tuple in the source array, at ith location in this array.
  // Note that memory allocation is performed as necessary to hold the data.
  // This pure virtual function is redeclared here to avoid
  // declaration hidden warnings.
  virtual void InsertTuple(vtkIdType i, vtkIdType j, vtkAbstractArray* source)
  {
    printf("InsertTuple\n");
  }

  // Description:
  // Insert the jth tuple in the source array, at the end in this array.
  // Note that memory allocation is performed as necessary to hold the data.
  // Returns the location at which the data was inserted.
  // This pure virtual function is redeclared here to avoid
  // declaration hidden warnings.
  virtual vtkIdType InsertNextTuple(vtkIdType j, vtkAbstractArray* source)
  {
    printf("InsertTuple\n");
    return j;
  }

  // Description:
  // Given a list of point ids, return an array of tuples.
  // You must insure that the output array has been previously
  // allocated with enough space to hold the data.
  void GetTuples(vtkIdList *ptIds, vtkAbstractArray *aa)
  {
    vtkDataArray *outArray = vtkDataArray::FastDownCast(aa);
    if (!outArray)
      {
      vtkWarningMacro("Input is not a vtkDataArray.");
      return;
      }

    if ((outArray->GetNumberOfComponents() != this->GetNumberOfComponents()))
      {
      vtkWarningMacro("Number of components for input and output do not match");
      return;
      }

    vtkIdType num=ptIds->GetNumberOfIds();
    for (vtkIdType i=0; i<num; i++)
      {
      outArray->SetTuple(i, this->GetTuple(ptIds->GetId(i)));
      }
  }

  // Description:
  // Get the tuples for the range of points ids specified
  // (i.e., p1->p2 inclusive). You must insure that the output array has
  // been previously allocated with enough space to hold the data.
  //virtual void GetTuples(vtkIdType p1, vtkIdType p2, vtkAbstractArray *output);


  // Description:
  // Set the ith tuple in this array as the interpolated tuple value,
  // given the ptIndices in the source array and associated
  // interpolation weights.
  // This method assumes that the two arrays are of the same type
  // and strcuture.
  //virtual void InterpolateTuple(vtkIdType i, vtkIdList *ptIndices,
  //  vtkAbstractArray* source,  double* weights);

  // Description
  // Insert the ith tuple in this array as interpolated from the two values,
  // p1 and p2, and an interpolation factor, t.
  // The interpolation factor ranges from (0,1),
  // with t=0 located at p1. This method assumes that the three arrays are of
  // the same type. p1 is value at index id1 in source1, while, p2 is
  // value at index id2 in source2.
  //virtual void InterpolateTuple(vtkIdType i,
  //  vtkIdType id1, vtkAbstractArray* source1,
  //  vtkIdType id2, vtkAbstractArray* source2, double t);

  // Description:
  // Get the data tuple at ith location. Return it as a pointer to an array.
  // Note: this method is not thread-safe, and the pointer is only valid
  // as long as another method invocation to a vtk object is not performed.
  virtual double *GetTuple(vtkIdType i)
  {
    static std::vector<double> vec;
    vec = std::vector<double>(this->GetNumberOfComponents());
    GetTuple(i, &vec[0]);
    return &vec[0];
  }

  // Description:
  // Get the data tuple at ith location by filling in a user-provided array,
  // Make sure that your array is large enough to hold the NumberOfComponents
  // amount of data being returned.
  virtual void GetTuple(vtkIdType i, double * tuple)
  {
    std::vector<dist::Variant> varvec = pArray->getVector(i);
    for (int c = 0; c < this->GetNumberOfComponents(); c++)
      tuple[c] = boost::get<Real> ( varvec[c] );
  }

  // Description:
  // These methods are included as convenience for the wrappers.
  // GetTuple() and SetTuple() which return/take arrays can not be
  // used from wrapped languages. These methods can be used instead.
  //double GetTuple1(vtkIdType i);
  //double* GetTuple2(vtkIdType i);
  //double* GetTuple3(vtkIdType i);
  //double* GetTuple4(vtkIdType i);
  //double* GetTuple6(vtkIdType i);
  //double* GetTuple9(vtkIdType i);

  // Description:
  // Set the data tuple at ith location. Note that range checking or
  // memory allocation is not performed; use this method in conjunction
  // with SetNumberOfTuples() to allocate space.
  virtual void SetTuple(vtkIdType i, const float * tuple) {
    printf("SetTuple\n");
  }

  virtual void SetTuple(vtkIdType i, const double * tuple) {
    printf("SetTuple\n");
  }
  // Description:
  // These methods are included as convenience for the wrappers.
  // GetTuple() and SetTuple() which return/take arrays can not be
  // used from wrapped languages. These methods can be used instead.
#if 0
  void SetTuple1(vtkIdType i, double value);
  void SetTuple2(vtkIdType i, double val0, double val1);
  void SetTuple3(vtkIdType i, double val0, double val1, double val2);
  void SetTuple4(vtkIdType i, double val0, double val1, double val2,
                 double val3);
  void SetTuple6(vtkIdType i, double val0, double val1, double val2,
                 double val3, double val4, double val5);
  void SetTuple9(vtkIdType i, double val0, double val1, double val2,
                 double val3, double val4, double val5, double val6,
                 double val7, double val8);
#endif

  // Description:
  // Insert the data tuple at ith location. Note that memory allocation
  // is performed as necessary to hold the data.
  virtual void InsertTuple(vtkIdType i, const float * tuple) {
    printf("InsertTuple\n");
  }
  virtual void InsertTuple(vtkIdType i, const double * tuple){
    printf("InsertTuple\n");
  }
  // Description:
  // These methods are included as convenience for the wrappers.
  // InsertTuple() which takes arrays can not be
  // used from wrapped languages. These methods can be used instead.
#if 0
  void InsertTuple1(vtkIdType i, double value);
  void InsertTuple2(vtkIdType i, double val0, double val1);
  void InsertTuple3(vtkIdType i, double val0, double val1, double val2);
  void InsertTuple4(vtkIdType i, double val0, double val1, double val2,
                    double val3);
  void InsertTuple9(vtkIdType i, double val0, double val1, double val2,
                    double val3, double val4, double val5, double val6,
                    double val7, double val8);
#endif

  // Description:
  // Insert the data tuple at the end of the array and return the location at
  // which the data was inserted. Memory is allocated as necessary to hold
  // the data.
  virtual vtkIdType InsertNextTuple(const float * tuple) {
    printf("InsertNextTuple\n");
    return 0;
  }
  virtual vtkIdType InsertNextTuple(const double * tuple) {
    printf("InsertNextTuple\n");
    return 0;
  }

  // Description:
  // These methods are included as convenience for the wrappers.
  // InsertTuple() which takes arrays can not be
  // used from wrapped languages. These methods can be used instead.
#if 0
  void InsertNextTuple1(double value);
  void InsertNextTuple2(double val0, double val1);
  void InsertNextTuple3(double val0, double val1, double val2);
  void InsertNextTuple4(double val0, double val1, double val2,
                        double val3);
  void InsertNextTuple9(double val0, double val1, double val2,
                        double val3, double val4, double val5, double val6,
                        double val7, double val8);
#endif

  // Description:
  // These methods remove tuples from the data array. They shift data and
  // resize array, so the data array is still valid after this operation. Note,
  // this operation is fairly slow.
  virtual void RemoveTuple(vtkIdType id) {
    printf("RemoveTuple\n");
  }
  virtual void RemoveFirstTuple() {
    printf("RemoveTuple\n");
  }
  virtual void RemoveLastTuple() {
    printf("RemoveTuple\n");
  }

  // Description:
  // Return the data component at the ith tuple and jth component location.
  // Note that i is less than NumberOfTuples and j is less than
  // NumberOfComponents.
  //virtual double GetComponent(vtkIdType i, int j);

  // Description:
  // Set the data component at the ith tuple and jth component location.
  // Note that i is less than NumberOfTuples and j is less than
  //  NumberOfComponents. Make sure enough memory has been allocated
  // (use SetNumberOfTuples() and SetNumberOfComponents()).
  //virtual void SetComponent(vtkIdType i, int j, double c);

  // Description:
  // Insert the data component at ith tuple and jth component location.
  // Note that memory allocation is performed as necessary to hold the data.
  //virtual void InsertComponent(vtkIdType i, int j, double c);

  // Description:
  // Get the data as a double array in the range (tupleMin,tupleMax) and
  // (compMin, compMax). The resulting double array consists of all data in
  // the tuple range specified and only the component range specified. This
  // process typically requires casting the data from native form into
  // doubleing point values. This method is provided as a convenience for data
  // exchange, and is not very fast.
  //virtual void GetData(vtkIdType tupleMin, vtkIdType tupleMax, int compMin,
  //                     int compMax, vtkDoubleArray* data);

  // Description:
  // Deep copy of data. Copies data from different data arrays even if
  // they are different types (using doubleing-point exchange).
  //virtual void DeepCopy(vtkAbstractArray *aa);
  //virtual void DeepCopy(vtkDataArray *da);

  // Description:
  // Fill a component of a data array with a specified value. This method
  // sets the specified component to specified value for all tuples in the
  // data array.  This methods can be used to initialize or reinitialize a
  // single component of a multi-component array.
  //virtual void FillComponent(int j, double c);

  // Description:
  // Copy a component from one data array into a component on this data array.
  // This method copies the specified component ("fromComponent") from the
  // specified data array ("from") to the specified component ("j") over all
  // the tuples in this data array.  This method can be used to extract
  // a component (column) from one data array and paste that data into
  // a component on this data array.
  //virtual void CopyComponent(int j, vtkDataArray *from,
  //                           int fromComponent);

  // Description:
  // Get the address of a particular data index. Make sure data is allocated
  // for the number of items requested. Set MaxId according to the number of
  // data values requested.
  virtual void* WriteVoidPointer(vtkIdType id, vtkIdType number) {
    printf("WriteVoidPointer\n");
    return NULL;
  }

  // Description:
  // Return the memory in kilobytes consumed by this data array. Used to
  // support streaming and reading/writing data. The value returned is
  // guaranteed to be greater than or equal to the memory required to
  // actually represent the data represented by this object. The
  // information returned is valid only after the pipeline has
  // been updated.
  //virtual unsigned long GetActualMemorySize();

  // Description:
  // Create default lookup table. Generally used to create one when none
  // is available.
  //void CreateDefaultLookupTable();

  // Description:
  // Set/get the lookup table associated with this scalar data, if any.
  //void SetLookupTable(vtkLookupTable *lut);
  //vtkGetObjectMacro(LookupTable,vtkLookupTable);

  // Description:
  // The range of the data array values for the given component will be
  // returned in the provided range array argument. If comp is -1, the range
  // of the magnitude (L2 norm) over all components will be provided. The
  // range is computed and then cached, and will not be re-computed on
  // subsequent calls to GetRange() unless the array is modified or the
  // requested component changes.
  // THIS METHOD IS NOT THREAD SAFE.
  //void GetRange(double range[2], int comp)

  // Description:
  // Return the range of the data array values for the given component. If
  // comp is -1, return the range of the magnitude (L2 norm) over all
  // components.The range is computed and then cached, and will not be
  // re-computed on subsequent calls to GetRange() unless the array is
  // modified or the requested component changes.
  // THIS METHOD IS NOT THREAD SAFE.
  //double* GetRange(int comp)

  // Description:
  // Return the range of the data array. If the array has multiple components,
  // then this will return the range of only the first component (component
  // zero). The range is computed and then cached, and will not be re-computed
  // on subsequent calls to GetRange() unless the array is modified.
  // THIS METHOD IS NOT THREAD SAFE.
  //double* GetRange()

  // Description:
  // The the range of the data array values will be returned in the provided
  // range array argument. If the data array has multiple components, then
  // this will return the range of only the first component (component zero).
  // The range is computend and then cached, and will not be re-computed on
  // subsequent calls to GetRange() unless the array is modified.
  // THIS METHOD IS NOT THREAD SAFE.
  //void GetRange(double range[2])

  // Description:
  // These methods return the Min and Max possible range of the native
  // data type. For example if a vtkScalars consists of unsigned char
  // data these will return (0,255).
#if 0
  void GetDataTypeRange(double range[2]);
  double GetDataTypeMin();
  double GetDataTypeMax();
  static void GetDataTypeRange(int type, double range[2]);
  static double GetDataTypeMin(int type);
  static double GetDataTypeMax(int type);
#endif

  // Description:
  // Return the maximum norm for the tuples.
  // Note that the max. is computed every time GetMaxNorm is called.
  //virtual double GetMaxNorm();

  // Description:
  // Creates an array for dataType where dataType is one of
  // VTK_BIT, VTK_CHAR, VTK_SIGNED_CHAR, VTK_UNSIGNED_CHAR, VTK_SHORT,
  // VTK_UNSIGNED_SHORT, VTK_INT, VTK_UNSIGNED_INT, VTK_LONG,
  // VTK_UNSIGNED_LONG, VTK_DOUBLE, VTK_DOUBLE, VTK_ID_TYPE.
  // Note that the data array returned has be deleted by the
  // user.
  //static vtkDataArray* CreateDataArray(int dataType);

  // Description:
  // This key is used to hold tight bounds on the range of
  // one component over all tuples of the array.
  // Two values (a minimum and maximum) are stored for each component.
  // When GetRange() is called when no tuples are present in the array
  // this value is set to { VTK_DOUBLE_MAX, VTK_DOUBLE_MIN }.
  //static vtkInformationDoubleVectorKey* COMPONENT_RANGE();

  // Description:
  // This key is used to hold tight bounds on the $L_2$ norm
  // of tuples in the array.
  // Two values (a minimum and maximum) are stored for each component.
  // When GetRange() is called when no tuples are present in the array
  // this value is set to { VTK_DOUBLE_MAX, VTK_DOUBLE_MIN }.
  //static vtkInformationDoubleVectorKey* L2_NORM_RANGE();

  // Description:
  // Copy information instance. Arrays use information objects
  // in a variety of ways. It is important to have flexibility in
  // this regard because certain keys should not be coppied, while
  // others must be. NOTE: Up to the implmeneter to make sure that
  // keys not inteneded to be coppied are excluded here.
  //virtual int CopyInformation(vtkInformation *infoFrom, int deep=1);

  // Description:
  // Method for type-checking in FastDownCast implementations.
  //virtual int GetArrayType() { return DataArray; }

  //------------------------------------------------------------------------------
  inline static eddaSamplingArray* FastDownCast(vtkAbstractArray *source)
  {
    switch (source->GetArrayType())
      {
      case DataArrayTemplate:
      case TypedDataArray:
      case DataArray:
      case MappedDataArray:
        return static_cast<eddaSamplingArray*>(source);
      default:
        return NULL;
      }
  }
protected:
  // Description:
  // Compute the range for a specific component. If comp is set -1
  // then L2 norm is computed on all components. Call ClearRange
  // to force a recomputation if it is needed. The range is copied
  // to the range argument.
  // THIS METHOD IS NOT THREAD SAFE.
  //virtual void ComputeRange(double range[2], int comp);

  // Description:
  // Computes the range for each component of an array, the length
  // of \a ranges must be two times the number of components.
  // Returns true if the range was computed. Will return false
  // if you try to compute the range of an array of length zero.
  //virtual bool ComputeScalarRange(double* ranges);

  // Returns true if the range was computed. Will return false
  // if you try to compute the range of an array of length zero.
  //virtual bool ComputeVectorRange(double range[2]);

  // Construct object with default tuple dimension (number of components) of 1.
  eddaSamplingArray() {
    pArray = NULL;
  }

  ~eddaSamplingArray() {
    if (pArray)
      delete pArray;
  }


public : // vtkAbstractArray

  // Description:
  // Release storage and reset array to initial state.
  virtual void Initialize() {
    if (pArray)
      delete pArray ;
    pArray = NULL;
  }

  virtual int GetDataType() {
    return VTK_DOUBLE;
  }

  virtual int GetDataTypeSize() {
    return static_cast<int>(sizeof(double));
  }

  virtual void SetNumberOfTuples(vtkIdType number) {
    this->Size = number * this->GetNumberOfComponents();         // allocated size of data
    this->MaxId = number;        // maximum index inserted thus far
  }

  virtual void InsertTuples(vtkIdList *dstIds, vtkIdList *srcIds,
                            vtkAbstractArray* source) {
    printf("InsertTuples\n");
  }

  virtual void *GetVoidPointer(vtkIdType id) {
    static std::vector<double> data; // array data type simulates double
    bool refill = false;
    if (data.size() != this->GetDataSize()) {
      data.resize(this->GetDataSize());
      refill = true;
    } else {
      refill = (rand() % 10 == 0);
    }

    if (refill) {
      //printf("Refilling random array (id=%d)\n", (int)id);
      for (int i=0; i<this->GetNumberOfTuples(); i++)
      {
        this->GetTuple(i, &data[i*this->GetNumberOfComponents()]);
      }
    }
    return &data[id*this->GetNumberOfComponents()];
  }

  virtual void Squeeze() {
    printf("Squeeze");
  }

  virtual int Resize(vtkIdType numTuples) {
    return 0;
  }
  virtual void SetVoidArray(void *vtkNotUsed(array),
                            vtkIdType vtkNotUsed(size),
                            int vtkNotUsed(save)) {

  }
  virtual vtkArrayIterator* NewIterator()  {
    printf("vtkArrayIterator\n");
    return NULL;
  }

  // Return the indices where a specific value appears.
  virtual vtkIdType LookupValue(vtkVariant value) {return 0;}
  virtual void LookupValue(vtkVariant value, vtkIdList* ids) {}

  virtual void SetVariantValue(vtkIdType idx, vtkVariant value) {}

  virtual void DataChanged() {}
  virtual void ClearLookup() {}
  virtual int Allocate(vtkIdType sz, vtkIdType ext=1000) {
    printf("Allocate\n");
    return 0;
  }
  virtual void InsertTuples(vtkIdType dstStart, vtkIdType n, vtkIdType srcStart,
                            vtkAbstractArray* source) {}

  virtual void InsertVariantValue(vtkIdType idx, vtkVariant value) {}


private:
  eddaSamplingArray(const eddaSamplingArray& a);  // Not implemented.
  void operator=(const eddaSamplingArray& a);  // Not implemented.
};


#endif //eddaSamplingArray_h
