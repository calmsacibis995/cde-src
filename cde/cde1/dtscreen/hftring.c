#ifndef lint
static char sccsid[] = "@(#)07	1.3  src/gos/2d/XTOP/aixclients/dtscreen/hftring.c, xclients, gos410, gos4.17293b 3/25/93 14:54:57";
#endif
/*
 *  COMPONENT_NAME: XCLIENTS
 *
 *  FUNCTIONS:  hftring
 *
 *  ORIGINS: 16,24,27
 *
 *  (C) COPYRIGHT International Business Machines Corp. 1992, 1993
 *  All Rights Reserved
 *  Licensed Materials - Property of IBM
 *
 *  US Government Users Restricted Rights - Use, duplication or
 *  disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

/*              include file for message texts          */
#include <limits.h>
#include <nl_types.h>
#define MF_DTSCREEN "dtscreen.cat"

#include <nl_types.h>
#include <locale.h>
extern nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include "dtscreen.h"

#define AIX_31

#ifdef AIX_31
#define HFTRFILE    "/dev/hft/0"
#define HFQSTAT     hftqstat
#define HFSMGRCMD   hftsmgrcmd
#define HFQEIO      HFQERROR
#define HFCSMGR     HFTCSMGR
#define HFQSMGR     HFTQSMGR
#define HFVTSTATUS  (HFVTACTIVE | HFVTNOHOTKEY)
#define HF_VTIODN   hf_vtid
#else
#define HFTRFILE    "/dev/hft/mgr"
#define HFQSTAT     hfqstat
#define HFSMGRCMD   hfsmgrcmd
#define HFVTSTATUS  HFVTACTIVE
#define HF_VTIODN   hf_vtiodn
#endif

/* Declare global variables.                                                 */
int                        hft_file; 
extern int                 errno;


#ifdef MIT_R5
void hft_error(s)
char  *s;
{                                                                 
}/* end hft_error */
#endif


void  open_hft_file(hft_file_name)
char *hft_file_name;
{
}/* end open_hft_file_ring */



void close_hft_file()
{
}/* end close_hft_file */
 


void get_hft_ring()
{
}/* end get_hft_ring */



void process_hft(hft_action, hft_vtid)
int       hft_action;
unsigned  hft_vtid;
{
}/* end process_hft */



void scan_hft_ring(hft_action)
{
}/* end scan_hft_ring */

void hide_hft_ring()
{
}/* end hide_hft_ring */

void unhide_hft_ring()
{
}/* end unhide_hft_ring */
