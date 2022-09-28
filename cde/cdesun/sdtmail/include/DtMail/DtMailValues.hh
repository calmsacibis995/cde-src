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
#pragma ident "@(#)DtMailValues.hh	1.19 04/10/96"
#endif

#ifndef _DTMAILVALUE_HH
#define _DTMAILVALUE_HH

#include <DtVirtArray.hh>

// Dates consist of a date, and an offset from GMT.
//
#include <time.h>

struct DtMailValueDate {
    time_t	dtm_date;
    time_t	dtm_tz_offset_secs;
};

// An address consists of 3 components. The addressee is the string that
// is used by the transport to deliver the mail to the user. The person
// is the string that represents this person in the real world (e.g. Bob Patterson).
// The namespace is used to identify which address space the name belongs to.
// This can be set to DtMailAddressDefault to specify the library should figure
// out the appropriate name space.
//

#define DtMailAddressDefault	"Dt:Mail:Address:Default"

struct DtMailValueAddress {
    char	*dtm_address;
    char	*dtm_person;
    char	*dtm_namespace;

    DtMailValueAddress(void);
    DtMailValueAddress(const DtMailValueAddress &);
    DtMailValueAddress(const char * address,
		       const char * person,
		       const char * namespace);

    DtMailValueAddress(const char * address, const int addr_len,
		       const char * person, const int per_len,
		       const char * namespace);
    ~DtMailValueAddress(void);
};

class DtMailAddressSeq : public DtVirtArray<DtMailValueAddress *> {
  public:
    DtMailAddressSeq(int size = 8);
    ~DtMailAddressSeq(void);
};

class DtMailValue {
  public:
    DtMailValue(const char * value);
    virtual ~DtMailValue(void);

    virtual operator const char *(void);

    virtual const char * operator= (const char *);

    virtual DtMailValueDate toDate(void);
#ifdef DEAD_WOOD
    virtual void fromDate(const DtMailValueDate &);
#endif /* DEAD_WOOD */

    virtual DtMailAddressSeq * toAddress(void);
#ifdef DEAD_WOOD
    virtual void fromAddress(const DtMailAddressSeq & list);
#endif /* DEAD_WOOD */

    // This method gives the raw format used to store the value.
    //
    virtual const char * raw(void);

  protected:
    char *	_value;
};

class DtMailValueSeq : public DtVirtArray<DtMailValue *> {
  public:
    DtMailValueSeq(int size = 8);
    ~DtMailValueSeq(void);

    void clear(void);
};

#endif
