/* $XConsortium: log.h /main/cde1_maint/2 1995/09/06 08:28:54 lehors $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _LOG_H
#define _LOG_H

#pragma ident "@(#)log.h	1.13 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"
#include "cm.h"
#include "rtable4.h"

#define _DtCMS_VERSION1	1
#define _DtCMS_VERSION4	4
#define _DtCMS_DEFAULT_LOG	"callog"
#define _DtCMS_DEFAULT_BAK	".calbak"
#define _DtCMS_DEFAULT_TMP	".caltmp"
#define _DtCMS_DEFAULT_DEL	".caldel"
#define	_DtCMS_DEFAULT_DIR	"/usr/spool/calendar"
#define	_DtCMS_DEFAULT_MODE	(S_IRUSR|S_IRGRP|S_IWGRP)

typedef enum {
	_DtCmsLogAdd, _DtCmsLogRemove
} _DtCmsLogOps; 

extern CSA_return_code  _DtCmsVerifyLogFileInfo(char *, time_t, long);
extern CSA_return_code  _DtCmsUpdateLogFileInfo(char *, time_t *, long *);

extern CSA_return_code	_DtCmsAppendAppt4ByFN(char*, Appt_4*, _DtCmsLogOps);
extern CSA_return_code	_DtCmsAppendAppt4ByFD(int, Appt_4*, _DtCmsLogOps);

extern CSA_return_code	_DtCmsAppendCalAttrsByFN(char *file,
						  int size,
						  cms_attribute * attrs);
extern CSA_return_code	_DtCmsAppendCalAttrsByFD(int f,
						  int size,
						  cms_attribute * attrs);

extern CSA_return_code	_DtCmsAppendEntryByFN(char *,
						cms_entry *,
						_DtCmsLogOps);
extern CSA_return_code	_DtCmsAppendEntryByFD(int,
						cms_entry *,
						_DtCmsLogOps);

extern CSA_return_code _DtCmsAppendHTableByFN(char *file,
						uint size,
						char **names,
						int *types);

extern CSA_return_code _DtCmsAppendHTableByFD(int fd,
						uint size,
						char **names,
						int *types);

extern CSA_return_code	_DtCmsAppendAccessByFN(char*,
						Access_Entry_4 *);
extern CSA_return_code	_DtCmsAppendAccessByFD(int, Access_Entry_4 *);

extern CSA_return_code	_DtCmsCreateLogV1(char*, char *);

extern CSA_return_code	_DtCmsCreateLogV2(char *owner, char *file);

extern CSA_return_code _DtCmsWriteVersionString(char *file, int version);

extern int _DtCmsSetFileMode(char *file,
				int fd,
				int uid,
				int gid,
				mode_t mode,
				boolean_t changeeuid,
				boolean_t printerr);

extern CSA_return_code	_DtCmsRemoveLog(char *calendar, char *user);

extern char	*_DtCmsGetBakFN	(char*);
extern char	*_DtCmsGetLogFN	(char*);
extern char	*_DtCmsGetTmpFN	(char*);
extern char	*_DtCmsGetDelFN	(char*);

extern boolean_t _DtCmsPrintAppt4(caddr_t data);
extern void	_DtCmsPrintExceptions(int len, int *exceptions);

extern CSA_return_code	_DtCmsGetFileSize(char *calendar, int *size);

extern void	_DtCmsTruncateFile(char *calendar, int size);

#endif
