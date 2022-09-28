/* $XConsortium: laccess.c /main/cde1_maint/1 1995/07/17 19:59:21 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)laccess.c	1.8 96/11/12 Sun Microsystems, Inc."

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "laccess.h"
#include "lutil.h"

/******************************************************************************
 * forward declaration of static functions used within the file
 ******************************************************************************/
static void get_component(char **str, char *comp, char token);
static void get_last_component(char *head, char **tail, char *comp, char token);
static boolean_t match_forward(char *str1, char *str2);
static boolean_t match_backward(char *str1, char *str2);

/*****************************************************************************
 * extern functions used in the library
 *****************************************************************************/

/*
 * Correct format assumed, i.e. str = label1[.label2 ...]
 * Compare str2 against str1 which should be more fully qualified than str2
 */
extern boolean_t
_DtCmIsSamePath(char *str1, char *str2)
{
	if (str1 == NULL || str2 == NULL)
		return(_B_FALSE);

	/* check format */
	if (*str1 == '.' || *str2 == '.')
		return (_B_FALSE); /* bad format */

	if (match_forward(str1, str2) == _B_TRUE)
		return (_B_TRUE);
	else
		return (match_backward(str1, str2));
}

/*
 * compare user1 and user2
 * user1 = user@host[.domain]
 * user2 = any of these formats: user, user@host[.domain], user@domain
 */
extern boolean_t
_DtCmIsSameUser(char *user1, char *user2)
{
	char *str1, *str2, *str3;

	if (user1 == NULL || user2 == NULL)
		return (_B_FALSE);

	/* compare user name */
	if (str1 = strchr(user1, '@')) *str1 = NULL;
	if (str2 = strchr(user2, '@')) *str2 = NULL;

	if (strcmp(user1, user2)) {
		if (str1) *str1 = '@';
		if (str2) *str2 = '@';
		return (_B_FALSE);
	}
	if (str1) *str1 = '@';
	if (str2) *str2 = '@';

	/* if only user name is specified, don't need to check anymore */
	if (str2 == NULL)
		return (_B_TRUE);

	if (str1 = strchr(str1, '.')) {
		/* user1 contains domain info */
		str1 = strchr(user1, '@');
		return (_DtCmIsSamePath(++str1, ++str2));
	} else {
		char buf[BUFSIZ];
		int res;

		str1 = strchr(user1, '@');

		/* just compare host name */
		if (str3 = strchr(++str2, '.')) {
		  	*str3 = NULL;
		}
		res = strcmp(++str1, str2);
		if (str3) {
		  	*str3 = '.';
		}
		
		return (res == 0 ? _B_TRUE : _B_FALSE);
	}
}

/*****************************************************************************
 * static functions used within the file
 *****************************************************************************/

/*
 * str consists of components separated by token
 * get and copy the first component into comp and
 * strip it out of str, so str would point to the first
 * token or the null terminator.
 */
static void
get_component(char **str, char *comp, char token)
{
	char *ptr;

	*comp = 0;

	if (str == NULL)
		return;
	else
		ptr = *str;

	while (ptr && *ptr != 0 && *ptr != token)
		*comp++ = *ptr++;

	*str = ptr;

	*comp = 0;
}

/*
 * head and tail points to the first and last character
 * of a string which consists of components separated by token.
 * get and copy the last component into comp and
 * strip it out of the string, so tail would point to the last
 * token or the head of the string.
 */
static void
get_last_component(char *head, char **tail, char *comp, char token)
{
	char *ptr, *cptr;

	*comp = 0;

	if (tail == NULL)
		return;
	else
		cptr = *tail;

	while (cptr != head && *cptr != token)
		cptr--;

	if (*cptr == token)
		ptr = cptr + 1;
	else
		ptr = cptr;

	while (ptr != (*tail + 1))
		*comp++ = *ptr++;

	*tail = cptr;

	*comp = 0;
}

static boolean_t
match_forward(char *str1, char *str2)
{
	char com1[BUFSIZ], com2[BUFSIZ];

	if (str1 == NULL || str2 == NULL)
		return (_B_FALSE);

	while (_B_TRUE) {
		get_component(&str1, com1, '.');
		get_component(&str2, com2, '.');

		if (*com1) {
			if (*com2 == NULL)
				return (_B_TRUE);
		} else {
			if (*com2 == NULL)
				return (_B_TRUE);
			else
				return (_B_FALSE);
		}

		if (strcasecmp(com1, com2) != 0)
			return (_B_FALSE);

		/* take care of case: a.b a. */
		if (strcmp(str2, ".") == 0
		    && (strcmp(str1, ".") != 0 || *str1 != NULL))
			return (_B_FALSE);

		/* skip "." */
		if (*str1 == '.') {
			if (*str2 == NULL)
				return (_B_TRUE);
			else {
				str1++;
				str2++;
			}
		} else if (strcmp(str2, ".") == 0 || *str2 == NULL)
			return (_B_TRUE);
		else
			return (_B_FALSE);
	}
}

static boolean_t
match_backward(char *str1, char *str2)
{
	int len1, len2;
	char *ptr1, *ptr2;
	char com1[BUFSIZ], com2[BUFSIZ];

	if (str1 == NULL || str2 == NULL)
		return (_B_FALSE);

	len1 = strlen(str1);
	len2 = strlen(str2);
	if (len2 > len1)
		return (_B_FALSE);
	else if (len2 == 0)
		return (_B_TRUE);

	ptr1 = (len1 ? (str1 + len1 - 1) : str1);
	ptr2 = (len2 ? (str2 + len2 - 1) : str2);

	if (*ptr1 == '.' && ptr1 != str1)
		ptr1--;

	if (*ptr2 == '.' && ptr2 != str2)
		ptr2--;

	while (_B_TRUE) {
		get_last_component(str1, &ptr1, com1, '.');
		get_last_component(str2, &ptr2, com2, '.');

		if (*com1) {
			if (*com2 == NULL)
				return (_B_TRUE);
		} else {
			if (*com2 == NULL)
				return (_B_TRUE);
			else
				return (_B_FALSE);
		}

		if (strcasecmp(com1, com2) != 0)
			return (_B_FALSE);

		/* skip "." */
		if (*ptr1 == '.') {
			if (ptr1 != str1)
				ptr1--;
			else
				return (_B_FALSE); /* bad format */
		} else
			return (_B_TRUE); /* done */

		if (*ptr2 == '.') {
			if (ptr2 != str2)
				ptr2--;
			else
				return (_B_FALSE); /* bad format */
		} else
			return (_B_TRUE); /* done */
	}
}


