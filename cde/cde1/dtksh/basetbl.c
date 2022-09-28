/* $XConsortium: basetbl.c /main/cde1_maint/1 1995/07/17 23:06:14 drk $ */
/** @(#)basetbl.c	1.17 **/

/*	Copyright (c) 1991, 1992 UNIX System Laboratories, Inc.   */
/*	All Rights Reserved     */

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF UNIX System Laboratories, Inc.    */
/*	The copyright notice above does not evidence any       */
/*	actual or intended publication of such source code.    */

#include "stdio.h"
#include <sys/types.h>
#include "exksh.h"
#include "msgs.h"

/*
 * Declare all strings in one place to avoid duplication
 */
char STR_uint[] = "uint";
static char STR_intp[] = "intp";
static char STR_int[] = "int";
static char STR_dint[] = "dint";
char STR_unsigned_long[] = "unsigned_long";
static char STR_longp[] = "longp";
static char STR_long[] = "long";
static char STR_dlong[] = "dlong";
static char STR_ushort[] = "ushort";
static char STR_short[] = "short";
static char STR_dshort[] = "dshort";
static char STR_unchar[] = "unchar";
static char STR_char[] = "char";
char STR_string_t[] = "string_t";

struct memtbl T_uint[] = {
	{ (char *) STR_uint, (char *) STR_uint, K_INT, F_SIMPLE, -1, 0, 0, 0, 0, 0, sizeof(uint), 0 }, NULL
};
struct memtbl T_dint[] = {
	{ (char *) STR_dint, (char *) STR_dint, K_DINT, F_SIMPLE, -1, 0, 0, 0, 0, 0, sizeof(int) },  NULL
};
struct memtbl T_int[] = {
	{ (char *) STR_int, (char *) STR_int, K_INT, F_SIMPLE, -1, 0, 0, 0, 0, 0, sizeof(int) },  NULL
};
struct memtbl T_intp[] = {
	{ (char *) STR_intp, (char *) STR_intp, K_INT, F_SIMPLE, -1, 1, 0, 0, 0, 0, sizeof(int) },  NULL
};
struct memtbl T_unsigned_long[] = {
	{ (char *) STR_unsigned_long, (char *) STR_unsigned_long, K_LONG, F_SIMPLE, -1, 0, 0, 0, 0, 0, sizeof(unsigned long), 0 }, NULL
};
struct memtbl T_long[] = {
	{ (char *) STR_long, (char *) STR_long, K_LONG, F_SIMPLE, -1, 0, 0, 0, 0, 0, sizeof(long) },  NULL
};
struct memtbl T_dlong[] = {
	{ (char *) STR_dlong, (char *) STR_dlong, K_DLONG, F_SIMPLE, -1, 0, 0, 0, 0, 0, sizeof(long) },  NULL
};
struct memtbl T_longp[] = {
	{ (char *) STR_longp, (char *) STR_longp, K_LONG, F_SIMPLE, -1, 1, 0, 0, 0, 0, sizeof(long) },  NULL
};
struct memtbl T_ushort[] = {
	{ (char *) STR_ushort, (char *) STR_ushort, K_SHORT, F_SIMPLE, -1, 0, 0, 0, 0, 0, sizeof(ushort) },  NULL
};
struct memtbl T_short[] = {
	{ (char *) STR_dshort, (char *) STR_dshort, K_DSHORT, F_SIMPLE, -1, 0, 0, 0, 0, 0, sizeof(short) },  NULL
};
struct memtbl T_dshort[] = {
	{ (char *) STR_short, (char *) STR_short, K_SHORT, F_SIMPLE, -1, 0, 0, 0, 0, 0, sizeof(short) },  NULL
};
struct memtbl T_unchar[] = {
	{ (char *) STR_unchar, (char *) STR_unchar, K_CHAR, F_SIMPLE, -1, 0, 0, 0, 0, 0, sizeof(unsigned char) },  NULL
};
struct memtbl T_char[] = {
	{ (char *) STR_char, (char *) STR_char, K_CHAR, F_SIMPLE, -1, 0, 0, 0, 0, 0, sizeof(char) },  NULL
};
struct memtbl T_string_t[] = {
	{ (char *) STR_string_t, (char *) STR_string_t, K_STRING, F_TYPE_IS_PTR, -1, 0, 0, -1, 0, 0, sizeof(char *) },  NULL
};
struct memtbl *basemems[] = {
	T_uint,
	T_int,
	T_dint,
	T_intp,
	T_unsigned_long,
	T_long,
	T_dlong,
	T_longp,
	T_ushort,
	T_short,
	T_dshort,
	T_unchar,
	T_char,
	T_string_t,
	NULL
};

struct symarray basedefs[] = {
	{ "PRDECIMAL", PRDECIMAL },
	{ "PRHEX", PRHEX },
	{ "PRMIXED", PRMIXED },
	{ "PRMIXED_SYMBOLIC", PRMIXED_SYMBOLIC },
	{ "PRNAMES", PRNAMES },
	{ "PRSYMBOLIC", PRSYMBOLIC },
	{ NULL, 0 }
};
