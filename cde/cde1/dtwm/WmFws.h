/* 
 * (c) Copyright 1995 Sun Microsystems, Inc. 
 * ALL RIGHTS RESERVED 
 */ 
/*   $RCSfile: WmRws.h,v $ $Revision: 1.00 $ $Date: 95/08/09 00:00:00 $ */


#ifdef _NO_PROTO

extern void FwsInitializeWindow();
extern void FwsInitializeProperties();
extern void FwsClientInfo();
extern void FwsClearClientInfo();
extern Boolean IsFwsClient();
extern void FwsUnmapClient();
extern void FwsMapClient();
extern void FwsReserveIconSpace();
extern void FwsUnreserveIconSpace();
extern Boolean HandleEventsOnFwsWindow();
extern void FwsDestroyWindow();
extern void FwsQuit();
extern void FwsLowerClientWindow();

#else /* _NO_PROTO */

extern void FwsInitializeWindow (void);
extern void FwsInitializeProperties (void);
extern void FwsClientInfo (ClientData *pCD, Bool new);
extern void FwsClearClientInfo (ClientData *pCD);
extern Boolean IsFwsClient (Window client);
extern void FwsUnmapClient (ClientData *pCD);
extern void FwsMapClient (ClientData *pCD);
extern void FwsReserveIconSpace (WmWorkspaceData *pWS,
				 IconPlacementData *pIPD);
extern void FwsUnreserveIconSpace (WmWorkspaceData *pWS,
				   IconPlacementData *pIPD);
extern Boolean HandleEventsOnFwsWindow (ClientData *pCD, XEvent *event);
extern void FwsDestroyWindow (Window window);
extern void FwsQuit (void);
extern void FwsLowerClientWindow (void);

#endif /* _NO_PROTO */
