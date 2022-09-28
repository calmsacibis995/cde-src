/*                                                                            *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                           *
 * (c) Copyright 1993, 1994 International Business Machines Corp.             *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                            *
 * (c) Copyright 1993, 1994 Novell, Inc.                                      *
 */
/*Add a string to the XA_RESOURCE_MANAGER*/
/*
 *                        COPYRIGHT 1987
 *                 DIGITAL EQUIPMENT CORPORATION
 *                     MAYNARD, MASSACHUSETTS
 *                      ALL RIGHTS RESERVED.
 *
 * THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT NOTICE AND
 * SHOULD NOT BE CONSTRUED AS A COMMITMENT BY DIGITAL EQUIPMENT CORPORATION.
 * DIGITAL MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR
 * ANY PURPOSE.  IT IS SUPPLIED "AS IS" WITHOUT EXPRESS OR IMPLIED WARRANTY.
 *
 * IF THE SOFTWARE IS MODIFIED IN A MANNER CREATING DERIVATIVE COPYRIGHT RIGHTS,
 * APPROPRIATE LEGENDS MAY BE PLACED ON THE DERIVATIVE WORK IN ADDITION TO THAT
 * SET FORTH ABOVE.
 *
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Digital Equipment Corporation not be
 * used in advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.
 */

/*Lifted from xrdb(1X)*/
/* -*-C-*-
*******************************************************************************
*
* File:         addToResource.c
* RCS:          $XConsortium: addToRes.c /main/cde1_maint/3 1995/10/08 22:22:14 pascale $
* Description:  Source code for adding strings to RESOURCE_PROPERTY on 
                  default root window
* Author:       DEC, Robert Williams
* Created:      Thu Apr 26 14:42:08 PDT 1990
* Modified:	Kim Dronesen
* Language:     C
* Package:      N/A
* Status:       Experimental (Do Not Distribute)
*
* (C) Copyright 1990, Hewlett-Packard, all rights reserved.
*
*******************************************************************************
*/
/*$XConsortium: addToRes.c /main/cde1_maint/3 1995/10/08 22:22:14 pascale $ */

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xos.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <Dt/DtP.h>
#include <Dt/SessionM.h>

/****************************************/
/* Global variables */
/****************************************/

typedef struct _Entry {
    char *tag, *value;
    int lineno;
    Bool usable;
} Entry;
typedef struct _Buffer {
    char *buff;
    int  room, used;
    Bool freebuff;
} Buffer;
typedef struct _Entries {
    Entry *entry;
    int   room, used;
} Entries;

#define INIT_BUFFER_SIZE 10000
#define INIT_ENTRY_SIZE 500


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Buffer * _DtAllocBuffer() ;
extern void _DtFreeBuffer() ;
extern void _DtAppendToBuffer() ;
extern void _DtAppendEntryToBuffer() ;

extern Entries * _DtAllocEntries() ;
extern void _DtFreeEntries() ;

static void _DtAddEntry() ;
static int _DtCompareEntries() ;
static void _DtAppendEntryToBuffer() ;
static char * _DtFindFirst() ;
static void _DtGetEntries() ;
static void _DtMergeEntries() ;
static void _DtAddToResProp() ;

#else

static Buffer * _DtAllocBuffer(char **buff);
static void _DtFreeBuffer(Buffer *b);
static void _DtAppendToBuffer( 
                        register Buffer *b,
                        char *str,
                        register int len) ;
static void _DtAppendEntryToBuffer( 
                        register Buffer *buffer,
                        Entry entry) ;

static Entries * _DtAllocEntries( void) ;
static  void _DtFreeEntries( Entries *) ;
static void _DtAddEntry( 
                        register Entries *e,
                        Entry entry) ;
static int _DtCompareEntries( 
                        Entry *e1,
                        Entry *e2) ;
static char * _DtFindFirst( 
                        register char *string,
                        register char dest) ;
static void _DtGetEntries( 
                        register Entries *entries,
                        Buffer *buff,
                        int dosort) ;
static void _DtMergeEntries( 
                        Buffer *buffer,
                        Entries new,
                        Entries old) ;
static void _DtAddToResProp(
                        Display *dpy,
                        unsigned int id,
                        Entries db) ;
static void _getWinProp(
                        Display *dpy,
                        unsigned int id,
                        Window *win,
                        Atom *prop);

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

/****************************************/
/*The meat*/
/****************************************/

static Buffer *
#ifdef _NO_PROTO
_DtAllocBuffer( buff )
        char **buff;
#else
_DtAllocBuffer(
        char **buff )
#endif /* _NO_PROTO */
{
    Buffer *b = (Buffer *)malloc(sizeof(Buffer));
    b->room = INIT_BUFFER_SIZE;
    b->buff = buff ? *buff : (char *)malloc(INIT_BUFFER_SIZE*sizeof(char));
    b->used = buff && *buff ? strlen(*buff) : 0;
    b->freebuff = buff ? False : True;
    return(b);
}

static void
#ifdef _NO_PROTO
_DtFreeBuffer( b )
        Buffer *b;
#else
_DtFreeBuffer(
        Buffer *b)
#endif /* _NO_PROTO */
{
    if (b->freebuff == True) free(b->buff);
    free(b);
}

static void 
#ifdef _NO_PROTO
_DtAppendToBuffer( b, str, len )
        register Buffer *b ;
        char *str ;
        register int len ;
#else
_DtAppendToBuffer(
        register Buffer *b,
        char *str,
        register int len )
#endif /* _NO_PROTO */
{
    while (b->used + len > b->room) {
        b->buff = (char *)realloc(b->buff, 2*b->room*(sizeof(char)));
        b->room *= 2;
    }
    strncpy(b->buff + b->used, str, len);
    b->used += len;
}

static Entries *
#ifdef _NO_PROTO
_DtAllocEntries( )
#else
_DtAllocEntries( void )
#endif /* _NO_PROTO */
{
    Entries *e = (Entries *)malloc(sizeof(Entries));
    e->room = INIT_ENTRY_SIZE;
    e->used = 0;
    e->entry = (Entry *)malloc(INIT_ENTRY_SIZE*sizeof(Entry));
    return(e);
}

static void
#ifdef _NO_PROTO
_DtFreeEntries( e)
    Entries *e;
#else
_DtFreeEntries( 
    Entries *e)
#endif /* _NO_PROTO */
{
    int n = 0;

    while (n < e->used)
    {
	free(e->entry[n].tag);
	free(e->entry[n].value);
	n++;
    }
    free(e->entry);
    free(e);
}

static void 
#ifdef _NO_PROTO
_DtAddEntry( e, entry )
        register Entries *e ;
        Entry entry ;
#else
_DtAddEntry(
        register Entries *e,
        Entry entry )
#endif /* _NO_PROTO */
{
    register int n;

    for (n = 0; n < e->used; n++)
    {
        if (strcmp(e->entry[n].tag, entry.tag) == 0)
	{ /* overwrite old entry  - free its memory first*/
	    free(e->entry[n].tag);
	    free(e->entry[n].value);
            e->entry[n] = entry;
            return ;  /* ok to leave, now there's only one of each tag in db */
        }
    }

    if (e->used == e->room) {
        e->entry = (Entry *)realloc(e->entry, 2*e->room*(sizeof(Entry)));
        e->room *= 2;
    }
    entry.usable = True;
    e->entry[e->used++] = entry;
}

static int 
#ifdef _NO_PROTO
_DtCompareEntries( e1, e2 )
        Entry *e1 ;
        Entry *e2 ;
#else
_DtCompareEntries(
        Entry *e1,
        Entry *e2 )
#endif /* _NO_PROTO */
{
    return strcmp(e1->tag, e2->tag);
}

static void 
#ifdef _NO_PROTO
_DtAppendEntryToBuffer( buffer, entry )
        register Buffer *buffer ;
        Entry entry ;
#else
_DtAppendEntryToBuffer(
        register Buffer *buffer,
        Entry entry )
#endif /* _NO_PROTO */
{
    _DtAppendToBuffer(buffer, entry.tag, strlen(entry.tag));
    _DtAppendToBuffer(buffer, ":\t", 2);
    _DtAppendToBuffer(buffer, entry.value, strlen(entry.value));
    _DtAppendToBuffer(buffer, "\n", 1);
}

static char * 
#ifdef _NO_PROTO
_DtFindFirst( string, dest )
        register char *string ;
        register char dest ;
#else
_DtFindFirst(
        register char *string,
        register char dest )
#endif /* _NO_PROTO */
{
    int len;

    for (;;) {
        if((len = mblen(string, MB_CUR_MAX)) > 1) {
            string += len;
            continue;
        }
        if (*string == '\0')
            return NULL;
        if (*string == '\\') {
            if (*++string == '\0')
                return NULL;
        }
        else if (*string == dest)
            return string;
        string++;
    }
}

static void 
#ifdef _NO_PROTO
_DtGetEntries( entries, buff, dosort )
        register Entries *entries ;
        Buffer *buff ;
        int dosort ;
#else
_DtGetEntries(
        register Entries *entries,
        Buffer *buff,
        int dosort )
#endif /* _NO_PROTO */
{
    register char *line, *colon, *temp, *str, *temp2;
    Entry entry;
    register int length;
    int lineno = 0;

    str = buff->buff;
    if (!str) return;
    for (; str < buff->buff + buff->used; str = line + 1)
    {
        line = _DtFindFirst(str, '\n');
        lineno++;
        if (line == NULL)
            break;
        if (str[0] == '!')
            continue;
        if (str[0] == '\n')
            continue;
        if (str[0] == '#')
	{
            int dummy;
            if (sscanf (str, "# %d", &dummy) == 1) lineno = dummy - 1;
            continue;
        }
        for (temp = str;
             *temp && *temp != '\n' && isascii(*temp) && isspace((u_char)*temp);
             temp++) ;
        if (!*temp || *temp == '\n') continue;

        colon = _DtFindFirst(str, ':');
        if (colon == NULL)
            break;
        if (colon > line)
	{
            line[0] = '\0';
            fprintf (stderr,
                     "%s:  colon missing on line %d, ignoring entry \"%s\"\n",
                     "dtprefs", lineno, str);
            continue;
        }

	temp2 = str;
        while (temp2[0] == ' ' || temp2[0] == '\t')
	{
            temp2++;
        }
        temp = (char *)malloc((length = colon - temp2) + 1);
        strncpy(temp, temp2, length);
        temp[length] = '\0';
        while (temp[length-1] == ' ' || temp[length-1] == '\t')
            temp[--length] = '\0';
        entry.tag = temp;
	

	temp2 = colon + 1;
        while (temp2[0] == ' ' || temp2[0] == '\t')
	{
            temp2++;
        }
        temp = (char *)malloc((length = line - temp2) + 1);
        strncpy(temp, temp2, length);
        temp[length] = '\0';
        entry.value = temp;
        entry.lineno = lineno;

        _DtAddEntry(entries, entry);
    }
    
    if (dosort && (entries->used > 0))
      qsort(entries->entry, entries->used, sizeof(Entry), 
	    (int (*)(const void *, const void *))_DtCompareEntries);
}

static void 
#ifdef _NO_PROTO
_DtMergeEntries( buffer, new, old )
        Buffer *buffer ;
        Entries new ;
        Entries old ;
#else
_DtMergeEntries(
        Buffer *buffer,
        Entries new,
        Entries old )
#endif /* _NO_PROTO */
{
    int n, o, cmp;

    buffer->used = 0;
    n = o = 0;
    while ((n < new.used) && (o < old.used))
    {
        cmp = strcmp(new.entry[n].tag, old.entry[o].tag);
        if (cmp > 0)
	{
            _DtAppendEntryToBuffer(buffer, old.entry[o]);
	    o++;
	}
        else
	{
            _DtAppendEntryToBuffer(buffer, new.entry[n]);
	    n++;
            if (cmp == 0)
	    {
                o++;
	    }
        }
    }
    
    while (n < new.used)
    {
        _DtAppendEntryToBuffer(buffer, new.entry[n]);
	n++;
    }
    while (o < old.used)
    {
        _DtAppendEntryToBuffer(buffer, old.entry[o]);
	o++;
    }
    
    _DtAppendToBuffer(buffer, "\0", 1);
}

static void
#ifdef _NO_PROTO
_getWinProp( dpy, id, win, prop)
        Display *dpy;
        unsigned int id;
        Window *win;
        Atom *prop;
#else
_getWinProp(
        Display *dpy,
        unsigned int id,
        Window *win,
        Atom *prop)
#endif
{
  static Bool init = True;
  static Window winprop;
  static Atom xa_resmgr;
  static Atom xa_prefs;

  if (init == True)
  {
    winprop = XRootWindow(dpy, 0);
    xa_resmgr = XA_RESOURCE_MANAGER;
    xa_prefs = XInternAtom (dpy, _XA_DT_SM_PREFERENCES, False);
    init = False;
  }

  *win = winprop;
  *prop = id == _DT_ATR_RESMGR ? xa_resmgr : xa_prefs;
}


static void 
#ifdef _NO_PROTO
_DtAddToResProp( dpy, id, db )
        Display *dpy ;
        unsigned int id;
        Entries db;
#else
_DtAddToResProp(
        Display *dpy,
        unsigned int id,
        Entries db)
#endif /* _NO_PROTO */
{
    char *xdefs;
    Buffer *oldBuffer, *newBuffer;
    Entries *oldDB;
    int                 defStatus;
    Atom                actualType;
    int                 actualFormat;
    unsigned long       nitems, leftover;
    Window              win;
    Atom                prop;

   /*
    * Get window and property
    */
    _getWinProp(dpy, id, &win, &prop);
    if (win == (Window)0)
    {
      return;
    }

   /*
    * Get resource database from specified window and property.
    */
    defStatus = XGetWindowProperty(dpy, win,
				   prop, 0L,
				   100000000L,False,XA_STRING,&actualType,
				   &actualFormat,&nitems,&leftover,
				   (unsigned char**) &xdefs);


   /*
    * Allocate oldBuffer and init from resource database string
    */
    oldBuffer = _DtAllocBuffer(&xdefs);

   /*
    * Allocate oldDB
    */
    oldDB = _DtAllocEntries();

   /*
    * Convert oldBuffer to oldDB.
    */
    _DtGetEntries(oldDB, oldBuffer, 1);

   /*
    * Init empty newBuffer, then populate by merging db into oldDB.
    */
    newBuffer = _DtAllocBuffer(NULL);
    _DtMergeEntries(newBuffer, db, *oldDB);

   /*
    * Finally, store newBuffer into resource database.
    */
    XChangeProperty (dpy, win, prop,
		     XA_STRING, 8, PropModeReplace,
		     (unsigned char *)newBuffer->buff, newBuffer->used);

    XSync(dpy, False);

   /*
    * Free buffer memory
    */
    if (oldBuffer->buff) XFree(oldBuffer->buff);
    _DtFreeBuffer(oldBuffer);
    _DtFreeBuffer(newBuffer);
    _DtFreeEntries(oldDB);
}

char *
#ifdef _NO_PROTO
_DtGetResString( dpy, id)
        Display *dpy ;
        unsigned int id;
#else
_DtGetResString(
        Display *dpy,
        unsigned int id)
#endif /* _NO_PROTO */
{
    char *xdefs;
    Buffer *oldBuffer, *newBuffer;
    Entries *oldDB;
    int                 defStatus;
    Atom                actualType;
    int                 actualFormat;
    unsigned long       nitems, leftover;
    Window              win;
    Atom                prop;

   /*
    * Get window and property
    */
    _getWinProp(dpy, id, &win, &prop);
    if (win == (Window)0)
    {
      return NULL;
    }

   /*
    * Get resource database from specified window and property.
    */
    defStatus = XGetWindowProperty(dpy, win,
				   prop, 0L,
				   100000000L,False,XA_STRING,&actualType,
				   &actualFormat,&nitems,&leftover,
				   (unsigned char**) &xdefs);

    return(xdefs);
}

void 
#ifdef _NO_PROTO
_DtAddToResource( dpy, data )
        Display *dpy ;
        char *data ;
#else
_DtAddToResource(
        Display *dpy,
        char *data )
#endif /* _NO_PROTO */
{
  _DtAddResString( dpy, data, _DT_ATR_RESMGR|_DT_ATR_PREFS);
}

void 
#ifdef _NO_PROTO
_DtAddResString( dpy, data, flags )
        Display *dpy ;
        char *data ;
        unsigned int flags;
#else
_DtAddResString(
        Display *dpy,
        char *data, 
        unsigned int flags)
#endif /* _NO_PROTO */
{
    char *xdefs;
    int i;
    Buffer *buffer; 
    Entries *newDB;
    int                 defStatus;
    Atom                actualType;
    int                 actualFormat;
    unsigned long       nitems, leftover;

    if((data == NULL) || (*data == NULL))
    {
	return;
    }
  
   /*
    * Init buffer with input data
    */ 
    buffer = _DtAllocBuffer(&data); 

   /*
    * Init, then populate, newDB from buffer
    */
    newDB = _DtAllocEntries();
    _DtGetEntries(newDB, buffer, 1);

    if (flags & _DT_ATR_RESMGR)
    {
     /*
      * Merge newDB into RESOURCE_MANAGER
      */
      _DtAddToResProp(dpy, _DT_ATR_RESMGR, *newDB);
    }

    if (flags & _DT_ATR_PREFS)
    {
     /*
      * Merge newDB into _DT_SM_PREFERENCES
      */
      _DtAddToResProp(dpy, _DT_ATR_PREFS, *newDB);
    }

   /*
    * Free objects
    */
    _DtFreeBuffer(buffer);
    _DtFreeEntries(newDB);
}
