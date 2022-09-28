/*
 *+SNOTICE
 *
 *
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
 *	Copyright 1996 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)filter.h	1.2 03/25/96"
#endif

#ifndef _FILTER_H_
#define _FILTER_H_

struct filter {
	struct filter *f_next;
	struct filter *f_prev;
	char *f_name;		     /* filter name */
	int f_state; 	 	     /* filter state, can be ON or OFF */
	struct options *f_options;   /* for testing only */
	struct expr *f_expr;	     /* matching criteria that will be matched */
	struct act *f_act;	     /* Action list when a filter is matched */
	char *f_chainedfilter;	     /* the next filter we will jump to */
	int f_consume;	             /* stop remaining filters */
	int f_ignorecase;	     /* case (in)sensitive when matching criteria */
	char *f_grammar;	     /* the whole string of this filter */
};

struct options {
	struct options *o_next;
	char *o_string;
	char *o_value;
};

struct expr {
	struct expr *e_left;		
	struct expr *e_right;
	int e_type;		     /* expression type, #define 262-274 */
	char *e_string1;
	char *e_string2;
	int e_size;		     
	int e_relop;		     /* #define 284-289 */
};

struct act {
	struct act *a_next;
	int a_type;		     /* action type, #define 277-280 and 293 */
	char *a_string;
};

#define RULE 257
#define ENDRULE 258
#define AND 259
#define OR 260
#define NOT 261
#define LPAREN 262
#define RPAREN 263
#define HEADER 264
#define BODY 265
#define ANY 266
#define ANYHEADER 267
#define ATTACHMENT 268
#define SIZE 269
#define HDRSIZE 270
#define INT 271
#define STRING 272
#define BEFORE 273
#define AFTER 274
#define OPTIONS 275
#define COMMA 276
#define RUN 277
#define PIPE 278
#define TEST 279
#define TEST_PIPE 280
#define SKIPREMAINING 281
#define NEXTRULE 282
#define IGNORECASE 283
#define GT 284
#define LT 285
#define GE 286
#define LE 287
#define EQUALS 288
#define NEQUALS 289
#define CRITERIA 290
#define ON 291
#define OFF 292
#define NEG 294
#define SAVE 293	

/* The variables in rule grammar */
#define SAVE_V	 	"save"
#define IGNORE_V	"ignorecaes"
#define RUN_V		"run"
#define CRITERIA_V	"criteria"
#define SKIP_V		"skipremaining"
#define NEXTRULE_V	"nextrule"
#define RULE_V		"rule"
#define END_V 		"endrule"
#define HEADER_V	"header"
#define BODY_V		"body"
#define SIZE_V		"size"
#define BEFORE_V	"before"
#define AFTER_V		"after"
#define ATTACH_V	"attachment"
#define AND_V		"and"
#define OR_V		"or"

#endif
