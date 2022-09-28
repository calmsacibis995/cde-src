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
#pragma ident "@(#)RFCFormat.hh	1.8 11/30/95"
#endif

#ifndef _RFCFORMAT_HH
#define _RFCFORMAT_HH

#include <DtMail/DtMail.hh>
#include <DtMail/Buffer.hh>

class RFCFormat : public DtCPlusPlusAllocator {
  public:
    RFCFormat(DtMail::Session * session);
    ~RFCFormat(void);

    virtual void msgToBuffer(DtMailEnv & error,
			     DtMail::Message & msg,
			     DtMailBoolean include_content_length,
			     DtMailBoolean include_unix_from,
			     DtMailBoolean include_bcc,
			     Buffer & headers,
			     Buffer & body);
    
  protected:
    DtMail::Session	*_session;
    DtMailBoolean	_use_cr;

    virtual void formatBodies(DtMailEnv & error,
			      DtMail::Message & msg,
			      DtMailBoolean include_content_length,
			      char ** extra_headers,
			      Buffer & buf) = 0;

    virtual void formatHeaders(DtMailEnv & error,
			       DtMail::Message & msg,
			       DtMailBoolean include_unix_from,
			       DtMailBoolean include_bcc,
			       const char * extra_headers,
			       Buffer & buf) = 0;

    void writeHeaders(DtMailEnv & error,
		      DtMail::Message & msg,
		      DtMailBoolean include_unix_from,
		      DtMailBoolean include_bcc,
		      const char * extra_headers,
		      const char ** suppress_headers,
		      Buffer & buf);

    virtual void rfc1522cpy(Buffer & buf, const char * value);
    void getCharSet(char * charset);
    void getCharSet(char * charset, char *special);

    void crlf(Buffer & buf)
  {
      const char * term = (_use_cr ? "\r\n" : "\n");
      int len = (_use_cr ? 2 : 1);
      buf.appendData(term, len);
  }

};

inline void
appendString(Buffer & buf, const char * str)
{
    buf.appendData(str, strlen(str));
}

#endif
