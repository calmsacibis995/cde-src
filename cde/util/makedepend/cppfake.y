/*
 * cppfake.y
 *
 *     this Yacc definition plus yylex() routine
 * evaluates expressions supplied to the 
 * "#if" preprocessor symbol.  It handles most
 * of the basic C expressions.
 *
 * Known design deficiencies:
 *
 *   1) does not handle macro replacement of 
 *      stuff within a #if expression, such as:
 *
 *      #define foo bar
 *      ...
 *      #if foo == bar
 *
 *
 * This code can be tested via the 
 * script "test.ksh"
 */

%term IF_DEFINED NUMERIC_VALUE END_LINE
%term SHIFT_LEFT SHIFT_RIGHT
/* 
 * the terminal symbol UNKNOWN_CHAR is returned by 
 * yylex() when it hits a character that it does
 * not know anything about.  This will force a 
 * syntax error in the yacc emitted parser
 */
%term UNKNOWN_CHAR

/*
 * as per K&R, mostly
 */
%right '?' ':'
%left LOGICAL_OR
%left LOGICAL_AND
%left '|'
%left '^'
%left '&'
%binary EQUALITY INEQUALITY
%binary '<' '>' LESS_OR_EQUAL GREATER_OR_EQUAL
%left SHIFT_LEFT SHIFT_RIGHT
%left '+' '-'
%left '*' '/' '%'
%right '!' '~'
%%

STATEMENT:
	exp END_LINE
		={ return( $1 ); }

exp:	  '(' exp ')'
		={$$ = $2;}
	| '!' exp
		= { $$ = !$2; }
	| '~' exp
		= { $$ = ~$2; }
	| exp '*' exp
		= { $$ = $1 * $3; }
	| exp '/' exp
		= { $$ = $1 / $3; }
	| exp '%' exp
		= { $$ = $1 % $3; }
	| exp '+' exp
		= { $$ = $1 + $3; }
	| exp '-' exp
		= { $$ = $1 - $3; }
	| exp SHIFT_LEFT exp
		= { $$ = $1 << $3; }
	| exp SHIFT_RIGHT exp
		= { $$ = $1 >> $3; }
	| exp '<' exp
		= { $$ = $1 < $3; }
	| exp '>' exp
		= { $$ = $1 > $3; }
	| exp LESS_OR_EQUAL exp
		= { $$ = $1 <= $3; }
	| exp GREATER_OR_EQUAL exp
		= { $$ = $1 >= $3; }
	| exp EQUALITY exp
		= { $$ = $1 == $3; }
	| exp INEQUALITY exp
		= { $$ = $1 != $3; }
	| exp '&' exp
		= { $$ = $1 & $3; }
	| exp '^' exp
		= { $$ = $1 ^ $3; }
	| exp '|' exp
		= { $$ = $1 | $3; }
	| exp LOGICAL_OR exp
		= { $$ = $1 || $3; }
	| exp LOGICAL_AND exp
		= { $$ = $1 && $3; }
	| exp '?' exp ':' exp
		= { $$ = $1 ? $3 : $5; }
	| IF_DEFINED '(' NUMERIC_VALUE ')'
		= { $$ = $3; }
	| IF_DEFINED NUMERIC_VALUE
		= { $$ = $2; }
	| NUMERIC_VALUE
		= { $$ = $1; }
%%

/* 
 * the following are used during the lexical analysis
 * of the input stream by yylex() below.
 */

#define IsWhiteSpace(x) (   (x==' ') || (x=='\t')    \
			)
#define IsNumeric(x)    (   (x>='0') && (x<='9')     \
			)
#define IsIdentifier(x) (   ((x>='@') && (x<='Z'))   \
                         || ((x>='a') && (x<='z'))   \
			 || ((x>=128) && (x<=255))   \
			 || ( x== '_')               \
			 || ( x== '$')               \
                        )
#define IsMemberChar(x) (   ((x>='0') && (x<='9'))   \
			 || ((x>='@') && (x<='Z'))   \
                         || ((x>='a') && (x<='z'))   \
			 || ((x>=128) && (x<=255))   \
			 || ( x== '_')               \
			 || ( x== '$')               \
                        )
#include "def.h"
#include <stdio.h>

struct DoubleCharSymbol {
    char *sym;
    int  Terminal;
} DoubleSym[] =  {
	"||", LOGICAL_OR,
	"&&", LOGICAL_AND,
	"==", EQUALITY,
	"!=", INEQUALITY,
	"<=", LESS_OR_EQUAL,
	">=", GREATER_OR_EQUAL,
	"<<", SHIFT_LEFT,
	">>", SHIFT_RIGHT,
	NULL, 0,
};
char FirstInDouble[] = "|&=!<>";
char SingleCharSymbol[] = "()<>!~&|^+-*/%?:";

yylex()
{
    boolean ConvertNumeric();
    extern char *inp;
    char SaveChar, *pEndOfToken;
    struct symtab *pSymtab, *lookup();
    long value, strtol();
    char *pos, *strchr();

    for ( ;; )
    {
	while ( IsWhiteSpace( *inp ) )
	    inp++;

	/*
	 * handle escaped endline condition by 
	 * blindly skipping to the next line.
	 * update the sort-of global line numbering.
	 * zap the ending newline to a null.
	 */
        if ( ( inp[0] == '\\' ) && ( inp[1] == NULL ) )
	{
	    char *NextNewLine;
	    /*
	     * from cppsetup.c
	     */
	    extern struct filepointer * currentfile;
	    currentfile->f_line++;

	    inp += 2;
	    for ( NextNewLine = inp; *NextNewLine != '\n'; NextNewLine++ )
	       ;
	    *NextNewLine++ = '\0';
	    currentfile->f_p = (char *) NextNewLine;
	    continue;
	}
	if ( *inp == NULL )
	{
	    return( END_LINE );
	}
	/*
	 * if we get a match on a double-symbol
	 * then return the corresponding Terminal
	 * element to our friendly neighborhood parser
	 */
	if ( strchr( FirstInDouble, *inp ) != NULL )
	{
	    struct DoubleCharSymbol *pD = DoubleSym;
	    for ( ; pD->sym; pD++ )
	    {
		if (    ( pD->sym[0] == inp[0] )
		     && ( pD->sym[1] == inp[1] ) )
		{
		    inp += 2;
		    return( pD->Terminal );
		}
	    }
	}
	if ( strchr( SingleCharSymbol, *inp ) != NULL )
	{
	    return( *inp++ );
	}
        if ( ConvertNumeric( &value, inp, &inp ))
	{
            yylval = value;
	    return( NUMERIC_VALUE );
	}
	if ( IsIdentifier( *inp ))
	{
	    pEndOfToken = inp;
	    for ( ; IsMemberChar( *pEndOfToken ); ++pEndOfToken )
		;
	    SaveChar = *pEndOfToken;
	    *pEndOfToken = '\0';

	    if ( strcmp( inp, "defined" ) == 0 )
	    {
		inp = pEndOfToken;
		*inp = SaveChar;
		return( IF_DEFINED );
	    }
	    else
	    {
		/*
		 * we have some identifier - see if it is 
		 * in the tables
		 */
		char dummy;
		char *pDummy = &dummy;

		pSymtab = lookup( inp );

		/*
		 */
		yylval = 0;
		if ( pSymtab->s_value ) 
		{
		    if ( ConvertNumeric( &value, pSymtab->s_value, &pDummy ))
			yylval =  value;
		    else
			yylval = 1;
		}

		inp = pEndOfToken;
		*inp = SaveChar;
		return( NUMERIC_VALUE );
	    }
	}
	/*
	 * hmm, don't know what to do with this
	 * character.  Let's force a syntax error
	 * rather than going into an infinite loop 
	 * here.
	 */
	fprintf( stderr, "makedepend: Forcing a syntax error in lex.\n" );
	fflush(  stderr );
	return( UNKNOWN_CHAR );
    }
}

boolean
ConvertNumeric( RetValue, pCharIn, ppEndChar )
long *RetValue;
char *pCharIn, **ppEndChar;
{
	if ( *pCharIn == '0' )
	{
	    ++pCharIn;
	    if ( ( *pCharIn == 'x' ) || ( *pCharIn == 'X' ) )
	    {
		++pCharIn;
		*RetValue = strtol( pCharIn, ppEndChar, 16 );
		return( TRUE );
	    }
	    else
	    {
		*RetValue = strtol( pCharIn, ppEndChar, 8 );
		return( TRUE );
	    }
	}
	if ( IsNumeric( *pCharIn ))
	{
	    *RetValue = strtol( pCharIn, ppEndChar, 10 );
	    return( TRUE );
	}
	return( FALSE );
}
