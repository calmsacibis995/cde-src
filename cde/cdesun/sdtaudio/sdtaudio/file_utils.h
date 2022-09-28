/*
 * (c) Copyright 1996 Sun Microsystems, Inc.
 */


#pragma ident "@(#)file_utils.h	1.3 96/10/16 SMI"


#ifndef _FILE_UTILS_H_
#define _FILE_UTILS_H_

Boolean		SaveNeeded(WindowData *);
Boolean         CanRead(Widget, char *);
Boolean         CanWrite(Widget, char *);
void            ConvertCompoundToChar (XmString, char *);

#endif				/* _FILE_UTILS_H_ */
