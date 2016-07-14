/*=========================================================================

  Program:   ParaView
  Module:    vtkSMGlobalPropertiesLinkUndoElement.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSMGlobalPropertiesLinkUndoElement
// .SECTION Description
// This UndoElement is used to link or unlink GlobalPropertyManager property
// to a property of an arbitrary Proxy.
// This class is automatically build inside the vtkSMProxyManager when
// GlobalPropertyLinks are changed.
// FIXME: This class is currrently non-functional. I need to re-energize this
// class. It's falling down the priority chain for now.

#ifndef vtkSMGlobalPropertiesLinkUndoElement_h
#define vtkSMGlobalPropertiesLinkUndoElement_h

#include "vtkPVServerManagerCoreModule.h" //needed for exports
#include "vtkSMUndoElement.h"

class vtkSMProxy;

class VTKPVSERVERMANAGERCORE_EXPORT vtkSMGlobalPropertiesLinkUndoElement : public vtkSMUndoElement
{
public:
  static vtkSMGlobalPropertiesLinkUndoElement* New();
  vtkTypeMacro(vtkSMGlobalPropertiesLinkUndoElement, vtkSMUndoElement);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Undo the operation encapsulated by this element.
  virtual int Undo();

  // Description:
  // Redo the operation encaspsulated by this element.
  virtual int Redo();

  // Description:
  // Provide the informations needed to restore the previous state
  void SetLinkState( const char* mgrname,
                     const char* globalpropname,
                     vtkSMProxy* proxy, const char* propname, bool isAddAction);

protected:
  vtkSMGlobalPropertiesLinkUndoElement();
  ~vtkSMGlobalPropertiesLinkUndoElement();

  // State ivars
  char* GlobalPropertyManagerName;
  char* GlobalPropertyName;
  vtkTypeUInt32 ProxyGlobalID;
  char* ProxyPropertyName;
  bool IsLinkAdded;

  // Setter for iVar
  vtkSetStringMacro(GlobalPropertyManagerName);
  vtkSetStringMacro(GlobalPropertyName);
  vtkSetStringMacro(ProxyPropertyName);

  int UndoRedoInternal(bool undo);
private:
  vtkSMGlobalPropertiesLinkUndoElement(const vtkSMGlobalPropertiesLinkUndoElement&) VTK_DELETE_FUNCTION;
  void operator=(const vtkSMGlobalPropertiesLinkUndoElement&) VTK_DELETE_FUNCTION;

};

#endif
