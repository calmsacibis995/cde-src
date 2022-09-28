/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the doubly linked list object class.

#pragma ident "@(#)DoubleLinkedList.cc	1.8 96/07/16 SMI"

// Include Files.
#include <SDtMail/DoubleLinkedList.hh>

#if !defined(NDEBUG)
int SdmDoubleLinkedList::AssertLinkedList()
{
  assert(_dsll_numberOfListElements > 1 || (_dsll_tail == _dsll_head));
  assert(_dsll_numberOfListElements > 0 || ((!_dsll_tail) && (!_dsll_head)));
  assert(_dsll_numberOfListElements == 0 || (_dsll_tail && _dsll_head));
  return(1);
}
#endif
 

// Object: SdmDoubleLinkedList
// Method: Constructor
// Description: 
// Create an empty list
//
SdmDoubleLinkedList::SdmDoubleLinkedList()
{
  _dsll_head = (SdmDoubleLinkedListLink *)0;
  _dsll_tail = (SdmDoubleLinkedListLink *)0;
  _dsll_numberOfListElements = 0;
}

// Object: SdmDoubleLinkedList
// Method: Destructor
// Description:
// Empty the list of all items
//
SdmDoubleLinkedList::~SdmDoubleLinkedList()
{
  // Remove all elements
  ClearAndDestroyAllElements();
}

SdmLinkedListElement *SdmDoubleLinkedList::operator[] (int index)
{
  assert(_dsll_numberOfListElements);
  assert(AssertLinkedList());
 
  SdmDoubleLinkedListLink *dsllLink = _dsll_head;
  int i = 0;
 
  // Spin through looking for this entry in the list
  // dsllLink points to "current" entry
  // dsllLink_prev points to the "previous" entry in the scan
  //
  while (dsllLink) {
    if (i == index) {
      // Found this element on the list - remove it
      return (dsllLink->_dsllLink_element);
    }
    else
    {
      dsllLink = dsllLink->_dsllLink_next;
      i++;
    }
  }
  // Element not found on list - could probably just ignore the
  // request but assert out so it can be checked to see if it is
  // a programming failure...
  assert(dsllLink != 0);
}

// Object: SdmDoubleLinkedList
// Method: ClearAllElements
// Description:
// delete all elements from the list
//
void SdmDoubleLinkedList::ClearAllElements()
{
  SdmDoubleLinkedListLink *dsllLink = _dsll_head, *currentLink;
  long expectedElements = _dsll_numberOfListElements;
  
  _dsll_head = (SdmDoubleLinkedListLink *)0;
  _dsll_tail = (SdmDoubleLinkedListLink *)0;
  _dsll_numberOfListElements = 0;

  // While there are any items on the list, remove them
  //
  while (dsllLink) {
    currentLink = dsllLink;
    dsllLink = dsllLink->_dsllLink_next;
    currentLink->_dsllLink_element->_dsllElement_LinkCount--;  // decrement count;
    delete currentLink;
    expectedElements--;
  }
    
  // The list must now be empty - assert empty conditions
  //
  assert(!expectedElements);
  assert(AssertLinkedList());
}


// Object: SdmDoubleLinkedList
// Method: ClearAndDestroyAllElements
// Description:
// delete all elements from the list and delete each element.
//
void SdmDoubleLinkedList::ClearAndDestroyAllElements()
{
  SdmDoubleLinkedListLink *dsllLink = _dsll_head, *currentLink;
  long expectedElements = _dsll_numberOfListElements;
  
  _dsll_head = (SdmDoubleLinkedListLink *)0;
  _dsll_tail = (SdmDoubleLinkedListLink *)0;
  _dsll_numberOfListElements = 0;

  // While there are any items on the list, remove link and the element.
  //    
  while (dsllLink) {
    currentLink = dsllLink;
    dsllLink = dsllLink->_dsllLink_next;
    currentLink->_dsllLink_element->_dsllElement_LinkCount--;  // decrement count
    
    // delete element only if it's link count is zero.
    if (!currentLink->_dsllLink_element->_dsllElement_LinkCount) {
      delete currentLink->_dsllLink_element;    
    }
    delete currentLink;
    expectedElements--;
  }

  // The list must now be empty - assert empty conditions
  //
  assert(!expectedElements);
  assert(AssertLinkedList());
}

// Object: SdmDoubleLinkedList
// Method: RemoveElementFromList_L
// Description:
// Given an element that is on the list, walk the list and
// remove one instance of the element from the list. Adjust
// various list and link variables as appropriate.
//
void SdmDoubleLinkedList::RemoveElementFromList(SdmLinkedListElement *lle)
{
  assert(lle);
  assert(lle->_dsllElement_object);
  assert(_dsll_numberOfListElements);
  assert(AssertLinkedList());

  SdmDoubleLinkedListLink *dsllLink = _dsll_head;

  // Spin through looking for this entry in the list
  // dsllLink points to "current" entry
  // dsllLink_prev points to the "previous" entry in the scan
  //
  while (dsllLink) {
    if (dsllLink->_dsllLink_element == lle) {
      // Found this element on the list - remove it
      assert(lle->_dsllElement_LinkCount);
      if (_dsll_head == dsllLink) {
        assert (!dsllLink->_dsllLink_prev);
        _dsll_head = dsllLink->_dsllLink_next;
      }
      if (_dsll_tail == dsllLink) {
        assert(!dsllLink->_dsllLink_next);
        _dsll_tail = dsllLink->_dsllLink_prev;
      }
      if (dsllLink->_dsllLink_next)
        dsllLink->_dsllLink_next->_dsllLink_prev = dsllLink->_dsllLink_prev;
      if (dsllLink->_dsllLink_prev)
        dsllLink->_dsllLink_prev->_dsllLink_next = dsllLink->_dsllLink_next;
        
      _dsll_numberOfListElements--;
      lle->_dsllElement_LinkCount--;
      delete dsllLink;
      assert(AssertLinkedList());
      return;
    }
    else
    {
      dsllLink = dsllLink->_dsllLink_next;
    }
  }
  // Element not found on list - could probably just ignore the
  // request but assert out so it can be checked to see if it is
  // a programming failure...
  assert(dsllLink != 0);
}

// Object: SdmDoubleLinkedList
// Method: RemoveElementFromList_L
// Description:
// Given an index to an element that is on the list,
// walk the list and remove the element from the list at the
// index specified. Adjust various list and link variables 
// as appropriate.
//
void SdmDoubleLinkedList::RemoveElementFromList(int index)
{
  assert(_dsll_numberOfListElements);
  assert(AssertLinkedList());

  SdmDoubleLinkedListLink *dsllLink = _dsll_head;
  int i = 0;

  // Spin through looking for this entry in the list
  // dsllLink points to "current" entry
  // dsllLink_prev points to the "previous" entry in the scan
  //
  while (dsllLink) {
    if (i == index) {
      // Found this element on the list - remove it
      assert(dsllLink->_dsllLink_element->_dsllElement_LinkCount);
      if (_dsll_head == dsllLink) {
	assert (!dsllLink->_dsllLink_prev);
	_dsll_head = dsllLink->_dsllLink_next;
      }
      if (_dsll_tail == dsllLink) {
	assert(!dsllLink->_dsllLink_next);
	_dsll_tail = dsllLink->_dsllLink_prev;
      }
      if (dsllLink->_dsllLink_next)
        dsllLink->_dsllLink_next->_dsllLink_prev = dsllLink->_dsllLink_prev;
      if (dsllLink->_dsllLink_prev)
        dsllLink->_dsllLink_prev->_dsllLink_next = dsllLink->_dsllLink_next;
      _dsll_numberOfListElements--;
      dsllLink->_dsllLink_element->_dsllElement_LinkCount--;
      delete dsllLink;
      assert(AssertLinkedList());
      return;
    }
    else
    {
      dsllLink = dsllLink->_dsllLink_next;
      i++;
    }
  }
  // Element not found on list - could probably just ignore the
  // request but assert out so it can be checked to see if it is
  // a programming failure...
  assert(dsllLink != 0);
}

// Object: SdmDoubleLinkedList
// Method: AddElementToList
// Description:
// Given an element that can be added to the list, add the element
// to the end of list.
//
void SdmDoubleLinkedList::AddElementToList(SdmLinkedListElement *lle)
{
  assert(AssertLinkedList());
  assert(lle);
  assert(lle->_dsllElement_object);

  // If the list is empty, place this element directly at the head
  // else place this element after the last element on the list
  //
  if (!_dsll_head) {
    assert(!_dsll_numberOfListElements);
    _dsll_head = new SdmDoubleLinkedListLink(lle);
    _dsll_tail = _dsll_head;
  }
  else {
    assert(_dsll_numberOfListElements);
    _dsll_tail->_dsllLink_next = new SdmDoubleLinkedListLink(lle);
    _dsll_tail->_dsllLink_next->_dsllLink_prev = _dsll_tail;
    _dsll_tail = _dsll_tail->_dsllLink_next;
  }
  _dsll_numberOfListElements++;
  lle->_dsllElement_LinkCount++;
  assert(AssertLinkedList());
}

// Object: SdmLinkedList
// Method: FindElement
// Description:
// Find the given element in the list and return its index
//
int SdmDoubleLinkedList::FindElement(SdmLinkedListElement *lle)
{
  assert(AssertLinkedList());
  assert(lle);
 
  SdmDoubleLinkedListLink *dsllLink = _dsll_head;
  int index = 0;
 
  while (dsllLink) {
    if (dsllLink->_dsllLink_element == lle) {
      // Found this element on the list
      return index;
    }
    dsllLink = dsllLink->_dsllLink_next;
    index++;
  }
  return Sdm_Not_Found;
}

// Object: SdmDoubleLinkedListIterator
// Method: Constructor
// Description:
// Create a reset iterator that points to the indicated linked list
//
SdmDoubleLinkedListIterator::SdmDoubleLinkedListIterator(SdmDoubleLinkedList *ll)
{
  _dsllIterator_list = ll;
  _dsllIterator_current = (SdmDoubleLinkedList::SdmDoubleLinkedListLink *)0;
  _dsllIterator_next = (SdmDoubleLinkedList::SdmDoubleLinkedListLink *)0;
  _dsllIterator_prev = (SdmDoubleLinkedList::SdmDoubleLinkedListLink *)0;
}

// Object: SdmDoubleLinkedListIterator
// Method: Destructor
// Description:
// Reset the iterator, causing any next to fetch from the beginning
//
SdmDoubleLinkedListIterator::~SdmDoubleLinkedListIterator()
{
  if (_dsllIterator_current)
    ResetIterator();
}

// Object: SdmDoubleLinkedListIterator
// Method: ResetIterator
// Description:
// Reset the iterator - the next invokation of NextListElement will
// retrieve the first element
//
void SdmDoubleLinkedListIterator::ResetIterator()
{
  _dsllIterator_current = (SdmDoubleLinkedListLink *)0;
}


// Object: SdmDoubleLinkedListIterator
// Method: NextListElement
// Description:
// Return the next element on the linked list.
//
void *SdmDoubleLinkedListIterator::NextListElement() {
  // If the list object itself is NIL, just return
  // This happens if you create an iterator on a list item that
  // has not yet been created.
  //
  if (!_dsllIterator_list)
    return((void *)0);

  // Determine the "next" element to return:
  // -> if the list is empty, there is no current element
  // -> if there is no current element, fetch the first element
  // -> if there is a current element, fetch the next element
  //
  if (!_dsllIterator_list)
    _dsllIterator_current = _dsllIterator_next = (SdmDoubleLinkedListLink *)0;
  else if (!_dsllIterator_current)
    _dsllIterator_current = _dsllIterator_list->ListHead();
  else
    _dsllIterator_current = _dsllIterator_current->_dsllLink_next;

  // If no current element returned we are at the end of the list;
  // return a null pointer; otherwise, set the next element to the 
  // current elements next and return the current elements undelrying object
  //
  if (!_dsllIterator_current)
    return((void *)0);
  else
    return(_dsllIterator_current->_dsllLink_element->_dsllElement_object);
}


// Object: SdmDoubleLinkedListIterator
// Method: PrevListElement
// Description:
// Return the next element on the linked list.
//
void *SdmDoubleLinkedListIterator::PrevListElement() {
  // If the list object itself is NIL, just return
  // This happens if you create an iterator on a list item that
  // has not yet been created.
  //
  if (!_dsllIterator_list)
    return((void *)0);

  // Determine the "previous" element to return:
  // -> if the list is empty, there is no current element
  // -> if there is no current element, fetch the first element
  // -> if there is a current element, fetch the prev element
  //
  if (!_dsllIterator_list)
    _dsllIterator_current = (SdmDoubleLinkedListLink *)0;
  else if (!_dsllIterator_current)
    _dsllIterator_current = _dsllIterator_list->ListTail();
  else
    _dsllIterator_current = _dsllIterator_current->_dsllLink_prev;

  // If no current element returned we are at the beginning of the list;
  // return a null pointer; otherwise, set the prev element to the 
  // current elements prev and return the current elements underlying object
  //
  if (!_dsllIterator_current)
    return((void *)0);
  else
    return(_dsllIterator_current->_dsllLink_element->_dsllElement_object);
}

//--------------------------------------------------------------------


