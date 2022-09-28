/*
 *+SNOTICE
 *
 *
 *	$Revision: 1.2 $
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
#pragma ident "@(#)NotDynamic.C	1.3 03/16/94"
#endif

#include <stdio.h>
#include <unistd.h>

#include "NotDynamic.hh"

NotDynamic::NotDynamic(const char * path)
: DynamicLib(path)
{
}

NotDynamic::~NotDynamic(void)
{
}

struct SymTable {
    const char *	symbol;
    void *		entry;
};

#include <DtMail/DtMail.hh>

extern "C" void * RFCMetaFactory(const char * op);
extern "C" void * V3MetaFactory(const char * op);

static const SymTable symbol_table[] = {
{ "RFCMetaFactory",	RFCMetaFactory },
{ "V3MetaFactory",	V3MetaFactory },
{ NULL,			NULL }
};

void *
NotDynamic::getSym(const char * sym)
{
    for (int i = 0; symbol_table[i].symbol; i++) {
	if (strcmp(symbol_table[i].symbol, sym) == 0) {
	    return(symbol_table[i].entry);
	}
    }

    return(NULL);
}
