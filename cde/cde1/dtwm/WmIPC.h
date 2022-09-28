/* $XConsortium: WmIPC.h /main/cde1_maint/1 1995/07/18 01:56:21 drk $ */
/* 
 * (c) Copyright 1987, 1988, 1989, 1990, 1992 HEWLETT-PACKARD COMPANY 
 * ALL RIGHTS RESERVED 
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = ""
#endif
#endif


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern void dtInitialize() ;
extern void dtInitializeMessaging() ;
extern void dtCloseIPC() ;
extern void dtReadyNotification() ;
extern void dtInvokeDropAction() ;
extern void WmStopWaiting() ;
extern void dtSendWorkspaceModifyNotification();

#else

extern void dtInitialize( 
                        char *program_name,
                        XtAppContext appContext) ;
extern void dtInitializeMessaging( void ) ;
extern void dtCloseIPC( void ) ;
extern void dtReadyNotification( void ) ;
extern void WmStopWaiting( void ) ;
extern void dtSendWorkspaceModifyNotification(
	WmScreenData 		*pSD, 
	Atom 			aWs, 
	int 			iType);

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

/****************************   eof    ***************************/
