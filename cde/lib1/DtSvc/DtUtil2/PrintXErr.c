/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

/* Copyright    Massachusetts Institute of Technology    1985, 1986, 1987 */

/*
 * $XConsortium: PrintXErr.c /main/cde1_maint/1 1995/07/14 20:37:50 drk $
 */

/* **  (c) Copyright Hewlett-Packard Company, 1990.*/

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*Lifted from xlib code.  How to print a reasonably complete message */
/*and NOT exit.*/
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/*$XConsortium: PrintXErr.c /main/cde1_maint/1 1995/07/14 20:37:50 drk $*/

#include <X11/Xlib.h>
#include <X11/Xlibint.h>

int 
#ifdef _NO_PROTO
_DtPrintDefaultError( dpy, event, msg )
        Display *dpy ;
        XErrorEvent *event ;
        char *msg ;
#else
_DtPrintDefaultError(
        Display *dpy,
        XErrorEvent *event,
        char *msg )
#endif /* _NO_PROTO */
{
#define BUFSIZ 1024
    char buffer[BUFSIZ], fp[BUFSIZ];
    char mesg[BUFSIZ];
    char number[32];
    char *mtype = "XlibMessage";
    register _XExtension *ext = (_XExtension *)NULL;
    XGetErrorText(dpy, event->error_code, buffer, BUFSIZ);
    XGetErrorDatabaseText(dpy, mtype, "XError", "X Error", mesg, BUFSIZ);
    (void) sprintf(msg, "%s:  %s\n  ", mesg, buffer);
    XGetErrorDatabaseText(dpy, mtype, "MajorCode", "Request Major code %d",
        mesg, BUFSIZ);
    (void) sprintf(fp, mesg, event->request_code);
    strcat(msg, fp);
    if (event->request_code < 128) {
        sprintf(number, "%d", event->request_code);
        XGetErrorDatabaseText(dpy, "XRequest", number, "", buffer, BUFSIZ);
    } else {
        for (ext = dpy->ext_procs;
             ext && (ext->codes.major_opcode != event->request_code);
             ext = ext->next)
          ;
        if (ext)
            strcpy(buffer, ext->name);
        else
            buffer[0] = '\0';
    }
    (void) sprintf(fp, " (%s)\n  ", buffer);
    strcat(msg, fp);
    if (event->request_code >= 128) {
        XGetErrorDatabaseText(dpy, mtype, "MinorCode", "Request Minor code %d",
                              mesg, BUFSIZ);
        (void) sprintf(fp, mesg, event->minor_code);
        strcat(msg, fp);
        if (ext) {
            sprintf(mesg, "%s.%d", ext->name, event->minor_code);
            XGetErrorDatabaseText(dpy, "XRequest", mesg, "", buffer, BUFSIZ);
            (void) sprintf(fp, " (%s)", buffer);
        }
        strcat(fp, "\n  ");
        strcat(msg, fp);
    }
    if (event->error_code >= 128) {
        /* kludge, try to find the extension that caused it */
        buffer[0] = '\0';
        for (ext = dpy->ext_procs; ext; ext = ext->next) {
            if (ext->error_string)
                (*ext->error_string)(dpy, event->error_code, &ext->codes,
                                     buffer, BUFSIZ);
            if (buffer[0])
                break;
        }
        if (buffer[0])
            sprintf(buffer, "%s.%d", ext->name,
                    event->error_code - ext->codes.first_error);
        else
            strcpy(buffer, "Value");
        XGetErrorDatabaseText(dpy, mtype, buffer, "Value 0x%x", mesg, BUFSIZ);
        if (*mesg) {
            (void) sprintf(fp, mesg, event->resourceid);
            strcat(fp, "\n  ");
            strcat(msg, fp);
        }
    } else if ((event->error_code == BadWindow) ||
               (event->error_code == BadPixmap) ||
               (event->error_code == BadCursor) ||
               (event->error_code == BadFont) ||
               (event->error_code == BadDrawable) ||
               (event->error_code == BadColor) ||
               (event->error_code == BadGC) ||
               (event->error_code == BadIDChoice) ||
               (event->error_code == BadValue) ||
               (event->error_code == BadAtom)) {
        if (event->error_code == BadValue)
            XGetErrorDatabaseText(dpy, mtype, "Value", "Value 0x%x",
                                  mesg, BUFSIZ);
        else if (event->error_code == BadAtom)
            XGetErrorDatabaseText(dpy, mtype, "AtomID", "AtomID 0x%x",
                                  mesg, BUFSIZ);
        else
            XGetErrorDatabaseText(dpy, mtype, "ResourceID", "ResourceID 0x%x",
                                  mesg, BUFSIZ);
        (void) sprintf(fp, mesg, event->resourceid);
	strcat(fp, "\n  ");
        strcat(msg, fp);
    }    
    XGetErrorDatabaseText(dpy, mtype, "ErrorSerial", "Error Serial #%d",
        mesg, BUFSIZ);
    (void) sprintf(fp, mesg, event->serial);
    strcat(fp, "\n  ");
    strcat(msg, fp);
    XGetErrorDatabaseText(dpy, mtype, "CurrentSerial", "Current Serial #%d",
        mesg, BUFSIZ);
    (void) sprintf(fp, mesg, dpy->request);
    strcat(fp, "\n  ");
    strcat(msg, fp);
    if (event->error_code == BadImplementation) return 0;
    return 1;
}
