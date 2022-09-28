/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Sdm Thread TypeDefs.

#ifndef _SDM_THREADDEFS_H
#define _SDM_THREADDEFS_H

#pragma ident "@(#)ThreadDefs.hh	1.22 96/05/13 SMI"

/* In this class we can actually do #ifdef UITHREADS and #ifdef POSIXTHREADS
 * kind of stuff.
 */

#include <thread.h>

typedef thread_t SdmThreadId;

#endif
