/*
 *	$Revision: /main/cde1_maint/2 $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *
 *	(c) Copyright 1993,1994,1995 Sun Microsystems, Inc. 
 *		All rights reserved.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define SUN_DB
#ifdef	SUN_DB
#include <sys/utsname.h>
#include <dirent.h>
#include <sys/mman.h>
#include <sys/param.h>
#endif
#include <string.h>
#include <Dt/DbReader.h>
#include "Dt/DtsMM.h"
#include "Dt/DtNlUtils.h"

extern char *strdup(const char *);
static int MMValidateDb(DtDirPaths *dirs, char *suffix);
static int _debug_print_name(char *name, char *label);

#ifdef	DEBUG
static	debug = 1;
#else
static	debug = 0;
#endif
#define	DP	if(debug)

static	const	int	padmem = 10;
typedef	int	(*genfunc)(const void *, const void *);

static	DtDtsMMDatabase	*db_list;
static	caddr_t		mmaped_db = 0;
static	size_t		mmaped_size = 0;
static	int		mmaped_fd = 0;
static	DtDtsMMHeader	*head = 0;

void *
_DtDtsMMGetPtr(int index)
{
	DtShmIntList  int_list;

	if(!mmaped_db)
	{
		_DtDtsMMInit(0);
	}
	int_list = (DtShmIntList)&mmaped_db[sizeof(DtDtsMMHeader)];
	return((void *)&int_list[index]);
}

int
_DtDtsMMGetPtrSize(int index)
{
	DtShmIntList  int_list;

	if(!mmaped_db)
	{
		_DtDtsMMInit(0);
	}
	int_list = (DtShmIntList)&mmaped_db[sizeof(DtDtsMMHeader)];
	return(int_list[index-1]);
}

int *
_DtDtsMMGetDCNameIndex(int *size)
{
	*size = _DtDtsMMGetPtrSize(head->name_list_offset);	
	return((int*)_DtDtsMMGetPtr(head->name_list_offset));
}

int *
_DtDtsMMGetDbName(DtDtsMMDatabase *db, DtShmBoson boson)
{
	DtShmInttab	tab = (DtShmInttab)_DtDtsMMGetPtr(db->nameIndex);
	return((int *)_DtShmFindIntTabEntry(tab, boson));
}

int *
_DtDtsMMGetNoNameIndex(int *size)
{
	if(head->no_name_offset == -1)
	{
		*size = 0;
		return(0);
	}
	*size = _DtDtsMMGetPtrSize(head->no_name_offset);	
	return((int*)_DtDtsMMGetPtr(head->no_name_offset));
}

/* returns the pointer to buffer only name list */
int *
_DtDtsMMGetBufferIndex(int *size)
{
	int	*list = (int*)_DtDtsMMGetNoNameIndex(size);
	*size -= head->buffer_start_index;
	return(&list[head->buffer_start_index]);
}

DtShmInttab
_DtDtsMMGetFileList(void)
{
	DtShmInttab file_index = 
			(DtShmStrtab)_DtDtsMMGetPtr(head->files_offset);
	return(file_index);
}

const char *
_DtDtsMMBosonToString(DtShmBoson boson)
{
	DtShmStrtab str_table;

	if(!mmaped_db)
	{
		_DtDtsMMInit(0);
	}

	str_table = (DtShmStrtab)_DtDtsMMGetPtr(head->str_tbl_offset);
	if(boson == 0)
	{
		return(0);
	}
	else
	{
		return(_DtShmBosonToString(str_table, boson));
	}
}

DtShmBoson
_DtDtsMMStringToBoson(const char *string)
{
	DtShmStrtab str_table;

	if(!mmaped_db)
	{
		_DtDtsMMInit(0);
	}

	if(string && *string)
	{
		str_table = (DtShmStrtab)_DtDtsMMGetPtr(head->str_tbl_offset);
		return(_DtShmStringToBoson(str_table, string));
	}
	else
	{
		return(-1);
	}
}

void
_DtDtsMMPrintFld(int fld, DtDtsMMField *fld_ptr, FILE *fd_in)
{
	const	char	*tmp;
	const	char	*tmpv;
	FILE	*fd = fd_in;

	if(!fd) fd = stdout;

	tmp = _DtDtsMMBosonToString(fld_ptr->fieldName);
	tmpv = _DtDtsMMBosonToString(fld_ptr->fieldValue);
	fprintf(fd, "\t\t[%d]\t%s(%d)\t%s(%d)\n", fld, tmp,fld_ptr->fieldName,
		tmpv?tmpv:"(NULL)", fld_ptr->fieldValue);
}

void
_DtDtsMMPrintRec(int rec, DtDtsMMRecord	*rec_ptr, FILE *fd_in)
{
	int		fld;
	DtDtsMMField	*fld_ptr;
	DtDtsMMField	*fld_ptr_list;
	const	char	*tmp;
	FILE	*fd = fd_in;

	if(!fd) fd = stdout;

	tmp = _DtDtsMMBosonToString(rec_ptr->recordName);
	fprintf(fd, "\tRec[%d] name = %s(%d)\n\t%d Fields\n", rec,
		tmp, rec_ptr->recordName,
		rec_ptr->fieldCount);
	fld_ptr_list = _DtDtsMMGetPtr(rec_ptr->fieldList);
	for(fld = 0; fld < rec_ptr->fieldCount; fld++)
	{
		fld_ptr = &fld_ptr_list[fld];
		_DtDtsMMPrintFld(fld, fld_ptr, fd);
	}
}

void
_DtDtsMMPrintDb(int db, DtDtsMMDatabase *db_ptr, FILE *fd_in)
{
	int		rec;
	DtDtsMMRecord	*rec_ptr;
	DtDtsMMRecord	*rec_ptr_list;
	const	char	*tmp;
	FILE	*fd = fd_in;

	if(!fd) fd = stdout;

	fprintf(fd, "DB[%d] ", db);
	tmp =  _DtDtsMMBosonToString(db_ptr->databaseName);
	fprintf(fd, "name = %s(%d)\n", tmp, db_ptr->databaseName);
	fprintf(fd, "%d Records\n", db_ptr->recordCount);
	rec_ptr_list = _DtDtsMMGetPtr(db_ptr->recordList);
	for(rec = 0; rec < db_ptr->recordCount; rec++)
	{
		rec_ptr = &rec_ptr_list[rec];
		_DtDtsMMPrintRec(rec, rec_ptr, fd);
	}
}

void
_DtDtsMMPrint(FILE *org_fd)
{
	int		db;
	DtDtsMMDatabase	*db_ptr;
	FILE		*fd = org_fd;
	const	char	*tmp;

	if(!mmaped_db)
	{
		_DtDtsMMInit(0);
	}

	for(db = 0; db < head->num_db; db++)
	{
		db_ptr = &db_list[db];
		if(fd == 0)
		{
			chdir("/tmp");
			tmp = _DtDtsMMBosonToString(db_ptr->databaseName);
			if((fd = fopen(tmp, "w")) == NULL)
			{
				perror(tmp);
				continue;
			}
		}
		_DtDtsMMPrintDb(db, db_ptr, fd);
		if(org_fd == 0)
		{
			fclose(fd);
			fd = 0;
		}
	}
}

int
_DtDtsMMCompareRecordNames(DtDtsMMRecord *a, DtDtsMMRecord *b)
{
	return (a->recordName - b->recordName);
}

int
_DtDtsMMCompareFieldNames(DtDtsMMField *a, DtDtsMMField *b)
{
	return (a->fieldName - b->fieldName);
}

#include <Dt/Dts.h>

void
_DtDtsMMInit(int override)
{
	DtDirPaths *dirs = _DtGetDatabaseDirPaths();
	char	*CacheFile = _DtDtsMMCacheName(1);
	if(override)
	{
		_DtDtsMMCreateDb(dirs, CacheFile, override);
		_debug_print_name(CacheFile, "Init");
	}
	else
	{
		int success = _DtDtsMMapDB(CacheFile);
		if(success)
		{
			if(!MMValidateDb(dirs, ".dt"))
			{
				success = 0;
			}
			else
			{
				_debug_print_name(CacheFile, "Mapped");
			}
		}
		if(!success)
		{
			_debug_print_name(CacheFile, "Pirvate");
			free(CacheFile);
			CacheFile = _DtDtsMMCacheName(0);
			_DtDtsMMCreateDb(dirs,
				CacheFile,
				override);
		}
	}
	free(CacheFile);
	_DtFreeDatabaseDirPaths(dirs);
	return;
}

char **
_DtsMMListDb()
{
	int	i;
	char	**list;

	if(!mmaped_db)
	{
		_DtDtsMMInit(0);
	}

	list = (char **)malloc((head->num_db+1)*sizeof(char *));
	for ( i = 0; i < head->num_db; i++ )
	{
		list[i] = (char *)_DtDtsMMBosonToString(db_list[i].databaseName);
	}
	list[i] = 0;
	return(list);
}


DtDtsMMDatabase *
_DtDtsMMGet(const char *name)
{
	int		i;
	DtShmBoson	boson = _DtDtsMMStringToBoson(name);

	
	if(!mmaped_db)
	{
		_DtDtsMMInit(0);
	}
	for(i = 0; i < head->num_db; i++)
	{
		if(db_list[i].databaseName == boson)
		{
			return(&db_list[i]);
		}
	}
	return(NULL);
}


DtDtsMMField *
_DtDtsMMGetField(DtDtsMMRecord *rec, const char *name)
{
	register int i;
	int		fld;
	DtDtsMMField	*fld_ptr;
	DtDtsMMField	*fld_ptr_list;

	/*
	 * Field names have been quarked so quark 'name' and
	 * do a linear search for the quark'ed field name.
	 */
	DtShmBoson	tmp = _DtDtsMMStringToBoson (name);

	fld_ptr_list = _DtDtsMMGetPtr(rec->fieldList);
	for (i = 0; i < rec->fieldCount; i++)
	{
		fld_ptr = &fld_ptr_list[i];
		if (fld_ptr->fieldName == tmp)
		{
			return (fld_ptr);
		}
	}
	return(NULL);
}

const char *
_DtDtsMMGetFieldByName(DtDtsMMRecord *rec, const char *name)
{
	DtDtsMMField	*result;

	result = _DtDtsMMGetField(rec, name);
	if(result)
	{
		return(_DtDtsMMBosonToString(result->fieldValue));
	}
	else
	{
		return(NULL);
	}

}

DtDtsMMRecord *
_DtDtsMMGetRecordByName(DtDtsMMDatabase *db, const char *name)
{
	DtDtsMMRecord	srch;
	DtDtsMMRecord	*result;
	DtDtsMMRecord	*s = &srch;
	register int i;
	DtShmBoson 	name_quark = _DtDtsMMStringToBoson(name);
	DtDtsMMRecord	*rec_ptr;
	DtDtsMMRecord	*rec_ptr_list;

	/*
	 * If the fields are not sorted in alphanumeric order
	 * by name a binary search will fail.  So do the slow but
	 * sure linear search.
	 */
	rec_ptr_list = _DtDtsMMGetPtr(db->recordList);

	for (i = 0; i < db->recordCount; i++)
	{
		rec_ptr = &rec_ptr_list[i];
		if (rec_ptr->recordName == name_quark)
		{
			return (rec_ptr);
		}
	}
	return NULL;
}
int
_DtDtsMMPathHash(DtDirPaths *dirs)
{
	int	pathhash = 0;
	DIR	*dirp;
	struct	dirent	*dp;
	int	suffixLen;
	int	nameLen;
	char	*file_suffix;
	char	*next_path;
	char	*suffix = ".dt";
	int	i;
	char	*cur_dir = getcwd(0,MAXPATHLEN);
	struct	stat	buf;

	for(i = 0; dirs->paths[i] ; i++)
	{
		if(chdir(dirs->paths[i]) == -1)
		{
			continue;
		}
		dirp = opendir (".");
		for(dp = readdir(dirp); dp ; dp = readdir(dirp))
		{
			if ((int)strlen (dp->d_name) >= (int)strlen(suffix))
			{
				suffixLen = DtCharCount(suffix);
				nameLen = DtCharCount(dp->d_name);
				file_suffix = (char *)_DtGetNthChar(dp->d_name,
						nameLen - suffixLen);
				stat(dp->d_name, &buf);
				if (file_suffix &&
					(strcmp(file_suffix, suffix) == 0) &&
					(buf.st_mode&S_IFREG))
				{
					char *c = dirs->paths[i];
					while(*c)
					{
						pathhash += (int)*c;
						c++;
					}
					break;
				}
			}
		}
		closedir(dirp);
	}
	chdir(cur_dir);
	free(cur_dir);
	return(pathhash);
}

char *
_DtDtsMMCacheName(int override)
{
	char	*dsp = getenv("DISPLAY");
	char	*results = 0;
	char	*c;

	if(override && dsp)
	{
		results = malloc(strlen(_DTDTSMMTEMPDIR)+
				 strlen(_DTDTSMMTEMPFILE)+
				strlen(dsp)+3);
		sprintf(results, "%s/%s%s\0",
				_DTDTSMMTEMPDIR,
				_DTDTSMMTEMPFILE,
				dsp);
		c = strchr(results, ':');
		c = strchr(c, '.');
		if(c)
		{
			*c = '\0';
		}
	}
	else
	{
		results = (char *)tempnam(_DTDTSMMTEMPDIR, _DTDTSMMTEMPFILE);
	}
	return(results);
}


int
_DtDtsMMapDB(const char *CacheFile)
{
	struct	stat	buf;
	int	success = FALSE;

	mmaped_fd  = open(CacheFile, O_RDONLY, 0400);
	if(mmaped_fd !=  -1)
	{
		buf.st_uid = 0;
		if(fstat(mmaped_fd, &buf) == 0 && buf.st_uid == getuid())
		{
			mmaped_db = (char *)mmap(NULL,
					buf.st_size,
					PROT_READ,
#if defined(sun) || defined(USL)
					/* MAP_NORESERVE is only supported
					   on sun and novell platforms */
					MAP_SHARED|MAP_NORESERVE,
#else
					MAP_SHARED,
#endif
					mmaped_fd,
					NULL);
			if(mmaped_db != (void *) -1)
			{
			        char *mallocd_db;
				success = TRUE;
				mmaped_size = buf.st_size;
				/* workaround dbx check -access bug 4051381
			        mallocd_db = (char *)XtMalloc( mmaped_size );
				memcpy( mallocd_db, mmaped_db, mmaped_size );
				mmaped_db = mallocd_db;
				*/
				head = (DtDtsMMHeader *)mmaped_db;
				db_list = (DtDtsMMDatabase *)_DtDtsMMGetPtr(head->db_offset);
			}
			else
			{
				perror(CacheFile);
			}
		}
	}
	if(!success)
	{
		mmaped_db = 0;
	}
	return(success);
}

static int
MMValidateDb(DtDirPaths *dirs, char *suffix)
{
	DIR 			*dirp;
	struct dirent		*direntp;
	struct stat		buf;
	struct stat		new_buf;
	int			size = sizeof(buf.st_mtime);
	DtShmBoson		*boson_list = 0;
	time_t			*mtime_list;
	int			count = 0;
	int			i;
	const char		*file;
	int			pathhash = _DtDtsMMPathHash(dirs);

	if(head->pathhash != pathhash)
	{
		return(0);
	}
	count = head->files_count;
	mtime_list = _DtDtsMMGetPtr(head->mtimes_offset);
	boson_list = _DtDtsMMGetPtr(head->files_offset);
	for(i = 0; i < count; i++)
	{
		file = _DtDtsMMBosonToString(boson_list[i]);
		stat(file, &buf);
		if(mtime_list[i]  != buf.st_mtime)
		{
			return(0);
		}
	}
	return(1);

}

char *
_DtDtsMMExpandValue(const char *value)
{
	char *newval;

	if(!value)
	{
		return NULL;
	}
	newval = XtNewString(value);

	_DtDbFillVariables(&newval,0);
	return(newval);
}

void
_DtDtsMMSafeFree(char *value)
{
	if(value && !_DtDtsMMIsMemory(value))
	{
		free(value);
	}
}

int
_DtDtsMMIsMemory(const char *value)
{
	if((caddr_t)value < mmaped_db || (caddr_t)value > mmaped_db+mmaped_size)
	{
		return(0);
	}
	else
	{
		return(1);
	}
}

int
_DtDtsMMUnLoad()
{
	int	error = 0;

	_DtDtsClear();
	if(mmaped_db == 0)
	{
		return(error);
	}
	if(munmap(mmaped_db, mmaped_size) == -1)
	{
		perror("munmap of dts_cache file");
		error = -1;
	}
	if(close(mmaped_fd) == -1)
	{
		perror("close of dts_cache file");
	}

	db_list = 0;
	mmaped_db = 0;
	mmaped_size = 0;
	mmaped_fd = 0;
	head = 0;
	return(error);
}

#include "Dt/UserMsg.h"

static int
_debug_print_name(char *name, char *label)
{
	static char	*db = (char *)-1;

	if(db == (char *)-1)
	{
		db = getenv("MMAP_DEBUG");
	}

	if(db)
		_DtSimpleError(db,
			DtInformation,
			NULL,
			"%s - db name = %s\n", label,
			name);
	return(0);
}

