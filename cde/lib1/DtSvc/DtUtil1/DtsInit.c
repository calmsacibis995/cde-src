/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*
 * +SNOTICE
 * 
 * $XConsortium: DtsInit.c /main/cde1_maint/4 1995/10/04 11:59:35 gtsang $
 * 
 * RESTRICTED CONFIDENTIAL INFORMATION:
 * 
 * The information in this document is subject to special restrictions in a
 * confidential disclosure agreement bertween HP, IBM, Sun, USL, SCO and
 * Univel.  Do not distribute this document outside HP, IBM, Sun, USL, SCO,
 * or Univel wihtout Sun's specific written approval.  This documment and all
 * copies and derivative works thereof must be returned or destroyed at Sun's
 * request.
 * 
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 * 
 * +ENOTICE
 */
#include <stdio.h>
#include <sys/types.h>

#ifdef __hpux
#include <ndir.h>
#else

#if defined(sun) || defined(USL) || defined(sco) || defined(__uxp__)
#include <dirent.h>
#else
#include <sys/dir.h>
#endif				/* sun || USL */

#endif				/* __hpux */

#include <ctype.h>
#include <string.h>

#ifdef NLS16
#include <limits.h>
#endif

#include <sys/stat.h>
#include <sys/param.h>		/* MAXPATHLEN, MAXHOSTNAMELEN */
#include <Dt/DbReader.h>
#include <Dt/DtsDb.h>
#include <Dt/Dts.h>

extern char	*strdup(const char *);

static int      cur_dc_count = 0;
static int      cur_da_count = 0;
#define	_DtFT_NUM_FIELDS	20

int
_DtDtsNextDCSeq()
{
	return(cur_dc_count++);
}

int
_DtDtsNextDASeq()
{
	return(cur_da_count++);
}

void
_DtDtsSeqReset()
{
	cur_dc_count = 0;
	cur_da_count = 0;
}
void
_DtDtsDCConverter(DtDtsDbField * fields,
	       DtDbPathId pathId,
	       char *hostPrefix,
	       Boolean rejectionStatus)
{
	DtDtsDbDatabase *db = _DtDtsDbGet(DtDTS_DC_NAME);
	DtDtsDbRecord  *rec;
	DtDtsDbField   *fld;
	int             i = 0;

	while (fields[i].fieldName && fields[i].fieldValue)
	{
		if (i == 0)
		{
			if(rec=_DtDtsDbGetRecordByName(db,fields[i].fieldValue))
			{
				char *value = _DtDtsDbGetFieldByName(rec,DtDTS_DA_IS_SYNTHETIC);
				/*
				 * Check if the record is SYNTHETIC --
				 * if so then replace it with this real
				 * definition -- otherwise return.
				 */
				if (value && !strcmp(value,"True") )
				{
					/* free up the current record */
					_DtDtsDbDeleteRecord(rec,db);
				}
				else
					return;
			}
			rec = _DtDtsDbAddRecord(db);
			rec->recordName = XrmStringToQuark(fields[i].fieldValue);
			rec->seq = _DtDtsNextDCSeq();
			rec->pathId = (int)pathId;
		}
		else
		{
			fld = _DtDtsDbAddField(rec);
			fld->fieldName = fields[i].fieldName;
			fld->fieldValue = strdup(fields[i].fieldValue);
		}
		i++;
	}
}

void
_DtDtsDAConverter(DtDtsDbField * fields,
	       DtDbPathId pathId,
	       char *hostPrefix,
	       Boolean rejectionStatus)
{
	DtDtsDbDatabase *db = _DtDtsDbGet(DtDTS_DA_NAME);
	DtDtsDbRecord  *rec;
	DtDtsDbField   *fld;
	int             i = 0;


	while (fields[i].fieldName && fields[i].fieldValue)
	{
		if (i == 0)
		{
			if(rec = _DtDtsDbGetRecordByName(db, fields[i].fieldValue))
			{
				char *value = _DtDtsDbGetFieldByName(rec,DtDTS_DA_IS_SYNTHETIC);
				/*
				 * Check if the record is SYNTHETIC --
				 * if so then replace it with this real
				 * definition -- otherwise return.
				 */
				if (value && !strcmp(value,"True") )
				{
					/* free up the current record */
					_DtDtsDbDeleteRecord(rec,db);
				}
				else
					return;
			}
			rec = _DtDtsDbAddRecord(db);
			rec->recordName = XrmStringToQuark(fields[i].fieldValue);
			rec->seq = _DtDtsNextDASeq();
			fld = _DtDtsDbAddField(rec);
			fld->fieldName = XrmStringToQuark(DtDTS_DA_DATA_HOST);
			fld->fieldValue = hostPrefix?strdup(hostPrefix):(char *) 0;
			rec->pathId = (int)pathId;
		}
		else
		{
			fld = _DtDtsDbAddField(rec);
			fld->fieldName = fields[i].fieldName;
			fld->fieldValue = strdup(fields[i].fieldValue);
		}
		i++;
	}
}


/******************************************************************************
 *
 * DtDtsLoadDataTypes -
 *
 *   Reads in the file types and action databases.
 *
 *   Also initializes the variable DtMaxFileTypes to the number
 *   of entries in the file types database.
 *
 * MODIFIED:
 *
 *   DtMaxFtFileTypes
 *   DtMaxFileTypes - set to the number of real filetypes 
 *	 ( Holdovers from previous filetypes stuff -- still used by
 *	   some clients. i.e. dtfile )
 *
 *****************************************************************************/

void
DtDtsLoadDataTypes(void)
{
	/* with new mmap database this function is not needed to 
	 * load the database. Just to initialize it.
	 */

	_DtDtsMMUnLoad();
}
