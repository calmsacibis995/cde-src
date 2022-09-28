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
#pragma ident "@(#)ImplConfigTable.hh	1.12 09/29/94"
#endif

#ifndef _IMPLCONFIGTABLE_HH
#define _IMPLCONFIGTABLE_HH

#include <DtMail/DtMailValues.hh>

struct ImplConfigTable {
    char	*impl_name;
    char	*lib_name;
    char	*meta_entry_point;
};

static const ImplConfigTable initial_impls[] = {
{ "Internet MIME", NULL,	"RFCMetaFactory" },
{ "Sun Mail Tool", NULL,	"V3MetaFactory" },
{ NULL,		NULL,		NULL }
};

#endif
