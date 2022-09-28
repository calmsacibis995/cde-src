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
#pragma ident "@(#)RFCMIME.hh	1.25 11/30/95"
#endif

#ifndef _RFCMIME_HH
#define _RFCMIME_HH

#include "RFCFormat.hh"

class RFCMIME : public RFCFormat {
  public:
    RFCMIME(DtMail::Session *);
    ~RFCMIME(void);

    static void readBase64(char * dest, int & size, const char * text,
			   const unsigned long text_len);

    static void readQPrint(char * dest, int & size, const char * text,
			   const unsigned long text_len);

    static void readTextEnriched(char * dest, int & size, const char * text,
			   const unsigned long text_len);

    void writeBase64(Buffer & buf, const char * bp, 
		     const unsigned long len,
		     const unsigned long max_encoded_line_length = 72);
  
    void writeQPrint(Buffer & buf, const char * bp, 
		     const unsigned long len);

    // digest is:
    // unsigned char digest[16];
    //
    static void md5PlainText(const char * bp,
			     const unsigned long len,
			     unsigned char * digest);

    enum Encoding {
	MIME_7BIT,
	MIME_8BIT,
	MIME_QPRINT,
	MIME_BASE64
	};

  protected:
    virtual void formatBodies(DtMailEnv & error,
			      DtMail::Message & msg,
			      DtMailBoolean include_content_length,
			      char ** extra_headers,
			      Buffer & buf);

    virtual void formatHeaders(DtMailEnv & error,
			       DtMail::Message & msg,
			       DtMailBoolean include_unix_from,
			       DtMailBoolean include_bcc,
			       const char * extra_headers,
			       Buffer & buf);

    virtual void rfc1522cpy(Buffer & buf, const char * value);

  private:

    void getMIMEType(DtMail::BodyPart * bp, char * mime_type, DtMailBoolean & is_text);
    Encoding getHdrEncodingType(const char *, const unsigned int, DtMailBoolean);
    Encoding getEncodingType(const char *, const unsigned int, DtMailBoolean, int *);
    Encoding getClearEncoding(const char *, const unsigned int, int *);
    void writeContentHeaders(Buffer & buf, const char * type,
			     const char * name,
			     const Encoding, const char * digest, int eightbit);
    
    void writePlainText(Buffer & buf, const char * bp, const unsigned long len);
	void owCompat(Buffer & buf, char * type, char * name,
		 char *contents, unsigned long len);
};

#endif
