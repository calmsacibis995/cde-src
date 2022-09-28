/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm Todo Vector Class.

#pragma ident "@(#)TodoVec.cc	1.28 97/03/14 SMI"

// Include Files
#include <DataStructs/TodoVec.hh>

SdmTodo::SdmTodo(void (*todoFunc) (SdmPrim*, void*), SdmPrim* todoThis,
		 void* todoData)
    : SdmLinkedListElement(this)
{
  _TodoThis = todoThis; 
  _TodoData = todoData; 
  _TodoFunc = todoFunc;
}

SdmTodo::~SdmTodo()
{
}

SdmBoolean
SdmTodo::operator== (const SdmTodo& todo)
{
  if (this == &todo)
    return Sdm_True;
  else 
    return Sdm_False;
}

int
SdmTodoVector::AddTodoFunc(void (*todoFunc) (SdmPrim*, void*), 
  SdmPrim* todoThis, void* todoData)
{
  SdmTodo* todo;
  todo = new SdmTodo(todoFunc, todoThis, todoData);
  _ToDoList.AddElementToList(todo);
  return (_ToDoList.ElementCount() - 1);
}

void
SdmTodoVector::RemTodoFunc(const int order)
{
  _ToDoList.RemoveElementFromList(order);
}

