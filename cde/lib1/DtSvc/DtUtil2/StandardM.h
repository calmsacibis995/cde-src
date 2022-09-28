/* $XConsortium: StandardM.h /main/cde1_maint/1 1995/07/17 18:15:30 drk $ */
/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:     StandardM.h
 **
 **   Project:  DT Library
 **
 **   Description: Defines standard parameters necessary for communication
 **			in the DT environment
 **
 **   (c) Copyright 1990 by Hewlett-Packard Company
 **
 ****************************************************************************
 ************************************<+>*************************************/

/*
 * BMS Tool Class
 */
#define DtMSG_SERVER_TOOL_CLASS           	"MSG-SERVER"

/*
 * Special case that means "all tools"
 */
#define DtALL_TOOL_CLASS			"-"

/*
 * Define for the stop message (which requests that a tool quit)
 */
#define DtSTOP					"STOP"

/*
 * Define for the generic message telling tools that
 * the session is going down.  This is a (N)otification message.
 */
#define DtEND_SESSION				"END_SESSION"

/*
 * _DtMessage to reload the types databases (filetypes,actions, drag).  This is
 * a (N)otification message.
 */
#define DtRELOAD_TYPES_DB                       "RELOAD_TYPES_DB"
#define DtRELOAD_DB_NGROUP			"RELOAD_DB_NGROUP"

/*
 * Command and data used for status requests and responses.
 */
#define DtSTATUS_COMMAND		"STATUS"
#define DtSTATUS_READY_ALL_FTYPES	"* READY"
#define DtSTATUS_BUSY_ALL_FTYPES	"* BUSY"
