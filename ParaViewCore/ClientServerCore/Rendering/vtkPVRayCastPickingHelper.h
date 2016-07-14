/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPVRayCastPickingHelper.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPVRayCastPickingHelper - helper class that used selection and ray
// casting to find the intersection point between the user picking point
// and the concreate cell underneath.

#ifndef vtkPVRayCastPickingHelper_h
#define vtkPVRayCastPickingHelper_h

#include "vtkPVClientServerCoreRenderingModule.h" //needed for exports
#include "vtkObject.h"
class vtkAlgorithm;
class vtkDataSet;

class VTKPVCLIENTSERVERCORERENDERING_EXPORT vtkPVRayCastPickingHelper : public vtkObject
{
public:
  static vtkPVRayCastPickingHelper *New();
  vtkTypeMacro(vtkPVRayCastPickingHelper,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set input on which the selection apply
  void SetInput(vtkAlgorithm*);

  // Description:
  // Set the selection that extract the cell that intersect the ray
  void SetSelection(vtkAlgorithm*);

  // Description:
  // Set the point 1 that compose the ray
  vtkSetVector3Macro(PointA, double);
  vtkGetVector3Macro(PointA, double);

  // Description:
  // Set the point 2 that compose the ray
  vtkSetVector3Macro(PointB, double);
  vtkGetVector3Macro(PointB, double);

  // Description:
  // Set the flag to use directly selected points on mesh as intersection
  vtkSetMacro(SnapOnMeshPoint, bool);
  vtkGetMacro(SnapOnMeshPoint, bool);

  // Description:
  // Compute the intersection
  void ComputeIntersection();

  // Descritpion:
  // Provide access to the resulting intersection
  vtkGetVector3Macro(Intersection, double);

protected:
  vtkPVRayCastPickingHelper();
  ~vtkPVRayCastPickingHelper();

  // Description:
  // Compute the intersection using provided dataset
  void ComputeIntersectionFromDataSet(vtkDataSet* ds);

  double Intersection[3];
  double PointA[3];
  double PointB[3];
  bool SnapOnMeshPoint;
  vtkAlgorithm* Input;
  vtkAlgorithm* Selection;

private:
  vtkPVRayCastPickingHelper(const vtkPVRayCastPickingHelper&) VTK_DELETE_FUNCTION;
  void operator=(const vtkPVRayCastPickingHelper&) VTK_DELETE_FUNCTION;
};

#endif
