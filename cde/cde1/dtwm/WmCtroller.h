#ifdef WSM
/* 
 * (c) Copyright 1987, 1988, 1989, 1990, 1992 HEWLETT-PACKARD COMPANY 
 * ALL RIGHTS RESERVED 
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = ""
#endif
#endif

#ifdef _NO_PROTO

extern void ShowController() ;
extern Boolean ValidWsName() ;
extern void ChangeWorkspaceTitle() ;

#else

extern void ShowController( 
                        WmScreenData *pSD) ;
extern Boolean ValidWsName( 
                        unsigned char *title) ;
extern void ChangeWorkspaceTitle( 
                        WmWorkspaceData *pWS,
                        XmString xmstr) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

#endif /* WSM */
