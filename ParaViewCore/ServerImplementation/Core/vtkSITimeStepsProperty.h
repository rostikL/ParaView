/*=========================================================================

  Program:   ParaView
  Module:    vtkSITimeStepsProperty.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSITimeRangeProperty
// .SECTION Description
// SIProperty that deals with TimeRange on Algorithm object type

#ifndef vtkSITimeStepsProperty_h
#define vtkSITimeStepsProperty_h

#include "vtkPVServerImplementationCoreModule.h" //needed for exports
#include "vtkSIProperty.h"

class VTKPVSERVERIMPLEMENTATIONCORE_EXPORT vtkSITimeStepsProperty : public vtkSIProperty
{
public:
  static vtkSITimeStepsProperty* New();
  vtkTypeMacro(vtkSITimeStepsProperty, vtkSIProperty);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkSITimeStepsProperty();
  ~vtkSITimeStepsProperty();

  friend class vtkSIProxy;

  // Description:
  // Pull the current state of the underneath implementation
  virtual bool Pull(vtkSMMessage*);

private:
  vtkSITimeStepsProperty(const vtkSITimeStepsProperty&) VTK_DELETE_FUNCTION;
  void operator=(const vtkSITimeStepsProperty&) VTK_DELETE_FUNCTION;

};

#endif
