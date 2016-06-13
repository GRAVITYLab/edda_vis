#ifndef VTK_COMMON_H
#define VTK_COMMON_H

#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkDataArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkFieldData.h>
#include <vtkStructuredGrid.h>

#define vsp_new(T,x) vtkSmartPointer<T> x = vtkSmartPointer<T>::New()

#endif
