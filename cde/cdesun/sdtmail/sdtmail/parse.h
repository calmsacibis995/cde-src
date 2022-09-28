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
#pragma ident "@(#)parse.h	1.1 03/21/96"
#endif

#ifndef _PARSE_H_
#define _PARSE_H_

struct filter *basefilter;

#ifdef __cplusplus
extern "C" {
#endif
struct expr *buildtree(int, struct expr*, struct expr*);
struct expr *buildleaf(int, void*, void*, void*);
struct act *buildact(int, char*);
struct filter *reorder_filter(struct filter*, struct filter*);
struct filter *alloc_filter();
struct options *alloc_options();
struct expr *alloc_expr();
struct act *alloc_act();

struct filter* copy_filter(struct filter *);
struct act *copy_act(struct act *);
struct expr *copy_expr(struct expr *);
void free_filter(struct filter*);
void free_expr(struct expr*);
void free_act(struct act*);

char *scopy();
void parse_rules(char*);
#ifdef __cplusplus
}
#endif

#define RULEFILE ".mailrules"
#endif
