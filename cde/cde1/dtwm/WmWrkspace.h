/* $XConsortium: WmWrkspace.h /main/cde1_maint/2 1995/10/09 13:46:25 pascale $ */
#ifdef WSM
/* 
 * (c) Copyright 1987,1988,1989,1990,1992,1993,1994 HEWLETT-PACKARD COMPANY 
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 * ALL RIGHTS RESERVED 
 */ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = ""
#endif
#endif

/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern void ChangeToWorkspace() ;
extern Boolean DuplicateWorkspaceName ();
extern void UpdateWorkspacePresenceProperty() ;
#ifdef HP_VUE
extern void UpdateWorkspaceInfoProperty() ;
#endif /* HP_VUE */
extern WmWorkspaceData * CreateWorkspace() ;
extern void DeleteWorkspace() ;
extern Boolean GetClientWorkspaceInfo() ;
extern Boolean WorkspaceIsInCommand() ;
extern Boolean ConvertNamesToIDs() ;
extern void CheckForPutInAllRequest() ;
extern Boolean FindWsNameInCommand() ;
extern void PutClientIntoWorkspace() ;
extern void TakeClientOutOfWorkspace() ;
extern WmWorkspaceData * GetWorkspaceData() ;
extern unsigned char * GenerateWorkspaceName() ;
extern Boolean InWindowList() ;
extern Boolean ClientInWorkspace() ;
extern WsClientData * GetWsClientData() ;
extern void SetClientWsIndex() ;
extern Boolean ProcessWorkspaceHints() ;
extern void ProcessWorkspaceHintList() ;
extern void RemoveSingleClientFromWorkspaces() ;
extern void RemoveSubtreeFromWorkspaces() ;
#ifdef PANELIST
extern WorkspaceID * GetListOfOccupiedWorkspaces() ;
#endif /* PANELIST */
extern void HonorAbsentMapBehavior();
extern void RemoveClientFromWorkspaces() ;
extern void AddSingleClientToWorkspaces() ;
extern void AddSubtreeToWorkspaces() ;
extern void AddClientToWorkspaces() ;
extern void AddClientToWsList() ;
extern void RemoveClientFromWsList() ;
extern Boolean F_CreateWorkspace() ;
extern Boolean F_DeleteWorkspace() ;
extern Boolean F_GotoWorkspace() ;
extern Boolean F_AddToAllWorkspaces() ;
extern Boolean F_Remove() ;
extern int GetCurrentWorkspaceIndex() ;
extern void InsureIconForWorkspace() ;
extern Boolean GetLeaderPresence() ;
extern Boolean GetMyOwnPresence() ;
extern void ReserveIdListSpace() ;
extern void SaveWorkspaceResources() ;
extern void SaveResources() ;
extern void AddStringToResourceData() ;
#ifdef PANELIST
extern void SetCurrentWorkspaceButton();
extern void SetFrontPanelTitle();
#endif /* PANELIST */

#else

extern void ChangeToWorkspace( 
                        WmWorkspaceData *pNewWS) ;
extern Boolean DuplicateWorkspaceName (
			WmScreenData *pSD, 
			unsigned char *name, 
			int num);
extern void UpdateWorkspacePresenceProperty( 
                        ClientData *pCD) ;
#ifdef HP_VUE
extern void UpdateWorkspaceInfoProperty( 
                        WmScreenData *pSD) ;
#endif /* HP_VUE */
extern WmWorkspaceData * CreateWorkspace( 
                        WmScreenData *pSD,
                        unsigned char *name) ;
extern void DeleteWorkspace( 
                        WmWorkspaceData *pWS) ;
extern Boolean GetClientWorkspaceInfo( 
                        ClientData *pCD,
                        long manageFlags) ;
extern Boolean WorkspaceIsInCommand( 
                        Display *dpy,
                        ClientData *pCD,
                        WorkspaceID **ppIDs,
                        unsigned int *pNumIDs) ;
extern Boolean ConvertNamesToIDs( 
                        WmScreenData *pSD,
                        unsigned char *pchIn,
                        WorkspaceID **ppAtoms,
                        unsigned int *pNumAtoms) ;
extern void CheckForPutInAllRequest( 
                        ClientData *pCD,
                        Atom *pIDs,
                        unsigned int numIDs) ;
extern Boolean FindWsNameInCommand( 
                        int argc,
                        char *argv[],
                        unsigned char **ppch) ;
extern void PutClientIntoWorkspace( 
                        WmWorkspaceData *pWS,
                        ClientData *pCD) ;
extern void TakeClientOutOfWorkspace( 
                        WmWorkspaceData *pWS,
                        ClientData *pCD) ;
extern WmWorkspaceData * GetWorkspaceData( 
                        WmScreenData *pSD,
                        WorkspaceID wsID) ;
extern unsigned char * GenerateWorkspaceName( 
                        WmScreenData *pSD,
                        int wsnum) ;
extern Boolean InWindowList( 
                        Window w,
                        Window wl[],
                        int num) ;
extern Boolean ClientInWorkspace( 
                        WmWorkspaceData *pWS,
                        ClientData *pCD) ;
extern WsClientData * GetWsClientData( 
                        WmWorkspaceData *pWS,
                        ClientData *pCD) ;
extern void SetClientWsIndex( 
                        ClientData *pCD) ;
extern Boolean ProcessWorkspaceHints( 
                        ClientData *pCD) ;
extern void ProcessWorkspaceHintList( 
                        ClientData *pCD,
                        WorkspaceID *pIDs,
                        unsigned int numIDs) ;
extern void RemoveSingleClientFromWorkspaces( 
                        ClientData *pCD,
                        WorkspaceID *pIDs,
                        unsigned int numIDs) ;
extern void RemoveSubtreeFromWorkspaces( 
                        ClientData *pCD,
                        WorkspaceID *pIDs,
                        unsigned int numIDs) ;
#ifdef PANELIST
extern WorkspaceID * GetListOfOccupiedWorkspaces( 
			ClientData *pCD,
                        int *numIDs) ;
#endif /* PANELIST */
extern void HonorAbsentMapBehavior(
			ClientData *pCD) ;
extern void RemoveClientFromWorkspaces( 
                        ClientData *pCD,
                        WorkspaceID *pIDs,
                        unsigned int numIDs) ;
extern void AddSingleClientToWorkspaces( 
                        ClientData *pCD,
                        WorkspaceID *pIDs,
                        unsigned int numIDs) ;
extern void AddSubtreeToWorkspaces( 
                        ClientData *pCD,
                        WorkspaceID *pIDs,
                        unsigned int numIDs) ;
extern void AddClientToWorkspaces( 
                        ClientData *pCD,
                        WorkspaceID *pIDs,
                        unsigned int numIDs) ;
extern void AddClientToWsList( 
                        WmWorkspaceData *pWS,
                        ClientData *pCD) ;
extern void RemoveClientFromWsList( 
                        WmWorkspaceData *pWS,
                        ClientData *pCD) ;
extern Boolean F_CreateWorkspace( 
                        String args,
                        ClientData *pCD,
                        XEvent *event) ;
extern Boolean F_DeleteWorkspace( 
                        String args,
                        ClientData *pCD,
                        XEvent *event) ;
extern Boolean F_GotoWorkspace( 
                        String args,
                        ClientData *pCD,
                        XEvent *event) ;
extern Boolean F_AddToAllWorkspaces( 
                        String args,
                        ClientData *pCD,
                        XEvent *event) ;
extern Boolean F_Remove( 
                        String args,
                        ClientData *pCD,
                        XEvent *event) ;
extern int GetCurrentWorkspaceIndex( 
                        WmScreenData *pSD) ;
extern void InsureIconForWorkspace( 
                        WmWorkspaceData *pWS,
                        ClientData *pCD) ;
extern Boolean GetLeaderPresence( 
                        ClientData *pCD,
                        WorkspaceID **ppIDs,
                        unsigned int *pnumIDs) ;
extern Boolean GetMyOwnPresence( 
                        ClientData *pCD,
                        WorkspaceID **ppIDs,
                        unsigned int *pnumIDs) ;
extern void ReserveIdListSpace( 
                        int numIDs) ;
extern void SaveWorkspaceResources( 
                        WmWorkspaceData *pWS,
                        unsigned long flags) ;
extern void SaveResources( 
                        WmScreenData *pSD) ;
extern void AddStringToResourceData( 
                        char *string,
                        char **pdata,
                        int *plen) ;
#ifdef PANELIST
extern void SetCurrentWorkspaceButton(
			WmScreenData *pSD);
extern void SetFrontPanelTitle( 
			WmScreenData *pSD);
#endif /* PANELIST */

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/
#endif /* WSM */


