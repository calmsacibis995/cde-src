/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $XConsortium: WmResParse.h /main/cde1_maint/2 1995/10/09 13:45:18 pascale $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#include <stdio.h>

#ifdef _NO_PROTO

extern void ProcessWmFile ();
extern void ProcessCommandLine ();
extern void ProcessMotifBindings ();
#ifdef WSM
extern Boolean         FindDtSessionMatch ();
extern void            WmDtGetHelpArgs();
extern void             GetActionIndex ();
extern void            GetFunctionTableValues ();
extern void             GetExecIndex ();
extern void             GetNopIndex ();
extern Boolean          GetSessionHintsInfo ();

#endif /* WSM */
extern FILE          * FopenConfigFile ();
extern void            FreeMenuItem ();
#ifndef WSM
extern unsigned char * GetNextLine ();
#endif /* not WSM */
#ifdef WSM
extern unsigned char * GetStringC ();
extern void SystemCmd ();
#else /* WSM */
extern unsigned char * GetString ();
#endif /* WSM */

extern Boolean          ParseBtnEvent ();
extern Boolean          ParseKeyEvent ();
extern void            ParseButtonStr ();
extern void            ParseKeyStr ();
extern MenuItem      * ParseMwmMenuStr ();
 
#ifdef WSM
extern void             ParseSessionClientState ();
extern void             ParseSessionCommand ();
extern void             ParseSessionGeometry ();
extern void             ParseSessionItems ();
extern void             ParseSessionWorkspaces ();
extern void             ParseSessionHost ();
extern void             ParseDtSessionHint ();
#endif /* WSM */
extern int             ParseWmFunction ();
extern void            ProcessWmFile ();
extern void            PWarning ();
extern void            SaveMenuAccelerators ();
extern void             ScanAlphanumeric ();
#ifndef WSM
extern void            ScanWhitespace ();
#endif /* not WSM */
extern void            ToLower ();
extern void		SyncModifierStrings();
#ifdef PANELIST
extern void DeleteTempConfigFileIfAny ();
extern Boolean ParseWmFunctionArg ();
extern Boolean ParseWmFuncMaybeStrArg ();
extern Boolean ParseWmFuncStrArg ();
extern Boolean ParseWmFuncActionArg ();
#endif /* PANELIST */

#else /* _NO_PROTO */

#ifdef PANELIST
extern void            ProcessWmFile (WmScreenData *pSD, Boolean bNested);
#else /* PANELIST */
extern void ProcessWmFile (WmScreenData *pSD);
#endif /* PANELIST */
extern void ProcessCommandLine (int argc,  char *argv[]);
extern void ProcessMotifBindings (void);
#ifdef WSM
extern Boolean         FindDtSessionMatch(int commandArgc, 
					   char **commandArgv, 
					   ClientData *pCD, 
					   WmScreenData *pSD, 
					   char **pWorkSpaceList,
					   char *clientMachine);
extern void            WmDtGetHelprgs(char *args, 
				       unsigned char** volume, 
				       unsigned char** topic, 
				       int *argsCount);
extern void GetActionIndex (int tableSize, int *actionIndex);
extern void            GetFunctionTableValues (int *execIndex, int *nopIndex,
		    int *actionIndex);
extern void GetNopIndex (int tableSize, int *nopIndex);
extern void GetExecIndex (int tableSize, int *execIndex);
extern Boolean GetSessionHintsInfo (WmScreenData *pSD, long numItems);
#endif /* WSM */
extern FILE          * FopenConfigFile (void);
extern void            FreeMenuItem (MenuItem *menuItem);
#ifndef WSM
extern unsigned char * GetNextLine (void);
#endif /* not WSM */
#ifdef WSM
extern unsigned char * GetStringC (unsigned char **linePP, Boolean SmBehavior);
extern void SystemCmd (char *pchCmd);
#else /* WSM */
extern unsigned char * GetString (unsigned char **linePP);
#endif /* WSM */
extern Boolean ParseBtnEvent (unsigned char  **linePP,
                              unsigned int *eventType,
                              unsigned int *button,
                              unsigned int *state,
                              Boolean      *fClick);

extern void            ParseButtonStr (WmScreenData *pSD, unsigned char *buttonStr);
extern void            ParseKeyStr (WmScreenData *pSD, unsigned char *keyStr);
extern Boolean ParseKeyEvent (unsigned char **linePP, unsigned int *eventType,
		       KeyCode *keyCode,  unsigned int *state);
extern MenuItem      * ParseMwmMenuStr (WmScreenData *pSD, unsigned char *menuStr);
#ifdef WSM
extern void ParseSessionClientState (WmScreenData *pSD, int count,
			      unsigned char *string);
extern void ParseSessionCommand (WmScreenData *pSD,  int count,
			  unsigned char **commandString);
extern void ParseSessionGeometry (WmScreenData *pSD, int count,
			   unsigned char *string);
extern void ParseSessionItems (WmScreenData *pSD);
extern void ParseSessionWorkspaces (WmScreenData *pSD,  int count,
			     unsigned char *string);
extern void ParseSessionHost (WmScreenData *pSD,  int count,
			     unsigned char *string);
extern void ParseDtSessionHints (WmScreenData *pSD, unsigned char *property);
#endif /* WSM */
extern int             ParseWmFunction (unsigned char **linePP, unsigned int res_spec, WmFunction *pWmFunction);
extern void            PWarning (char *message);
extern void            SaveMenuAccelerators (WmScreenData *pSD, MenuSpec *newMenuSpec);
extern void      ScanAlphanumeric (unsigned char **linePP);
#ifndef WSM
extern void            ScanWhitespace(unsigned char  **linePP);
#endif /* not WSM */
extern void            ToLower (unsigned char  *string);
extern void		SyncModifierStrings(void);
#ifdef PANELIST
extern void DeleteTempConfigFileIfAny (void);
extern Boolean ParseWmFunctionArg (
		unsigned char **linePP,
		int ix, 
		WmFunction wmFunc, 
		void **ppArg,
		String sClientName,
		String sTitle);
extern Boolean ParseWmFuncMaybeStrArg (unsigned char **linePP, 
				       WmFunction wmFunction, String *pArgs);
extern Boolean ParseWmFuncStrArg (unsigned char **linePP, 
				       WmFunction wmFunction, String *pArgs);
extern Boolean ParseWmFuncActionArg (unsigned char **linePP, 
				  WmFunction wmFunction, String *pArgs);
#endif /* PANELIST */
#endif /* _NO_PROTO */
#ifdef WSM
#define ToLower(s)		(_DtWmParseToLower (s))
#define GetNextLine()		(_DtWmParseNextLine (wmGD.pWmPB))
#define GetSmartSMString(s)	(_DtWmParseNextTokenC (s, True))
#define GetSmartString(s)	(_DtWmParseNextTokenC (s, False))
#define GetString(s)		(_DtWmParseNextTokenC (s, False))
#define ScanWhitespace(s)	(_DtWmParseSkipWhitespaceC (s))
#endif /* WSM */
