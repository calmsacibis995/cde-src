/*
 *+SNOTICE
 *
 *
 *	$Revision: 1.3 $
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
#pragma ident "@(#)AliasExpand.hh	1.1 06/29/94"
#endif

#ifndef _ALIASEXPAND_HH
#define _ALIASEXPAND_HH

#include <DtMail/DtMail.hh>
#include <DtMail/HashTable.hh>

class AliasKey : public ObjectKey {
  public:
    AliasKey(const char * str);
    ~AliasKey(void);
 
    int operator==(ObjectKey &);
    int operator!=(ObjectKey &);
    int operator<(ObjectKey &);
    int operator>(ObjectKey &);
    int operator<=(ObjectKey &);
    int operator>=(ObjectKey &);
 
    HashVal hashValue(void);
 
  private:
    char *      _str;
};

void rfcAliasExpand(DtMailEnv &,
		    DtMail::MailRc & mailrc,
		    DtMailAddressSeq & addrs);

#endif
