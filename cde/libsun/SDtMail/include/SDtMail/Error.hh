/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm Error Class.
// --> It implements the "error container" that is the basis
// --> for all error handling in the library

#ifndef _SDM_ERROR_H
#define _SDM_ERROR_H

#pragma ident "@(#)Error.hh	1.38 97/01/29 SMI"

// Includes

#include <stdarg.h>
#include <string.h>

// Forward declarations

class SdmInternalErrorObject;

// SdmError object
// 

class SdmError {

public:

  // constructors and destructor

  SdmError();
  SdmError (const SdmError&);
  virtual ~SdmError();

  // operators

  SdmError& operator= (const SdmErrorCode val);		// reset container, set major error code
  SdmError& operator= (const SdmError& rhs);		// set contents from existing error object
  operator SdmErrorCode() const 			// return major error code
  { return(_majorErrorCode); }
  SdmErrorCode operator[] (const int minorIndex);	// return specified minor error code

  // methods

  void		AddMajorErrorCode(SdmErrorCode errorCode);	// add major error, pushing any current to minor list
  void		AddMajorErrorCodeAndMessage(SdmErrorCode errorCode, ...);	// add major error and format message, pushing any current to minor list
  void		AddMajorErrorCodeAndMessageWithFormat(SdmErrorCode errorCode, const char* format, ...);	// add major error and format message, pushing any current to minor list
  void		AddMinorErrorCode(SdmErrorCode errorCode);	// add minor error
  void		AddMinorErrorCodeAndMessage(SdmErrorCode errorCode, ...);	// add minor error and format message
  void		AddMinorErrorCodeAndMessageWithFormat(SdmErrorCode errorCode, const char* format, ...);	// add minor error and format message
  const char* 	ErrorMessage();					// return error message for major error
  const char*	ErrorMessage(const int minorIndex);		// return error message for specified minor error
  void		ErrorMessageMajorAndMinor(SdmString& r_minorMessages);	// return all major and minor errors in one string
  SdmErrorCode	ErrorCode() 					// return error code for major error
  { return(_majorErrorCode); }
  SdmErrorCode  ErrorCode(const int minorIndex);		// return error code for specified minor error
  SdmBoolean	IsErrorContained(const SdmErrorCode errorCode);	// return indication if error code is set anywhere in container
  int		MinorCount();					// return number of minor errors set
  const char* 	OnlineHelpIndex() 				// return online help index for major error
  { return (char *)0; }
  void		Reset() 					// reset this error container to empty state
  { *this = Sdm_EC_Success; }
  void		SetMajorErrorCode(SdmErrorCode errorCode)
  { *this = errorCode; }
  void		SetMajorErrorCodeAndMessage(SdmErrorCode errorCode, ...);

private:

  // Private data members and methods
  // The major error is held by a single internal error object "_majorErrorObject" with a copy 
  // of the code in "_majorErrorCode" for convenience in fetching in-line. 
  // Minor errors are held by a series of zero or more internal error objects
  // as indicated by "_minorErrorStackSize".

  void				_Reset();
  void				_PushMinorErrorCode(SdmInternalErrorObject* meo);
  SdmErrorCode			_majorErrorCode;
  SdmInternalErrorObject*	_majorErrorObject;
  SdmInternalErrorObject**	_minorErrorObjectStack;
  int				_minorErrorStackSize;
};

#endif
