/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.3
*/ 
/*   $XConsortium: WmFunction.h /main/cde1_maint/2 1995/10/03 17:24:51 lehors $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef _NO_PROTO
#ifdef WSM
extern Boolean F_Action ();
#endif /* WSM */
extern Boolean F_Beep ();
extern Boolean F_Lower ();
extern void Do_Lower ();
extern Boolean F_Circle_Down ();
extern Boolean F_Circle_Up ();
extern Boolean F_Focus_Color ();
extern Boolean F_Exec ();
extern Boolean F_Quit_Mwm ();
extern void Do_Quit_Mwm ();
extern Boolean F_Focus_Key ();
extern void Do_Focus_Key ();
#ifdef WSM
extern Boolean F_Goto_Workspace();
#endif /* WSM */
#ifdef WSM
extern Boolean F_Help();
extern Boolean F_Help_Mode();
#endif /* WSM */
extern Boolean F_Next_Key ();
extern Boolean F_Prev_Cmap ();
extern Boolean F_Prev_Key ();
extern Boolean F_Pass_Key ();
#ifdef WSM
extern Boolean F_Marquee_Selection ();
#endif /* WSM */
extern Boolean F_Maximize ();
extern Boolean F_Menu ();
extern Boolean F_Minimize ();
extern Boolean F_Move ();
extern Boolean F_Next_Cmap ();
extern Boolean F_Nop ();
extern Boolean F_Normalize ();
extern Boolean F_Normalize_And_Raise ();
extern Boolean F_Pack_Icons ();
extern Boolean F_Post_SMenu ();
#ifdef PANELIST
extern Boolean F_Post_FpMenu ();
extern Boolean F_Push_Recall ();
#endif /* PANELIST */
extern Boolean F_Kill ();
extern Boolean F_Refresh ();
extern Boolean F_Resize ();
extern Boolean F_Restart ();
extern Boolean F_Restore ();
extern Boolean F_Restore_And_Raise ();
extern void Do_Restart ();
extern void RestartWm ();
extern void DeFrameClient ();
#ifdef WSM
extern Boolean F_Send ();
#endif /* WSM */
extern Boolean F_Send_Msg ();
extern Boolean F_Separator ();
extern Boolean F_Raise ();
extern void Do_Raise ();
extern Boolean F_Raise_Lower ();
extern Boolean F_Refresh_Win ();
extern Boolean F_Set_Behavior ();
#ifdef WSM
extern Boolean F_Set_Context ();
#endif
extern void Do_Set_Behavior ();
extern Boolean F_Title ();
extern Boolean F_Screen ();
extern Time GetFunctionTimestamp ();
#if defined(PANELIST)
extern Boolean F_Toggle_Front_Panel ();
extern Boolean F_Version ();
#endif /* PANELIST */
#ifdef WSM
extern Boolean F_Next_Workspace ();
extern Boolean F_Prev_Workspace ();
extern Boolean F_Workspace_Presence ();
#if defined(DEBUG) 
extern Boolean F_ZZ_Debug ();
#endif /* DEBUG */
#endif /* WSM */
extern void ReBorderClient ();
extern void ClearDirtyStackEntry ();	/* Fix for 5325 */

#else /* _NO_PROTO */

#ifdef WSM
extern Boolean F_Action (String actionName, ClientData *pCD, XEvent *event);
#endif /* WSM */
extern Boolean F_Beep (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Lower (String args, ClientData *pCD, XEvent *event);
extern void Do_Lower (ClientData *pCD, ClientListEntry *pStackEntry, int flags);
extern Boolean F_Circle_Down (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Circle_Up (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Focus_Color (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Exec (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Quit_Mwm (String args, ClientData *pCD, XEvent *event);
extern void Do_Quit_Mwm (Boolean diedOnRestart);
extern Boolean F_Focus_Key (String args, ClientData *pCD, XEvent *event);
extern void Do_Focus_Key (ClientData *pCD, Time focusTime, long flags);
#ifdef WSM
extern Boolean F_Goto_Workspace (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Help (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Help_Mode (String args, ClientData *pCD, XEvent *event);
#endif /* WSM */
extern Boolean F_Next_Key (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Prev_Cmap (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Prev_Key (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Pass_Key (String args, ClientData *pCD, XEvent *event);
#ifdef WSM
extern Boolean F_Marquee_Selection (String args, ClientData *pCD, XEvent *event);
#endif /* WSM */
extern Boolean F_Maximize (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Menu (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Minimize (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Move (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Next_Cmap (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Nop (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Normalize (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Normalize_And_Raise (String args, ClientData *pCD, 
				      XEvent *event);
extern Boolean F_Pack_Icons (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Post_SMenu (String args, ClientData *pCD, XEvent *event);
#ifdef PANELIST
extern Boolean F_Post_FpMenu (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Push_Recall (String args, ClientData *pCD, XEvent *event);
#endif /* PANELIST */
extern Boolean F_Kill (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Refresh (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Resize (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Restart (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Restore (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Restore_And_Raise (String args, ClientData *pCD, 
				      XEvent *event);
extern void Do_Restart (Boolean dummy);
extern void RestartWm (long startupFlags);
extern void DeFrameClient (ClientData *pCD);
#ifdef WSM
extern Boolean F_Send (String args, ClientData *pCD, XEvent *event);
#endif /* WSM */
extern Boolean F_Send_Msg (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Separator (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Raise (String args, ClientData *pCD, XEvent *event);
extern void Do_Raise (ClientData *pCD, ClientListEntry *pStackEntry, int flags);
extern Boolean F_Raise_Lower (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Refresh_Win (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Set_Behavior (String args, ClientData *pCD, XEvent *event);
#ifdef WSM
extern Boolean F_Set_Context (String args, ClientData *pCD, XEvent *event);
#endif
extern void Do_Set_Behavior (Boolean dummy);
extern Boolean F_Title (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Screen (String args, ClientData *pCD, XEvent *event);
#if defined(PANELIST)
extern Boolean F_Toggle_Front_Panel (String args, ClientData *pCD, 
				     XEvent *event);
extern Boolean F_Version (String args, ClientData *pCD, XEvent *event);
#endif /* PANELIST */
#ifdef WSM
extern Boolean F_Next_Workspace (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Prev_Workspace (String args, ClientData *pCD, XEvent *event);
extern Boolean F_Workspace_Presence (String args, ClientData *pCD, 
				     XEvent *event);
#if defined(DEBUG)
extern Boolean F_ZZ_Debug (String, ClientData *, XEvent *);
#endif /* DEBUG */
#endif /* WSM */
extern Time GetFunctionTimestamp (XButtonEvent *pEvent);
extern void ReBorderClient (ClientData *pCD, Boolean reMapClient);
extern void ClearDirtyStackEntry (ClientData *pCD);	/* Fix for 5325 */
#endif /* _NO_PROTO */

#ifdef PANELIST

typedef struct _WmPushRecallArg {
    int ixReg;
    WmFunction wmFunc;
    void *pArgs;
} WmPushRecallArg;


#endif /* PANELIST */
