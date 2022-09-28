/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is a linked list object class.

#ifndef _SDM_LINKEDLIST_H
#define _SDM_LINKEDLIST_H

#pragma ident "@(#)LinkedList.hh	1.22 97/03/14 SMI"

#include <assert.h>

/*
 * Quick mumble on linked lists:
 * 
 * List -> Link      -> Link      -> Link        -> (0)
 *         |-> Element  |-> Element  |-> Element
 * 	       |-> OBJ      |-> OBJ      |-> OBJ
 * 
 * The linked list objects deal with "elements", and so any object that
 * inherits from the linked list element class may then be placed onto a
 * linked list. The extra "link" object is used so that an
 * "element/object" may be placed on more than one list (or multiply in
 * the same list) at a time.
 */

// Forward references
//
class SdmLinkedListElement;
class SdmLinkedListIterator;
class SdmLinkedList;
class SdmDoubleLinkedList;
class SdmDoubleLinkedListIterator;

// Object: SdmLinkedListElement
// What: An element that can be inserted and manipulated on a linked list
// Description:
// Object that can be inserted or manipulated on linked lists;
// Inherit this class in order for objects of that type to be able
// to be placed on and manipulated by the linked list objects. The
// constructor for the real object must include this initializer:
//   SdmLinkedListElement(this)
// The class itself is very simple - it is just a container class
// that holds the address of the real object itself.
//
class SdmLinkedListElement
{
  friend	SdmLinkedList;
  friend	SdmLinkedListIterator;
  friend	SdmDoubleLinkedList;
  friend	SdmDoubleLinkedListIterator;
public:
  SdmLinkedListElement(void *object) :
    _dsllElement_object(object), _dsllElement_LinkCount(0) {}

  virtual   ~SdmLinkedListElement()
            { assert(!_dsllElement_LinkCount); }
  
private:
  void		*_dsllElement_object;		// -> object this element represents
  int		_dsllElement_LinkCount;		// # times this element is linked into a list
};

// Object: SdmLinkedList
// What: A linked list
// Description:
// Object capable of holding list of "SdmLinkedListElement" objects;
// This object can have "SdmLinkedListElement" objects added to and otherwise
// manipulated on a list maintained by the object.
//
class SdmLinkedList {
  friend	SdmLinkedListIterator;

private:
  // This object is what is actually inserted onto a linked list;
  // it contains the links themselves and a pointer to the object
  // (the "link element") that this link represents
  //
  class SdmLinkedListLink
  {
  public:
    SdmLinkedListLink(SdmLinkedListElement *lle)
    { 
      _dsllLink_next = (SdmLinkedListLink *)0; 
      _dsllLink_element = lle; 
    }
    ~SdmLinkedListLink() {};
  private:
    friend	SdmLinkedList;
    friend	SdmLinkedListIterator;
    SdmLinkedListLink		*_dsllLink_next;
    SdmLinkedListElement	*_dsllLink_element;
  };

public:
  SdmLinkedList();
  ~SdmLinkedList();

  // Return the element at the index position in the list
  SdmLinkedListElement *SdmLinkedList::operator[] (int index);

  // Remove all elements on the list.
  void	SdmLinkedList::ClearAllElements(void);

  // Remove all elements on the list.  Delete element if it's link count is zero.
  void	SdmLinkedList::ClearAndDestroyAllElements(void);

  // Append an element to the end of the list.
  void	SdmLinkedList::AddElementToList(SdmLinkedListElement *lle);

  // Match the give element with one in the list and return the position.
  // If the element isn't found, Sdm_Not_Found (-1) is returned.
  int   SdmLinkedList::FindElement (SdmLinkedListElement *lle);

  // Search for the given element and delete the first occurrence.
  void	SdmLinkedList::RemoveElementFromList(SdmLinkedListElement *lle);

  // Remove the element at index position.
  void	SdmLinkedList::RemoveElementFromList(int index);

  // Return the number of elements
  int	SdmLinkedList::ElementCount() const { return (_dsll_numberOfListElements); }

  // Return the first element.
  SdmLinkedList::SdmLinkedListLink *SdmLinkedList::ListHead() { return (_dsll_head); }

private:
#if !defined(NDEBUG)
  int	SdmLinkedList::AssertLinkedList();
#endif
  SdmLinkedList::SdmLinkedListLink  	*_dsll_head;
  SdmLinkedList::SdmLinkedListLink  	*_dsll_tail;
  long					_dsll_numberOfListElements;
};

// Object: SdmLinkedListIterator
// What: Object capable of iterating through linked list elements
// Description:
// An iterator object may be created on a linked list, and may then
// be used to iterate through the list contents. 
//
class SdmLinkedListIterator {
public:
  SdmLinkedListIterator(SdmLinkedList *ll);

  virtual ~SdmLinkedListIterator();

  void SdmLinkedListIterator::ResetIterator();
  void *SdmLinkedListIterator::NextListElement();

private:
  SdmLinkedList				*_dsllIterator_list;
  SdmLinkedList::SdmLinkedListLink	*_dsllIterator_current;
  SdmLinkedList::SdmLinkedListLink	*_dsllIterator_next;
};

#endif	//  _SDM_LINKEDLIST_H
