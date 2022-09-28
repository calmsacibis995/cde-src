/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm Token Object Class.

#ifndef _SDM_TOKEN_H
#define _SDM_TOKEN_H

#pragma ident "@(#)Token.hh	1.41 97/04/29 SMI"

// Include Files
#include <SDtMail/Sdtmail.hh>
#include <SDtMail/Prim.hh>
#include <SDtMail/LinkedList.hh>

class SdmTokenIterator;

class SdmToken : public SdmLinkedListElement {

public:
  SdmToken();
  virtual ~SdmToken();

  SdmToken(const SdmToken&);
  SdmToken& operator=(const SdmToken&);

public:
  SdmObjectSignature	Signature() { return (_TokenObjSignature); }
  SdmBoolean    CheckValue(SdmString &tkcName, SdmString &tkcParameter) const;
  SdmBoolean    CheckValue(SdmString &tkcName, SdmString &tkcParameter, SdmString &tkcValue) const;
  void		ClearTokenList();
  void		ClearValue(SdmString &tkcName);
  void		ClearValue(SdmString &tkcName, SdmString &tkcParameter);
  void		ClearValue(SdmString &tkcName, SdmString &tkcParameter, SdmString &tkcValue);
  SdmBoolean	CompareExact(const SdmToken &tk) const;
  SdmBoolean	CompareLoose(const SdmToken &tk) const;
  SdmBoolean	FetchValue(SdmString &r_tkcParameter, SdmString &tkcName) const;
  SdmBoolean	FetchValue(SdmString &r_tkcValue, SdmString &tkcName, SdmString &tkcParameter) const;
  void		SetValue(SdmString &tkcName, SdmString &tkcParameter);
  void		SetValue(SdmString &tkcName, SdmString &tkcParameter, SdmString &tkcValue);

  // convenience definitions allowing a const char * to be passed in as
  // a SdmString parameter, causing one to be created automagically
  //
  void		SetValue(const char *tkcName, const char *tkcParameter) 
                { SetValue(SdmString(tkcName), SdmString(tkcParameter)); }
  void		SetValue(const char *tkcName, const char *tkcParameter, const char *tkcValue)
                { SetValue(SdmString(tkcName), SdmString(tkcParameter), SdmString(tkcValue)); }
  SdmBoolean    CheckValue(char *tkcName, char *tkcParameter) const
                { return(CheckValue(SdmString(tkcName), SdmString(tkcParameter))); }
  SdmBoolean    CheckValue(char *tkcName, char *tkcParameter, char *tkcValue) const
                { return(CheckValue(SdmString(tkcName), SdmString(tkcParameter), SdmString(tkcValue))); }
  SdmBoolean    CheckValue(const char *tkcName, const char *tkcParameter) const
                { return(CheckValue(SdmString(tkcName), SdmString(tkcParameter))); }
  SdmBoolean    CheckValue(const char *tkcName, const char *tkcParameter, const char *tkcValue) const
                { return(CheckValue(SdmString(tkcName), SdmString(tkcParameter), SdmString(tkcValue))); }
  void		ClearValue(const char *tkcName)
                { ClearValue(SdmString(tkcName)); }
  void		ClearValue(const char *tkcName, const char *tkcParameter)
                { ClearValue(SdmString(tkcName), SdmString(tkcParameter)); }
  void		ClearValue(const char *tkcName, const char *tkcParameter, const char *tkcValue)
                { ClearValue(SdmString(tkcName), SdmString(tkcParameter), SdmString(tkcValue)); }
  SdmBoolean	FetchValue(SdmString &r_tkcParameter, const char *tkcName) const
                { return(FetchValue(r_tkcParameter, SdmString(tkcName))); }
  SdmBoolean	FetchValue(SdmString &r_tkcValue, const char *tkcName, const char *tkcParameter) const
                { return(FetchValue(r_tkcValue, SdmString(tkcName), SdmString(tkcParameter))); }

private:
  friend	SdmTokenIterator;
  static SdmObjectSignature	_TokenObjSignatureCounter;	// highest signature to date
  SdmObjectSignature		_TokenObjSignature;		// signature of this token
  SdmLinkedList			_tkcList;			// list of all token components
};

class SdmToken_List : public SdmLinkedListElement {

friend SdmToken;

public:
  SdmToken_List(SdmString &tkcName, SdmString &tkcParameter) : SdmLinkedListElement(this) {
    tkc_name = tkcName;
    tkc_parameter = tkcParameter;
    tkc_value = "";
  }

  SdmToken_List(SdmString &tkcName, SdmString &tkcParameter, SdmString &tkcValue) : SdmLinkedListElement(this) {
    tkc_name = tkcName;
    tkc_parameter = tkcParameter;
    tkc_value = tkcValue;
  }

  virtual ~SdmToken_List() {};

private:
  friend	SdmTokenIterator;
    SdmString	tkc_name;
    SdmString	tkc_parameter;
    SdmString	tkc_value;
};

class SdmTokenIterator : public SdmLinkedListIterator {
public:
  SdmTokenIterator(const SdmToken *tk) : SdmLinkedListIterator((SdmLinkedList*)&tk->_tkcList) { assert(tk); };
  virtual ~SdmTokenIterator() {};
  SdmBoolean SdmTokenIterator::NextToken(SdmString &tkcName, SdmString &tkcParameter, SdmString &tkcValue);
};
#endif
