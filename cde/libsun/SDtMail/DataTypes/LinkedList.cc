/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the linked list object class.

#pragma ident "@(#)LinkedList.cc	1.24 97/03/21 SMI"

// Include Files.
#include <SDtMail/LinkedList.hh>

#if !defined(NDEBUG)


int	SdmLinkedList::AssertLinkedList()
{
  assert(_dsll_numberOfListElements > 1 || (_dsll_tail == _dsll_head));
  assert(_dsll_numberOfListElements > 0 || ((!_dsll_tail) && (!_dsll_head)));
  assert(_dsll_numberOfListElements == 0 || (_dsll_tail && _dsll_head));
  return(1);
}
#endif



// Object: SdmLinkedList
// Method: Constructor
// Description: 
// Create an empty list
//
SdmLinkedList::SdmLinkedList()
{
  _dsll_head = (SdmLinkedListLink *)0;
  _dsll_tail = (SdmLinkedListLink *)0;
  _dsll_numberOfListElements = 0;
}

// Object: SdmLinkedList
// Method: Destructor
// Description:
// Empty the list of all items
//
SdmLinkedList::~SdmLinkedList()
{
  // Remove all elements
  ClearAllElements();
}

SdmLinkedListElement *SdmLinkedList::operator[] (int index)
{
  assert(_dsll_numberOfListElements);
  assert(AssertLinkedList());
 
  SdmLinkedListLink *dsllLink = _dsll_head;
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

// Object: SdmLinkedList
// Method: RemoveElementFromList_L
// Description:
// Given an element that is on the list, walk the list and
// remove one instance of the element from the list. Adjust
// various list and link variables as appropriate.
//
void SdmLinkedList::RemoveElementFromList(SdmLinkedListElement *lle)
{
  assert(lle);
  assert(lle->_dsllElement_object);
  assert(_dsll_numberOfListElements);
  assert(AssertLinkedList());

  SdmLinkedListLink *dsllLink = _dsll_head;
  SdmLinkedListLink *dsllLink_prev = 0;

  // Spin through looking for this entry in the list
  // dsllLink points to "current" entry
  // dsllLink_prev points to the "previous" entry in the scan
  //
  while (dsllLink) {
    if (dsllLink->_dsllLink_element == lle) {
      // Found this element on the list - remove it
      assert(lle->_dsllElement_LinkCount);
      if (_dsll_head == dsllLink) {
	assert (!dsllLink_prev);
	_dsll_head = dsllLink->_dsllLink_next;
      }
      if (_dsll_tail == dsllLink) {
	assert(!dsllLink->_dsllLink_next);
	_dsll_tail = dsllLink_prev;
      }
      if (dsllLink_prev)
	dsllLink_prev->_dsllLink_next = dsllLink->_dsllLink_next;
      _dsll_numberOfListElements--;
      lle->_dsllElement_LinkCount--;
      delete dsllLink;
      assert(AssertLinkedList());
      return;
    }
    dsllLink_prev = dsllLink;
    dsllLink = dsllLink->_dsllLink_next;
  }
  // Element not found on list - could probably just ignore the
  // request but assert out so it can be checked to see if it is
  // a programming failure...
  assert(dsllLink != 0);
}

// Object: SdmLinkedList
// Method: RemoveElementFromList_L
// Description:
// Given an element that is on the list, walk the list and
// remove one instance of the element from the list at the
// index specified. Adjust various list and link variables
// as appropriate.
//
void SdmLinkedList::RemoveElementFromList(int index)
{
  assert(_dsll_numberOfListElements);
  assert(AssertLinkedList());
 
  SdmLinkedListLink *dsllLink = _dsll_head;
  SdmLinkedListLink *dsllLink_prev = 0;
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
        assert (!dsllLink_prev);
        _dsll_head = dsllLink->_dsllLink_next;
      }
      if (_dsll_tail == dsllLink) {
        assert(!dsllLink->_dsllLink_next);
        _dsll_tail = dsllLink_prev;
      }
      if (dsllLink_prev)
	dsllLink_prev->_dsllLink_next = dsllLink->_dsllLink_next;
      _dsll_numberOfListElements--;
      dsllLink->_dsllLink_element->_dsllElement_LinkCount--;
      delete dsllLink;
      assert(AssertLinkedList());
      return;
    }
    else
    {
      dsllLink_prev = dsllLink;
      dsllLink = dsllLink->_dsllLink_next;
      i++;
    }
  }
  // Element not found on list - could probably just ignore the
  // request but assert out so it can be checked to see if it is
  // a programming failure...
  assert(dsllLink != 0);
}


// Object: SdmLinkedList
// Method: ClearAllElements
// Description:
// Remove all elements from the linked list.
//
void SdmLinkedList::ClearAllElements(void)
{
  SdmLinkedListLink *dsllLink = _dsll_head, *currentLink;
  long expectedElements = _dsll_numberOfListElements;
  
  _dsll_head = (SdmLinkedListLink *)0;
  _dsll_tail = (SdmLinkedListLink *)0;
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

// Object: SdmLinkedList
// Method: ClearAndDestroyAllElements
// Description:
// Remove all elements from the linked list.  Delete element if link count is zero.
//
void SdmLinkedList::ClearAndDestroyAllElements(void)
{
  SdmLinkedListLink *dsllLink = _dsll_head, *currentLink;
  long expectedElements = _dsll_numberOfListElements;
  
  _dsll_head = (SdmLinkedListLink *)0;
  _dsll_tail = (SdmLinkedListLink *)0;
  _dsll_numberOfListElements = 0;
  
  // While there are any items on the list, remove them
  //
  while (dsllLink) {
    currentLink = dsllLink;
    dsllLink = dsllLink->_dsllLink_next;
    currentLink->_dsllLink_element->_dsllElement_LinkCount--;  // decrement count;
    
    // delete element only if it's link count is zero.
    if (!currentLink->_dsllLink_element->_dsllElement_LinkCount) {
      delete (currentLink->_dsllLink_element);
    }
    delete currentLink;
    expectedElements--;
  }
  
  // The list must now be empty - assert empty conditions
  //
  assert(!expectedElements);
  assert(AssertLinkedList());
}


// Object: SdmLinkedList
// Method: AddElementToList
// Description:
// Given an element that can be added to the list, add the element
// to the end of list.
//
void SdmLinkedList::AddElementToList(SdmLinkedListElement *lle)
{
  assert(AssertLinkedList());
  assert(lle);
  assert(lle->_dsllElement_object);

  // If the list is empty, place this element directly at the head
  // else place this element after the last element on the list
  //
  if (!_dsll_head) {
    assert(!_dsll_numberOfListElements);
    _dsll_head = new SdmLinkedListLink(lle);
    _dsll_tail = _dsll_head;
  }
  else {
    assert(_dsll_numberOfListElements);
    _dsll_tail->_dsllLink_next = new SdmLinkedListLink(lle);
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
int SdmLinkedList::FindElement(SdmLinkedListElement *lle)
{
  assert(AssertLinkedList());
  assert(lle);

  SdmLinkedListLink *dsllLink = _dsll_head;
  int index = 0;

  while (dsllLink) {
    if (dsllLink->_dsllLink_element == lle) {
      // Found this element on the list
      return index;
    }
    dsllLink = dsllLink->_dsllLink_next;
    index++;
  }
  return -1; // Sdm_Not_Found
}

// Object: SdmLinkedListIterator
// Method: Constructor
// Description:
// Create a reset iterator that points to the indicated linked list
//
SdmLinkedListIterator::SdmLinkedListIterator(SdmLinkedList *ll)
{
  _dsllIterator_list = ll;
  _dsllIterator_current = (SdmLinkedList::SdmLinkedListLink *)0;
  _dsllIterator_next = (SdmLinkedList::SdmLinkedListLink *)0;
}

// Object: SdmLinkedListIterator
// Method: Destructor
// Description:
// Reset the iterator, causing any next to fetch from the beginning
//
SdmLinkedListIterator::~SdmLinkedListIterator()
{
  if (_dsllIterator_current)
    ResetIterator();
}

// Object: SdmLinkedListIterator
// Method: ResetIterator
// Description:
// Reset the iterator - the next invokation of NextListElement will
// retrieve the first element
//
void SdmLinkedListIterator::ResetIterator()
{
  _dsllIterator_current = _dsllIterator_next = (SdmLinkedListLink *)0;
}

// Object: SdmLinkedListIterator
// Method: NextListElement
// Description:
// Return the next element on the linked list.
//
void *SdmLinkedListIterator::NextListElement() {
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
    _dsllIterator_current = _dsllIterator_next = (SdmLinkedListLink *)0;
  else if (!_dsllIterator_current)
    _dsllIterator_current = _dsllIterator_list->ListHead();
  else
    _dsllIterator_current = _dsllIterator_next;

  // If no current element returned we are at the end of the list;
  // return a null pointer; otherwise, set the next element to the 
  // current elements next and return the current elements undelrying object
  //
  if (!_dsllIterator_current) {
    _dsllIterator_next = (SdmLinkedListLink *)0;
    return((void *)0);
    }
  _dsllIterator_next = _dsllIterator_current->_dsllLink_next;

  return(_dsllIterator_current->_dsllLink_element->_dsllElement_object);
}
