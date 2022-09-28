
# line 29 "parse.y"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <syslog.h>
#include <DtMail/DtMailError.hh>
#include "filter.h"
#include <parse.h>

# line 39 "parse.y"
typedef union
#ifdef __cplusplus
	YYSTYPE
#endif
 {
	char *sval;
	int ival;
	struct filter *fval;
	struct options *oval;
	struct expr *eval;
	struct act *aval;
} YYSTYPE;
# define RULE 257
# define ENDRULE 258
# define AND 259
# define OR 260
# define NOT 261
# define LPAREN 262
# define RPAREN 263
# define HEADER 264
# define BODY 265
# define ANY 266
# define ANYHEADER 267
# define ATTACHMENT 268
# define SIZE 269
# define HDRSIZE 270
# define INT 271
# define STRING 272
# define BEFORE 273
# define AFTER 274
# define OPTIONS 275
# define COMMA 276
# define RUN 277
# define PIPE 278
# define TEST 279
# define TEST_PIPE 280
# define SKIPREMAINING 281
# define NEXTRULE 282
# define IGNORECASE 283
# define GT 284
# define LT 285
# define GE 286
# define LE 287
# define EQUALS 288
# define NEQUALS 289
# define CRITERIA 290
# define ON 291
# define OFF 292
# define SAVE 293
# define NEG 294

#ifdef __STDC__
#include <stdlib.h>
#include <string.h>
#else
#include <malloc.h>
#include <memory.h>
#endif

#include <values.h>

#ifdef __cplusplus

#ifndef yyerror
	void yyerror(const char *);
#endif

#ifndef yylex
#ifdef __EXTERN_C__
	extern "C" { int yylex(void); }
#else
	int yylex(void);
#endif
#endif
	int yyparse(void);

#endif
#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
extern int yychar;
extern int yyerrflag;
YYSTYPE yylval;
YYSTYPE yyval;
typedef int yytabelem;
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
#if YYMAXDEPTH > 0
int yy_yys[YYMAXDEPTH], *yys = yy_yys;
YYSTYPE yy_yyv[YYMAXDEPTH], *yyv = yy_yyv;
#else	/* user does initial allocation */
int *yys;
YYSTYPE *yyv;
#endif
static int yymaxdepth = YYMAXDEPTH;
# define YYERRCODE 256

# line 224 "parse.y"



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

	DtMailEnv error;
	error.clear();
	error.logError(DTM_FALSE, "Filter rule syntax error %s at line %d", s, line_number);
}


/* we fail horribly if we run out of memory.  this is probably
 * not the right thing to do...
 */
static void
no_memory()
{
	DtMailEnv error;
	error.clear();
	error.logError(DTM_TRUE, "Out of memory!");

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
		DtMailEnv error;	
		error.clear();
		error.logError(DTM_TRUE, "fatal error: fell out of buildleaf (%#x)", op);
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
	DtMailEnv error;
	error.clear();
	error.logError(DTM_FALSE, "Unknown keyword %s on line %d", string, line_number);

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
				DtMailEnv error;	
				error.clear();
				error.logError(DTM_FALSE, "Unterminated string on line %d",line_number);
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
	DtMailEnv error;
	error.clear();
	error.logError(DTM_FALSE, "Unrecognized character %c on line %d", c, line_number);

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

    if (!src_f) 
	return NULL;

    struct filter *f = alloc_filter();

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

yytabelem yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 49
# define YYLAST 128
yytabelem yyact[]={

    27,    29,    30,    31,    26,    21,    74,    75,    59,    60,
    61,    62,    63,    64,    76,    56,    28,    59,    60,    61,
    62,    72,    64,    12,    13,    15,    17,    43,    34,    35,
    14,    36,    37,     5,     3,    38,    39,    55,    83,    80,
    40,    41,    82,    54,    81,    79,    77,    68,    66,    51,
    65,    57,     4,    48,    47,    46,    45,    44,    19,    18,
     9,     7,    78,    51,    50,    51,    50,    71,    67,    49,
    23,    10,    58,    32,    16,    24,    11,    25,    22,    33,
    20,     8,    42,     6,     2,     1,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,    52,    53,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,    69,    70,     0,    73 };
yytabelem yypact[]={

-10000000,  -223,-10000000,  -211,  -212,  -187,  -268,-10000000,  -246,  -263,
-10000000,  -257,-10000000,-10000000,  -213,  -214,  -285,-10000000,-10000000,-10000000,
-10000000,  -192,  -277,  -233,  -255,-10000000,-10000000,  -215,  -216,  -217,
  -218,  -219,  -194,-10000000,  -233,  -233,  -229,  -273,  -221,  -276,
  -222,  -224,  -190,  -225,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,
  -233,  -233,-10000000,  -196,  -267,  -274,  -226,-10000000,  -209,-10000000,
-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,-10000000,  -210,
-10000000,-10000000,  -227,  -232,  -228,  -230,  -234,-10000000,-10000000,-10000000,
-10000000,-10000000,-10000000,-10000000 };
yytabelem yypgo[]={

     0,    85,    84,    83,    82,    81,    80,    73,    79,    78,
    77,    76,    72,    75,    74 };
yytabelem yyr1[]={

     0,     1,     1,     2,     2,     2,     5,     5,     5,     3,
    11,    11,    14,    14,     6,     7,     7,     7,     7,     8,
     8,     8,     8,     8,     8,     8,     8,     8,     8,     8,
     8,     8,    12,    12,    12,    12,    12,    12,     9,     9,
    10,    10,    10,    10,    10,    13,    13,     4,     4 };
yytabelem yyr2[]={

     0,     1,     5,    19,     5,     5,     7,     7,     3,     3,
     3,     3,     1,     3,     9,     3,     7,     7,     5,     7,
     5,     7,     9,     9,     9,     9,     9,     3,     5,     7,
     5,     5,     3,     3,     3,     3,     3,     3,     5,     1,
     5,     5,     5,     5,     5,     1,     3,     1,     5 };
yytabelem yychk[]={

-10000000,    -1,    -2,   257,   275,   256,    -3,   272,    -5,   272,
   258,   -11,   291,   292,   276,   288,   -14,   283,   272,   272,
    -6,   290,    -9,   262,   -13,   -10,   281,   277,   293,   278,
   279,   280,    -7,    -8,   261,   262,   264,   265,   268,   269,
   273,   274,    -4,   282,   272,   272,   272,   272,   272,   263,
   260,   259,    -7,    -7,   272,   266,   288,   272,   -12,   284,
   285,   286,   287,   288,   289,   272,   272,   258,   272,    -7,
    -7,   263,   288,   -12,   273,   274,   288,   272,   271,   272,
   271,   272,   272,   272 };
yytabelem yydef[]={

     1,    -2,     2,     0,     0,     0,     0,     9,     4,     8,
     5,    12,    10,    11,     0,     0,     0,    13,     6,     7,
    39,     0,    45,     0,    47,    38,    46,     0,     0,     0,
     0,     0,     0,    15,     0,     0,     0,     0,    27,     0,
     0,     0,     0,     0,    40,    41,    42,    43,    44,    14,
     0,     0,    18,     0,    20,     0,     0,    28,     0,    32,
    33,    34,    35,    36,    37,    30,    31,     3,    48,    16,
    17,    19,    36,     0,     0,     0,     0,    21,    29,    22,
    24,    25,    26,    23 };
typedef struct
#ifdef __cplusplus
	yytoktype
#endif
{ char *t_name; int t_val; } yytoktype;
#ifndef YYDEBUG
#	define YYDEBUG	0	/* don't allow debugging */
#endif

#if YYDEBUG

yytoktype yytoks[] =
{
	"RULE",	257,
	"ENDRULE",	258,
	"AND",	259,
	"OR",	260,
	"NOT",	261,
	"LPAREN",	262,
	"RPAREN",	263,
	"HEADER",	264,
	"BODY",	265,
	"ANY",	266,
	"ANYHEADER",	267,
	"ATTACHMENT",	268,
	"SIZE",	269,
	"HDRSIZE",	270,
	"INT",	271,
	"STRING",	272,
	"BEFORE",	273,
	"AFTER",	274,
	"OPTIONS",	275,
	"COMMA",	276,
	"RUN",	277,
	"PIPE",	278,
	"TEST",	279,
	"TEST_PIPE",	280,
	"SKIPREMAINING",	281,
	"NEXTRULE",	282,
	"IGNORECASE",	283,
	"GT",	284,
	"LT",	285,
	"GE",	286,
	"LE",	287,
	"EQUALS",	288,
	"NEQUALS",	289,
	"CRITERIA",	290,
	"ON",	291,
	"OFF",	292,
	"SAVE",	293,
	"NEG",	294,
	"-unknown-",	-1	/* ends search */
};

char * yyreds[] =
{
	"-no such reduction-",
	"filters : /* empty */",
	"filters : filters filter",
	"filter : RULE name state ignorecase criteria actions consume next ENDRULE",
	"filter : OPTIONS strings",
	"filter : error ENDRULE",
	"strings : strings COMMA STRING",
	"strings : STRING EQUALS STRING",
	"strings : STRING",
	"name : STRING",
	"state : ON",
	"state : OFF",
	"ignorecase : /* empty */",
	"ignorecase : IGNORECASE",
	"criteria : CRITERIA LPAREN expression RPAREN",
	"expression : term",
	"expression : expression OR expression",
	"expression : expression AND expression",
	"expression : NOT expression",
	"term : LPAREN expression RPAREN",
	"term : HEADER STRING",
	"term : BODY EQUALS STRING",
	"term : HEADER STRING EQUALS STRING",
	"term : HEADER ANY EQUALS STRING",
	"term : HEADER STRING relop INT",
	"term : HEADER STRING BEFORE STRING",
	"term : HEADER STRING AFTER STRING",
	"term : ATTACHMENT",
	"term : ATTACHMENT STRING",
	"term : SIZE relop INT",
	"term : BEFORE STRING",
	"term : AFTER STRING",
	"relop : GT",
	"relop : LT",
	"relop : GE",
	"relop : LE",
	"relop : EQUALS",
	"relop : NEQUALS",
	"actions : actions action",
	"actions : /* empty */",
	"action : RUN STRING",
	"action : SAVE STRING",
	"action : PIPE STRING",
	"action : TEST STRING",
	"action : TEST_PIPE STRING",
	"consume : /* empty */",
	"consume : SKIPREMAINING",
	"next : /* empty */",
	"next : NEXTRULE STRING",
};
#endif /* YYDEBUG */
# line	1 "/usr/ccs/bin/yaccpar"
/*
 * Copyright (c) 1993 by Sun Microsystems, Inc.
 */

#pragma ident	"@(#)yaccpar	6.12	93/06/07 SMI"

/*
** Skeleton parser driver for yacc output
*/

/*
** yacc user known macros and defines
*/
#define YYERROR		goto yyerrlab
#define YYACCEPT	return(0)
#define YYABORT		return(1)
#define YYBACKUP( newtoken, newvalue )\
{\
	if ( yychar >= 0 || ( yyr2[ yytmp ] >> 1 ) != 1 )\
	{\
		yyerror( "syntax error - cannot backup" );\
		goto yyerrlab;\
	}\
	yychar = newtoken;\
	yystate = *yyps;\
	yylval = newvalue;\
	goto yynewstate;\
}
#define YYRECOVERING()	(!!yyerrflag)
#define YYNEW(type)	malloc(sizeof(type) * yynewmax)
#define YYCOPY(to, from, type) \
	(type *) memcpy(to, (char *) from, yynewmax * sizeof(type))
#define YYENLARGE( from, type) \
	(type *) realloc((char *) from, yynewmax * sizeof(type))
#ifndef YYDEBUG
#	define YYDEBUG	1	/* make debugging available */
#endif

/*
** user known globals
*/
int yydebug;			/* set to 1 to get debugging */

/*
** driver internal defines
*/
#define YYFLAG		(-10000000)

/*
** global variables used by the parser
*/
YYSTYPE *yypv;			/* top of value stack */
int *yyps;			/* top of state stack */

int yystate;			/* current state */
int yytmp;			/* extra var (lasts between blocks) */

int yynerrs;			/* number of errors */
int yyerrflag;			/* error recovery flag */
int yychar;			/* current input token number */



#ifdef YYNMBCHARS
#define YYLEX()		yycvtok(yylex())
/*
** yycvtok - return a token if i is a wchar_t value that exceeds 255.
**	If i<255, i itself is the token.  If i>255 but the neither 
**	of the 30th or 31st bit is on, i is already a token.
*/
#if defined(__STDC__) || defined(__cplusplus)
int yycvtok(int i)
#else
int yycvtok(i) int i;
#endif
{
	int first = 0;
	int last = YYNMBCHARS - 1;
	int mid;
	wchar_t j;

	if(i&0x60000000){/*Must convert to a token. */
		if( yymbchars[last].character < i ){
			return i;/*Giving up*/
		}
		while ((last>=first)&&(first>=0)) {/*Binary search loop*/
			mid = (first+last)/2;
			j = yymbchars[mid].character;
			if( j==i ){/*Found*/ 
				return yymbchars[mid].tvalue;
			}else if( j<i ){
				first = mid + 1;
			}else{
				last = mid -1;
			}
		}
		/*No entry in the table.*/
		return i;/* Giving up.*/
	}else{/* i is already a token. */
		return i;
	}
}
#else/*!YYNMBCHARS*/
#define YYLEX()		yylex()
#endif/*!YYNMBCHARS*/

/*
** yyparse - return 0 if worked, 1 if syntax error not recovered from
*/
#if defined(__STDC__) || defined(__cplusplus)
int yyparse(void)
#else
int yyparse()
#endif
{
	register YYSTYPE *yypvt;	/* top of value stack for $vars */

#if defined(__cplusplus) || defined(lint)
/*
	hacks to please C++ and lint - goto's inside switch should never be
	executed; yypvt is set to 0 to avoid "used before set" warning.
*/
	static int __yaccpar_lint_hack__ = 0;
	switch (__yaccpar_lint_hack__)
	{
		case 1: goto yyerrlab;
		case 2: goto yynewstate;
	}
	yypvt = 0;
#endif

	/*
	** Initialize externals - yyparse may be called more than once
	*/
	yypv = &yyv[-1];
	yyps = &yys[-1];
	yystate = 0;
	yytmp = 0;
	yynerrs = 0;
	yyerrflag = 0;
	yychar = -1;

#if YYMAXDEPTH <= 0
	if (yymaxdepth <= 0)
	{
		if ((yymaxdepth = YYEXPAND(0)) <= 0)
		{
			yyerror("yacc initialization error");
			YYABORT;
		}
	}
#endif

	{
		register YYSTYPE *yy_pv;	/* top of value stack */
		register int *yy_ps;		/* top of state stack */
		register int yy_state;		/* current state */
		register int  yy_n;		/* internal state number info */
	goto yystack;	/* moved from 6 lines above to here to please C++ */

		/*
		** get globals into registers.
		** branch to here only if YYBACKUP was called.
		*/
	yynewstate:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;
		goto yy_newstate;

		/*
		** get globals into registers.
		** either we just started, or we just finished a reduction
		*/
	yystack:
		yy_pv = yypv;
		yy_ps = yyps;
		yy_state = yystate;

		/*
		** top of for (;;) loop while no reductions done
		*/
	yy_stack:
		/*
		** put a state and value onto the stacks
		*/
#if YYDEBUG
		/*
		** if debugging, look up token value in list of value vs.
		** name pairs.  0 and negative (-1) are special values.
		** Note: linear search is used since time is not a real
		** consideration while debugging.
		*/
		if ( yydebug )
		{
			register int yy_i;

			printf( "State %d, token ", yy_state );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ++yy_ps >= &yys[ yymaxdepth ] )	/* room on stack? */
		{
			/*
			** reallocate and recover.  Note that pointers
			** have to be reset, or bad things will happen
			*/
			int yyps_index = (yy_ps - yys);
			int yypv_index = (yy_pv - yyv);
			int yypvt_index = (yypvt - yyv);
			int yynewmax;
#ifdef YYEXPAND
			yynewmax = YYEXPAND(yymaxdepth);
#else
			yynewmax = 2 * yymaxdepth;	/* double table size */
			if (yymaxdepth == YYMAXDEPTH)	/* first time growth */
			{
				char *newyys = (char *)YYNEW(int);
				char *newyyv = (char *)YYNEW(YYSTYPE);
				if (newyys != 0 && newyyv != 0)
				{
					yys = YYCOPY(newyys, yys, int);
					yyv = YYCOPY(newyyv, yyv, YYSTYPE);
				}
				else
					yynewmax = 0;	/* failed */
			}
			else				/* not first time */
			{
				yys = YYENLARGE(yys, int);
				yyv = YYENLARGE(yyv, YYSTYPE);
				if (yys == 0 || yyv == 0)
					yynewmax = 0;	/* failed */
			}
#endif
			if (yynewmax <= yymaxdepth)	/* tables not expanded */
			{
				yyerror( "yacc stack overflow" );
				YYABORT;
			}
			yymaxdepth = yynewmax;

			yy_ps = yys + yyps_index;
			yy_pv = yyv + yypv_index;
			yypvt = yyv + yypvt_index;
		}
		*yy_ps = yy_state;
		*++yy_pv = yyval;

		/*
		** we have a new state - find out what to do
		*/
	yy_newstate:
		if ( ( yy_n = yypact[ yy_state ] ) <= YYFLAG )
			goto yydefault;		/* simple state */
#if YYDEBUG
		/*
		** if debugging, need to mark whether new token grabbed
		*/
		yytmp = yychar < 0;
#endif
		if ( ( yychar < 0 ) && ( ( yychar = YYLEX() ) < 0 ) )
			yychar = 0;		/* reached EOF */
#if YYDEBUG
		if ( yydebug && yytmp )
		{
			register int yy_i;

			printf( "Received token " );
			if ( yychar == 0 )
				printf( "end-of-file\n" );
			else if ( yychar < 0 )
				printf( "-none-\n" );
			else
			{
				for ( yy_i = 0; yytoks[yy_i].t_val >= 0;
					yy_i++ )
				{
					if ( yytoks[yy_i].t_val == yychar )
						break;
				}
				printf( "%s\n", yytoks[yy_i].t_name );
			}
		}
#endif /* YYDEBUG */
		if ( ( ( yy_n += yychar ) < 0 ) || ( yy_n >= YYLAST ) )
			goto yydefault;
		if ( yychk[ yy_n = yyact[ yy_n ] ] == yychar )	/*valid shift*/
		{
			yychar = -1;
			yyval = yylval;
			yy_state = yy_n;
			if ( yyerrflag > 0 )
				yyerrflag--;
			goto yy_stack;
		}

	yydefault:
		if ( ( yy_n = yydef[ yy_state ] ) == -2 )
		{
#if YYDEBUG
			yytmp = yychar < 0;
#endif
			if ( ( yychar < 0 ) && ( ( yychar = YYLEX() ) < 0 ) )
				yychar = 0;		/* reached EOF */
#if YYDEBUG
			if ( yydebug && yytmp )
			{
				register int yy_i;

				printf( "Received token " );
				if ( yychar == 0 )
					printf( "end-of-file\n" );
				else if ( yychar < 0 )
					printf( "-none-\n" );
				else
				{
					for ( yy_i = 0;
						yytoks[yy_i].t_val >= 0;
						yy_i++ )
					{
						if ( yytoks[yy_i].t_val
							== yychar )
						{
							break;
						}
					}
					printf( "%s\n", yytoks[yy_i].t_name );
				}
			}
#endif /* YYDEBUG */
			/*
			** look through exception table
			*/
			{
				register int *yyxi = yyexca;

				while ( ( *yyxi != -1 ) ||
					( yyxi[1] != yy_state ) )
				{
					yyxi += 2;
				}
				while ( ( *(yyxi += 2) >= 0 ) &&
					( *yyxi != yychar ) )
					;
				if ( ( yy_n = yyxi[1] ) < 0 )
					YYACCEPT;
			}
		}

		/*
		** check for syntax error
		*/
		if ( yy_n == 0 )	/* have an error */
		{
			/* no worry about speed here! */
			switch ( yyerrflag )
			{
			case 0:		/* new error */
				yyerror( "syntax error" );
				goto skip_init;
			yyerrlab:
				/*
				** get globals into registers.
				** we have a user generated syntax type error
				*/
				yy_pv = yypv;
				yy_ps = yyps;
				yy_state = yystate;
			skip_init:
				yynerrs++;
				/* FALLTHRU */
			case 1:
			case 2:		/* incompletely recovered error */
					/* try again... */
				yyerrflag = 3;
				/*
				** find state where "error" is a legal
				** shift action
				*/
				while ( yy_ps >= yys )
				{
					yy_n = yypact[ *yy_ps ] + YYERRCODE;
					if ( yy_n >= 0 && yy_n < YYLAST &&
						yychk[yyact[yy_n]] == YYERRCODE)					{
						/*
						** simulate shift of "error"
						*/
						yy_state = yyact[ yy_n ];
						goto yy_stack;
					}
					/*
					** current state has no shift on
					** "error", pop stack
					*/
#if YYDEBUG
#	define _POP_ "Error recovery pops state %d, uncovers state %d\n"
					if ( yydebug )
						printf( _POP_, *yy_ps,
							yy_ps[-1] );
#	undef _POP_
#endif
					yy_ps--;
					yy_pv--;
				}
				/*
				** there is no state on stack with "error" as
				** a valid shift.  give up.
				*/
				YYABORT;
			case 3:		/* no shift yet; eat a token */
#if YYDEBUG
				/*
				** if debugging, look up token in list of
				** pairs.  0 and negative shouldn't occur,
				** but since timing doesn't matter when
				** debugging, it doesn't hurt to leave the
				** tests here.
				*/
				if ( yydebug )
				{
					register int yy_i;

					printf( "Error recovery discards " );
					if ( yychar == 0 )
						printf( "token end-of-file\n" );
					else if ( yychar < 0 )
						printf( "token -none-\n" );
					else
					{
						for ( yy_i = 0;
							yytoks[yy_i].t_val >= 0;
							yy_i++ )
						{
							if ( yytoks[yy_i].t_val
								== yychar )
							{
								break;
							}
						}
						printf( "token %s\n",
							yytoks[yy_i].t_name );
					}
				}
#endif /* YYDEBUG */
				if ( yychar == 0 )	/* reached EOF. quit */
					YYABORT;
				yychar = -1;
				goto yy_newstate;
			}
		}/* end if ( yy_n == 0 ) */
		/*
		** reduction by production yy_n
		** put stack tops, etc. so things right after switch
		*/
#if YYDEBUG
		/*
		** if debugging, print the string that is the user's
		** specification of the reduction which is just about
		** to be done.
		*/
		if ( yydebug )
			printf( "Reduce by (%d) \"%s\"\n",
				yy_n, yyreds[ yy_n ] );
#endif
		yytmp = yy_n;			/* value to switch over */
		yypvt = yy_pv;			/* $vars top of value stack */
		/*
		** Look in goto table for next state
		** Sorry about using yy_state here as temporary
		** register variable, but why not, if it works...
		** If yyr2[ yy_n ] doesn't have the low order bit
		** set, then there is no action to be done for
		** this reduction.  So, no saving & unsaving of
		** registers done.  The only difference between the
		** code just after the if and the body of the if is
		** the goto yy_stack in the body.  This way the test
		** can be made before the choice of what to do is needed.
		*/
		{
			/* length of production doubled with extra bit */
			register int yy_len = yyr2[ yy_n ];

			if ( !( yy_len & 01 ) )
			{
				yy_len >>= 1;
				yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
				yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
					*( yy_ps -= yy_len ) + 1;
				if ( yy_state >= YYLAST ||
					yychk[ yy_state =
					yyact[ yy_state ] ] != -yy_n )
				{
					yy_state = yyact[ yypgo[ yy_n ] ];
				}
				goto yy_stack;
			}
			yy_len >>= 1;
			yyval = ( yy_pv -= yy_len )[1];	/* $$ = $1 */
			yy_state = yypgo[ yy_n = yyr1[ yy_n ] ] +
				*( yy_ps -= yy_len ) + 1;
			if ( yy_state >= YYLAST ||
				yychk[ yy_state = yyact[ yy_state ] ] != -yy_n )
			{
				yy_state = yyact[ yypgo[ yy_n ] ];
			}
		}
					/* save until reenter driver code */
		yystate = yy_state;
		yyps = yy_ps;
		yypv = yy_pv;
	}
	/*
	** code supplied by user is placed in this switch
	*/
	switch( yytmp )
	{
		
case 1:
# line 69 "parse.y"
{ yyval.fval = NULL; } break;
case 2:
# line 71 "parse.y"
{	
			if (yypvt[-0].fval) {
				yypvt[-0].fval->f_prev = yypvt[-1].fval;
				yyval.fval = yypvt[-0].fval;
				basefilter = yypvt[-0].fval;
				printf("%s\n", yypvt[-0].fval);
			}
		} break;
case 3:
# line 82 "parse.y"
{
			struct filter *f = alloc_filter();
			f->f_name = yypvt[-7].sval;
			f->f_state = yypvt[-6].ival;
			f->f_ignorecase = yypvt[-5].ival;
			f->f_expr = yypvt[-4].eval;
			f->f_act = yypvt[-3].aval;
			f->f_consume = yypvt[-2].ival;
			f->f_chainedfilter = yypvt[-1].sval;
			yyval.fval = f;
		} break;
case 4:
# line 94 "parse.y"
{
			struct filter *f = alloc_filter();

			f->f_options = yypvt[-0].oval;
			yyval.fval = f;
		} break;
case 5:
# line 100 "parse.y"
{ yyerrok; } break;
case 6:
# line 106 "parse.y"
{	
			struct options *o = alloc_options();
			struct options *o2;

			/* warning: n^2 algorithm to get last element */
			for(o2 = yypvt[-2].oval; o2->o_next; o2 = o2->o_next);
			o2->o_next = o;

			yyval.oval = yypvt[-2].oval ;
			o->o_string = yypvt[-0].sval;
		} break;
case 7:
# line 118 "parse.y"
{
			struct options *o = alloc_options();

			o->o_string = yypvt[-2].sval;
			o->o_value = yypvt[-0].sval;
			yyval.oval = o;
		} break;
case 8:
# line 126 "parse.y"
{	
		    
			struct options *o = alloc_options();
			o->o_string = yypvt[-0].sval;
			yyval.oval = o;
		} break;
case 9:
# line 134 "parse.y"
{ yyval.sval = yypvt[-0].sval; } break;
case 10:
# line 137 "parse.y"
{ yyval.ival = 1; } break;
case 11:
# line 138 "parse.y"
{ yyval.ival = 0; } break;
case 12:
# line 142 "parse.y"
{ yyval.ival = NULL; } break;
case 13:
# line 143 "parse.y"
{ yyval.ival = 1; } break;
case 14:
# line 146 "parse.y"
{ yyval.eval = yypvt[-1].eval; } break;
case 15:
# line 149 "parse.y"
{ yyval.eval = yypvt[-0].eval; } break;
case 16:
# line 150 "parse.y"
{ yyval.eval = buildtree(OR, yypvt[-2].eval, yypvt[-0].eval); } break;
case 17:
# line 151 "parse.y"
{ yyval.eval = buildtree(AND, yypvt[-2].eval, yypvt[-0].eval); } break;
case 18:
# line 152 "parse.y"
{ yyval.eval = buildtree(NOT, yypvt[-0].eval, NULL); } break;
case 19:
# line 155 "parse.y"
{ yyval.eval = yypvt[-1].eval; } break;
case 20:
# line 157 "parse.y"
{ yyval.eval = buildleaf(HEADER, yypvt[-0].sval, NULL, NULL); } break;
case 21:
# line 159 "parse.y"
{ yyval.eval = buildleaf(BODY, yypvt[-0].sval, NULL, NULL); } break;
case 22:
# line 161 "parse.y"
{ yyval.eval = buildleaf(HEADER, yypvt[-2].sval, yypvt[-0].sval, NULL); } break;
case 23:
# line 163 "parse.y"
{ yyval.eval = buildleaf(ANYHEADER, yypvt[-0].sval, NULL, NULL); } break;
case 24:
# line 165 "parse.y"
{ yyval.eval = buildleaf(HDRSIZE, yypvt[-2].sval, (void*)yypvt[-1].ival, (void*)yypvt[-0].ival); } break;
case 25:
# line 167 "parse.y"
{ yyval.eval = buildleaf(BEFORE, yypvt[-2].sval, yypvt[-0].sval, NULL); } break;
case 26:
# line 169 "parse.y"
{ yyval.eval = buildleaf(AFTER, yypvt[-2].sval, yypvt[-0].sval, NULL); } break;
case 27:
# line 171 "parse.y"
{ yyval.eval = buildleaf(ATTACHMENT, NULL, NULL, NULL); } break;
case 28:
# line 173 "parse.y"
{ yyval.eval = buildleaf(ATTACHMENT, yypvt[-0].sval, NULL, NULL); } break;
case 29:
# line 175 "parse.y"
{ yyval.eval = buildleaf(SIZE, (void*)yypvt[-1].ival, (void*)yypvt[-0].ival, NULL); } break;
case 30:
# line 177 "parse.y"
{ yyval.eval = buildleaf(BEFORE, yypvt[-0].sval, NULL, NULL); } break;
case 31:
# line 179 "parse.y"
{ yyval.eval = buildleaf(AFTER, yypvt[-0].sval, NULL, NULL); } break;
case 32:
# line 182 "parse.y"
{ yyval.ival = GT; } break;
case 33:
# line 183 "parse.y"
{ yyval.ival = LT; } break;
case 34:
# line 184 "parse.y"
{ yyval.ival = GE; } break;
case 35:
# line 185 "parse.y"
{ yyval.ival = LE; } break;
case 36:
# line 186 "parse.y"
{ yyval.ival = EQUALS; } break;
case 37:
# line 187 "parse.y"
{ yyval.ival = NEQUALS; } break;
case 38:
# line 191 "parse.y"
{
			if (yypvt[-1].aval) {
				struct act *a;

				/* warning: n^2 algorithm */
				for (a = yypvt[-1].aval; a->a_next; a = a->a_next);
				a->a_next = yypvt[-0].aval;

				yyval.aval = yypvt[-1].aval;
			} else {
				yyval.aval = yypvt[-0].aval;
			}
		} break;
case 39:
# line 204 "parse.y"
{ yyval.aval = NULL; } break;
case 40:
# line 207 "parse.y"
{ yyval.aval = buildact(RUN, yypvt[-0].sval); } break;
case 41:
# line 208 "parse.y"
{ yyval.aval = buildact(SAVE, yypvt[-0].sval); } break;
case 42:
# line 209 "parse.y"
{ yyval.aval = buildact(PIPE, yypvt[-0].sval); } break;
case 43:
# line 210 "parse.y"
{ yyval.aval = buildact(TEST, yypvt[-0].sval); } break;
case 44:
# line 211 "parse.y"
{ yyval.aval = buildact(TEST_PIPE, yypvt[-0].sval); } break;
case 45:
# line 215 "parse.y"
{ yyval.ival = NULL; } break;
case 46:
# line 216 "parse.y"
{ yyval.ival = 1; } break;
case 47:
# line 220 "parse.y"
{ yyval.sval = NULL; } break;
case 48:
# line 221 "parse.y"
{ yyval.sval = yypvt[-0].sval; } break;
# line	532 "/usr/ccs/bin/yaccpar"
	}
	goto yystack;		/* reset registers in driver code */
}

