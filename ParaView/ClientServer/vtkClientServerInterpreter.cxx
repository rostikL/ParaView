/*=========================================================================

  Program:   Visualization Toolkit
  Module:    vtkClientServerInterpreter.cxx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkClientServerInterpreter.h"

#include "vtkClientServerStream.h"
#include "vtkCommand.h"
#include "vtkHashMap.txx"
#include "vtkObjectFactory.h"

#include <vtkstd/vector>

vtkStandardNewMacro(vtkClientServerInterpreter);
vtkCxxRevisionMacro(vtkClientServerInterpreter, "1.1.2.5");

//----------------------------------------------------------------------------
// Internal container instantiations.
static inline vtkClientServerCommandFunction
vtkContainerCreateMethod(vtkClientServerCommandFunction d1)
{
  return vtkContainerDefaultCreate(d1);
}
static inline void vtkContainerDeleteMethod(vtkClientServerCommandFunction) {}

template class vtkHashMap<vtkTypeUInt32, vtkClientServerStream*>;
template class vtkHashMap<const char*, vtkClientServerCommandFunction>;

//----------------------------------------------------------------------------
class vtkClientServerInterpreterInternals
{
public:
  typedef vtkstd::vector<vtkClientServerNewInstanceFunction> NewInstanceFunctionsType;
  NewInstanceFunctionsType NewInstanceFunctions;
};

//----------------------------------------------------------------------------
vtkClientServerInterpreter::vtkClientServerInterpreter()
{
  this->Internal = new vtkClientServerInterpreterInternals;
  this->IDToMessageMap = IDToMessageMapType::New();
  this->ClassToFunctionMap = ClassToFunctionMapType::New();
  this->LastResultMessage = new vtkClientServerStream;
  this->LogStream = 0;
}

//----------------------------------------------------------------------------
vtkClientServerInterpreter::~vtkClientServerInterpreter()
{
  // Delete any remaining messages.
  vtkHashMapIterator<vtkTypeUInt32, vtkClientServerStream*>* hi =
    this->IDToMessageMap->NewIterator();
  vtkClientServerStream* tmp;
  while(!hi->IsDoneWithTraversal())
    {
    hi->GetData(tmp);
    delete tmp;
    hi->GoToNextItem();
    }
  hi->Delete();

  this->IDToMessageMap->Delete();
  this->ClassToFunctionMap->Delete();

  delete this->LastResultMessage;

  delete this->Internal;
}

//----------------------------------------------------------------------------
vtkObjectBase*
vtkClientServerInterpreter::GetObjectFromID(vtkClientServerID id)
{
  // Get the message corresponding to this ID.
  if(const vtkClientServerStream* tmp = this->GetMessageFromID(id))
    {
    // Retrieve the object from the message.
    vtkObjectBase* obj = 0;
    if(tmp->GetNumberOfArguments(0) == 1 && tmp->GetArgument(0, 0, &obj))
      {
      return obj;
      }
    else
      {
      vtkErrorMacro("Attempt to get an object for ID " << id.ID
                    << " whose message does not contain exactly one object.");
      return 0;
      }
    }
  else
    {
    vtkErrorMacro("Attempt to get object for ID " << id.ID
                  << " that is not present in the hash table.");
    return 0;
    }
}

//----------------------------------------------------------------------------
vtkClientServerID
vtkClientServerInterpreter::GetIDFromObject(vtkObjectBase* key)
{
  // Search the hash table for the given object.
  vtkHashMapIterator<vtkTypeUInt32, vtkClientServerStream*>* hi =
    this->IDToMessageMap->NewIterator();
  vtkClientServerStream* msg;
  vtkTypeUInt32 id = 0;
  while (!hi->IsDoneWithTraversal())
    {
    hi->GetData(msg);
    vtkObjectBase* obj;
    if(msg->GetArgument(0, 0, &obj) && obj == key)
      {
      hi->GetKey(id);
      break;
      }
    hi->GoToNextItem();
    }
  hi->Delete();

  // Convert the result to an ID object.
  vtkClientServerID result = {id};
  return result;
}

//----------------------------------------------------------------------------
int vtkClientServerInterpreter::ProcessStream(const unsigned char* msg,
                                              size_t msgLength)
{
  vtkClientServerStream css;
  css.SetData(msg, msgLength);
  return this->ProcessStream(css);
}

//----------------------------------------------------------------------------
int vtkClientServerInterpreter::ProcessStream(const vtkClientServerStream& css)
{
  for(int i=0; i < css.GetNumberOfMessages(); ++i)
    {
    if(!this->ProcessOneMessage(css, i))
      {
      const char* errorMessage;
      if(this->LastResultMessage->GetNumberOfMessages() > 0 &&
         this->LastResultMessage->GetCommand(0) ==
         vtkClientServerStream::Error &&
         this->LastResultMessage->GetArgument(0, 0, &errorMessage))
        {
        vtkErrorMacro(<< errorMessage);
        }
      return 0;
      }
    }
  return 1;
}

//----------------------------------------------------------------------------
int
vtkClientServerInterpreter::ProcessOneMessage(const vtkClientServerStream& css,
                                              int message)
{
  // Look for known commands in the message.
  vtkClientServerStream::Commands cmd = css.GetCommand(message);
  switch(cmd)
    {
    case vtkClientServerStream::New:
      return this->ProcessCommandNew(css, message);
    case vtkClientServerStream::Invoke:
      return this->ProcessCommandInvoke(css, message);
    case vtkClientServerStream::Delete:
      return this->ProcessCommandDelete(css, message);
    case vtkClientServerStream::AssignResult:
      return this->ProcessCommandAssignResult(css, message);
    default:
      break;
    }

  vtkErrorMacro("Message " << message << " with type "
                << static_cast<int>(cmd) << " ("
                << vtkClientServerStream::GetStringFromCommand(cmd)
                << ") cannot be executed.");
  return 0;
}

//----------------------------------------------------------------------------
int
vtkClientServerInterpreter
::ProcessCommandNew(const vtkClientServerStream& css, int midx)
{
  // Make sure we have some instance creation functions registered.
  if(this->Internal->NewInstanceFunctions.size() == 0)
    {
    vtkErrorMacro("Attempt to create object with no NewInstanceFunctions.");
    return 0;
    }

  // Get the class name and desired ID for the instance.
  const char* cname = 0;
  vtkClientServerID id;
  if(css.GetNumberOfArguments(midx) == 2 &&
     css.GetArgument(midx, 0, &cname) && css.GetArgument(midx, 1, &id))
    {
    // Find a NewInstance function that knows about the class.
    int created = 0;
    for(vtkClientServerInterpreterInternals::NewInstanceFunctionsType::iterator
          it = this->Internal->NewInstanceFunctions.begin();
        !created && it != this->Internal->NewInstanceFunctions.end(); ++it)
      {
      if((*(*it))(this, cname, id) == 0)
        {
        created = 1;
        }
      }
    if(created)
      {
      vtkClientServerInterpreter::NewCallbackInfo info;
      info.Type = cname;
      info.ID = id.ID;
      this->InvokeEvent(vtkCommand::UserEvent+1, &info);
      return 1;
      }
    else
      {
      vtkErrorMacro("Cannot create object of type \"" << cname << "\".");
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
int
vtkClientServerInterpreter
::ProcessCommandInvoke(const vtkClientServerStream& css, int midx)
{
  // Create a message with all known id_value arguments expanded.
  vtkClientServerStream msg;
  this->ExpandMessage(css, midx, msg);

  // Reset the result to empty before processing the message.
  this->LastResultMessage->Reset();

  // Get the object and method to be invoked.
  vtkObjectBase* obj;
  const char* method;
  if(msg.GetNumberOfArguments(0) >= 2 &&
     msg.GetArgument(0, 0, &obj) && msg.GetArgument(0, 1, &method))
    {
    // Log the invocation.
    if(this->LogStream)
      {
      *this->LogStream << "---------------------------------------"
                       << "---------------------------------------\n";
      msg.Print(*this->LogStream);
      }

    // Find the command function for this object's type.
    if(vtkClientServerCommandFunction func = this->GetCommandFunction(obj))
      {
      // Try to invoke the method.
      if(func(this, obj, method, msg, *this->LastResultMessage) == 0)
        {
        // Success.  Log the result.
        if(this->LogStream)
          {
          this->LastResultMessage->Print(*this->LogStream);
          }
        return 1;
        }
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
int
vtkClientServerInterpreter
::ProcessCommandDelete(const vtkClientServerStream& msg, int midx)
{
  vtkClientServerID id;
  if(msg.GetNumberOfArguments(midx) == 1 && msg.GetArgument(midx, 0, &id))
    {
    // If the value is an object, invoke the event callback.
    if(vtkObjectBase* obj = this->GetObjectFromID(id))
      {
      vtkClientServerInterpreter::NewCallbackInfo info;
      info.Type = obj->GetClassName();
      info.ID = id.ID;
      this->InvokeEvent(vtkCommand::UserEvent+2, &info);
      }

    // Remove the ID from the map.
    vtkClientServerStream* item;
    this->IDToMessageMap->GetItem(id.ID, item);
    this->IDToMessageMap->RemoveItem(id.ID);
    delete item;
    return 1;
    }

  return 0;
}

//----------------------------------------------------------------------------
int
vtkClientServerInterpreter
::ProcessCommandAssignResult(const vtkClientServerStream& msg, int midx)
{
  vtkClientServerID id;
  if(msg.GetNumberOfArguments(midx) == 1 && msg.GetArgument(midx, 0, &id))
    {
    return this->AssignResultToID(id);
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkClientServerInterpreter::ExpandMessage(const vtkClientServerStream& in,
                                              int inIndex,
                                              vtkClientServerStream& out)
{
  // Reset the output and make sure we have input.
  out.Reset();
  if(inIndex < 0 || inIndex >= in.GetNumberOfMessages())
    {
    return 0;
    }

  // Copy the command.
  out << in.GetCommand(inIndex);

  // Copy all arguments while expanding id_value arguments.
  for(int a=0; a < in.GetNumberOfArguments(inIndex); ++a)
    {
    if(in.GetArgumentType(inIndex, a) == vtkClientServerStream::id_value)
      {
      vtkClientServerID id;
      in.GetArgument(inIndex, a, &id);

      // If the ID is in the map, expand it.  Otherwise, leave it.
      const vtkClientServerStream* tmp = this->GetMessageFromID(id);
      if(tmp)
        {
        for(int b=0; b < tmp->GetNumberOfArguments(0); ++b)
          {
          out << tmp->GetArgument(0, b);
          }
        }
      else
        {
        out << in.GetArgument(inIndex, a);
        }
      }
    else
      {
      out << in.GetArgument(inIndex, a);
      }
    }

  // End the message.
  out << vtkClientServerStream::End;

  return 1;
}

//----------------------------------------------------------------------------
int vtkClientServerInterpreter::AssignResultToID(vtkClientServerID id)
{
  // Make sure the ID doesn't exist.
  vtkClientServerStream* tmp;
  if(this->IDToMessageMap->GetItem(id.ID, tmp) == VTK_OK)
    {
    vtkErrorMacro("Attempt to assign ID " << id.ID
                  << " that is already in the hash table.");
    return 0;
    }

  // Copy the result to store it in the map.
  vtkClientServerStream& lrm = *this->LastResultMessage;
  vtkClientServerStream* copy = new vtkClientServerStream(lrm);
  this->IDToMessageMap->SetItem(id.ID, copy);
  return 1;
}

//----------------------------------------------------------------------------
const vtkClientServerStream*
vtkClientServerInterpreter::GetMessageFromID(vtkClientServerID id)
{
  // Look for special LastReturnMessage.
  if(id.ID == 0)
    {
    return this->LastResultMessage;
    }

  // Find the message in the map.
  vtkClientServerStream* tmp;
  if(this->IDToMessageMap->GetItem(id.ID, tmp) != VTK_OK)
    {
    vtkErrorMacro("Attempt to get message for ID " << id.ID
                  << " that is not in the hash table.");
    return 0;
    }
  return tmp;
}

//----------------------------------------------------------------------------
int vtkClientServerInterpreter::NewInstance(vtkObjectBase* obj,
                                            vtkClientServerID id)
{
  // Store the object in the last result.
  vtkClientServerStream& lrm = *this->LastResultMessage;
  lrm.Reset();
  lrm << vtkClientServerStream::Reply << obj << vtkClientServerStream::End;

  // Last result holds a reference.  Remove reference from ::New()
  // call in generated code.
  obj->Delete();

  // Enter the last result into the map with the given id.
  return this->AssignResultToID(id);
}

//----------------------------------------------------------------------------
void
vtkClientServerInterpreter
::AddCommandFunction(const char* cname, vtkClientServerCommandFunction func)
{
  this->ClassToFunctionMap->SetItem(cname, func);
}

//----------------------------------------------------------------------------
vtkClientServerCommandFunction
vtkClientServerInterpreter::GetCommandFunction(vtkObjectBase* obj)
{
  // Lookup the function for this object's class.
  vtkClientServerCommandFunction res = 0;
  const char* cname = obj->GetClassName();
  this->ClassToFunctionMap->GetItem(cname, res);
  if(!res)
    {
    vtkErrorMacro("Cannot find command function for \"" << cname << "\".");
    }
  return res;
}

//----------------------------------------------------------------------------
void
vtkClientServerInterpreter
::AddNewInstanceFunction(vtkClientServerNewInstanceFunction f)
{
  this->Internal->NewInstanceFunctions.push_back(f);
}
