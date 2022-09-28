//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 *
 * tt_status_stub.cc
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */

/* 
 * This file just includes the status page for error codes for the stub
 * library.
 */

#include "api/c/tt_c.h"

char _tt_api_status_page[(int)TT_STATUS_LAST];
