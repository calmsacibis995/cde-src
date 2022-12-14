/*
 *+SNOTICE
 *
 *
 *	$Revision: 1.1 $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement bertween
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel wihtout
 *	Sun's specific written approval.  This documment and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)MIMEBodyPart.C	1.47 02/13/96"
#endif

#include <EUSCompat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <Dt/Dts.h>

#include <DtMail/DtMail.hh>
#include "RFCImpl.hh"
#include "RFCMIME.hh"
#include <DtMail/Threads.hh>
#include "md5.h"

// For CHARSET
#include <locale.h>
#include <LocaleXlate.h>

MIMEBodyPart::MIMEBodyPart(DtMailEnv & error,
			 DtMail::Message * parent,
			 const char * start,
			 const int len,
			 RFCEnvelope * body_env)
: RFCBodyPart(error, parent, start, len, body_env)
{
    // A single part message. We are done for now.
    //
    error.clear();
    return;
}

MIMEBodyPart::MIMEBodyPart(DtMailEnv & error,
			   DtMail::Message * parent,
			   const char * start,
			   const char ** end,
			   const char * boundary)
: RFCBodyPart(error, parent, start, 0, NULL)
{
    error.clear();

    // We are sitting at the start of a boundary. We need to get
    // past the boundary to do the real processing.
    //
    const char * body_end;
    for (body_end = _body_text; body_end <= *end && *body_end != '\n'; body_end++) {
	continue;
    }
    body_end += 1;

    if (body_end > *end) {
	// Don't know. Give up!
	*end = body_end;
	_body_len = *end - start + 1;

	// Need a bogus envelope for other uses.
	//
	_body_env = new RFCEnvelope(error, parent, NULL, 0);
	return;
    }

    const char * env_start = body_end;
    _my_env = DTM_TRUE;
    if (isTerm(env_start)) {
	_body_env = new RFCEnvelope(error, parent, env_start, 0);
    }
    else {
	// Find the blank line where the envelope ends.
	//
	for (; body_end <= *end; body_end++) {
	    if (*body_end == '\n') {
		int blank_only = 1;
		for (const char * blank = body_end + 1;
		     blank <= *end && *blank != '\n'; blank++) {
		    if (!isspace((unsigned char)*blank)) {
			blank_only = 0;
			break;
		    }
		}
		if (blank_only) {
		    break;
		}
	    }
	}
	_body_env = new RFCEnvelope(error,
				    parent,
				    env_start,
				    body_end - env_start + 1);
	body_end += 1;
    }

    // Chew everything up to the next new line, which should be only
    // a CRLF.
    //
    for (;body_end <= *end && *body_end != '\n'; body_end++) {
	continue;
    }
    body_end += 1;

    _body_text = body_end; // This is where the body really starts.

    // Now we need to find the end of the body. MIME doesn't have
    // any predefined length fields so we have to use the boundaries.
    //
    int bndry_len = strlen(boundary);
    for (;body_end <= *end; body_end++) {
	if (*body_end == '\n' &&
	    *(body_end + 1) == '-' &&
	    *(body_end + 2) == '-' &&
	    strncmp(body_end + 3, boundary, bndry_len) == 0) {
	    break;
	}
    }

    if (*(body_end - 1) == '\r') {
	body_end -= 1;
    }
    _body_len = body_end - _body_text + 1;

    // MIME says the CRLF preceding a boundary belongs to the boundary.
    // We will pull it off here rather than do it on entry to this
    // method for the next message.
    for (;body_end <= *end && *body_end != '\n'; body_end++) {
	continue;
    }
    body_end += 1;

    // Computing the end here is a little different. If the boundary
    // ends with a "--" as well, then we are at the real end of
    // the message.
    //
    const char * bndry_end = body_end + 2 + strlen(boundary);
    // Check to make sure we're not going off the end of the message.
    if ((bndry_end + 1) > *end)
	return;

    if (*bndry_end == '-' && *(bndry_end + 1) == '-') {
	*end = *end + 1;
    }
    else {
	*end = body_end;
    }

    return;
}

MIMEBodyPart::~MIMEBodyPart(void)
{
}

#ifdef DEAD_WOOD
DtMailChecksumState
MIMEBodyPart::checksum(DtMailEnv & error)
{
    error.clear();

    // Look for the Content-MD5 header. If it is not present, then
    // the state is unknown and we can punt.
    //
    DtMailEnv my_error;
    DtMailValueSeq value;
    _body_env->getHeader(my_error, "Content-MD5", DTM_FALSE, value);
    if (my_error.isSet()) {
	return(DtMailCheckUnknown);
    }


    if (_body_type == NULL) {
	getDtType(error);
	if (error.isSet()) {
	    return(DtMailCheckUnknown);
	}
    }

    char stored_digest[32];
    int stored_size = 0;
    RFCMIME::readBase64(stored_digest, stored_size,
			*(value[0]), strlen(*(value[0])));
    if (stored_size != 16) {
	// The MD5 sum must be 16 bytes, or we have a bad checksum.
	//
	return(DtMailCheckBad);
    }

    // See if we call this text. We need to handle md5 checksums
    // different for text. They must be computed with CRLF line
    // termination.
    //
    char * text_type = DtDtsDataTypeToAttributeValue(_body_type,
						     DtDTS_DA_IS_TEXT,
						     NULL);

    unsigned char digest[16];
    if (text_type && strcasecmp(text_type, "true") == 0) {
	RFCMIME::md5PlainText(_body, _body_decoded_len, digest);
    }
    else {
	MD5_CTX context;
	MD5Init(&context);
	MD5Update(&context, (unsigned char *)_body, _body_decoded_len);
	MD5Final(digest, &context);
    }

    free(text_type);

    if (memcmp(digest, stored_digest, sizeof(digest)) == 0) {
	return(DtMailCheckGood);
    }
    else {
	return(DtMailCheckBad);
    }
}
#endif /* DEAD_WOOD */

static int
countTypes(char ** types)
{
    for (int count = 0; *types; types++, count++) {
	continue;
    }

    return(count);
}

void
MIMEBodyPart::getDtType(DtMailEnv & error)
{
    MutexLock lock_scope(_obj_mutex);
    MutexLock dt_lib_lock(_DtMutex);

//    error.clear();

    char * mime_type;
    DtMailValueSeq value;

    if (_body_env) {
	_body_env->getHeader(error, "Content-Type", DTM_FALSE, value);
    }

    if (_body_env && !error.isSet()) {
        // Handle "Content-Type: text" problem with /usr/lib/mail.local
        //
        if (strcasecmp(*(value[0]), "text")==0)
	  mime_type = strdup("text/plain");
	else
	  mime_type = strdup(*(value[0]));
    }
    else {
	error.clear();
	mime_type = strdup("text/plain");
    }

    for (char * end = mime_type; *end; end++) {
	if (*end == ';' || isspace((unsigned char)*end)) {
	    break;
	}
	if (isupper(*end)) {
	    *end = tolower(*end);
	}
    }
    *end = 0;

    char ** types = DtDtsFindAttribute(DtDTS_DA_MIME_TYPE,
				       mime_type);

    // We must have an exact 1:1 mapping between the mime type and the
    // CDE type to use this inverse mapping. If we have no hits then we
    // dont have any thing to use. If we have several hits, then we have
    // no idea which type is the correct type.
    //
    if (countTypes(types) == 1) {
	// We will use the first name. It may be wrong, but
	// it is the best we can do at this point.
	//
	_body_type = strdup(types[0]);
	DtDtsFreeDataTypeNames(types);
	free(mime_type);
	return;
    }
    DtDtsFreeDataTypeNames(types);

    // We need the bits so we can type the buffer and get
    // a type for the object. This is where things can get
    // very slow for the user.
    //
    loadBody(error);
    if (error.isSet()) {
	return;
    }

    char * name = getNameHeaderVal(error);
    if (error.isSet()) {
	error.clear();
    }

    if (name == NULL) {
	// We have written a name pattern for text parts that will match
	// the name "text" as a TEXT part. If we have no name and we are
	// text/plain, then we will use this name rule to get the right
	// type for the part.
	//
	if (strcasecmp(mime_type, "text/plain") == 0) {
	    name = strdup("text");
	}
    }

    char * type = DtDtsBufferToDataType(_body, _body_decoded_len, name);
    if (!type) {
	_body_type = strdup("UNKNOWN");
    } else {
	_body_type = strdup(type);
	DtDtsFreeDataType(type);
    }
    if (mime_type)
      free(mime_type);
    if (name)
      free(name);
}

void
MIMEBodyPart::loadBody(DtMailEnv & error)
{
// For CHARSET
	char *cs = NULL, *to_cs = NULL, *from_cs = NULL;

// There is no reason to clear the error object because it is assumed 
// that whoever instantiated it, cleared it.
//  error.clear();
  
  if (_body) {
    return;
  }

  // If there is any encoding done to the body, reverse it
  //
  DtMailValueSeq value;
  if (_body_env) {
    _body_env->getHeader(error, "Content-Transfer-Encoding",
			 DTM_FALSE, value);
  }
  
  if (_body_env && !error.isSet()) {
    const char * enc = *(value[0]);
    if (strcasecmp(enc, "base64") == 0) {
      // Decoded bodies will always be smaller than
      // encoded bodies.
      //_body = (char *)malloc(_body_len); 

      _body = (char *)malloc(_body_len+1);
      int size = _body_decoded_len = 0;
      _must_free_body = DTM_TRUE;
      RFCMIME::readBase64(_body, size, _body_text, _body_len);
      _body_decoded_len = size;

      // Changed this temporarily until after release. We really should not
      // be null terminating these buffers.
      (_body)[_body_decoded_len] = 0;
    }
    else if (strcasecmp(enc, "quoted-printable") == 0) {
      _body = (char *)malloc(_body_len + 20);
      _must_free_body = DTM_TRUE;
      int size = _body_decoded_len = 0;
      RFCMIME::readQPrint(_body, size, _body_text, _body_len);
      _body_decoded_len = size;

      // Changed this temporarily until after release. We really should not
      // be null terminating these buffers.
      (_body)[_body_decoded_len] = 0;
    }
    else {
    // Default case is no transfer encoding applies (7bit==8bit==binary)
    _body = (char *)_body_text;
    _must_free_body = DTM_FALSE;
    _body_decoded_len = _body_len;
    }
  }
  else {
    // Default case is no transfer encoding applies.
    error.clear(); 
    _body = (char *)_body_text;
    _must_free_body = DTM_FALSE;
    _body_decoded_len = _body_len;
   }

// For CHARSET
	// Get charset from content-type field
	char *ret = NULL;
	value.clear();
	_body_env->getHeader(error, "Content-Type", DTM_FALSE, value);
	if (error.isNotSet()) {
       cs = csFromContentType(value);
       if ( cs == NULL ) {
	      // Random allocation for cs
		  cs = (char *)calloc(128, sizeof(char));
		  DtXlateOpToStdLocale(DtLCX_OPER_SETLOCALE,
			 setlocale(LC_CTYPE, NULL),
			 NULL,
			 NULL,
			 &ret);
		  strcpy(cs, "DEFAULT");
		  strcat(cs, ".");
		  strcat(cs, ret);
	   }
	} else {    // No Content-Type
	   // We'll be flexible here.  If Content-Type header is missing, we'll
	   // still try to convert from the locale specific default codeset.
	   error.clear();
	   // Random allocation for cs
	   cs = (char *)calloc(128, sizeof(char));
	   DtXlateOpToStdLocale(DtLCX_OPER_SETLOCALE,
		 setlocale(LC_CTYPE, NULL),
		 NULL,
		 NULL,
		 &ret);
	   strcpy(cs, "DEFAULT");
	   strcat(cs, ".");
	   strcat(cs, ret);
	}

	// Handle ISO-2022-INT, RFC approved, or private encoding names
	if ( strcasecmp(cs, "ISO-2022-INT-1") == 0 ) {
	   // Need to obtain charset from encoding
	}  // RFC approved and private names are not treated differently.

	// Get iconv name from charset - this is the "from" name.
    from_cs = NULL;
    from_cs = csToConvName(cs);

	// Get current locale's iconv name - this is the "to" name.
	to_cs = NULL;
	to_cs = locToConvName();

    if ( from_cs && to_cs ) {
	  if ( strcasecmp(from_cs, to_cs) != 0 ) {
	  unsigned long tmp_len = (unsigned long) _body_decoded_len;
	  if (csConvert(&_body, tmp_len, (int)_must_free_body, from_cs, to_cs)) {
		_must_free_body = DTM_TRUE;
		_body_decoded_len = (int) tmp_len;
	  }
	  }
	}
	if ( cs )
	free ( cs );
	if ( from_cs )
	    free( from_cs );
	if ( to_cs )
	    free ( to_cs );
	if ( ret )
	    free( ret );
	
// End of For CHARSET

    // Clear the error condition before proceeding.  This is done
    // because functions that take a DtMailEnv object as a parameter
    // expect it to be cleared.  (Already cleared above)
    // error.clear();  

  // If the body is text/enriched, convert it to text/plain
  // At some point the front end should be able to handle this via
  // data typing and do an 'intelligent' conversion, in which
  // case this code can be removed
  //

  value.clear();
  if (_body_env) {
    _body_env->getHeader(error, "Content-Type", DTM_FALSE, value);
  }

  if (_body_env && !error.isSet() &&
      ((strncasecmp(*(value[0]), "text/enriched", 13) == 0)
       || (strncasecmp(*(value[0]), "text/richtext", 13) == 0))) {
    char *new_body = (char *)malloc((unsigned)(_body_len*2));
    int size = 0;
    RFCMIME::readTextEnriched(new_body, size, _body, _body_decoded_len);
    new_body = (char *)realloc(new_body, size+2);
    if (_must_free_body == DTM_TRUE)
      free(_body);
    _must_free_body = DTM_TRUE;
    _body_decoded_len = size;
    _body = new_body;
    // Changed this temporarily until after release. We really should not
    // be null terminating these buffers.
    (_body)[_body_decoded_len] = 0;
  }
  error.clear();
  return;
}

// NOTES ON HANDLING OF "FILE NAMES" FOR BODY PARTS IN MIME COMPLIANT ENTITIES:
// (see full description in evaluation for bug 1189035)
// 
// The ability to provide a "file name" for a body part in a MIME compliant
// entity is partially addressed in RFC 1341 [MIME: Multipurpose Internet Mail
// Extentions] and RFC 1521 [which obsoleted RFC 1341].
// 
// RFC 1341 proposed a solution to the file name problem (Content-Type:
// type; name=name); RFC 1521 subsequently depreciated this solution in
// favor of a to-be-defined future specification of "Content-Disposition".
// 
// In essence there is an anticipation that "Content-Disposition" will
// be defined in a future RFC such that the notion of a "file name" may
// be given to a body part of a MIME compliant entity.
// 
// OpenWindows mailtool currently recognizes "Content-Description" on MIME
// compliant entities and uses that information as the "file name" for
// an attachment (as per a loose interpretation of RFC 1521).
// 
// CDE DtMail currently sends out and recognizes "X-Content-Name" on MIME
// compliant entities and uses that information as the "file name" for an
// attachment. This is essentially a non-standard header field as per RFC
// 1521:
// 
//     "X-" fields may be created for experimental or private purposes, 
//     with the recognition that the information they contain may be 
//     lost at some gateways.
// 
// Since OpenWindows mailtool does not understand "X-Content-Name", any
// body part in e-mail originating from CDE DtMail does not appear to 
// have a file name when read by mailtool.
// 
// Given these facts:
// 
//     . OpenWindows mailtool cannot be changed until at least the Solaris
//       2.5 release (if at all).
// 
//     . OpenWindows mailtool is recognizing a valid MIME header field 
//       which is essentially "free form" in nature.
// 
//     . CDE DtMail is currently using an experimental or private field 
//       which is not part of the standard and is not guaranteed to survive
//       transport across gateways.
// 
//     . There is no officially proscribed method for providing the "file
//       name" of a body part in a MIME compliant entity in RFC 1521.
// 
// The reasonable approach to take to solve this problem is:
// 
// 1. Have DtMail use "Content-Description" to transmit the "file name"
//    for a body part - this achieves compatibility with mailtool.
// 
// 2. Have DtMail also continue to use "X-Content-Name" to transmit the
//    "file name" - this maintains compatibility with previous versions
//    of DtMail which only recognize this header.
// 
// 3. Have Dtmail recognize both "Content-Description" and "X-Content-Name"
//    as specifying the "file name" for a body part. In the case that both
//    fields are present, "Content-Description" takes precedence over
//    "X-Content-Name".
// 
// 4. When "Content-Disposition" is properly defined and included as part
//    of an updated MIME specification, revisit this issue.
//
// 
// So, since dtmail currently *ignores* the "Content-Description" field,
// we overload the "getNameHeaderVal" function to use "Content-Description"
// first, then the older unofficial experimental "X-Content-Name". If
// and when Content-Disposition takes hold, it will have to override
// Content-Description when both are present.
//
// Addendum to file name issue [01-05-06]
// 
// Apparently a number of "RFC 1341 compliant" mailers use the "name" attribute
// of the "Content-Type" header to specify the attachment name. In order for
// dtmail to interoperate with the largest number of MIME type mailers in
// distribution, dtmail should also recognize and use this attribute.
//
// In addition, support for "RFC 1806 compliant" mailers that are implementing
// the "Content-Disposition" header is added.
//

char *
MIMEBodyPart::getDescription(DtMailEnv & error)
{
  // Dont have this return anything without checking
  // ramifications with getNameHeaderValue

// No need to clear error object here because we assume it has already
// been cleared by the caller and nothing has touched it in this method.
//    error.clear();

    return(NULL);
}

char *
MIMEBodyPart::getNameHeaderVal(DtMailEnv & error)
{
    DtMailValueSeq value;
    const char *namefield = (const char *)0;

    if (_body_env == NULL) {
// No need to clear the error object...it is unchanged from the 
// state we received it in.
//	error.clear();
	return(NULL);
    }

    // Recognize "Content-Disposition" header field [RFC 1806]
    // Brute force approach: if present, look for "filename=" attribute
    // as specifying the attachment name and if present, accept it
    //
    error.clear();
    _body_env->getHeader(error, "Content-Disposition", DTM_FALSE, value);
    if(error.isNotSet()) {
      // see if filename=x attribute specified
      //
      for (const char *namefieldx = (*(value[0])); *namefieldx; namefieldx++) {
	if (strncasecmp(namefieldx, "filename=", 9) == 0) {
	  break;
	}
      }
      if (*namefieldx) {
	// Get past uninteresting cruft.
	//
	namefield = namefieldx + 9;
      }
    }

    // Recognize "Content-Type" header field [RFC 1341 compatibility]
    // Brute force approach: if present, look for "name=" attribute
    // as specifying the attachment name and if present, accept it
    //
    if (!namefield) {
      value.clear();	// clear out any previous values
      error.clear();	// clear out any previous error value
      _body_env->getHeader(error, "Content-Type", DTM_FALSE, value);
      if(error.isNotSet()) {
	// see if name=x attribute specified
	//
	for (const char *namefieldx = (*(value[0])); *namefieldx; namefieldx++) {
	  if (strncasecmp(namefieldx, "name=", 5) == 0) {
	    break;
	  }
	}
	if (*namefieldx) {
	  // Get past uninteresting cruft.
	  //
	  namefield = namefieldx + 5;
	}
      }
    }

    // If a namefield has been identified, pull out the name itself and return it
    //
    if (namefield) {	
      // Find the end of the attribute value. This will be either the end of
      // the line, a quote, or a semi-colon.
      //
      int quoted = 0;
      if (*namefield == '"') {
	namefield += 1;
	quoted = 1;
      }
      for (const char * nf_end = namefield; *nf_end; nf_end++) {
	if (quoted) {
	  if (*nf_end == '"') {
	    break;
	  }
	}
	else {
	  if (*nf_end == ';' || *nf_end == '\r' || *nf_end == '\n') {
	    break;
	  }
	}
      }
      int len = nf_end - namefield + 1;
      char * result = (char *)malloc(len);
      strncpy(result, namefield, len - 1);
      result[len - 1] = 0;
      return(result);
    }

    value.clear();	// clear out any previous values
    error.clear();	// clear out any previous error value

    // In keeping with the current undefined nature of
    // file names for body parts in RFC 1521, and to be
    // compatible with OpenWindows mailtool, the first
    // overriding "name" comes from "Content-Description"
    //
    _body_env->getHeader(error, "Content-Description",
			 DTM_FALSE, value);
    if (error.isNotSet()) {
	return(strdup(*(value[0])));
    }
    error.clear();

    // Next we remain compatible with previous versions
    // of DtMail that used "X-Content-Name" instead of
    // "Content-Description" for the file name of a
    // body part (1-27-95)
    //
    _body_env->getHeader(error, "X-Content-Name",
			 DTM_FALSE, value);
    if (error.isNotSet()) {
	return(strdup(*(value[0])));
    }
    error.clear();

    // This code was in dtmail on 1-27-95 and even though
    // that version of dtmail did not send out "Content-Name"
    // (which is not part of RFC 1341 or 1521) it doesnt
    // hurt to recognize it if its the only header there.
    //
    _body_env->getHeader(error, "Content-Name",
			 DTM_FALSE, value);
    if (error.isNotSet()) {
	return(strdup(*(value[0])));
    }
    error.clear(); // NULL is the real error here.

    // No name for this body part
    //
    return(NULL);
}

char *
MIMEBodyPart::getName(DtMailEnv & error)
{
    char * h_name = getNameHeaderVal(error);
    // don't care about the error returned by getNameHeaderVal()
    error.clear();
    if (h_name) {
	return(h_name);
    }

    if (!_body_type) {
	getDtType(error);
	if (error.isSet()) {
	    error.clear();
	    return(strdup("Attachment"));
	}
    }

    char * pat = DtDtsDataTypeToAttributeValue(_body_type,
					       "NAME_TEMPLATE",
					       NULL);

    if (pat) {
	int max_len = strlen(pat) + 20;
	char * name = new char[max_len];
	sprintf(name, pat, "Attachment");
	DtDtsFreeAttributeValue(pat);
	return(name);
    }

    return(strdup("Attachment"));
}

void
MIMEBodyPart::setName(DtMailEnv & error, const char * name)
{
    if (_body_env) {
	_body_env->setHeader(error, "X-Content-Name", DTM_TRUE, name);
    }
}

unsigned long
MIMEBodyPart::getLength(DtMailEnv & error)
{
    MutexLock lock_scope(_obj_mutex);

    loadBody(error);
    if (error.isSet()) {
	// propogate the error back to the caller.
	return (0);
    }

    // We have to treat external bodies differently. The headers on these
    // parts contain useful information to the client.
    //
    const char * mime_type;
    DtMailValueSeq value;

    if (_body_env) {
	_body_env->getHeader(error, "Content-Type", DTM_FALSE, value);
    }

    if (_body_env && !error.isSet()) {
	mime_type = *(value[0]);
    }
    else {
	// only need to clear the error object if getHeader returned 
	// an error condition.  We don't want to propogate the error
	// back up the calling sequence.
	error.clear();
	mime_type = "text/plain";
    }

    unsigned long len;

    if (strncasecmp(mime_type, "message/external-body", 21) == 0) {
	const char * contents = _body_start;
	for (;contents < (_body_text + _body_len); contents++) {
	    if (*contents == '\n') {
		break;
	    }
	}
	contents += 1;

	len = _body_len + (_body_text - contents);
    }
    else {
	len = _body_decoded_len;
    }

    return(len);
}

int
MIMEBodyPart::rfcSize(const char *, DtMailBoolean &)
{
    return(0);
}

char *
MIMEBodyPart::writeBodyParts(char * buf)
{
  return(buf);
}

const void *
MIMEBodyPart::getBody(DtMailEnv & error)
{
// No need to clear the error here, should be cleared by the object
// that instantiated it.
//    error.clear();

    if (!_body) {
	loadBody(error);
	// loadBody currently (version 1.31) clears the error before
	// returning, so the following check is not needed.  We'll
	// leave it alone on the chance that loadBody() will someday
	// return an error condition.
	if (error.isSet()) {
	    return(NULL);
	}
    }

    // We have to treat external bodies differently. The headers on these
    // parts contain useful information to the client.
    //
    const char * mime_type;
    DtMailValueSeq value;

    if (_body_env) {
	_body_env->getHeader(error, "Content-Type", DTM_FALSE, value);
    }

    if (_body_env && !error.isSet()) {
      // Handle "Content-Type: text" problem with /usr/lib/mail.local
      //
      if (strcasecmp(*(value[0]), "text")==0)
	mime_type = "text/plain";
      else
	mime_type = *(value[0]);
    }
    else {
	error.clear();
	mime_type = "text/plain";
    }

    const char * contents;

    if (strncasecmp(mime_type, "message/external-body", 21) == 0) {
	contents = _body_start;
	for (;contents < (_body_text + _body_len); contents++) {
	    if (*contents == '\n') {
		break;
	    }
	}
	contents += 1;
    }
    else {
	contents = _body;
    }

    return(contents);
}

// For CHARSET

// Given the Content-Type field, extract the charset value.
// Returns one of the following:
// 1) charset value,
//    Caller MUST FREE this return value.
// 2) NULL if charset is not specified but Content-Type specifies text/plain data.
//    If this routine returns NULL, that means charset is not specified but the data
//    is text/plain.  Therefore, (as in the case of MIMEBodyPart::loadBody) caller
//    will call csToConvName("DEFAULT.<locale>") and csToConvName will return
//    a default iconv conversion name for the current locale.
//    This is the case because some (old) mailers do not set the charset value
//    but encodes the message in a "popular" codeset.  The default conversion name
//    for a particular locale assumes the "popular" codeset used.
// 3) non NULL but invalid charset value if Content-Type does not specify text/plain data.
//    Caller MUST FREE this return value.
char *
MIMEBodyPart::csFromContentType(DtMailValueSeq &value)
{
   char *cs_str = NULL;
   char *val_ptr = NULL;
   int quoted = 0;

   // value[0] should be valid else error would have occurred before
   // this routine ever gets called.  And value index is 0 because
   // previous value stored should have been removed.
   const char *val = *(value[0]);

   // Check to see if Content-Type field specifies text/plain data.
   // If so, look for charset value
   // else, returns value in Content-Type field
   if ( strstr(val, "text") == NULL ) {
      if ( strstr(val, "TEXT") == NULL ) {
		cs_str = strdup(val);
		return cs_str;
      }
   } 
   // Get charset value
   val_ptr = strstr(val, "charset=");
   if ( val_ptr == NULL ) {
	  val_ptr = strstr(val, "CHARSET=");
   }
   if ( val_ptr == NULL ) {
	  return NULL;
   }
   val_ptr = val_ptr+8;

   // Check if charset value is quoted
   if ( val_ptr[0] == '"' ) {
	  val_ptr++;
	  quoted = 1;
   }
   if ( quoted ) {
	  cs_str = strdup(strtok(val_ptr, "\""));
   } else {
	  cs_str = (char *)calloc(strlen(val_ptr)+1, sizeof(char));
      sscanf(val_ptr, "%s", cs_str);
   }

   return cs_str;
}
// End of For CHARSET
