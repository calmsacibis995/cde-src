#ifndef lint
#ifdef  VERBOSE_REV_INFO
static char rcs_id[] = "$XConsortium: TermPrimParser.c /main/cde1_maint/1 1995/07/15 01:29:39 drk $";
#endif  /* VERBOSE_REV_INFO */
#endif  /* lint */

/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#include "TermPrimDebug.h"
#include "TermPrimP.h"
#include "TermPrimParserP.h"
#include "TermPrimBuffer.h"
#include <values.h>          

#ifdef	BBA
#pragma	BBA_IGNORE
#endif	/*BBA*/
void
ParseTrap()
{
    static int count = 0;

    (void) count++;
}

/*
** Parse the character, tell the calling routine if we are not
** in the start state.
*/
Boolean
_DtTermPrimParse
(
    Widget          w,
    unsigned char  *parseChar,
    int		    parseCharLen
)
{
    ParserContext   context = GetParserContext(w);
    StateEntry      thisEntry;
    StateEntry      thisPreParseEntry;

#ifdef    NOCODE
    /*
    ** This decision should be made somewhere else.
    */
    if (tp->t_modes.disp_func == 1)
    {
        in_disp_func();
        return(False);
    }
#endif /* NOCODE */

    if (parseCharLen == 1)
      *context->inputChar = *parseChar;
    else
      memmove(context->inputChar, parseChar, parseCharLen);

    context->inputCharLen = parseCharLen;

    if (isDebugFSet('p', 1)) {
#ifdef	BBA
#pragma	BBA_IGNORE
#endif	/*BBA*/
	static unsigned char debugChar;
	static Boolean first = True;
	char *c;

	if (parseCharLen == 1) {
	    if (first) {
		if (!(c = getenv("dttermDebugParseChar"))) {
		    c = "0x03";
		}
		debugChar = strtol(c, (char **) 0, 0);
		first = False;
	    }

	    if (*parseChar == debugChar) {
		ParseTrap();
		return(False);
	    }
	}
    }
	
    /*
    ** Determine which state entry to use.
    */
    thisPreParseEntry = context->stateTable->statePreParseEntry;
    thisEntry = context->stateTable->stateEntry;

    /* first run through the preParse entry... */
    if (thisPreParseEntry && (parseCharLen == 1)) {
	while ((*parseChar < thisPreParseEntry->lower) ||
		(*parseChar > thisPreParseEntry->upper)) {
	    thisPreParseEntry++;
	}
	/* if we hit the end, ignore it... */
	if ((0x00 == thisPreParseEntry->lower) &&
		(0xff == thisPreParseEntry->upper)) {
	    thisPreParseEntry = (StateEntry) 0;
	}
    }

    /* if we hit a valid preParseEntry, then let's execute it and
     * return...
     */
    if (thisPreParseEntry) {
	/*
	** Now change states.  If the next state is NULL, stay in the
	** current state.  This is for parse entries that do not break us
	** out of the current parse thread.  If we need to bail out of the
	** current parse thread, then we have a new state specified and
	** will switch to it.  We do this before we execute the function
	** incase the function needs to change the state as well...
	*/
	if (thisPreParseEntry->nextState) {
	    context->stateTable = thisPreParseEntry->nextState;
	}

	/*
	** Execute the action associated with the entry.
	*/
	if (thisPreParseEntry->action) {
	    (*thisPreParseEntry->action)(w);
	}

	return(!context->stateTable->startState);
    }

    /* HACK ALERT!!!!
     *
     * We need two different search algorithms - the first to deal
     * with single byte characters, the second to deal with multi-byte
     * characters.  For now, we will match multi-byte character with
     * the parse entry that covers 0..255.  If we find that this will
     * not work for everything, we may need to rethink this.
     */
    if (parseCharLen == 1) {
	while ((*parseChar < thisEntry->lower) ||
		(*parseChar > thisEntry->upper))
	{
	    thisEntry++;
	}
    } else {
	while ((0x00 != thisEntry->lower) ||
		(0xff != thisEntry->upper))
	{
	    thisEntry++;
	}
    }
	
    /*
    ** Now change states.  We do this before we execute the function incase
    ** the function needs to change the state as well...
    */
    context->stateTable = thisEntry->nextState;

    /*
    ** Execute the action associated with the entry.
    */
    if (thisEntry->action)
      {
        Boolean reverseVideo;
        XtVaGetValues(w, DtNreverseVideo, &reverseVideo, NULL);

	(*thisEntry->action)(w);

        XtVaSetValues(w, DtNreverseVideo, reverseVideo, NULL);
      }

    return(!context->stateTable->startState);
}

/*
**
**  _DtTermPrimParserClrStrParm()
**
**  clrStrParm clears the string parameters.
**
*/
void
_DtTermPrimParserClrStrParm
(
    Widget w
)
{
    ParserContext   context = GetParserContext(w);

    context->stringParms[0].length = 0;
    (void) memset(context->stringParms[0].str, '\x00', STR_SIZE + 1);

    context->stringParms[1].length = 0;
    (void) memset(context->stringParms[1].str, '\x00', STR_SIZE + 1);
}

/*
**
**  _DtTermPrimParserNextState()
**
**  nextState is a do nothing routine.  It is called when the only
**  action that needs to be preformed is changing states in the
**  state machine.
**
*/
void
_DtTermPrimParserNextState
(
    Widget w
)
{
}

/*
**
**  _DtTermPrimParserClearParm()
**
**  _DtTermPrimParserClearParm clears all common parameters.
**
*/
void
_DtTermPrimParserClearParm
(
    Widget w
)
{
    ParserContext  context = GetParserContext(w);
    int             i;

    for (i = 0; i < NUM_PARMS; i++) {
        context->parms[i] = 0;
    }
    context->workingNum = 0;
    context->workingNumIsDefault = True;
    context->sign = TermPARSER_SIGNnone;
}

/*
**
**  enterNum()
**
**  enterNum enters a numerical parameter.
**
*/
void
_DtTermPrimParserEnterNum
(
    Widget w
)
{
    ParserContext   context = GetParserContext(w);

    if ( context->workingNum < MAXINT>>4 ) 
        context->workingNum = context->workingNum * 10 +
	    (*context->inputChar - '0');
    context->workingNumIsDefault = False;
}

void
_DtTermPrimParserSaveSign
(
    Widget w
)
{
    ParserContext   context = GetParserContext(w);

    SetSign(context, (*GetInputChar(context) == '-') ? TermPARSER_SIGNnegative :
	    TermPARSER_SIGNpositive);
}

/*
**
**  _DtTermPrimParserNumParmPush()
**
**  _DtTermPrimParserNumParmPush() takes the number in workingNum and 
**   stores it in parm[parmNum].
**
*/
void
_DtTermPrimParserNumParmPush
(
    Widget w,
    int    parmNum
)
{
    ParserContext   context = GetParserContext(w);

    if ( parmNum < NUM_PARMS ) {
      context->parms[parmNum] = context->workingNum;
      context->workingNum     = 0;
      context->workingNumIsDefault = False;
    }
}

/*
** Initialize the context of the parser.
*/
void
_DtTermPrimParserInitContext
(
    Widget w
)
{
    DtTermPrimitiveWidget tw = (DtTermPrimitiveWidget) w;
    DtTermPrimData tpd = tw->term.tpd;

    tpd->context = (ParserContext) XtMalloc(sizeof(ParserContextRec));
    (void) memset(tpd->context, '\0', sizeof(ParserContextRec));

    tpd->context->stateTable =
	    *(((DtTermPrimitiveClassRec *) (tw->core.widget_class))->
	    term_primitive_class.parser_start_state);
    tpd->context->stateName  = START;
}

#ifdef TEST

#include <ctype.h>

#define testFunction(f) \
void f ( Widget w ) {printf(#f " called.\n");}

int cursor_col, cursor_row;
int logical_col, logical_row;

testFunction(_termCR)
testFunction(_termLF)
testFunction(_termTab)
testFunction(_termBackspace)
testFunction(RecordDC1Rcvd)
testFunction(_termWriteChar)
testFunction(_DtTermPrimBell)
testFunction(_termShiftOut)
testFunction(_termShiftIn)
testFunction(set_tab)
testFunction(clear_tab)
testFunction(clr_all_tab)
testFunction(set_left_marg)
testFunction(set_right_marg)
testFunction(clear_marg)
testFunction(curs_up)
testFunction(curs_down)
testFunction(curs_right)
testFunction(curs_left)
testFunction(home_down)
testFunction(home_up)
testFunction(clr_display)
testFunction(clear_line)
testFunction(insert_line)
testFunction(delete_line)
testFunction(delete_char)
testFunction(insert_char)
testFunction(off_insert_char)
testFunction(roll_up)
testFunction(roll_down)
testFunction(next_page)
testFunction(prev_page)
testFunction(format_mode_on)
testFunction(format_mode_off)
testFunction(display_func)
testFunction(term_status)
testFunction(rel_curs_pos)
testFunction(abs_curs_pos)
testFunction(enable_key)
testFunction(disable_key)
testFunction(enter_line)
testFunction(back_tab)
testFunction(user_key_menu_on)
testFunction(user_key_menu_off)
testFunction(second_status)
testFunction(mlock_on)
testFunction(mlock_off)
testFunction(start_unprotect)
testFunction(stop_field)

void
_parserPrintContext
(
    ParserContext   context
)
{
    printf("    inputChar  : ");
    if (isprint(*context->inputChar))
    {
        printf("'%c'\n", *context->inputChar);
    }
    else
    {
        printf("\x2X\n", *context->inputChar);
    }
    for (i = 0; i < 9; i++)
    {
        printf("    parm[%d]      : %d\n", i, context->parm[i]);
    }
    printf("    workingNum : %d\n", context->workingNum);
}

void
parseString
(
    Widget          w;
    unsigned char  *string
)
{
    ParserContext   context = GetParserContext(w);
    int i;

    _parserPrintContext(context);
    for (i = 0; i < strlen((char *)string); i++)
    {
        _DtTermPrimParse(context, string[i]);
        _parserPrintContext(context);
    }
}

/* the following is to allow for a single main function in the code... */
#define	parserMain	main
parserMain()
{
    parserContext   context;

    _parserInitContext(&context);

    cursor_col = 0;
    cursor_row = 0;

    logical_col = 5;
    logical_row = 5;

    printf("\nparsing <esc>&a5r5C\n");
    parseString(&context, (unsigned char *)"\033&a5r5C");

    printf("\nparsing <esc>a5r35C\n");
    parseString(&context, (unsigned char *)"\033&a5r35C");

    printf("\nparsing <esc>&a5r3r4c5C\n");
    parseString(&context, (unsigned char *)"\033&a5r3r4c5C");

    printf("\nparsing <esc>&arC\n");
    parseString(&context, (unsigned char *)"\033&arC");

    printf("\nparsing <esc>&a5C\n");
    parseString(&context, (unsigned char *)"\033&a5C");

    printf("\nparsing <esc>&a+5C\n");
    parseString(&context, (unsigned char *)"\033&a+5C");
}
#endif /* TEST */
