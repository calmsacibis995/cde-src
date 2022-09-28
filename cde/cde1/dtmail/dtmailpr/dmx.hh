
#ifndef _DMX_HH
#define _DMX_HH

/*
 *+SNOTICE
 *
 *	$:$
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement between
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel without
 *	Sun's specific written approval.  This document and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1994 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)dmx.hh	1.15 26 Jan 1995 14:29:57 SMI"
#endif


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>

#include <sys/types.h>
#include <ctype.h>

#if defined(sun) && defined(_XOPEN_SOURCE)
#ifndef B_TRUE
#define B_TRUE _B_TRUE
#endif
#ifndef B_FALSE
#define B_FALSE _B_FALSE
#endif
#endif   /* sun && _XOPEN_SOURCE */

#include <DtMail/DtMail.hh>
#include <DtMail/DtMailError.hh>
#include <DtMail/DtMailValues.hh>
#include <Dt/Dts.h>

extern const char *const	dmxversion;

enum DmxHeaderType { MSGLIST, MSGHEADER, NUMHDRTYPES };
enum DmxHeaders
{
	DMXFROM,
	DMXSUBJ,
	DMXCLENGTH,
	DMXSTATUS,
	DMXDATE,
	DMXTO,
	DMXV3CHARSET,
	DMXCONTENTTYPE,
	DMXNUMHDRS
};

// This is undoubtedly illegal, unethical, and immoral.  So sue me.
#if !defined(SunOS) && !defined(USL)
#undef boolean_t
typedef enum { B_FALSE, B_TRUE } boolean_t;
#endif // SunOS

// utils
char		*convertValueToString (DtMailValueSeq *value, int s);
boolean_t	handleError (DtMailEnv &, char *);
char		*formatHeader (DtMailHeaderLine &, enum DmxHeaderType);

class DmxMsg
{
	public:
		DmxMsg (void);

		char	*printHeader (enum DmxHeaderType);
		void	display (void);
		void	setHandle (DtMailMessageHandle &);
		void	setHeader (DtMailHeaderLine &);
		void	setMessage (DtMail::Message *);
		void	setInfo (char *);
		void	parse (void);
		void	getFlags (void);
	
	
		DtMailMessageHandle	msgHandle;
		DtMailHeaderLine	msgHeader;
		DtMail::Message		*message;

		DtMail::BodyPart	**bodyParts;
		int	numBPs;

		char		*addlInfo;
		boolean_t	cachedValues;
		boolean_t	isCurrent;
		boolean_t	hasAttachments;
		boolean_t	isNew;
		// other flags for status (read, unopened, etc.)
};


class DmxMailbox
{
	public:
		DmxMailbox (void);

		DtMail::MailBox	*mbox;
		int		messageCount;
		int		newMessages;
		int		firstNew;
		char		*name;

		void		loadMessages (void);
		void		createHeaderRequest (DtMailHeaderRequest &);
		void		printMailboxInfo (void);

		DmxMsg		msg [1024];
};



#endif // _DMX_HH
