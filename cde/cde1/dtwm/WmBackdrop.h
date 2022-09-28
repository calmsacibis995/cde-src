/* $XConsortium: WmBackdrop.h /main/cde1_maint/1 1995/07/18 01:55:12 drk $ */
#ifdef WSM
/* 
 * (c) Copyright 1987, 1988, 1989, 1990, 1992 HEWLETT-PACKARD COMPANY 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = ""
#endif
#endif

/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern void ChangeBackdrop() ;
extern void ProcessBackdropResources() ;
extern String FullBitmapFilePath() ;
extern void SetNewBackdrop() ;
extern Boolean IsBackdropWindow() ;

#else

extern void ChangeBackdrop( 
                        WmWorkspaceData *pWS) ;
extern void ProcessBackdropResources( 
                        WmWorkspaceData *pWS,
                        unsigned long callFlags) ;
extern String FullBitmapFilePath( 
                        String pch) ;
extern void SetNewBackdrop( 
                        WmWorkspaceData *pWS,
                        Pixmap pixmap,
                        String bitmapFile) ;
extern Boolean IsBackdropWindow( 
                        WmScreenData *pSD,
                        Window win) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

#endif /* WSM */
/****************************   eof    ***************************/
