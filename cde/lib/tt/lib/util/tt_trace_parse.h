/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
//%% 	$Revision: 1.3 $							
//%% 									
//%%  	RESTRICTED CONFIDENTIAL INFORMATION:                            
//%% 									
//%% 	The information in this document is subject to special		
//%% 	restrictions in a confidential disclosure agreement between	
//%% 	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this	
//%% 	document outside HP, IBM, Sun, USL, SCO, or Univel without	
//%% 	Sun's specific written approval.  This document and all copies	
//%% 	and derivative works thereof must be returned or destroyed at	
//%% 	Sun's request.							
//%% 									
//%% 	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.	
//%% 									
/*
 *
 * tt_trace_parse.h
 *
 * Copyright (c) 1993 by Sun Microsystems, Inc.
 */

#ifndef _TT_TRACE_PARSE_H
#define _TT_TRACE_PARSE_H

typedef enum {
	_TT_TRACE_VERSION = 1,	// Don't want a "false" value
	_TT_TRACE_FOLLOW,
	_TT_TRACE_WRITEFILE,
	_TT_TRACE_APPENDFILE,
	_TT_TRACE_FUNCTIONS,
	_TT_TRACE_ATTRIBUTES,
	_TT_TRACE_STATES,
	_TT_TRACE_OPS,
	_TT_TRACE_SENDER_PTYPES,
	_TT_TRACE_HANDLER_PTYPES,
#ifdef _OPT_TIMERS_SUNOS
	_TT_TRACE_TIMERS,
#endif

	// The above group is the keywords
	
	_TT_TRACE_NUMBER,
	_TT_TRACE_OFF,
	_TT_TRACE_ON,
	_TT_TRACE_FILENAME,
	_TT_TRACE_FUNC_VAL,
	_TT_TRACE_ALL,
	_TT_TRACE_NONE,
	_TT_TRACE_EDGE,
	_TT_TRACE_DELIVER,
	_TT_TRACE_DISPATCH,
	_TT_TRACE_STRING,
	_TT_TRACE_IDENTIFIER,
	_TT_TRACE_STATE_CREATED,
	_TT_TRACE_STATE_SENT,
	_TT_TRACE_STATE_HANDLED,
	_TT_TRACE_STATE_FAILED,
	_TT_TRACE_STATE_QUEUED,
	_TT_TRACE_STATE_STARTED,
	_TT_TRACE_STATE_REJECTED,
	_TT_TRACE_STATE_RETURNED,
	_TT_TRACE_STATE_ACCEPTED,
	_TT_TRACE_STATE_ABSTAINED,

	// The above group is the values

	_TT_TRACE_ENDLINE,	// Should be self-explanatory
	_TT_TRACE_ENDFILE,	// Ditto
	_TT_TRACE_UNKNOWN	// Ditto
	
} _Tt_trace_tokens;

#endif
