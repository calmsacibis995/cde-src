/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $XConsortium: WmEvent.h /main/cde1_maint/1 1995/07/15 01:48:03 drk $ */
/*
 * (c) Copyright 1987, 1988, 1989, 1990 HEWLETT-PACKARD COMPANY */

#ifdef WSM
/* Shared Variables */
extern int smAckState;

#endif /* WSM */
#ifdef _NO_PROTO
extern Boolean		CheckForButtonAction ();
extern Time		GetTimestamp ();
extern Boolean		HandleKeyPress ();
extern void		HandleWsButtonPress ();
extern void		HandleWsButtonRelease ();
extern void		HandleWsConfigureRequest ();
extern void		HandleWsEnterNotify ();
extern void		HandleWsFocusIn ();
extern Boolean		HandleWsKeyPress ();
extern void		HandleWsLeaveNotify ();
extern void		IdentifyEventContext ();
extern void		InitEventHandling ();
extern void		ProcessClickBPress ();
extern void		ProcessClickBRelease ();
extern void		PullExposureEvents ();
extern int		SetupKeyBindings ();
extern Boolean		WmDispatchMenuEvent ();
extern Boolean		WmDispatchWsEvent ();
extern void 		WmGrabButton ();
extern void 		WmGrabKey ();
extern void 		WmUngrabButton ();
#ifdef WSM
extern void		HandleDtWmClientMessage ();
extern void		HandleDtWmRequest ();
extern Boolean 		ReplayedButtonEvent ();
#endif /* WSM */

#else /* _NO_PROTO */

extern Boolean CheckForButtonAction (XButtonEvent *buttonEvent, 
				     Context context, Context subContext, 
				     ClientData *pCD);
extern Time GetTimestamp (void);
extern Boolean HandleKeyPress (XKeyEvent *keyEvent, KeySpec *keySpecs, 
			       Boolean checkContext, Context context, 
			       Boolean onlyFirst, ClientData *pCD);
extern void HandleWsButtonPress (XButtonEvent *buttonEvent);
extern void HandleWsButtonRelease (XButtonEvent *buttonEvent);
extern void HandleWsConfigureRequest (XConfigureRequestEvent *configureEvent);
extern void HandleWsEnterNotify (XEnterWindowEvent *enterEvent);
extern void HandleWsFocusIn (XFocusInEvent *focusEvent);
extern Boolean HandleWsKeyPress (XKeyEvent *keyEvent);
extern void HandleWsLeaveNotify (XLeaveWindowEvent *leaveEvent);
extern void IdentifyEventContext (XButtonEvent *event, ClientData *pCD, 
				  Context *pContext, int *pPartContext);
extern void InitEventHandling (void);
extern void ProcessClickBPress (XButtonEvent *buttonEvent, ClientData *pCD, 
				Context context, Context subContext);
extern void ProcessClickBRelease (XButtonEvent *buttonEvent, ClientData *pCD, 
				  Context context, Context subContext);
extern void PullExposureEvents (void);
extern int SetupKeyBindings (KeySpec *keySpecs, Window grabWindow, 
			     int keyboardMode, long context);
extern Boolean WmDispatchMenuEvent (XButtonEvent *event);
extern Boolean WmDispatchWsEvent (XEvent *event);
extern void WmGrabButton (Display *display, unsigned int button, 
		unsigned int modifiers, Window grab_window, 
		unsigned int event_mask, Bool owner_events, int pointer_mode, 
		int keyboard_mode, Window confine_to, Cursor cursor);
extern void WmGrabKey (Display *display, int keycode, unsigned int modifiers, 
		Window grab_window, Bool owner_events, int pointer_mode, 
		int keyboard_mode);
extern void WmUngrabButton (Display *display, unsigned int button,
		unsigned int modifiers, Window grab_window);
#ifdef WSM
extern void HandleDtWmClientMessage (XClientMessageEvent *clientEvent);
extern void HandleDtWmRequest (WmScreenData *pSD, XEvent *pev);
extern Boolean ReplayedButtonEvent (XButtonEvent *pevB1, XButtonEvent *pevB2);
#endif /* WSM */
#endif /* _NO_PROTO */
