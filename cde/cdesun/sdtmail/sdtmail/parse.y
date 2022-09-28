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
 *

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)parse.y	1.4 04/10/96"
#endif
*/

/* parse.y : the parse class for filters */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>
/* #include <DtMail/DtMailError.hh> */
#include "filter.h"
#include "parse.h"
%}

%union {
	char *sval;
	int ival;
	struct filter *fval;
	struct options *oval;
	struct expr *eval;
	struct act *aval;
}

%token RULE ENDRULE AND OR NOT
%token LPAREN RPAREN HEADER BODY ANY ANYHEADER ATTACHMENT SIZE HDRSIZE
%token <ival> INT
%token <sval> STRING
%token BEFORE AFTER OPTIONS COMMA
%token RUN PIPE TEST TEST_PIPE SKIPREMAINING NEXTRULE IGNORECASE
%token GT LT GE LE EQUALS NEQUALS CRITERIA ON OFF SAVE

%type <fval> filters filter
%type <sval> name next
%type <oval> strings
%type <eval> criteria expression term
%type <aval> actions action
%type <ival> state relop consume ignorecase

%left OR
%left AND
%right NEG

%%

filters	: 	/* empty */ { $$ = NULL; }
	|	filters filter 
		{	
			if ($2) {
				$2->f_prev = $1;
				$$ = $2;
				basefilter = $2;
				printf("%s\n", $2);
			}
		}
	;

filter	:	RULE name state ignorecase criteria actions consume next ENDRULE
		{
			struct filter *f = alloc_filter();
			f->f_name = $2;
			f->f_state = $3;
			f->f_ignorecase = $4;
			f->f_expr = $5;
			f->f_act = $6;
			f->f_consume = $7;
			f->f_chainedfilter = $8;
			$$ = f;
		}
	|	OPTIONS strings
		{
			struct filter *f = alloc_filter();

			f->f_options = $2;
			$$ = f;
		}
	|	error ENDRULE	{ yyerrok; }
	;


/* For OPTIONS stuff */
strings	:	strings COMMA STRING
		{	
			struct options *o = alloc_options();
			struct options *o2;

			/* warning: n^2 algorithm to get last element */
			for(o2 = $1; o2->o_next; o2 = o2->o_next);
			o2->o_next = o;

			$$ = $1 ;
			o->o_string = $3;
		}
	|	STRING EQUALS STRING
		{
			struct options *o = alloc_options();

			o->o_string = $1;
			o->o_value = $3;
			$$ = o;
		}
	|	STRING
		{	
		    
			struct options *o = alloc_options();
			o->o_string = $1;
			$$ = o;
		}
	;

name	:	STRING		{ $$ = $1; }
	;

state	: 	ON	{ $$ = 1; }
        |	OFF 	{ $$ = 0; }
        ;

/* The criteria matching will be case sensitive or insensitive */
ignorecase	:	/* empty */	{ $$ = NULL; }
	|	IGNORECASE		{ $$ = 1; }
	;

criteria	:	CRITERIA LPAREN expression RPAREN	{ $$ = $3; }
	;

expression	:	term			{ $$ = $1; }
	|	expression OR expression	{ $$ = buildtree(OR, $1, $3); }
	|	expression AND expression	{ $$ = buildtree(AND, $1, $3); }
	|	NOT expression	%prec NEG	{ $$ = buildtree(NOT, $2, NULL); }
	;

term	:	LPAREN expression RPAREN	{ $$ = $2; }
	|	HEADER STRING		
			{ $$ = buildleaf(HEADER, $2, NULL, NULL); }
	|	BODY EQUALS STRING	
			{ $$ = buildleaf(BODY, $3, NULL, NULL); }
	|	HEADER STRING EQUALS STRING
			{ $$ = buildleaf(HEADER, $2, $4, NULL); }
	|	HEADER ANY EQUALS STRING
			{ $$ = buildleaf(ANYHEADER, $4, NULL, NULL); }
	|	HEADER STRING relop INT
			{ $$ = buildleaf(HDRSIZE, $2, (void*)$3, (void*)$4); }
	|	HEADER STRING BEFORE STRING
			{ $$ = buildleaf(BEFORE, $2, $4, NULL); }
	|	HEADER STRING AFTER STRING
			{ $$ = buildleaf(AFTER, $2, $4, NULL); }
	|	ATTACHMENT		
			{ $$ = buildleaf(ATTACHMENT, NULL, NULL, NULL); }
	| 	ATTACHMENT STRING	
			{ $$ = buildleaf(ATTACHMENT, $2, NULL, NULL); }
	|	SIZE relop INT		
			{ $$ = buildleaf(SIZE, (void*)$2, (void*)$3, NULL); }
	|	BEFORE STRING		
			{ $$ = buildleaf(BEFORE, $2, NULL, NULL); }
	|	AFTER STRING		
			{ $$ = buildleaf(AFTER, $2, NULL, NULL); }
	;

relop	:	GT			{ $$ = GT; }
	|	LT			{ $$ = LT; }
	|	GE			{ $$ = GE; }
	|	LE			{ $$ = LE; }
	|	EQUALS			{ $$ = EQUALS; }
	|	NEQUALS			{ $$ = NEQUALS; }
	;

actions	:	actions action
		{
			if ($1) {
				struct act *a;

				/* warning: n^2 algorithm */
				for (a = $1; a->a_next; a = a->a_next);
				a->a_next = $2;

				$$ = $1;
			} else {
				$$ = $2;
			}
		}
	|	/* empty */	{ $$ = NULL; };
	;

action	:	RUN STRING	{ $$ = buildact(RUN, $2); }
 	|	SAVE STRING	{ $$ = buildact(SAVE, $2); }
	|	PIPE STRING	{ $$ = buildact(PIPE, $2); }
	|	TEST STRING	{ $$ = buildact(TEST, $2); }
	|	TEST_PIPE STRING { $$ = buildact(TEST_PIPE, $2); }
	;

/* Do we stop matching the next filter rule or not */
consume	:	/* empty */	{ $$ = NULL; }
        |	SKIPREMAINING	{ $$ = 1; }
	;

/* The next filter to match */
next    :       /* empty */             { $$ = NULL; }
        |       NEXTRULE STRING       { $$ = $2; }
        ;

%%


FILE *filter_file;
int line_number = 1;
extern int yydebug;
extern int numerrors = 0;

/* We open a file containing sets of rules and get a parsed filter list
 * "basefilter" back by calling this function.
 */
void
parse_rules(char* rulefile)
{
	int i;
	yydebug = 0;

        filter_file = fopen(rulefile, "r");

	if (filter_file != NULL) {
	    i = yyparse();
	}
	fclose(filter_file);
}


/* a useless error print routine */
void
yyerror(char *s)
{
	numerrors++;

/*	DtMailEnv error;
	error.clear();
	error.logError(DTM_FALSE, "Filter rule syntax error %s at line %d", s, line_number);
*/
}


/* we fail horribly if we run out of memory.  this is probably
 * not the right thing to do...
 */
static void
no_memory()
{
/*	DtMailEnv error;
	error.clear();
	error.logError(DTM_TRUE, "Out of memory!");
*/
	exit(1);
}



/* allocate space for a new string and initialize it */
char *
scopy(char s[])
{
	char *str;

	if (!s) return(NULL);

	str = (char *) calloc(strlen(s) + 1, 1);
	if (!str) no_memory();

	strcpy(str, s);
	return(str);
}



struct filter *
alloc_filter()
{
	struct filter *f;

	f = (struct filter *) calloc(1, sizeof(struct filter));
	if (!f) no_memory();
	memset(f, 0, sizeof(struct filter));

	return (f);
}

struct options *
alloc_options()
{
	struct options *o;

	o = (struct options *) calloc(1, sizeof(struct options));
	if (!o) no_memory();
	memset(o, 0, sizeof(struct options));

	return (o);
}

struct expr *
alloc_expr()
{
	struct expr *e;

	e = (struct expr *) calloc(1, sizeof(struct expr));
	if (!e) no_memory();
	memset(e, 0, sizeof(struct expr));

	return (e);
}

struct act *
alloc_act()
{
	struct act *a;

	a = (struct act *) calloc(1, sizeof(struct act));
	if (!a) no_memory();
	memset(a, 0, sizeof(struct act));

	return (a);
}


struct act *
buildact(int type, char *string)
{
	struct act *a;

	a = alloc_act();

	a->a_type = type;
	a->a_string = string;

	return (a);
}


struct expr *
buildtree(int op, struct expr *left, struct expr *right)
{
	struct expr *e = alloc_expr();


	e->e_type = op;
	e->e_left = left;
	e->e_right = right;

	return (e);
}


struct expr *
buildleaf(int op, void *arg1, void *arg2, void *arg3)
{
	struct expr *e = alloc_expr();



	e->e_type = op;
	switch(op) {
	case HEADER:
	case ANYHEADER:
	case BODY:
	case ATTACHMENT:
	case BEFORE:
	case AFTER:
		e->e_string1 = (char *) arg1;
		e->e_string2 = (char *) arg2;
		break;

	case SIZE:
		e->e_size = (int) arg2;
		e->e_relop = (int) arg1;
		break;
	
	case HDRSIZE:
		e->e_size = (int) arg3;
		e->e_relop = (int) arg2;
		e->e_string1 = (char *) arg1;
		break;

	default:
/*		DtMailEnv error;	
		error.clear();
		error.logError(DTM_TRUE, "fatal error: fell out of buildleaf (%#x)", op);
*/
		exit(1);
	}

	return (e);
}



/* generic get-next-character routine.  it is split out so we can keep
 * track of line numbers (not quite exactly though, because of the push
 * down stack in the parser...
 */
int
gc()
{
	register c;

	c = getc(filter_file);

	if (c == '\n') line_number++;

	return (c);
}

void
ungc(int c)
{
	if (c == '\n') line_number--;
	ungetc(c, filter_file);
}


int
lookup(char *string)
{
	if (!strcasecmp(string,"rule")) return (RULE);
	else if (!strcasecmp(string,"endrule")) return (ENDRULE);
	else if (!strcasecmp(string,"and")) return (AND);
	else if (!strcasecmp(string,"or")) return (OR);
	else if (!strcasecmp(string,"not")) return (NOT);
	else if (!strcasecmp(string,"header")) return (HEADER);
	else if (!strcasecmp(string,"attachment")) return (ATTACHMENT);
	else if (!strcasecmp(string,"size")) return (SIZE);
	else if (!strcasecmp(string,"before")) return (BEFORE);
	else if (!strcasecmp(string,"after")) return (AFTER);
	else if (!strcasecmp(string,"options")) return (OPTIONS);
	else if (!strcasecmp(string,"run")) return (RUN);
	else if (!strcasecmp(string,"save")) return (SAVE);
	else if (!strcasecmp(string,"pipe")) return (PIPE);
	else if (!strcasecmp(string,"test")) return (TEST);
	else if (!strcasecmp(string,"test_pipe")) return (TEST_PIPE);
	else if (!strcasecmp(string,"skipremaining")) return (SKIPREMAINING);
	else if (!strcasecmp(string,"ignorecase")) return (IGNORECASE);
	else if (!strcasecmp(string,"on")) return (ON);
	else if (!strcasecmp(string,"off")) return (OFF);
	else if (!strcasecmp(string,"criteria")) return (CRITERIA);
	else if (!strcasecmp(string,"nextfilter")) return (NEXTRULE);
	else if (!strcasecmp(string,"any")) return (ANY);
	else if (!strcasecmp(string,"body")) return (BODY);

	numerrors++;
/*	DtMailEnv error;
	error.clear();
	error.logError(DTM_FALSE, "Unknown keyword %s on line %d", string, line_number);
*/

	return(-1);
}


int
is_legal_char(int c, int first)
{
	if (isalpha(c)) return (1);
	if (isdigit(c) && ! first) return (1);
	switch (c) {
	case '_':
		return (1);
	}

	return (0);
}


int
yylex()
{
	register c;
	char buffer[10240];
	register char *bufp;
	char *bufend;
	register breakout;
	int num;
	int c2;

top:
	/* skip leading white space */
	do {
		c = gc();
	} while (c != EOF && isspace(c));

	switch (c) {
	case '#':
		/* comment leader */
		do {
			c = gc();
		} while (c != EOF && c != '\n');
		goto top;

	case '=':
		return (EQUALS);

	case '!':
		if ((c2 = gc()) == '=') {
			return (NEQUALS);
		}
		ungc(c2);
		break;

	case '(':
		return(LPAREN);

	case ')':
		return(RPAREN);

	case '>':
		c = gc();
		if(c == '=') {
		    return (GE);
		}
		ungc(c);
		return (GT);

	case '<':
		c = gc();
		if(c == '=') {
		    return (LE);
		}
		ungc(c);
		return (LT);

	case EOF:
		return (EOF);

	case ',':
		return (COMMA);

	case '"':
		/* we have the start of a string.  scan for the " terminator */
		bufp = buffer;
		bufend = &buffer[sizeof buffer -1];
		breakout = 0;

		do {
			c = gc();

			switch (c) {
			case '"':
				/* string end */
				breakout = 1;
				break;

			case '\\':
				c = gc();
				break;

			case '\n':
			case EOF:
				numerrors++;
/*				DtMailEnv error;	
				error.clear();
				error.logError(DTM_FALSE, "Unterminated string on line %d",line_number);
*/
				breakout = 1;
			}

			if (breakout) break;

			*bufp++ = c;

		} while (bufp < bufend);

		*bufp = '\0';
		yylval.sval = scopy(buffer);
		return (STRING);

	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		/* scan for an integer */
		num = 0;
		while (c != EOF && isdigit(c)) {
			num = num * 10 + c - '0';

			c = gc();
		}

		ungc(c);
		yylval.ival = num;

		return (INT);
	}

	if (is_legal_char(c,1)) {

		bufp = buffer;
		bufend = &buffer[sizeof buffer - 1];

		do {
			*bufp++ = c;

			c = gc();
		} while (c != EOF && is_legal_char(c,0) && bufp < bufend);

		ungc(c);
		*bufp = '\0';

		num = lookup(buffer);

		if (num == -1) {
			/* an error occurred */
			goto top;
		}

		return (num);
	}

	numerrors++;
/*	DtMailEnv error;
	error.clear();
	error.logError(DTM_FALSE, "Unrecognized character %c on line %d", c, line_number);
*/
	goto top;

}



/* because of the nature of the parser, it is more convenient
 * to get a handle on the last filter instead of the first.  We
 * reorder all the filter's here to get the other direction
 */
struct filter *
reorder_filter(struct filter *f, struct filter *next)
{

	while (f) {

		/* assign the next field */
		f->f_next = next;

		/* walk backwards one node */
		next = f;
		f = f->f_prev;
	}

	/* return the first node in the list (if any) */
	return(next);
}

struct filter* 
copy_filter(struct filter *src_f)
{
    struct filter *f;

    if (!src_f) 
	return NULL;

    f = alloc_filter();

    f->f_act = copy_act(src_f->f_act);	   /* Copy the action list */
    f->f_expr = copy_expr(src_f->f_expr);  /* Copy the expression tree */

    f->f_state = src_f->f_state;
    f->f_consume = src_f->f_consume;
    f->f_ignorecase = src_f->f_ignorecase;

    if (src_f->f_name)
	f->f_name = strdup(src_f->f_name);
    if (src_f->f_chainedfilter)
	f->f_chainedfilter = strdup(src_f->f_chainedfilter);
    if (src_f->f_grammar)
	f->f_grammar = strdup(src_f->f_grammar);

    return (f);
}

struct act*
copy_act(struct act *src_a)
{
    struct act *a;
    if (src_a == NULL)
	return NULL;

    a = alloc_act();
    a->a_next = copy_act(src_a->a_next);
    a->a_type = src_a->a_type;
    if (src_a->a_string)
	a->a_string = strdup(src_a->a_string);

    return a;
}

struct expr *
copy_expr(struct expr *src_e)
{
    struct expr *e;
    if (src_e == NULL)
	return NULL;

    e = alloc_expr();
    e->e_type = src_e->e_type;

    if (src_e->e_left == NULL) {
	if (src_e->e_string1)
	    e->e_string1 = strdup(src_e->e_string1);
	if  (src_e->e_string2)
	    e->e_string2 = strdup(src_e->e_string2);
	e->e_size = src_e->e_size;
	e->e_relop = src_e->e_relop;
    }
    else {
	e->e_left = copy_expr(src_e->e_left);
	e->e_right = copy_expr(src_e->e_right);
    }
    return e;
}

void free_filter(struct filter *f)
{
    if (!f) return;

    free(f->f_name);
    free(f->f_grammar);
    free(f->f_chainedfilter);
    free_expr(f->f_expr);
    free_act(f->f_act);
    free(f);
}

void free_expr(struct expr *e)
{
    if (!e) return;

    if (e->e_left)
	free_expr(e->e_left);
    if (e->e_right)
	free_expr(e->e_right);
    if (e->e_string1)
	free(e->e_string1);
    if (e->e_string2)
	free(e->e_string2);
    free(e);
}

void free_act(struct act *a)
{
    if (!a) return;

    if (a->a_next)
	free_act(a->a_next);
    free(a->a_string);
    free(a);
}

