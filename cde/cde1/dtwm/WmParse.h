/*
 * $XConsortium: WmParse.h /main/cde1_maint/1 1995/07/15 01:56:08 drk $
 */
/******************************<+>*************************************
 **********************************************************************
 **
 **  File:        WmParse.h
 **
 **  Project:     HP/Motif Workspace Manager (dtwm)
 **
 **  Description:
 **  -----------
 **  This file contains function definitions for the corresponding .c
 **  file
 **
 **
 **********************************************************************
 **
 ** (c) Copyright 1987, 1988, 1989, 1990, 1991 HEWLETT-PACKARD COMPANY
 ** ALL RIGHTS RESERVED
 **
 **********************************************************************
 **********************************************************************
 **
 **
 **********************************************************************
 ******************************<+>*************************************/
#ifndef _Dt_WmParse_h
#define _Dt_WmParse_h
#include <stdio.h>
#include <string.h>

/*
 * Definitions
 */
typedef struct _DtWmpParseBuf
{

    FILE *		pFile;		/* FILE variable */
    unsigned char *	pchLine;	/* ptr to current line */
    int			cLineSize;	/* # of bytes in pchLine */
    unsigned char *	pchNext;	/* ptr to next char in pchLine */
    int			lineNumber;	/* # of current line being parsed */

} DtWmpParseBuf;

/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern void _DtWmParseSkipWhitespace() ;
extern void _DtWmParseSkipWhitespaceC() ;
extern unsigned char * _DtWmParseNextToken () ;
extern unsigned char * _DtWmParseNextTokenC() ;
extern DtWmpParseBuf * _DtWmParseNewBuf() ;
extern void _DtWmParseDestroyBuf() ;
extern void _DtWmParseSetLine() ;
extern void _DtWmParseSetFile() ;
extern unsigned char * _DtWmParseNextLine() ;
extern unsigned char * _DtWmParseCurrentChar() ;
extern unsigned char * _DtWmParseNextChar() ;
extern int _DtWmParseLineNumber() ;
extern void _DtWmParseToLower() ;
extern unsigned int _DtWmParsePeekAhead() ;
extern unsigned char * _DtWmParseBackUp () ;
extern unsigned char * _DtWmParseFilenameExpand ();
extern unsigned char * _DtWmParseMakeQuotedString();
extern unsigned char * _DtWmParseExpandEnvironmentVariables ();
extern unsigned char * _DtWmParseNextTokenExpand ();

#else

extern void _DtWmParseSkipWhitespace( 
                        DtWmpParseBuf *pWmPB) ;
extern void _DtWmParseSkipWhitespaceC( 
                        unsigned char **linePP) ;
extern unsigned char * _DtWmParseNextToken (
			DtWmpParseBuf *pWmPB) ;
extern unsigned char * _DtWmParseNextTokenC( 
			unsigned char **linePP, 
                        Boolean SmBehavior) ;
extern DtWmpParseBuf * _DtWmParseNewBuf( void ) ;
extern void _DtWmParseDestroyBuf( 
                        DtWmpParseBuf *pWmPB) ;
extern void _DtWmParseSetLine( 
                        DtWmpParseBuf *pWmPB,
                        unsigned char *pch) ;
extern void _DtWmParseSetFile( 
                        DtWmpParseBuf *pWmPB,
                        FILE *pFile) ;
extern unsigned char * _DtWmParseNextLine( 
                        DtWmpParseBuf *pWmPB) ;
extern unsigned char * _DtWmParseCurrentChar( 
                        DtWmpParseBuf *pWmPB) ;
extern unsigned char * _DtWmParseNextChar( 
                        DtWmpParseBuf *pWmPB) ;
extern int _DtWmParseLineNumber( 
                        DtWmpParseBuf *pWmPB) ;
extern void _DtWmParseToLower (
			unsigned char  *string);
extern unsigned int _DtWmParsePeekAhead(
			unsigned char *currentChar,
			unsigned int currentLev) ;
extern unsigned char * _DtWmParseBackUp ( 
			DtWmpParseBuf *pWmPB, 
			unsigned char *pchTok) ;
extern unsigned char * _DtWmParseFilenameExpand (
			unsigned char *pchFilename
			);
extern unsigned char * _DtWmParseMakeQuotedString(
			unsigned char *pchLine);
extern unsigned char * _DtWmParseExpandEnvironmentVariables (
			unsigned char *pch,
			unsigned char *pchBrk
			);
extern unsigned char * _DtWmParseNextTokenExpand (
			DtWmpParseBuf *pWmPB 
			);
#endif /* _NO_PROTO */
#endif  /* _Dt_WmParse_h */
