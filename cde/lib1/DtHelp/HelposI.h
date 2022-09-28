/* $XConsortium: HelposI.h /main/cde1_maint/1 1995/07/17 17:41:19 drk $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:   DtosI.h
 **
 **  Project:  Rivers Project, 
 **
 **  Description:  Internal header file for our Dtos.c module
 **  -----------
 **
 **  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 Hewlett-Packard Company
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Novell, Inc.
 **
 *******************************************************************
 *************************************<+>*************************************/
#ifndef _DtosI_h
#define _DtosI_h


#ifdef MESSAGE_CAT
#define _DTGETMESSAGE(set, number, string) _DtHelpGetMessage(set, number, string)
#else /* MESSAGE_CAT */
#define _DTGETMESSAGE(set, number, string) string
#endif /* MESSAGE_CAT */


/*****************************************************************************
 * Function:	   Boolean _DtHelpOSGetHomeDirName(
 *
 *
 * Parameters:     
 *
 * Return Value:    String.
 *
 *
 * Description: 
 *
 *****************************************************************************/
#ifdef _NO_PROTO
extern void _DtHelpOSGetHomeDirName(); 
#else
extern void _DtHelpOSGetHomeDirName(
    String outptr);
#endif

/*****************************************************************************
 * Function:       _DtHelpGetUserSearchPath(
 *
 *
 * Parameters:
 *
 * Return Value:    String, owned by caller.
 *
 *
 * Description:
 *                  Gets the user search path for use
 *                  when searching for a volume.
 *                  Takes path from the environment,
 *                  or uses the default path.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
String _DtHelpGetUserSearchPath();
#else
String _DtHelpGetUserSearchPath(void);
#endif


/*****************************************************************************
 * Function:       _DtHelpGetSystemSearchPath(
 *
 *
 * Parameters:
 *
 * Return Value:    String, owned by caller.
 *
 *
 * Description:
 *                  Gets the system search path for use
 *                  when searching for a volume.
 *                  Takes path from the environment,
 *                  or uses the default path.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
String _DtHelpGetSystemSearchPath();
#else
String _DtHelpGetSystemSearchPath(void);
#endif



/*****************************************************************************
 * Function:	   Boolean _DtHelpGetMessage(
 *
 *
 * Parameters:     
 *
 * Return Value:   char *
 *
 *
 * Description:    This function will retreive the requested message from the
 *                 cache proper cache creek message catalog file.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
extern char *_DtHelpGetMessage();
#else
extern char *_DtHelpGetMessage(
        int set,
        int n,
        char *s);
#endif /* _NO_PROTO */


#endif /* _DtosI_h */
/* Do not add anything after this endif. */













