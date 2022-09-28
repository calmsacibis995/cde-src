/*
 *+SNOTICE
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
#pragma ident " @(#)PropSource.hh	1.6 10/12/95 "
#endif

#include <DtMail/DtMail.hh>

#ifndef _PROPSOURCE_HH
#define _PROPSOURCE_HH

struct PropDefaults {
    const char *	key;
    const char *	value;
};

// CLASS PropSource
// Virtual class is base class for props data accessing funcs
////////////////////////////////////////////////////////////////
class PropSource {

public:

  PropSource(DtMail::MailRc *m_rc,  char *search_key){ mail_rc = m_rc;
						       key = search_key; };

  virtual ~PropSource(){ ; }; // we don't create any memory

  virtual const char *getValue() = 0;
  const char *getKey() { return key; };
  virtual void setValue(char *value) = 0;

  virtual const char * getDefaultValue(void);
#ifdef DEAD_WOOD
  virtual void updateStore(void) = 0;
#endif /* DEAD_WOOD */

protected:
  
  char *key; // just a pointer NOT our memory
  DtMail::MailRc *mail_rc;

};


// CLASS MailRcSource
// Accesses the mailrc hash tables...
/////////////////////////////////////////////////////////////////

class MailRcSource : public PropSource {

public:
  
  MailRcSource( DtMail::MailRc *m_rc, char *search_key):PropSource(m_rc, search_key) 
                                                                      { ; };
  virtual ~MailRcSource(){;}; // we dont't create any memory

  virtual const char *getValue();
  virtual void setValue(char *value);

#ifdef DEAD_WOOD
  virtual void updateStore(void);
#endif /* DEAD_WOOD */

private:

};

#endif
