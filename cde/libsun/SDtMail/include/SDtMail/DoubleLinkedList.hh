/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is a doubly linked list object class.

#ifndef _SDM_DOUBLELINKEDLIST_H
#define _SDM_DOUBLELINKEDLIST_H

#pragma ident "@(#)DoubleLinkedList.hh	1.11 97/03/21 SMI"

#include <SDtMail/Sdtmail.hh>
#include <SDtMail/LinkedList.hh>
#include <assert.h>

// Forward references
//
class SdmDoubleLinkedListIterator;
class SdmDoubleLinkedList;


// Object: SdmDoubleLinkedList
// What: A doubly linked list
// Description:
// Object capable of holding list of "SdmLinkedListElement" objects;
// This object can have "SdmLinkedListElement" objects added to and otherwise
// manipulated on a list maintained by the object.
//
class SdmDoubleLinkedList {
  friend	SdmDoubleLinkedListIterator;

private:
  // This object is what is actually inserted onto a linked list;
  // it contains the links themselves and a pointer to the object
  // (the "link element") that this link represents
  //
  class SdmDoubleLinkedListLink
  {
  public:
    SdmDoubleLinkedListLink(SdmLinkedListElement *lle)
    { 
      _dsllLink_next = (SdmDoubleLinkedListLink *)0; 
      _dsllLink_prev = (SdmDoubleLinkedListLink *)0; 
      _dsllLink_element = lle; 
    }
    virtual SdmDoubleLinkedListLink::~SdmDoubleLinkedListLink()
      { };
  private:
    friend	SdmDoubleLinkedList;
    friend	SdmDoubleLinkedListIterator;
    SdmDoubleLinkedListLink	*_dsllLink_next;
    SdmDoubleLinkedListLink	*_dsllLink_prev;
    SdmLinkedListElement	*_dsllLink_element;
  };

public:
  SdmDoubleLinkedList();
  ~SdmDoubleLinkedList();

  // Return the element at the index position in the list
  SdmLinkedListElement *operator[] (int index);

  // Delete all elements on the list.
  void	SdmDoubleLinkedList::ClearAllElements();
  
  // Delete all elements on the list and destroy each element.
  void	SdmDoubleLinkedList::ClearAndDestroyAllElements();

  // Append an element to the end of the list.
  void	SdmDoubleLinkedList::AddElementToList(SdmLinkedListElement *lle);

  // Match the give element with one in the list and return the position.
  // If the element isn't found, Sdm_Not_Found (-1) is returned.
  int	SdmDoubleLinkedList::FindElement(SdmLinkedListElement *lle);

  // Search for the given element and delete the first occurrence.
  void	SdmDoubleLinkedList::RemoveElementFromList(SdmLinkedListElement *lle);

  // Remove the element at index position.
  void	SdmDoubleLinkedList::RemoveElementFromList(int index);

  // Return the number of elements
  int	SdmDoubleLinkedList::ElementCount() const
    { return (_dsll_numberOfListElements); }

  // Return the first element.
  SdmDoubleLinkedList::SdmDoubleLinkedListLink *SdmDoubleLinkedList::ListHead()
    { return (_dsll_head); }

  // Return the last element.
  SdmDoubleLinkedList::SdmDoubleLinkedListLink *SdmDoubleLinkedList::ListTail()
    { return (_dsll_tail); }

private:
#if !defined(NDEBUG)
  int	SdmDoubleLinkedList::AssertLinkedList();
#endif
  SdmDoubleLinkedList::SdmDoubleLinkedListLink *_dsll_head;
  SdmDoubleLinkedList::SdmDoubleLinkedListLink *_dsll_tail;
  long _dsll_numberOfListElements;
};

// Object: SdmDoubleLinkedListIterator
// What: Object capable of iterating through linked list elements
// Description:
// An iterator object may be created on a linked list, and may then
// be used to iterate through the list contents, either forwards
// or backwards.
//
class SdmDoubleLinkedListIterator {
public:
  SdmDoubleLinkedListIterator(SdmDoubleLinkedList *ll);

  ~SdmDoubleLinkedListIterator();

  void SdmDoubleLinkedListIterator::ResetIterator();
  void *SdmDoubleLinkedListIterator::NextListElement();
  void *SdmDoubleLinkedListIterator::PrevListElement();

private:
  SdmDoubleLinkedList				*_dsllIterator_list;
  SdmDoubleLinkedList::SdmDoubleLinkedListLink	*_dsllIterator_current;
  SdmDoubleLinkedList::SdmDoubleLinkedListLink	*_dsllIterator_next;
  SdmDoubleLinkedList::SdmDoubleLinkedListLink	*_dsllIterator_prev;
};

//-------------------------------------------------------------

#endif	//  _SDM_DOUBLELINKEDLIST_H
