/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm Todo Vector Class.

#ifndef _SDM_TODOVEC_H
#define _SDM_TODOVEC_H

#pragma ident "@(#)TodoVec.hh	1.31 97/03/14 SMI"

// Include Files
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Prim.hh>
#include <SDtMail/DoubleLinkedList.hh>

class SdmTodo : public SdmLinkedListElement {

public:
  SdmPrim* _TodoThis;
  void* _TodoData;
  void (*_TodoFunc) (SdmPrim*, void*);

public:
  SdmTodo(void (*todoFunc) (SdmPrim*, void*), SdmPrim* todoThis, void*
	  todoData);
  virtual ~SdmTodo();
  SdmBoolean operator== (const SdmTodo& todo);

};

class SdmTodoVector {

public:
  SdmTodoVector() {}
  virtual ~SdmTodoVector() 
    {  _ToDoList.ClearAndDestroyAllElements(); }
  

public:
  int AddTodoFunc(void (*todoFunc) (SdmPrim*, void*), SdmPrim* todoThis,
		  void* todoData);
  void RemTodoFunc(const int order);

  int	ElementCount() const
    { return _ToDoList.ElementCount(); }

  SdmTodo *operator[] (int index) { return (SdmTodo*)_ToDoList[index]; }

private:
  SdmDoubleLinkedList    _ToDoList;
};

#endif
