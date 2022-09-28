/*
 * File:         lock.c $XConsortium: lock.c /main/cde1_maint/1 1995/07/14 20:40:27 drk $
 * Language:     C
 *
 * (C) Copyright 1989, Hewlett-Packard, all rights reserved.
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc.				*
 */

#include <X11/Xlib.h>


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern int _DtGetLock() ;
extern void _DtReleaseLock() ;
extern int _DtTestLock() ;

#else

extern int _DtGetLock( Display *display, char *lock_name) ;
extern void _DtReleaseLock( Display *display, char *lock_name) ;
extern int _DtTestLock( Display *display, char *lock_name) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/


int 
#ifdef _NO_PROTO
_DtGetLock( display, lock_name )
        Display *display ;
        char *lock_name ;
#else
_DtGetLock(
        Display *display,
        char *lock_name )
#endif /* _NO_PROTO */
{
   Atom lock_atom;
   Window selection_owner;
   Window root_window;
  
   /* Do as much processing as possible before grabbing the server. */
   lock_atom = XInternAtom (display, lock_name, False);
   root_window = DefaultRootWindow (display);

   /* Do an atomic test-and-set of the lock.  To make this atomic we
      must grab the server. */
   XGrabServer (display);
   if ((selection_owner = XGetSelectionOwner (display, lock_atom)) == None)
      XSetSelectionOwner (display, lock_atom, root_window, CurrentTime);
   XUngrabServer (display);
   XFlush (display);
  
   /* If the lock was clear then it was successfully acquired. */
   if (selection_owner == None)
      return (1);
   else
      return (0);
}

void 
#ifdef _NO_PROTO
_DtReleaseLock( display, lock_name )
        Display *display ;
        char *lock_name ;
#else
_DtReleaseLock(
        Display *display,
        char *lock_name )
#endif /* _NO_PROTO */
{
   Atom lock_atom;
  
   lock_atom = XInternAtom (display, lock_name, False);
   XSetSelectionOwner (display, lock_atom, None, CurrentTime);
   XFlush (display);  /* added to release lock NOW */
}

int 
#ifdef _NO_PROTO
_DtTestLock( display, lock_name )
        Display *display ;
        char *lock_name ;
#else
_DtTestLock(
        Display *display,
        char *lock_name )
#endif /* _NO_PROTO */
{
   Atom lock_atom;
  
   lock_atom = XInternAtom (display, lock_name, False);
   
   if (XGetSelectionOwner (display, lock_atom) == None)
      return (0);
   else 
      return (1);
}
      
