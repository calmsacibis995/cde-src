/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.3
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$RCSfile: Mrmerror.c,v $ $Revision: 1.5 $ $Date: 94/04/19 07:48:07 $"
#endif
#endif

/*
*  (c) Copyright 1989, 1990, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */

/*
 *++
 *  FACILITY:
 *
 *      UIL Resource Manager (URM):
 *
 *  ABSTRACT:
 *
 *	All error signalling and handling routines are found here.
 *
 *--
 */


/*
 *
 *  INCLUDE FILES
 *
 */

/* Define _MrmConst before including MrmDecl.h, so that the
 * declarations will be in agreement with the definitions.
 */
#ifndef _MrmConst
#define _MrmConst XmConst
#endif

#include <stdio.h>
#include <Mrm/MrmAppl.h>
#include <Mrm/Mrm.h>


/*
 *
 *  TABLE OF CONTENTS
 *
 */


/*
 *
 *  DEFINE and MACRO DEFINITIONS
 *
 */


/*
 *
 *  EXTERNAL VARIABLE DECLARATIONS
 *
 */


/*
 *
 *  GLOBAL VARIABLE DECLARATIONS
 *
 */
externaldef(urm__err_out)		MrmCode	urm__err_out = URMErrOutStdout ;
externaldef(urm__latest_error_code)	MrmCode	urm__latest_error_code = 0 ;
externaldef(urm__latest_error_msg)	String	urm__latest_error_msg = NULL ;

/* Messages in the code */
externaldef(mrmmsg) _MrmConst char _MrmMsg_0000[] =
	 "Vector allocation failed";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0001[] =
	 "Buffer allocation failed";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0002[] =
	 "Invalid buffer";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0003[] = 
	"Get block failed";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0004[] =
	 "Put block failed";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0005[] =
	 "Invalid record header";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0006[] =
	 "Invalid context";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0007[] =
	 "Invalid data entry";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0008[] =
	 "Invalid segment entry";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0009[] =
	 "Unknown data entry type";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0010[] =
	 "Unexpected record type";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0011[] =
	 "File version %s is greater than Mrm version %s";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0012[] =
	 "Not the header record";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0013[] =
	 "Resource index out of range";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0014[] =
	 "NULL data pointer";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0015[] =
	 "Illegal overflow record";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0016[] =
	 "GT record mismatch";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0017[] =
	 "LT record mismatch";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0018[] =
	 "Unexpected parent record type";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0019[] =
	 "Record beyond EOF";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0020[] =
	 "Cannot swap record #%d type %d";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0021[] =
	 "Encountered unimplemented type %d, in Callback item %d";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0022[] =
	 "Encountered unimplemented argument type %d";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0023[] =
	 "NULL hierarchy id";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0024[] =
	 "Invalid hierarchy";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0025[] =
	 "Invalid interface module";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0026[] =
	 "Invalid widget record";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0027[] =
	 "Context allocation failed";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0028[] =
	 "Validation failed";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0029[] =
	 "Resource size too large";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0030[] =
	 "Display not yet opened";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0031[] =
	 "Could not open file %s";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0032[] =
	 "Could not open file %s - UID file version mismatch";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0033[] =
	 "XmGetPixmap failed with XmUNSPECIFIED_PIXMAP for %s";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0034[] =
	 "XCreateImage failed during IconImage conversion";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0035[] =
	 "XCreatePixmap failed during IconImage conversion";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0036[] =
	 "XCreateGC failed during IconImage conversion";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0037[] =
	 "XtMalloc failed during IconImage conversion";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0038[] =
	 "Could not load color '%s'";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0039[] =
	 "Could not convert RGB color/pixel '%d,%d,%d'";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0040[] =
	 "Invalid color descriptor type";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0041[] =
	 "Illegal Color item monochrome state %d";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0042[] =
	 "Cannot find literal '%s'";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0043[] =
	 "Invalid resource context";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0044[] =
	 "Too many widgets";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0045[] =
	 "Topmost index out of bounds";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0046[] =
	 "List allocation failed";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0047[] =
	 "List vector allocation failed";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0048[] =
	 "Vector re-allocation failed";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0049[] =
	 "String allocation failed";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0050[] =
	 "UID file is obsolete - has no compression table";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0051[] =
	 "Could not find class descriptor for class %s";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0052[] =
	 "Cannot find indexed widget '%s'";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0053[] =
	 "Cannot find RID widget '%x'";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0054[] =
	 "?? UNKNOWN key type %d";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0055[] =
	 "Unknown widget variety";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0056[] =
	 "Unresolved Widget reference in creation callback";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0057[] =
	 "Could not Fixup creation callbacks";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0058[] =
	 "Unknown child type";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0059[] =
	 "Child of parent not found";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0060[] =
	 "Widget reference not Indexed";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0061[] =
	 "Could not convert color/pixel '%s'";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0062[] =
	 "Could not uncompress string code %d";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0063[] =
	 "Unhandled resource group";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0064[] =
	 "Could not convert ASCIZ '%s' to compound string";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0065[] =
	 "Could not parse translation table '%s'";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0066[] =
	 "Could not convert identifier '%s'";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0067[] =
	 "Internal error: case MrmRtypeIconImage reached";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0068[] =
	 "Internal error: case MrmRtypeXBitmapFile reached";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0069[] =
	 "Internal error: Could not convert Display to String";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0070[] =
	 "Could not convert font '%s'";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0071[] =
	 "Could not convert fontset '%s'";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0072[] =
	 "Could not open one or more fonts for fontset '%s'";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0073[] =
	 "Could not add fontlist font '%s' to list";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0074[] =
	 "Could not add fontlist fontset '%s' to list";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0075[] =
	 "Could not convert class record name '%s'";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0076[] =
	 "Could not convert keysym string '%s' to KeySym";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0077[] =
	 "Cannot find indexed literal '%s'";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0078[] =
	 "Cannot find RID literal '%x'";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0079[] =
	 "Unknown literal key type %d";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0080[] =
	 "Invalid ColorTable literal type %d";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0081[] =
	 "Invalid ColorTable type code %d";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0082[] =
	 "Invalid Color literal type %d";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0083[] =
	 "Invalid ColorItem type code %d";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0084[] =
	 "Callback routine '%s' not registered";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0085[] =
	 "Widget reference not Indexed";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0086[] =
	 "Cannot find indexed widget resource '%s'";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0087[] =
	 "Cannot find RID widget resource '%x'";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0088[] =
	 "Unknown resource key type %d";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0089[] =
	 "Could not instantitate widget tree";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0090[] =
	 "Null user-defined class name";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0091[] =
	 "Invalid class code";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0092[] =
	 "Too many arguments";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0093[] =
	 "Invalid compression code";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0094[] =
	 "Invalid or unhandled type";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0095[] =
	 "Vector too big";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0096[] =
	 "Empty routine name";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0097[] =
	 "Char8Vector not yet implemented";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0098[] =
	 "CStringVector not yet implemented";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0099[] =
	 "Too many children";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0100[] =
	 "Null children list descriptor";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0101[] =
	 "Child index out of bounds";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0102[] =
	 "Null index";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0103[] =
	 "Invalid key type";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0104[] =
	 "Arg index out of bounds";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0105[] =
	 "Invalid widget/gadget record";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0106[] =
	 "Invalid callback descriptor";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0107[] =
	 "Callback item index out of bounds";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0108[] =
	 "Could not uncompress string code %d";
externaldef(mrmmsg) _MrmConst char _MrmMsg_0109[] =
         "Too many items";
/* BEGIN OSF Fix CR 4859 */
externaldef(mrmmsg) _MrmConst char _MrmMsg_0110[] =
       "Could not convert multibyte string to widecharacter string";
/* END OSF Fix CR 4859 */
     
/*
 *
 *  OWN VARIABLE DECLARATIONS
 *
 */
#ifdef _NO_PROTO
String Urm__UT_UrmCodeString ();
#endif

static	String	urm_codes_codstg[] = {
    "MrmFAILURE"
    ,"MrmSUCCESS"
    ,"MrmNOT_FOUND"
    ,"MrmCREATE_NEW"
    ,"MrmEXISTS"
    ,"URMIndex retry"
    ,"MrmNUL_GROUP"
    ,"MrmINDEX_GT"
    ,"MrmNUL_TYPE"
    ,"MrmINDEX_LT"
    ,"MrmWRONG_GROUP"
    ,"MrmPARTIAL_SUCCESS"
    ,"MrmWRONG_TYPE"
    ,"URM unused code 13"
    ,"MrmOUT_OF_RANGE"
    ,"URM unused code 15"
    ,"MrmBAD_RECORD"
    ,"URM unused code 17"
    ,"MrmNULL_DATA"
    ,"URM unused code 19"
    ,"MrmBAD_DATA_INDEX"
    ,"URM unused code 21"
    ,"MrmBAD_ORDER"
    ,"URM unused code 23"
    ,"MrmBAD_CONTEXT"
    ,"URM unused code 25"
    ,"MrmNOT_VALID"
    ,"URM unused code 27"
    ,"MrmBAD_BTREE"
    ,"URM unused code 29"
    ,"MrmBAD_WIDGET_REC"
    ,"URM unused code 31"
    ,"MrmBAD_CLASS_TYPE"
    ,"URM unused code 33"
    ,"MrmNO_CLASS_NAME"
    ,"URM unused code 35"
    ,"MrmTOO_MANY"
    ,"URM unused code 37"
    ,"MrmBAD_IF_MODULE"
    ,"URM unused code 39"
    ,"MrmNULL_DESC"
    ,"URM unused code 41"
    ,"MrmOUT_OF_BOUNDS"
    ,"URM unused code 43"
    ,"MrmBAD_COMPRESS"
    ,"URM unused code 45"
    ,"MrmBAD_ARG_TYPE"
    ,"URM unused code 47"
    ,"MrmNOT_IMP"
    ,"URM unused code 49"
    ,"MrmNULL_INDEX"
    ,"URM unused code 51"
    ,"MrmBAD_KEY_TYPE"
    ,"URM unused code 53"
    ,"MrmBAD_CALLBACK"
    ,"URM unused code 55"
    ,"MrmNULL_ROUTINE"
    ,"URM unused code 57"
    ,"MrmVEC_TOO_BIG"
    ,"URM unused code 59"
    ,"MrmBAD_HIERARCHY"
    ,"URM unused code 61"
    ,"MrmBAD_CLASS_CODE"
    } ;

static String urm_codes_invalidcode = "Invalid URM code" ;




Cardinal Urm__UT_Error (module, error, file_id, context_id, status)
    char			*module ;
    char			*error ;
    IDBFile			file_id ;
    URMResourceContextPtr	context_id ;
    Cardinal			status ;

/*
 *++
 *
 *  PROCEDURE DESCRIPTION:
 *
 *	This routine is an error signalling routine for use within URM.
 *	It currently just reports the error on the terminal.
 *
 *  FORMAL PARAMETERS:
 *
 *	module		Identifies the module (routine) detecting the error
 *	error		Brief description of the error
 *	file_id		if not NULL, the IDB file implicated in the error
 *	context_id	if not NULL, the URM resource implicated in the error
 *	status		the return code associated with the error.
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  FUNCTION VALUE:
 *
 *  SIDE EFFECTS:
 *
 *--
 */

{

/*
 *  External Functions
 */

/*
 *  Local variables
 */

char		msg[300] ;	/* error message */

/*
 * construct error message
 */

/* Old form
sprintf (msg, "%s detected error %s - %s", module, error,
    Urm__UT_UrmCodeString(status)) ;
 */
sprintf (msg, "%s: %s - %s", module, error, Urm__UT_UrmCodeString(status)) ;

/*
 * Print or save the message depending on the reporting style
 */
urm__latest_error_code = status ;

switch ( urm__err_out )
    {
    case URMErrOutMemory:
        if ( urm__latest_error_msg != NULL )
            XtFree (urm__latest_error_msg) ;
        urm__latest_error_msg = (String) XtMalloc (strlen(msg)+1) ;
        strcpy (urm__latest_error_msg, msg) ;
        return status ;
    case URMErrOutStdout:
    default:
        XtWarning (msg) ;
        return status ;
    }

}



Cardinal Urm__UT_SetErrorReport 

#ifndef _NO_PROTO
	(MrmCode report_type)
#else
	(report_type)
    MrmCode			report_type ;
#endif

/*
 *++
 *
 *  PROCEDURE DESCRIPTION:
 *
 *	This routine sets the URM error report type to a standard state
 *
 *  FORMAL PARAMETERS:
 *
 *	report_type	URMErrOutMemory	- save message in memory, don't print
 *			URMErrOutStdout	- report to stdout
 *
 *  IMPLICIT INPUTS:
 *
 *	urm__err_out
 *
 *  IMPLICIT OUTPUTS:
 *
 *	urm__err_out
 *
 *  FUNCTION VALUE:
 *
 *      MrmSUCCESS	operation succeeded
 *	MrmFAILURE	illegal state requested
 *
 *  SIDE EFFECTS:
 *
 *--
 */

{

/*
 *  External Functions
 */

/*
 *  Local variables
 */
switch ( report_type )
    {
    case URMErrOutMemory:
    case URMErrOutStdout:
        urm__err_out = report_type ;
        return MrmSUCCESS ;
    default:
        return MrmFAILURE ;
    }

}



MrmCode Urm__UT_LatestErrorCode ()

/*
 *++
 *
 *  PROCEDURE DESCRIPTION:
 *
 *	 Returns the current error code
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  FUNCTION VALUE:
 *
 *  SIDE EFFECTS:
 *
 *--
 */

{

/*
 *  External Functions
 */

/*
 *  Local variables
 */

return urm__latest_error_code ;

}



String Urm__UT_LatestErrorMessage ()

/*
 *++
 *
 *  PROCEDURE DESCRIPTION:
 *
 *	 Returns the current error message
 *
 *  FORMAL PARAMETERS:
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  FUNCTION VALUE:
 *
 *  SIDE EFFECTS:
 *
 *--
 */

{

/*
 *  External Functions
 */

/*
 *  Local variables
 */

return urm__latest_error_msg ;

}



String Urm__UT_UrmCodeString 

#ifndef _NO_PROTO
	(MrmCode cod)
#else
	(cod)
    MrmCode		cod ;
#endif
/*
 *++
 *
 *  PROCEDURE DESCRIPTION:
 *
 *	This routine returns a static string naming a URM return code.
 *
 *  FORMAL PARAMETERS:
 *
 *	cod		A URM return code
 *
 *  IMPLICIT INPUTS:
 *
 *  IMPLICIT OUTPUTS:
 *
 *  FUNCTION VALUE:
 *
 *  SIDE EFFECTS:
 *
 *--
 */

{

/*
 *  External Functions
 */

/*
 *  Local variables
 */
if ( cod>=MrmFAILURE && cod<=MrmBAD_CLASS_CODE )
    return urm_codes_codstg[cod] ;
return urm_codes_invalidcode ;

}

