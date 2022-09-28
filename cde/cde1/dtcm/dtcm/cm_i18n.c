/*******************************************************************************
**
**  cm_i18n.c
**
**  $XConsortium: cm_i18n.c /main/cde1_maint/3 1995/11/12 19:43:15 barstow $
**
**  RESTRICTED CONFIDENTIAL INFORMATION:
**
**  The information in this document is subject to special
**  restrictions in a confidential disclosure agreement between
**  HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
**  document outside HP, IBM, Sun, USL, SCO, or Univel without
**  Sun's specific written approval.  This document and all copies
**  and derivative works thereof must be returned or destroyed at
**  Sun's request.
**
**  Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
**
*******************************************************************************/

/*									*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc. 				*
 */

#pragma ident "@(#)cm_i18n.c	1.21 96/11/12 SMI"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <floatingpoint.h>

#include <locale.h>
#include <sys/param.h>       /* for MAXPATHLEN */
#include <Xm/Xm.h>
#include "util.h"
#include "timeops.h"
#include "cm_i18n.h"
#include "deskset.h"
/*
#include "gettext.h"
*/



/* The following routine is specific to using FMapType 3 composite fonts
 * in postscript.  Kanji, Asian specific?
 */
char *
euc_to_octal(char *srcStr)
{
	int inKanji = FALSE;
	char buf[64];
	static char dstStr[512];
	int i;
	int len = cm_strlen(srcStr);

#ifdef SVR4
	memset(dstStr, 0, sizeof(dstStr));
#else
	bzero(dstStr, sizeof(dstStr));
#endif /* SVR4 */
	for (i = 0; i < len; i++) {
		if (inKanji) {
			if (!isEUC(srcStr[i])) {
				inKanji = FALSE;
				/* NOT NEEDED FOR FMapType 4 (or 5)
				cm_strcat(dstStr, "\\377\\000");
				*/
			}
		}
		else {
			if (isEUC(srcStr[i])) {
				inKanji = TRUE;
				/* NOT NEEDED FOR FMapType 4 (or 5)
				cm_strcat(dstStr, "\\377\\001");
				*/
			}
		}
		if (inKanji) {
			sprintf(buf, "\\%3.3o\\%3.3o", srcStr[i] & 0xff, srcStr[i+1] & 0xff);
			i++;
		}
		else {
			sprintf(buf, "%c", srcStr[i]);
		}
		cm_strcat(dstStr, buf);
	}
	return dstStr;
}


/* This routine should be in libdeskset.
 * This routine uses fconvert() to avoid locale conversion.
 */
/* 310 characters are the minimum needed to accommodate any double-precision
 * value + 1 null terminator.
 */
#define DBL_SIZE  311
/*
 *  Returns a null terminated formatted string.
 *  If error is encountered, such as malloc() failed, then return NULL.
 *  The caller of this function should beware that the return value is
 *  a static buffer declared within this function and the value of it may
 *  change.
 */
char *
cm_printf(double value, int decimal_pt)
{
	int sign = 0;
	int deci_pt = 0;
	int buf_cnt = 0;
	int formatted_cnt = 0;
	int buf_len = 0;
	char *buf = NULL;
	static char *formatted = NULL;

	if ( formatted != NULL ) {
		free(formatted);
		formatted = NULL;
	}
	if ( (value == (double)0) && (decimal_pt == 0) ) {
		formatted = (char *)cm_strdup("0");
		return formatted;
	}
	if ( (buf = (char *)malloc(DBL_SIZE + decimal_pt)) == NULL ) {
		return (char *)NULL;
	}
	if ( (formatted = (char *)calloc(1, DBL_SIZE + decimal_pt)) == NULL ) {
		free(buf);
		return (char *)NULL;
	}
#ifdef SunOS
	fconvert(value, decimal_pt, &deci_pt, &sign, buf);
#else
	/* this version, available on the HP and AIX machine is not reentrant. */

	strcpy(buf, fcvt(value, decimal_pt, &deci_pt, &sign));
#endif
	if ( sign ) {
		strcpy(formatted, "-");
	}
	buf_len = deci_pt + decimal_pt;
	if ( deci_pt ) {
		strncat(formatted, buf, deci_pt);
	} else {    /* zero */
		strcat(formatted, "0");	
	}
	if ( deci_pt == buf_len ) {
		strcat(formatted, "\0");
		free(buf);
		return formatted;
	}
	strcat(formatted, ".");
	for ( formatted_cnt = strlen(formatted), buf_cnt = deci_pt;  buf_cnt < buf_len;  buf_cnt++, formatted_cnt++ ) {
		formatted[formatted_cnt] = buf[buf_cnt];
	}
	formatted[formatted_cnt] = '\0';
	free(buf);
	return formatted;	
}
