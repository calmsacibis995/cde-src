/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * @(#)tt_isam.h	1.11 97/06/05
 *
 * TT version of the NetISAM isam.h file.
 *
 * Copyright (c) 1992 by Sun Microsystems, Inc.
 */

#ifndef  _TT_ISAM_H
#define  _TT_ISAM_H

#include <isam.h>

extern "C" { int isaddindex(int, keydesc*); }
extern "C" { int isbuild(const char*, int, keydesc*, int); }
extern "C" { int isclose(int); }
extern "C" { int iscntl(int, int, ...); }
extern "C" { int isdelcurr(int); }
extern "C" { int isdelrec(int, long); }
extern "C" { int iserase(const char*); }
extern "C" { int isfsync(int); }
extern "C" { int isindexinfo(int, keydesc *, int); }
extern "C" { int isopen(const char*, int); }
extern "C" { int isread(int, char*, int); }
extern "C" { int isrename(char*, char*); }
extern "C" { int isrewcurr(int, char*); }
extern "C" { int isrewrec(int, long, char*); }
extern "C" { int isstart(int, keydesc*, int, char*, int); }
extern "C" { int iswrite(int, char*); }
extern "C" { int isgarbage(char *); }
extern "C" { int isgetcurpos(int isfd, int *len, char **buf); }
extern "C" { int issetcurpos(int isfd, char *buf); }
extern "C" { int isrepair(const char *, int); }

#endif /* _TT_ISAM_H */
