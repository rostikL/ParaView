/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkPointHandleRepresentationSphere.h

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkPointHandleRepresentationSphere - represent the position of a point in display coordinates
// .SECTION Description
// This class is used to represent a vtkHandleWidget. It represents a
// position in 2D world coordinates using a x-y cursor (the cursor defined by
// an instance of vtkPolyData and generated by a vtkPolyDataAlgorithm).

// .SECTION See Also
// vtkHandleRepresentation vtkHandleWidget


#ifndef vtkPointHandleRepresentationSphere_h
#define vtkPointHandleRepresentationSphere_h

#include "vtkPVVTKExtensionsDefaultModule.h" //needed for exports
#include "vtkHandleRepresentation.h"

class vtkActor;
class vtkGlyph3D;
class vtkPoints;
class vtkPolyData;
class vtkPolyDataAlgorithm;
class vtkPolyDataMapper;
class vtkProperty;

class VTKPVVTKEXTENSIONSDEFAULT_EXPORT vtkPointHandleRepresentationSphere : public vtkHandleRepresentation
{
public:
  // Description:
  // Instantiate this class.
  static vtkPointHandleRepresentationSphere *New();

  // Description:
  // Standard methods for instances of this class.
  vtkTypeMacro(vtkPointHandleRepresentationSphere,vtkHandleRepresentation);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Specify the cursor shape with an instance of vtkPolyData. Note that
  // shape is assumed to be defined in the display coordinate system. By
  // default a sphere (the output of vtkSphereSource) shape is used.
  void SetCursorShape(vtkPolyData *cursorShape);
  vtkPolyData *GetCursorShape();

  // Description:
  // Set/Get the position of the point in display coordinates.  This overloads
  // the superclasses SetDisplayPosition in order to set the focal point
  // of the cursor.
  virtual void SetDisplayPosition(double xyz[3]);

  // Description:
  // Set/Get the handle properties when unselected and selected.
  void SetProperty(vtkProperty*);
  void SetSelectedProperty(vtkProperty*);
  vtkGetObjectMacro(Property,vtkProperty);
  vtkGetObjectMacro(SelectedProperty,vtkProperty);
  
  // Description:
  // Subclasses of vtkPointHandleRepresentationSphere must implement these
  // methods. These are the methods that the widget and its representation
  // use to communicate with each other.
  virtual double *GetBounds();
  virtual void BuildRepresentation();
  virtual void StartWidgetInteraction(double eventPos[2]);
  virtual void WidgetInteraction(double eventPos[2]);
  virtual int ComputeInteractionState(int X, int Y, int modify=0);

  // Description:
  // Methods to make this class behave as a vtkProp.
  virtual void ShallowCopy(vtkProp *prop);
  virtual void GetActors(vtkPropCollection *);
  virtual void ReleaseGraphicsResources(vtkWindow *);
  virtual int RenderOpaqueGeometry(vtkViewport *viewport);

  // Description:
  // Set/get the scalar value associated with this handle.
  vtkSetMacro(Scalar, double);
  vtkGetMacro(Scalar, double);
  
  // Description:
  // Set/get the flag whether to add a circle (disk) source around the sphere.
  void SetAddCircleAroundSphere(int);
  vtkGetMacro(AddCircleAroundSphere, int);
  vtkBooleanMacro(AddCircleAroundSphere, int);

  // Description:
  // Toggle whether this handle should be highlighted.
  void Highlight(int highlight);

protected:
  vtkPointHandleRepresentationSphere();
  ~vtkPointHandleRepresentationSphere();

  // Render the cursor
  vtkActor             *Actor;
  vtkPolyDataMapper    *Mapper;
  vtkGlyph3D           *Glypher;
  vtkPolyData          *CursorShape;
  vtkPolyData          *FocalData;
  vtkPoints            *FocalPoint;

  // Support picking
  double LastPickPosition[3];
  double LastEventPosition[2];
  
  // Methods to manipulate the cursor
  int  ConstraintAxis;
  void Translate(double eventPos[2]);
  void Scale(double eventPos[2]);
  
  // A flag to use the disk source
  int                   AddCircleAroundSphere;
  vtkActor             *DiskActor;
  vtkPolyDataMapper    *DiskMapper;
  vtkGlyph3D           *DiskGlypher;
  void CreateDefaultDiskSource();
  
  // Properties used to control the appearance of selected objects and
  // the manipulator in general.
  vtkProperty *Property;
  vtkProperty *SelectedProperty;
  void        CreateDefaultProperties();
  
  // The size of the hot spot.
  int    DetermineConstraintAxis(int constraint, double eventPos[2]);
  int    WaitingForMotion;
  int    WaitCount;

  double Scalar;

private:
  vtkPointHandleRepresentationSphere(const vtkPointHandleRepresentationSphere&) VTK_DELETE_FUNCTION;
  void operator=(const vtkPointHandleRepresentationSphere&) VTK_DELETE_FUNCTION;
};

#endif
