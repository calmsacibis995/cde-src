/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm Token Object Class.

#pragma ident "@(#)Token.cc	1.46 97/03/14 SMI"

// Include Files
#include <SDtMail/Token.hh>

#define TOKEN_SIGNATURE_VALID (_TokenObjSignature && _TokenObjSignature <= _TokenObjSignatureCounter)

SdmObjectSignature SdmToken::_TokenObjSignatureCounter = 0;   // 0 reserved for "none"

SdmToken::SdmToken()
  : SdmLinkedListElement(this)
{
  _TokenObjSignature = ++_TokenObjSignatureCounter;
}

SdmToken::~SdmToken()
{
  assert(TOKEN_SIGNATURE_VALID);
  ClearTokenList();
  _TokenObjSignature = 0;	// set signature to invalid so further use is flagged as error
}


SdmToken::SdmToken(const SdmToken &copy)
  : SdmLinkedListElement(this)
{
  _TokenObjSignature = ++_TokenObjSignatureCounter;
  *this = copy;
}

SdmToken&
SdmToken::operator=(const SdmToken &rhs)
{
  if (&rhs != this) {    
    ClearTokenList();
    SdmToken *copy = (SdmToken*)&rhs;

    SdmLinkedListIterator lit(&(copy->_tkcList));
    SdmToken_List *tkl;

    assert(TOKEN_SIGNATURE_VALID);
    while (tkl = (SdmToken_List *)(lit.NextListElement())) {

      if (!tkl->tkc_value.Length()) {
        _tkcList.AddElementToList(new SdmToken_List(tkl->tkc_name, 
            tkl->tkc_parameter));
      } else {
        _tkcList.AddElementToList(new SdmToken_List(tkl->tkc_name, 
            tkl->tkc_parameter, tkl->tkc_value));
      }
    }
  }

  // note: don't change the _TokenObjSignature.  it should stay the same.
  return *this;
}

void SdmToken::ClearTokenList()
{
  SdmLinkedListIterator lit(&_tkcList);
  SdmToken_List *tkl;

  assert(TOKEN_SIGNATURE_VALID);
  while (tkl = (SdmToken_List *)(lit.NextListElement())) {
    _tkcList.RemoveElementFromList(tkl);
    delete tkl;
  }
}

// See if a specific name/parameter pair is set and return a boolean indication
//
SdmBoolean SdmToken::CheckValue(SdmString &tkcName, SdmString &tkcParameter) const
{
  SdmLinkedListIterator lit((SdmLinkedList*)&_tkcList);
  SdmToken_List *tkl;

  assert(TOKEN_SIGNATURE_VALID);
  while (tkl = (SdmToken_List *)(lit.NextListElement())) {
    // Only if both values match and the value is empty do we return true
    //
    if (tkl->tkc_name != tkcName)
      continue;
    if (tkl->tkc_parameter != tkcParameter)
      continue;
    if (tkl->tkc_value.Length() != 0)
      continue;
    return(Sdm_True);
  }

  // If we drop through to here it means this entry is not present on the list
  return(Sdm_False);
}

// See if a specific name/parameter/value value is set and return a boolean indication
//
SdmBoolean SdmToken::CheckValue(SdmString &tkcName, SdmString &tkcParameter, SdmString &tkcValue) const
{
  SdmLinkedListIterator lit((SdmLinkedList*)&_tkcList);
  SdmToken_List *tkl;

  assert(TOKEN_SIGNATURE_VALID);
  while (tkl = (SdmToken_List *)(lit.NextListElement())) {
    // Only if all three values match do we return true
    //
    if (tkl->tkc_name != tkcName)
      continue;
    if (tkl->tkc_parameter != tkcParameter)
      continue;
    if (tkl->tkc_value.Length() == 0)
      continue;
    if (tkl->tkc_value != tkcValue)
      continue;
    return(Sdm_True);
  }

  // If we drop through to here it means this entry is not present on the list
  return(Sdm_False);
}

// Compare this token exactly against another
// Spin through our list of tokens and insist on a match from
// the passed in token
//
SdmBoolean SdmToken::CompareExact(const SdmToken &tk) const
{
  SdmTokenIterator myTkIt(this);
  SdmBoolean mySearchResult;
  SdmString myName, myParam, myVal;

  assert(TOKEN_SIGNATURE_VALID);
  if (_TokenObjSignature == tk._TokenObjSignature)
    return(Sdm_True);

  while ( (mySearchResult = myTkIt.NextToken(myName, myParam, myVal)) == Sdm_True) {
    SdmTokenIterator targTkIt(&tk);
    SdmString targName, targParam, targVal;
    SdmBoolean targSearchResult;

    while (( targSearchResult = targTkIt.NextToken(targName, targParam, targVal)) == Sdm_True) {
      if (myName.CaseCompare (targName) != SdmString::Sdm_Equal)
	continue;
      if (myParam.CaseCompare (targParam) != SdmString::Sdm_Equal)
	continue;
      if (targVal.Length() != myVal.Length())
	return(Sdm_False);
      if (*(const char *)targVal && 
        (myVal.CaseCompare (targVal) != SdmString::Sdm_Equal))
	continue;
      break;
    }
    // if the target search result is NIL, then we have a token that
    // has no corresponding match in the target - flunk the comparison
    //
    if (!targSearchResult)
      return(Sdm_False);
  }
  return(Sdm_True);
}

// Compare this token loosely against another
// Spin through our list of tokens and insist on a match only
// on those tokens that also exist in the target token
//
// The idea of a "loose match" is that you have a "reference" token that
// contains a series of values that represent all aspects of an object;
// and you have a matching token that represents the aspects of a specific
// object, and the "loose match" is successful if the aspects of the
// specific object are represented as a subset of the aspects of the
// reference object.
//
// Each token has three possible "values": name, parameter, value
// There are two permissible forms: name+parameter, or name+parameter+value
//
//
SdmBoolean SdmToken::CompareLoose(const SdmToken &tk) const
{
  SdmTokenIterator myTkIt(this);
  SdmBoolean mySearchResult;
  SdmString myName, myParam, myVal;

  assert(TOKEN_SIGNATURE_VALID);

  if (_TokenObjSignature == tk._TokenObjSignature)
    return(Sdm_True);

  // This outer while picks off the members of the refernece token one at a time
  //
  while ( (mySearchResult = myTkIt.NextToken(myName, myParam, myVal)) == Sdm_True) {
    SdmTokenIterator targTkIt(&tk);
    SdmString targName, targParam, targVal;
    SdmBoolean targSearchResult;
    SdmBoolean targNameSeen = Sdm_False;
    int myValLen = myVal.Length();

    // This inner loop spins through the members of the comparison token
    //
    while (( targSearchResult = targTkIt.NextToken(targName, targParam, targVal)) == Sdm_True) {
      int targValLen = targVal.Length();

      // See if the name matches - if not, go on to the next
      //
      if (myName.CaseCompare (targName) != SdmString::Sdm_Equal)
	continue;

      // See if the parameter matches - 
      // -- If the parameters match:
      // ---- If both have no values, submatch successful: continue
      //		("a b" == "a b")
      // ---- If both have values and they match, submatch successful: continue
      //		("a b c" == "a b c")
      // ---- If one has a value and the other does not, entire match fails: return false
      //		("a b" != "a b c" || "a b c" != "a b")
      // ---- ELSE get the next pair and search for a match
      // -- if the parameters do not match:
      // ---- If neither one has a value, this means that we found a "name match"
      //	that did not have matching parameters:
      // 		("a d" != "a b")
      //	In this case we remember this happened, and continue, because their
      //	may be a further matching name entry where the parameter will match.
      //	If we exhaust all target possibilities and this condition was remembered,
      //	then the entire match fails.
      // ---- ELSE get the next pair and search for a match
      //
      if (myParam.CaseCompare (targParam) == SdmString::Sdm_Equal) {
	// If both have no values, this sub-match is successful
	if (!myValLen && !targValLen) {
	  targNameSeen = Sdm_False;
	  break;
	}
	// If both have values and the values match, this sub-match is successful
	if ((myValLen == targValLen) && 
          (myVal.CaseCompare (targVal) == SdmString::Sdm_Equal))
	  break;
	// If one has a value and the other does not, this entire match fails.
	if (myValLen != targValLen)
	  return(Sdm_False);
      }
      else {
	if (!myValLen && !targValLen)
	  targNameSeen = Sdm_True;	// remember name seen with non-matching value
      }
    }
    // If the target search result is NIL, it means we exhausted the target 
    // and did not find a corresponding match. If the target name was seen
    // it means that we didnt find a matching parameter so the comparison fails.
    // Otherwise, this means that we have a token that
    // has no corresponding match in the target - since this is a loose
    // comparison, we can ignore it.
    if (targNameSeen)
      return(Sdm_False);
  }
  return(Sdm_True);
}

// Object: SdmTokenIterator
// Method: NextToken
// What: return the next token in the token list
// Description:
// Basically inherits the hard part from SdmLinkedListIterator which is primed
// when the object is created. This method simple fetches the next element
// (a SdmToken_List object) from the list - if present sets the passed in
// string arguments appropriately and returns Sdm_True, else returns Sdm_False.
// In all other ways acts just like a linked list iterator.
//
SdmBoolean SdmTokenIterator::NextToken(SdmString &tkcName, SdmString &tkcParameter, SdmString &tkcValue) 
{
  // Fetch the next token
  SdmToken_List *tkc = (SdmToken_List *)(NextListElement());

  // If this is the last token value, return Sdm_False
  if (!tkc)
    return(Sdm_False);

  // Got another value, set the passed in paramters and return Sdm_True
  tkcName = tkc->tkc_name;
  tkcParameter = tkc->tkc_parameter;
  tkcValue = tkc->tkc_value;
  return(Sdm_True);
};

// set a token given a name/parameter pair only
// scan through the list of defined tokens looking for this name/parameter pair
// if the pair is found already set, do nothing
// if the pair is found but subvalues are set, remove sub values
//
void SdmToken::SetValue(SdmString &tkcName, SdmString &tkcParameter)
{
  SdmLinkedListIterator lit(&_tkcList);
  SdmToken_List *tkl;

  assert(TOKEN_SIGNATURE_VALID);
  while (tkl = (SdmToken_List *)(lit.NextListElement())) {
    // If token name does not match, skip to next
    //
    if (tkl->tkc_name != tkcName)
      continue;
    // Token name matches - if parameter does not match, skip to next
    assert(tkl->tkc_parameter);
    if (tkl->tkc_parameter != tkcParameter)
      continue;
    // Token name and parameter match - if no subparameter we are all done
    if (!tkl->tkc_value.Length())
      return;
    // Subparameter present - remove from list
    _tkcList.RemoveElementFromList(tkl);
    delete tkl;
  }

  // If we drop through to here it means this entry is not present on the list
  // Add one
  assert(!tkl);
  _tkcList.AddElementToList(new SdmToken_List(tkcName, tkcParameter));
}

// set a token given a name/parameter/value triple
// scan through the list of defined tokens looking for this value triple
// if it is found, do nothing
// if it is found but only the name/value is set, remove it
//
void SdmToken::SetValue(SdmString &tkcName, SdmString &tkcParameter, SdmString &tkcValue)
{
  SdmLinkedListIterator lit(&_tkcList);
  SdmToken_List *tkl;

  assert(TOKEN_SIGNATURE_VALID);
  while (tkl = (SdmToken_List *)(lit.NextListElement())) {
    // If token name does not match, skip to next
    //
    if (tkl->tkc_name != tkcName)
      continue;
    // Token name matches - if parameter does not match, skip to next
    assert(tkl->tkc_parameter);
    if (tkl->tkc_parameter != tkcParameter)
      continue;
    // Token name and parameter match - if subparameter found and matches, return
    if (tkl->tkc_value.Length()) {
      if (tkl->tkc_value == tkcValue)
	return;
      continue;
    }
    // No subparameter present - remove from list
    _tkcList.RemoveElementFromList(tkl);
    delete tkl;
  }

  // If we drop through to here it means this entry is not present on the list
  // Add one
  assert(!tkl);
  _tkcList.AddElementToList(new SdmToken_List(tkcName, tkcParameter, tkcValue));
}

void SdmToken::ClearValue(SdmString &tkcName)
{
  SdmLinkedListIterator lit(&_tkcList);
  SdmToken_List *tkl;

  assert(TOKEN_SIGNATURE_VALID);
  while (tkl = (SdmToken_List *)(lit.NextListElement())) {
    // If token name does not match, skip to next
    //
    if (tkl->tkc_name != tkcName)
      continue;
    // Token name matches - remove occurance regardless of parameter/value
    _tkcList.RemoveElementFromList(tkl);
    delete tkl;
  }
}

void SdmToken::ClearValue(SdmString &tkcName, SdmString &tkcParameter)
{
  SdmLinkedListIterator lit(&_tkcList);
  SdmToken_List *tkl;

  assert(TOKEN_SIGNATURE_VALID);
  while (tkl = (SdmToken_List *)(lit.NextListElement())) {
    // If token name does not match, skip to next
    //
    if (tkl->tkc_name != tkcName)
      continue;
    // Token name matches - if parameter does not match, skip to next
    assert(tkl->tkc_parameter);
    if (tkl->tkc_parameter != tkcParameter)
      continue;
    // Token and parameter matches - remove occurrance regardless of value
    _tkcList.RemoveElementFromList(tkl);
    delete tkl;
  }
}

void SdmToken::ClearValue(SdmString &tkcName, SdmString &tkcParameter, SdmString &tkcValue)
{
  SdmLinkedListIterator lit(&_tkcList);
  SdmToken_List *tkl;

  assert(TOKEN_SIGNATURE_VALID);
  while (tkl = (SdmToken_List *)(lit.NextListElement())) {
    // If token name does not match, skip to next
    //
    if (tkl->tkc_name != tkcName)
      continue;
    // Token name matches - if parameter does not match, skip to next
    assert(tkl->tkc_parameter);
    if (tkl->tkc_parameter != tkcParameter)
      continue;
    // Token name and parameter match - if value found and matches, remove it
    if (tkl->tkc_value.Length()) {
      if (tkl->tkc_value == tkcValue) {
	_tkcList.RemoveElementFromList(tkl);
	delete tkl;
      }
    }
  }
}

// Fetch a singly named value
//
SdmBoolean SdmToken::FetchValue(SdmString &r_tkcParameter, SdmString &tkcName) const
{
  SdmLinkedListIterator lit((SdmLinkedList*)&_tkcList);
  SdmToken_List *tkl;

  assert(TOKEN_SIGNATURE_VALID);
  while (tkl = (SdmToken_List *)(lit.NextListElement())) {
    // If token name does not match, skip to next
    //
    if (tkl->tkc_name != tkcName)
      continue;
    // Token name matches - if subparameter is set, this is not a singly
    // named value - return an error
    if (tkl->tkc_value.Length())
      return(Sdm_False);
    // Token name matches and no subparameter is set, return parameter value
    assert(tkl->tkc_parameter);
    r_tkcParameter = tkl->tkc_parameter;
    return(Sdm_True);
  }

  // If we drop through to here it means this entry is not present on the list
  return(Sdm_False);
}

// Fetch a doubly named value
//
SdmBoolean SdmToken::FetchValue(SdmString &r_tkcValue, SdmString &tkcName, SdmString &tkcParameter) const
{
  SdmLinkedListIterator lit((SdmLinkedList*)&_tkcList);
  SdmToken_List *tkl;

  assert(TOKEN_SIGNATURE_VALID);
  while (tkl = (SdmToken_List *)(lit.NextListElement())) {
    // If token name does not match, skip to next
    //
    if (tkl->tkc_name != tkcName)
      continue;
    // Token name matches - if parameter does not match, skip to next
    assert(tkl->tkc_parameter);
    if (tkl->tkc_parameter != tkcParameter)
      continue;
    // Token name matches - if subparameter is not set, this is not a doubly
    // named value - return an error
    if (!tkl->tkc_value.Length())
      return(Sdm_False);
    // Token name matches and no subparameter is set, return parameter value
    r_tkcValue = tkl->tkc_value;
    return(Sdm_True);
  }

  // If we drop through to here it means this entry is not present on the list
  return(Sdm_False);
}
