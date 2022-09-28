/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */
 
// --> This is the Basic Character Buffer.
 
#pragma ident "@(#)ContentBuffer.cc	1.27 97/06/12 SMI"

// Include Files
#include <stropts.h>
#include <poll.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <SDtMail/ContentBuffer.hh>
#include <SDtMail/SystemUtility.hh>
#include <SDtMail/RepSWMR.hh>
#include <mail.h>
#include <SDtMail/Connection.hh>
#include <lcl/lcl.h>

// static constants
const char* SdmContentBuffer::Sdm_CBE_Base64 = "Base64";
const char* SdmContentBuffer::Sdm_CBE_QuotedPrintable = "QuotedPrintable";
const char* SdmContentBuffer::Sdm_CBE_Compressed = "Compressed";
const char* SdmContentBuffer::Sdm_CBE_UUEncoded = "UUEncoded";

//
// Global functions
//

static inline void
fill_copy(unsigned char * out,
          const unsigned char * in,
          const unsigned char * in_end,
          int size_needed)
{
    int in_size = in_end - in + 1;
    memcpy(out, in, in_size);
 
    memset(out + in_size, ' ', size_needed - in_size);
    out[size_needed - 1] = 0;
}
 
// decode_uue_char -- decode a single uuencoded character
// Arguments:
//  const int val       -- uuencoded character
// Outputs:
//  return char         -- un-uencoded character
// Description
//  Given a single uuencoded character (which is represented as
//  a 7-bit printable ascii character) return an unencoded 6-bit
//  character which can be used to assemble real encoded characters
//
 
static inline char
decode_uue_char(const int val)
{
    return((val - ' ') & 077);
}

static
const unsigned char *
decode_uue_line(char * buf,
            unsigned long & off,
            const unsigned char * encodedBp,
            const unsigned long decodedLen,
            const unsigned char * encodedEndBp)
{
    unsigned char line_buf[100];
    memset(&line_buf, 0, 100);
    const unsigned char * line = line_buf;
    const unsigned char * nextEncodedLineStart;
    const unsigned char * nl;
    unsigned long encodedLen;
    unsigned long countDown = decodedLen;
 
    // See if the line ends with a new line, and is within the range
    // of the buffer.
    // First, compute the length of the encoded byte stream on this line.
    // Given the decoded length, we can do this because the encodings are
    // emitted in groups of four bytes for every three bytes encoded.
    //
    encodedLen = ((decodedLen/3)+((decodedLen%3)!=0))*4;
    if ((encodedBp + encodedLen) > encodedEndBp || encodedBp[encodedLen] != '\n') {
        // Well, looks like some trailing white space was lost in
        // transmission. Lets copy the line and fill in what is needed.
        // (note: if this does happen the source could be corrupted, as the
        // encoded length is derived from the supposed number of decoded
        // characters we have to generate; if the former is off, how can
        // we yield correct bytes for the latter??)
        //
 
        for (nl = encodedBp; nl <= encodedEndBp && *nl != '\n'; nl++) {
            continue;
        }
 
        if (nl < (encodedBp + encodedLen)) {
            assert(sizeof(line_buf) >= encodedLen);
            fill_copy(line_buf, encodedBp, nl, encodedLen);
        }
        else {
            // We will ignore extra characters at the end of the line.
            // (great...)
            line = encodedBp;
        }
 
        nextEncodedLineStart = nl + 1;
    }
    else {
        line = encodedBp;
        nextEncodedLineStart = encodedBp + encodedLen + 1;
    }
 
    // Now the fun begins - obtain a -> the next free byte in the output stream
    // and then spin through the input stream decoding quartets of uuencoded data
    // into triplets of clear text, abstaining as necessary to provide for the
    // last few bytes of the file which may not be mod 3 in length. This routine
    // has been optimized for performance.
    //
    char *lbp = buf+off;        // -> next free byte in output stream
    const unsigned char * encE = (encodedBp + encodedLen);      // -> past last input byte
    for (const unsigned char * enc = encodedBp; enc < encE; enc += 4, countDown -= 3) {
        if (countDown >= 3) {
          *lbp++ = decode_uue_char(*enc) << 2 | decode_uue_char(*(enc + 1)) >> 4;
          *lbp++ = decode_uue_char(*(enc + 1)) << 4 | decode_uue_char(*(enc + 2)) >> 2;
          *lbp++ = decode_uue_char(*(enc + 2)) << 6 | decode_uue_char(*(enc + 3));
        }
        else if (countDown >= 2) {
          *lbp++ = decode_uue_char(*enc) << 2 | decode_uue_char(*(enc + 1)) >> 4;
          *lbp++ = decode_uue_char(*(enc + 1)) << 4 | decode_uue_char(*(enc + 2)) >> 2;
        }
        else if (countDown >= 1) {
          *lbp++ = decode_uue_char(*enc) << 2 | decode_uue_char(*(enc + 1)) >> 4;
        }
    }
    off += decodedLen;  // bump offset by the # of bytes we appended
    return(nextEncodedLineStart);
}

//
// SdmContentBuffer method implementation.
//
SdmContentBuffer::SdmContentBuffer() 
  : _IsDecoded(Sdm_False)
{
  _CharacterSet = NULL;
  _DataTypeCharSet = NULL;
}

SdmContentBuffer::SdmContentBuffer(const char* string)
  : _IsDecoded(Sdm_False)
{
  _Data = string;
  _CharacterSet = NULL;
  _DataTypeCharSet = NULL;
  assert((!_Data.Length()) || ((const char *)_Data)[_Data.Length()] == '\0');
}

SdmContentBuffer::SdmContentBuffer(const SdmString& string)
  : _IsDecoded(Sdm_False)
{
  _Data = string;
  _CharacterSet = NULL;
  _DataTypeCharSet = NULL;
  assert((!_Data.Length()) || ((const char *)_Data)[_Data.Length()] == '\0');
}

// This method provides for initializing a content buffer that is represented by
// a file in a read only fashion. This allows efficient mechanisms (such as mmap)
// to be used as opposed to malloc() and read()

SdmErrorCode SdmContentBuffer::MapAgainstFile(SdmError& err, 
				   const int fd,
				   const size_t numberOfBytes)
{
  _Data = NULL;			// purge out any old content data
  _CharacterSet = NULL;
  _DataTypeCharSet = NULL;
  _IsDecoded = Sdm_False;

  return(_Data.MapAgainstFile(err, fd, numberOfBytes));
}

SdmContentBuffer::~SdmContentBuffer()
{
  
}
    
SdmContentBuffer::SdmContentBuffer(const SdmContentBuffer& cBuf)
  : _IsDecoded(Sdm_False)
{
  *this = cBuf;
}

SdmContentBuffer& 
SdmContentBuffer::operator= (const SdmContentBuffer& cBuf)
{
  if (this == &cBuf)
    return *this;
  
  _Data = cBuf._Data;
  
  // clear the decode set and copy the elements from cBuf.
  _DecodeSet.ClearAllElements();
  SdmVectorIterator<SdmIntStr> iter(&_DecodeSet);
  SdmIntStr *item;
  while ((item = iter.NextListElement()) != NULL) {
    _DecodeSet.AddElementToList(*item);
  }
 
  _IsDecoded = cBuf._IsDecoded;
  _CharacterSet = cBuf._CharacterSet;
  _DataTypeCharSet = cBuf._DataTypeCharSet;

  return *this;
}

SdmContentBuffer& 
SdmContentBuffer::operator= (const SdmString& string)
{
  _Data = string;
  _IsDecoded = Sdm_False;
  return *this;
}

SdmContentBuffer& 
SdmContentBuffer::operator= (const char* string)
{
  _Data = string;
  _IsDecoded = Sdm_False;
  return *this;
}

SdmStringLength 
SdmContentBuffer::Length() const
{
   return _Data.Length();
}

SdmErrorCode 
SdmContentBuffer::GetContents(SdmError &err, SdmString &r_contents) const
{
  err = Sdm_EC_Success;
  SdmContentBuffer *me = (SdmContentBuffer*) this;

  assert((!_Data.Length()) || ((const char *)_Data)[_Data.Length()] == '\0');

  // if the data is not yet decoded, decode it.
  if (!_IsDecoded && _Data.Length() > 0) {
    if (me->Decode(err)) {
      return err;
    }
  }

  r_contents = _Data;
  assert((!_Data.Length()) || ((const char *)r_contents)[_Data.Length()] == '\0');

  return err;
}

SdmErrorCode   
SdmContentBuffer::AddEncodeType(SdmError &err, const SdmString& encodeType)
{
  err = Sdm_EC_Success;
  // reset this flag so that it gets decoded again.
  _IsDecoded = Sdm_False;
  _DecodeSet.AddElementToList(SdmIntStr(0, encodeType));
  return err;
}

SdmErrorCode   
SdmContentBuffer::AddEncodeType(SdmError &err, const char* encodeType)
{
  err = Sdm_EC_Success;
  // reset this flag so that it gets decoded again.
  _IsDecoded = Sdm_False;    
  _DecodeSet.AddElementToList(SdmIntStr(0, encodeType));  
  return err;
}

// I18N
SdmErrorCode   
SdmContentBuffer::SetCharacterSet(SdmError &err, const SdmString& charSet)
{
  err = Sdm_EC_Success;
  _CharacterSet = charSet;
  return err;
}

// I18N
SdmErrorCode   
SdmContentBuffer::GetCharacterSet(SdmError &err, SdmString& r_charSet) const
{
  err = Sdm_EC_Success;
  r_charSet = _CharacterSet;
  return err;
}

// I18N
SdmErrorCode   
SdmContentBuffer::SetDataTypeCharSet(SdmError &err, const SdmString& charSet)
{
  err = Sdm_EC_Success;
  _DataTypeCharSet = charSet;
  return err;
}

// I18N
SdmErrorCode   
SdmContentBuffer::GetDataTypeCharSet(SdmError &err, SdmString& r_charSet) const
{
  err = Sdm_EC_Success;
  r_charSet = _DataTypeCharSet;
  return err;
}

// I18N
SdmErrorCode   
SdmContentBuffer::GetPossibleCharacterSetList(SdmError &err, SdmStringL& r_charSetList, const char *inputBp, SdmBoolean forSend)
{
  SdmConnection *conn;
  LclError *ret;
  LCLd lcld;
  LCTd lclt;
  LclCharsetList *list;
  int i;

  err = Sdm_EC_Success;

  conn = SdmConnection::GetConnection();
  if (!conn) {
    return(err = Sdm_EC_ConnectionNotStarted);
  }

  lcld = conn->GetLocaleDescriptor();
  if (!lcld) {
    return(err = Sdm_EC_LclOpenFailed);
  }

  lclt = lct_create(lcld, LctNSourceType, LctNMsgText,
			LctNNone, inputBp,
			LctNSourceForm, LctNDisplayForm,
			LctNKeepReference, LctNKeepByReference, NULL);
  if (!lclt) {
    return(err = Sdm_EC_LctCreateFailure);
  }

  if(forSend == Sdm_False){ // display case	
    ret = lct_getvalues(lclt, LctNInComingStreamForm,
			LctNBodyPossibleCharsetList, &list, NULL);
  }
  else{ // sending case
    ret = lct_getvalues(lclt, LctNOutGoingStreamForm,
			LctNBodyPossibleCharsetList, &list, NULL);
  }
  if(ret){
    lcl_destroy_error(ret);
    lct_destroy(lclt);
    return(err = Sdm_EC_Fail);
  }

  for(i = 0; i < list->num; i++){
    r_charSetList.AddElementToList(list->charset[i]);
  }
  lcl_destroy_charset_list(list);
  lct_destroy(lclt);

  return err;
}


SdmErrorCode   
SdmContentBuffer::Decode(SdmError &err)
{
  err = Sdm_EC_Success;
  SdmVectorIterator<SdmIntStr> iter(&_DecodeSet);
  
  SdmIntStr *item;
  char *newData;
  unsigned long newLength;
  
  while ((item = iter.NextListElement()) != NULL) {
  
    // if we have not decoded this type, decode it.
    if (item->GetNumber() == 0) {
      SdmString encodeType = item->GetString();
      
      // Revisit!!  Should replace all the redundant code.  Should really store a map
      // from encodeType to decode method and just use that.
      if (encodeType == Sdm_CBE_Base64) {
        DecodeBase64Data(err, newData, newLength, (const char*)_Data, _Data.Length());
      } else if (encodeType == Sdm_CBE_QuotedPrintable) {
        DecodeQuotedPrintableData(err, newData, newLength, (const char*)_Data, _Data.Length());
      } else if (encodeType == Sdm_CBE_UUEncoded) {
        DecodeUUEncodedData(err, newData, newLength, (const char*)_Data, _Data.Length());
      } else if (encodeType == Sdm_CBE_Compressed) {
        DecodeCompressedData(err, newData, newLength, (const char*)_Data, _Data.Length());
      }

      if (err) {
	err.AddMajorErrorCode(Sdm_EC_Fail);
	return err;
      }

      // We are guaranteed that the results are null terminated but
      // the count provided does not include the null termination -
      // that is why the +1 is included in the second argument.
      // Let the _Data SdmString "own" newData
      assert((!newLength) || (newData[newLength] == '\0'));
      _Data = SdmString(&newData, (SdmStringLength)newLength, (size_t)newLength+1);

      item->SetNumber(1);
      
    } /* end if GetNumber */
  } /* end while */
  
  return err;
}

//
// I18N
// Attension 1:
//   Parameter body_charset should have been retrieved from mail
//   if body_charset is set to (char*)NULL, use default (may not corect)
// Attension 2:
//   Buffer content should be decoded beforehand
SdmErrorCode   
SdmContentBuffer::DecodeCharset(SdmError &err, char *&r_outputBp,
    int &r_outputLen, const char * inputBp, const unsigned long inputLen,
    SdmString &body_charset)
{
  SdmConnection	*conn;
  LCLd	lcld = (LCLd)NULL;
  LCTd	lclt = (LCTd)NULL;
  LclError	*ret = (LclError *)NULL;
  LclCharsetSegmentSet	*segs = (LclCharsetSegmentSet *)NULL;
  char	*body_buffer = (char *)NULL;

  err = Sdm_EC_Success;

  conn = SdmConnection::GetConnection();
  if (!conn) {
    err = Sdm_EC_ConnectionNotStarted;
    return err;
  }
  lcld = conn->GetLocaleDescriptor();
  if (!lcld) {
    err = Sdm_EC_LclOpenFailed;
    return err;
  }

  // If the original input string is not null terminated, then we need to make a copy
  // as the lcl library requires a null terminated string for input.

  if (inputBp[inputLen] != '\0') {
    body_buffer = (char *)SdmMemoryRepository::_ONew(inputLen+1, Sdm_MM_AnonymousAllocation);
    if(body_buffer == (char *)NULL){
      err = Sdm_EC_Fail;
      return err;
    }
    memcpy(body_buffer, inputBp, inputLen);
    body_buffer[inputLen] = (char)NULL;
  }

  // create lclt
  lclt = lct_create(lcld, LctNSourceType, LctNMsgText, LctNNone, body_buffer ? body_buffer : inputBp,
      LctNSourceForm, LctNInComingStreamForm,
      LctNKeepReference, LctNKeepByReference, NULL);

  if (!lclt) {
    if (body_buffer)
      SdmMemoryRepository::_ODel(body_buffer, Sdm_MM_AnonymousAllocation);
    err = Sdm_EC_Fail;
    return err;
  }

  // set source charset
  if(body_charset > "")
  {
    if(body_charset.CaseCompare("US-ASCII") != SdmString::Sdm_Equal)
    {
      lct_setvalues(lclt, LctNInComingStreamForm,
          LctNBodyCharset, (const char *)body_charset, NULL);
    }
  }

  // set encoding to None
  lct_setvalues(lclt, LctNInComingStreamForm,
      LctNBodyEncoding, "None", NULL);

  // get converted body
  ret = lct_getvalues(lclt, LctNDisplayForm, LctNBodySegment, &segs, NULL);
  if (ret && ((ret->error_code != LctErrorIconvNonIdenticalConversion) 
      && (ret->error_code != LctErrorIconvHalfDone))){
    lcl_destroy_error(ret);
    lct_destroy(lclt);
    if (body_buffer)
      SdmMemoryRepository::_ODel(body_buffer, Sdm_MM_AnonymousAllocation);
    err = Sdm_EC_Fail;
    return err;
  }

  r_outputBp = segs->seg[0].segment;
  r_outputLen = segs->seg[0].size;
  segs->seg[0].segment = (char *)NULL;
  lcl_destroy_segment_set(segs);

  /* destroy lclt */
  lct_destroy(lclt);
  if (body_buffer)
      SdmMemoryRepository::_ODel(body_buffer, Sdm_MM_AnonymousAllocation);

  return err;
}


/* Convert BASE64 contents to binary
 * Accepts: source
 *	    length of source
 *	    pointer to return destination length
 * Returns: destination as binary
 * Identical to the c-client base64 decoded BUT always guarantees the results are
 * null terminated so the localization library decoding stuff does not have to
 * force a copy.
 */

static void *rfc822_base64 (unsigned char *src,unsigned long srcl,unsigned long *len)
{
  char c;
  void *ret = malloc (*len = 4 + ((srcl * 3) / 4) + 1);
  char *d = (char *) ret;
  short e = 0;
  memset (ret,0,*len);		/* initialize block */
  *len = 0;			/* in case we return an error */
  while (srcl--) {		/* until run out of characters */
    c = *src++;			/* simple-minded decode */
    if (isupper (c)) c -= 'A';
    else if (islower (c)) c -= 'a' - 26;
    else if (isdigit (c)) c -= '0' - 52;
    else if (c == '+') c = 62;
    else if (c == '/') c = 63;
    else if (c == '=') {	/* padding */
      switch (e++) {		/* check quantum position */
      case 2:
	if (*src != '=') return NIL;
	break;
      case 3:
	e = 0;			/* restart quantum */
	break;
      default:			/* impossible quantum position */
	free (&ret);
	return NIL;
      }
      continue;
    }
    else continue;		/* junk character */
    switch (e++) {		/* install based on quantum position */
    case 0:
      *d = c << 2;		/* byte 1: high 6 bits */
      break;
    case 1:
      *d++ |= c >> 4;		/* byte 1: low 2 bits */
      *d = c << 4;		/* byte 2: high 4 bits */
      break;
    case 2:
      *d++ |= c >> 2;		/* byte 2: low 4 bits */
      *d = c << 6;		/* byte 3: high 2 bits */
      break;
    case 3:
      *d++ |= c;		/* byte 3: low 6 bits */
      e = 0;			/* reinitialize mechanism */
      break;
    }
  }
  *len = d - (char *) ret;	/* calculate data length */
  return ret;			/* return the string */
}

SdmErrorCode   
SdmContentBuffer::DecodeBase64Data(SdmError &err, char *&r_outputBp,
                                   unsigned long &r_outputLen, const char * inputBp,
                                   const unsigned long inputLen)
{
  err = Sdm_EC_Success;
  r_outputLen = 0;
  assert (inputBp != NULL);

  // Call the c-client's base64 decoder
  r_outputBp = (char*)rfc822_base64((unsigned char*)inputBp, inputLen, &r_outputLen);

  return (err = r_outputBp ? Sdm_EC_Success : Sdm_EC_DecodingError);
}

SdmErrorCode   
SdmContentBuffer::DecodeQuotedPrintableData(SdmError &err, char *&r_outputBp,
                                            unsigned long &r_outputLen, const char *inputBp,
                                            const unsigned long inputLen)
{
  err = Sdm_EC_Success;
  r_outputLen = 0;
  
  assert (inputBp != NULL);

  r_outputBp = (char*) malloc (inputLen+1);

  for (const char * cur = inputBp; cur < (inputBp + inputLen); cur++) {
    if (*cur == '=') {
      if (*(cur + 1) == '\n') {
        cur += 1;
        continue;
      }
      else if (*(cur + 1) == '\r' && *(cur + 2) == '\n') {
        cur += 2;
        continue;
      }
      else {
        if (isxdigit(*(cur + 1)) && isxdigit(*(cur + 2))) {
            char hex[3];
            hex[0] = *(cur + 1);
            hex[1] = *(cur + 2);
            hex[2] = 0;

            r_outputBp[r_outputLen++] = strtol(hex, NULL, 16);
            cur += 2;
            continue;
        }
      }
    }

    r_outputBp[r_outputLen++] = *cur;
  }
  r_outputBp[r_outputLen] = '\0';	// assure that the result is null terminated

  // Done decoding the entire source - reduce size of allocated buffer
  // so that unused space is returned to the free pool
  //
  if (r_outputLen < inputLen)
      r_outputBp = (char *)realloc(r_outputBp, r_outputLen+1);

  return err;
}

SdmErrorCode   
SdmContentBuffer::DecodeCompressedData(SdmError &err, char *&r_outputBp,
                                       unsigned long &r_outputLen, const char * inputBp,
                                       const unsigned long inputLen)
{
  err = Sdm_EC_Success;
  r_outputLen = 0;
  
  assert (inputBp != NULL);

  // We are passed a -> a place to store a pointer to the decoded information
  // and a call-by-ref of a place to store the number of bytes stored. Must 
  // allocate the clear text buffer. Unfortunately, the nature of the compression
  // algorithm makes it imposible to predict ahead of time how many bytes are
  // going to be needed to hold the results. So we allocate an initial buffer
  // that is 2x the size of the input (allowing for 50% compression). We grow
  // the buffer as necessary during the uncompression operation, and shrink it
  // to fit when the final clear text stream size becomes known

  struct pollfd pollFDS[2];                   // structure to use for poll() call
  const int childInputFD = 0;                 // index into pollFDS to send input to child
  const int childOutputFD = 1;                // index into pollFDS to read output from child

  long int interimOutputLength = 0;
  const long int blockSize = 4096;    // i/o done at least at this size
  long int interimOutputLimit = (inputLen+(blockSize-(inputLen%blockSize)));
  unsigned char *interimOutputBuffer = (unsigned char *)malloc((size_t)interimOutputLimit);
  assert(interimOutputBuffer);
  unsigned char *interimBp = interimOutputBuffer;

  // Fork off the uncompress function so its ready to process the data
  // 

  int inputPipe[2];           // input pipe descriptors (from child point of view)
  int outputPipe[2];          // output pipe descriptors (from child point of view)
  const int pipeReader = 0;   // pipe[0] is read side of pipe
  const int pipeWriter = 1;   // pipe[1] is write side of pipe
  pid_t childPid;             // pid for child process

  if (pipe(inputPipe) == -1) {                  // obtain pipe for child's input
      err = Sdm_EC_DecodingError;
      return err;
  }
  if (pipe(outputPipe) == -1) {               // obtain pipe for child's output
      (void) SdmSystemUtility::SafeClose(inputPipe[pipeReader]);
      (void) SdmSystemUtility::SafeClose(inputPipe[pipeWriter]);
      err = Sdm_EC_DecodingError;
      return err;
  }

  if (!(childPid = fork1())) {                 // child process
      // Need to clean up a bit before exec()ing the child
      // Close all non-essential open files, signals, etc.
      // NOTE: probably reduce priv's to invoking user too???
      //
      long maxOpenFiles = sysconf(_SC_OPEN_MAX);

      if (maxOpenFiles < 32)          // less than 32 descriptors?
        maxOpenFiles = 1024;          // dont believe it--assume lots
      for (int sig = 1; sig < NSIG; sig++)
        (void) signal(sig, SIG_DFL);                  // reset all signal handlers
      if (SdmSystemUtility::SafeDup2 (inputPipe[pipeReader], STDIN_FILENO) == -1)       // input pipe reader is stdin
        _exit (1);                                    // ERROR: exit with bad status

      (void) SdmSystemUtility::SafeClose(inputPipe[pipeWriter]);        // input pipe writer n/a
      if (SdmSystemUtility::SafeDup2 (outputPipe[pipeWriter], STDOUT_FILENO) == -1)     // output pipe writer is stdout
        _exit(1);                                     // ERROR: exit with bad status
      (void) SdmSystemUtility::SafeClose(outputPipe[pipeReader]);       // output pipe reader n/a
      // NOTE: we leave standard error output open
      for (int cfd = 3; cfd < maxOpenFiles; cfd++)
        (void) SdmSystemUtility::SafeClose(cfd);                        // close all open file descriptors
      (void) execl("/usr/bin/uncompress", "uncompress", "-qc", (char *)0); // try direct route first
      (void) execlp("uncompress", "uncompress", "-qc", (char *)0);    // failed - try via path
      _exit (1);                                      // failed!? return error exit code
  }

  if (childPid == -1)  {                               // fork failed??
    err = Sdm_EC_DecodingError;
    return err;                                        // yes: bail
  }

  (void) SdmSystemUtility::SafeClose(inputPipe[pipeReader]);            // input pipe reader n/a
  (void) SdmSystemUtility::SafeClose(outputPipe[pipeWriter]);           // output pipe writer n/a

#if defined(O_NONBLOCK)
  fcntl(inputPipe[pipeWriter], F_SETFL, O_NONBLOCK);  // we dont want to block writing to child
#elif defined(FNBIO)
  (void) fcntl(inputPipe[pipeWriter], F_SETFL, FNBIO);        // we dont want to block writing to child
#endif

  // Ok, uncompress is out there spinning its wheels waiting for us
  // enter a poll loop responding to file descriptor events
  //

  pollFDS[childInputFD].fd = inputPipe[pipeWriter];   // write input for child process here
  pollFDS[childInputFD].events = POLLOUT;     // tell us when data may be written w/o blocking
  pollFDS[childInputFD].revents = 0;          // no events pending
  pollFDS[childOutputFD].fd = outputPipe[pipeReader]; // read output from child process here
  pollFDS[childOutputFD].events = POLLIN;     // tell us when data may be read w/o blocking
  pollFDS[childOutputFD].revents = 0;         // no events pending

  unsigned char * currentInputBp = (unsigned char *)inputBp; // track -> input
  unsigned long currentInputCount = inputLen; // input bytes left to process

  while (poll(pollFDS, 2, INFTIM) > 0) {
      // process events on file descriptors
      // in case two events happen at once, handle the reading from the
      // process first, to make room for data that then may be written

      // process reading output from the child
      // Expand buffer as necessary to contain further data
      //
      if (pollFDS[childOutputFD].revents & POLLIN) {

          // if there is less than blockSize free bytes left in clear text stream,
          // expand the buffer by 10% before reading further data from child
          //
          if ((interimOutputLimit - interimOutputLength) < blockSize-1) {
              long int delta = (interimOutputLimit * 1.10);
              delta = delta+(blockSize-(delta%blockSize));

              interimOutputBuffer = (unsigned char *)realloc(interimOutputBuffer, (size_t)delta);
              assert(interimOutputBuffer);
              interimOutputLimit = delta;
              interimBp = interimOutputBuffer + interimOutputLength;
          }

          // attempt to read as much data from the child as possible
          //
          ssize_t readCount = SdmSystemUtility::SafeRead(pollFDS[childOutputFD].fd, interimBp, 
                                (size_t)(interimOutputLimit - interimOutputLength));

          if (readCount == -1) {              // error - nuke child and bail
              (void) kill(childPid, SIGKILL);
              break;
          }
          interimOutputLength += readCount;
          interimBp += readCount;
          if (readCount == 0)                         // end of file from the child?
              (void) SdmSystemUtility::SafeClose(pollFDS[childOutputFD].fd);    // yes, close down child output pipe
      }

      // process writing input to the child
      //
      if (pollFDS[childInputFD].revents & POLLOUT) {

        // child input queue has foom for more data
        // if no more data to send, close the child input pipe down
        //
        if (currentInputCount == 0) {
            (void) SdmSystemUtility::SafeClose(pollFDS[childInputFD].fd);
            continue;
        }

        // attempt to send as much data as the child will accept
        //
        ssize_t writeCount = SdmSystemUtility::SafeWrite(pollFDS[childInputFD].fd, currentInputBp, (size_t)currentInputCount);
        if (writeCount == -1) {               // error - nuke child and bail
            (void) kill(childPid, SIGKILL);
            break;
        }
        currentInputCount -= writeCount;      // bump back by # bytes child accepted
        currentInputBp += writeCount;         // bump forward index into input stream
        continue;

      }

      // if both the input and output file descriptors become invalid, then
      // we are done processing the data stream; otherwise, continue to spin
      //
      if ( (pollFDS[childInputFD].revents & (POLLHUP|POLLNVAL)) && (pollFDS[childOutputFD].revents & (POLLHUP|POLLNVAL)) )
          break;
  }

  // all done processing data -- perform cleanup work
  //
  (void) SdmSystemUtility::SafeClose(inputPipe[pipeWriter]);    // make sure child input pipe closed
  (void) SdmSystemUtility::SafeClose(outputPipe[pipeReader]);   // make sure child output pipe closed

  // The following code waits for the child process to terminate.  Since
  // dtmail now ignores SIGCHLD to avoid having to wait for audio processes,
  // the waitpid would now fail.  It is now just assumed the compress
  // succeeds providing it has returned some data.
#if 0
  int childStatus;

  while (SdmSystemUtility::SafeWaitpid(childPid, &childStatus, 0) >= 0) // retrieve child status
      ;
  // Hard choices follow: for some reason the uncompress function has exited
  // with a non-zero status - *something* has failed. If so, toss any output
  // that may have been generated, and return a failure indication
  if (childStatus != 0) {
      assert(interimOutputBuffer);
      free((char *)interimOutputBuffer);      // toss any output
      err = Sdm_EC_Fail;
      return err;
  }
#endif

  if (interimOutputLength == 0)
    return (err = Sdm_EC_DecodingError);  // Should probably introduce a more descriptive error.

  // All is well - fixup callers output variables and return Also, reduce
  // size of allocated buffer so that unused space is returned to the free
  // pool
  assert(interimOutputLength == (interimBp-interimOutputBuffer));
  if (interimOutputLength < interimOutputLimit)
    interimOutputBuffer = (unsigned char *)realloc(interimOutputBuffer, (size_t)(interimOutputLength+1));
  assert(interimOutputBuffer);
  interimOutputBuffer[interimOutputLength] = '\0';	// assure that the result is null terminated

  r_outputLen = interimOutputLength;            // stuff output length in caller's variable
  r_outputBp = (char *)interimOutputBuffer;     // stuff -> clear text in caller's variable

  return err;
}

SdmErrorCode   
SdmContentBuffer::DecodeUUEncodedData(SdmError &err, char *&r_outputBp,
                                      unsigned long &r_outputLen, const char * inputBp,
                                      const unsigned long inputLen)
{
  err = Sdm_EC_Success;
  r_outputLen = 0;

  assert (inputBp != NULL);

  // We are not really interested in the "begin <mode> <name>" line,
  // so let's blow by it.
  //
  const char * line_1 = inputBp;

  if (strncmp(inputBp, "begin ", 6) == 0) {
      for (line_1 = inputBp; line_1 < (inputBp + inputLen); line_1++) {
          if (*line_1 == '\n') {
              break;
          }
      }
  }
  line_1 += 1;

  if (line_1 >= (inputBp + inputLen)) {
      err = Sdm_EC_DecodingError;
      return err;
  }

  // We are passed a -> a place to store a pointer to the decoded information
  // and a call-by-ref of a place to store the number of bytes stored. Must
  // allocate the buffer. Fortunately, the uuencoding algorithm is regular
  // encoding 3 8-bit bytes into 4 6-bit bytes, so we can allocate a buffer
  // that is big enough to hold the decoded results, plus a little slosh
  // because the length we compute from includes algrithmic overhead.
  //

  int totalOutputLen = (int)((inputLen/4)*3);
  r_outputBp = (char *)malloc(totalOutputLen+1);
  r_outputLen = 0;

  // The first character of each line tells us how many characters
  // to the next new line. We will loop through each line, and
  // decode the characters. Any line shortages are made up with
  // spaces for the decoding algorithm.
  //
  // Note: a properly uuencoded file is ended with two lines:
  //  1- a line beginning with a space (which works out to 0 bytes)
  //  2- the word "end" on a line by itself.
  //
  // In bug #1196898 the following end sequence was encountered:
  //   "L9&%N8V4@=&AE('1I9&4N+BXB"CX@"0D)(" @(" @1RX@0G)O;VMS"CX@"@H@"
  //   ""
  //   "end"
  //   ""
  //   "Janice Anthes, SE                janice.anthes@west.sun.COM"
  //
  // Where there was a blank line with no leading space in it before
  // the "end" line, so we need to add:
  //  3- a completely blank line (which is an invalid uuencoded line)
  // 
  //
  for (const unsigned char * encodedBp = (const unsigned char *)line_1;
       (const char *)encodedBp < (inputBp + inputLen);) {

      if ( (*encodedBp == '\n')
        || ( (*encodedBp == 'e')
             && (*(encodedBp+1) == 'n')
             && (*(encodedBp+2) == 'd')
             && (*(encodedBp+3) == '\n') ) ) {
        break;
      }

      int decodedLen = decode_uue_char(*encodedBp);   // # of DECODED bytes

      if (decodedLen <= 0 || decodedLen > 45) {
          // End of the buffer.
          break;
      }

      // If whitespace was removed from the end of lines during
      // transmission, the above calculation for the size of the output
      // buffer will be too small.  See bug #1239362.
      if (decodedLen + r_outputLen > totalOutputLen) {
        // Add the decodedLen (to be sure the data will fit) PLUS another
        // 10%, since it's likely other lines will suffer the same
        // truncation problem.
#if 0
        fprintf(stderr, "realloc'ing output buffer from %d to %d!\n",
                totalOutputLen, r_outputLen + decodedLen + totalOutputLen / 10);
#endif
        totalOutputLen = r_outputLen + decodedLen + totalOutputLen / 10;
        r_outputBp = (char*)realloc(r_outputBp, totalOutputLen+1);
        assert(r_outputBp);
      }
      encodedBp += 1; // bop past the # decoded bytes character
      encodedBp = decode_uue_line(r_outputBp,
                               r_outputLen,
                               encodedBp,
                               decodedLen,
                               (const unsigned char *)(inputBp + inputLen));
      assert(r_outputLen <= totalOutputLen);
  }

  // Done decoding the entire source - reduce size of allocated buffer
  // so that unused space is returned to the free pool
  //

  if (r_outputLen < totalOutputLen)
      r_outputBp = (char *)realloc(r_outputBp, r_outputLen+1);

  r_outputBp[r_outputLen] = '\0';	// assure that the result is null terminated

  return err;
}
