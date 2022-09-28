
/*
 *	$XConsortium: merge_c.c /main/cde1_maint/3 1995/10/16 14:26:07 rcs $
 *
 *	@(#)merge_c.c	1.9 27 Mar 1995	cde_app_builder/src/abmf
 *
 * 	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement between
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel without
 *	Sun's specific written approval.  This document and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 */


/*
 *  merge_c.c - merge C source files with "magic" comments
 */

#include <assert.h>
#include "parse_cP.h"
#include "merge_cP.h"

/*************************************************************************
**                                                                      **
**       Constants (#define and const)					**
**                                                                      **
**************************************************************************/

/*************************************************************************
**                                                                      **
**       Private Functions (C declarations and macros)			**
**                                                                      **
**************************************************************************/

static int	merge_files_by_segment(
			File		oldFile, 
			CSegArray	oldSegs, 
			STRING		oldFileName,
			File		newFile, 
			CSegArray	newSegs, 
			STRING		newFileName,
			File		*mergedFileOutPtr,
			File		*deltaFileOutPtr,
			STRING		mainFuncAlternateName
		);

static int	match_all_segments(
			CSegArray	newSegs, 
			CSegArray	oldSegs,
			STRING		mainFuncAlternateName
		);

static CSeg	match_segment(
			CSeg 		oldSeg, 
			CSegArray	oldSegArray,
			CSegArray	newSegArray,
			STRING		mainFuncAlternateName
);

static CSeg	match_named_segment(
			STRING 		segName, 
			CSegArray	segArray
		);

/*************************************************************************
**                                                                      **
**       Data     	                                                **
**                                                                      **
**************************************************************************/

/*************************************************************************
**                                                                      **
**       Function Definitions                                           **
**                                                                      **
**************************************************************************/

int	
abmfP_merge_c_files(
			File	oldFile,
			STRING	oldFileName,
			BOOL	oldFileIsTmp,
			File	newFile,
			STRING	newFileName,
			BOOL	newFileIsTmp,
			File	*mergedFileOutPtr,
			File	*deltaFileOutPtr,
			STRING	mainFuncAlternateName
)
{
    int		return_value = 0;
    int		rc = 0;				/* return code */
    CSegArray	newSegs = NULL;
    CSegArray	oldSegs = NULL;
    File	mergedFile = NULL;
    File	deltaFile = NULL;
    char	fileName[MAXPATHLEN+1];
    *fileName = 0;

    /*
    if (debugging())
    {
	long	off = 0;
	FILE	*fp = NULL;
	int	c = 0;

	if (oldFile != NULL)
	{
	    fp = util_fopen_locked("merge-old", "w");
	    assert(fp != NULL);
	    off = ftell(oldFile);
	    while ((c = fgetc(oldFile)) != EOF)
	    {
		fputc(c, fp);
	    }
	    util_fclose(fp);
	    fseek(oldFile, off, SEEK_SET);
	}

	if (newFile != NULL)
	{
	    fp = util_fopen_locked("merge-new", "w");
	    assert(fp != NULL);
	    off = ftell(newFile);
	    while ((c = fgetc(newFile)) != EOF)
	    {
		fputc(c, fp);
	    }
	    util_fclose(fp);
	    fseek(newFile, off, SEEK_SET);
	}
    }
    */

    if (oldFile != NULL)
    {
	*fileName = 0;
	if (oldFileIsTmp)
	{
	    strcat(fileName, "tmpfile::");
	}
	strcat(fileName, oldFileName);
	rc = abmfP_parse_c_file(oldFile, fileName, &oldSegs);
	return_if_err(rc,rc);
    }
    if (newFile != NULL)
    {
	*fileName = 0;
	if (newFileIsTmp)
	{
	    strcat(fileName, "tmpfile::");
	}
	strcat(fileName, newFileName);
        rc = abmfP_parse_c_file(newFile, fileName, &newSegs);
	return_if_err(rc,rc);
    }

    rc = merge_files_by_segment(
			oldFile, oldSegs, oldFileName,
			newFile, newSegs, newFileName,
			&mergedFile, 
			&deltaFile,
			mainFuncAlternateName);
    if (rc < 0)
    {
	return_value = rc;
    }
    else
    {
	*mergedFileOutPtr = mergedFile;
	if (deltaFileOutPtr != NULL)
	{
	    *deltaFileOutPtr = deltaFile;
	}
    }

epilogue:
    cseg_array_destroy(oldSegs);
    cseg_array_destroy(newSegs);
    return return_value;
}


/*
 * Merges segments in C files
 *
 * Assumes: all segments and user segments are sorted by beginning offset.
 */
static int
merge_files_by_segment(
			File		oldFile, 
			CSegArray	oldSegs, 
			STRING		oldFileName,
			File		newFile, 
			CSegArray	newSegs, 
			STRING		newFileName,
			File		*mergedFileOutPtr,
			File		*deltaFileOutPtr,
			STRING		mainFuncAlternateName
)
{
    int		return_value = 0;
    int		numSegsChanged = 0;
    int		c = 0;
    long	oldFileOffset = 0;
    long	nextModOffset = 0;
    long	nextDeltaOffset = 0;
    File	mergedFile = NULL;
    File	deltaFile = NULL;
    CUserSeg	oldUserSeg = NULL;
    CUserSeg	newUserSeg = NULL;

    *mergedFileOutPtr = NULL;
    *deltaFileOutPtr = NULL;

    numSegsChanged = match_all_segments(
			oldSegs, newSegs, mainFuncAlternateName);
    if (numSegsChanged < 1)
    {
	return 0;
    }

    mergedFile = tmpfile();
    if (deltaFileOutPtr != NULL)
    {
	deltaFile = tmpfile();
    }
    rewind(oldFile);
    oldFileOffset = 0;

    oldUserSeg = oldSegs->firstUserSeg;
    while ((oldUserSeg != NULL) && (oldUserSeg->clientData == NULL))
    {
	oldUserSeg = oldUserSeg->next;
    }
    assert(oldUserSeg != NULL);		/* at least one change exists */
    nextModOffset = oldUserSeg->offset;
    while ((c = fgetc(oldFile)) != EOF)
    {
	++oldFileOffset;
	if (oldFileOffset == (nextModOffset+1))
	{
	    newUserSeg = ((CUserSeg)(oldUserSeg->clientData));
	    if (   (deltaFile != NULL) 
		&& (!util_streq(oldUserSeg->text, newUserSeg->text)) )
	    {
		fprintf(deltaFile, 
			"\n========================================\n");
		fprintf(deltaFile,
			"----- FROM (old file, line %d) -----\n"
			"%s",
		    oldUserSeg->line, oldUserSeg->text);
		fprintf(deltaFile,
			"----- TO (new file, line %d) -----\n"
			"%s",
		    newUserSeg->line, newUserSeg->text);
		fprintf(deltaFile,
			"-----\n");
	    }

	    fputs(newUserSeg->text, mergedFile);
	    oldFileOffset = oldUserSeg->offset + oldUserSeg->length;
	    fseek(oldFile, oldFileOffset, SEEK_SET);

	    oldUserSeg = oldUserSeg->next;
    	    while ((oldUserSeg != NULL) && (oldUserSeg->clientData == NULL))
            {
	        oldUserSeg = oldUserSeg->next;
            }
	    nextModOffset = (oldUserSeg == NULL? -1:oldUserSeg->offset);
	}
	else
	{
	    fputc(c, mergedFile);
	}
    }

    (*mergedFileOutPtr) = mergedFile;
    (*deltaFileOutPtr) = deltaFile;
    return return_value;
}


/*
 * Sets clientData to point to corresponding segments and user segments, IFF 
 * the segments have changed. 
 *
 * Returns the # of user segments that have actually changed.
 */
static int
match_all_segments(
			CSegArray	oldSegs, 
			CSegArray 	newSegs,
			STRING		mainFuncAlternateName
)
{
    CSeg	oldSeg = NULL;
    CSeg	newSeg = NULL;
    BOOL	segChanged = FALSE;
    int		userSegChangeCount = 0;
    int		userSegCount = 0;
    int		i = 0;

    for (i = 0; i < oldSegs->numSegs; ++i)
    {
	oldSeg = &(oldSegs->segs[i]);
	newSeg = match_segment(oldSeg, oldSegs, newSegs, mainFuncAlternateName);

	oldSeg->clientData = NULL;
	segChanged = FALSE;
	if (newSeg != NULL)
	{
	    for (userSegCount = 0; 
		    (userSegCount < oldSeg->userSegs.numSegs)
		 && (userSegCount < newSeg->userSegs.numSegs);
		++userSegCount)
	    {
		if (!util_streq(oldSeg->userSegs.segs[userSegCount].text,
		    		newSeg->userSegs.segs[userSegCount].text))
		{
		    segChanged = TRUE;
		    ++userSegChangeCount;

		    oldSeg->userSegs.segs[userSegCount].clientData =
			&(newSeg->userSegs.segs[userSegCount]);
		}

		#ifdef BOGUS /* (debugging stuff) */
	            printf("[%s] %d -> %d\n",
			oldSeg->userSegs.segs[userSegCount].clientData == NULL?
				" ":"X",
		        oldSeg->userSegs.segs[userSegCount].line,
			newSeg->userSegs.segs[userSegCount].line);
		#endif /* BOGUS */
	    }
	}
	if (segChanged)
	{
	    oldSeg->clientData = (void *)newSeg;
	}
    } /* for i */

    return userSegChangeCount;
}


static CSeg
match_segment(
			CSeg 		oldSeg, 
			CSegArray	oldSegArray,
			CSegArray 	newSegArray,
			STRING		mainFuncAlternateName
)
{
    CSeg	newSeg = NULL;
    int		oldSegIndex = -1;
    int		oldSegNum = -1;
    int		newSegIndex = -1;
    int		newSegNum = 0;
    int		i = 0;

    oldSegIndex = -1;
    if (oldSeg->type == CSEG_GLOBAL)
    {
	/*
	 * Global segments are unnamed - find number of segment
	 */
	oldSegNum = -1;
	for (i = 0; i < oldSegArray->numSegs; ++i)
	{
	    if (oldSegArray->segs[i].type == CSEG_GLOBAL)
	    {
		++oldSegNum;
	    }
	    if (&(oldSegArray->segs[i]) == oldSeg)
	    {
		break;
	    }
	}
	oldSegIndex = i;

	/* these asserts check to see if seg is in array at all */
	assert(oldSegNum >= 0);
	assert((oldSegIndex >= 0) && (oldSegIndex < oldSegArray->numSegs));


	/*
	 * Find matching segment in new array
	 */
	newSegNum = -1;
	for (i = 0; i < newSegArray->numSegs; ++i)
	{
	    if (newSegArray->segs[i].type == CSEG_GLOBAL)
	    {
		++newSegNum;
	    }
	    if (oldSegNum == newSegNum)
	    {
		break;
	    }
	}
	if (newSegNum != oldSegNum)
	{
	    goto epilogue;
	}
	newSegIndex = i;

	newSeg = &(newSegArray->segs[newSegIndex]);
    }
    else
    {
	/* Not global - find segment by name */
	newSeg = match_named_segment(oldSeg->name, newSegArray);
	if ((newSeg == NULL) && (mainFuncAlternateName != NULL))
	{
	    if (util_streq(oldSeg->name, "main"))
	    {
		 newSeg = match_named_segment(
				mainFuncAlternateName, newSegArray);
		 #ifdef DEBUG
		 if (newSeg != NULL)
		 {
		     util_dprintf(1, 
			 "DEBUG: Function name changed: %s -> %s\n",
					mainFuncAlternateName, "main");
		 }
		 #endif
	    }
	    else if (util_streq(oldSeg->name, mainFuncAlternateName))
	    {
		 newSeg = match_named_segment("main", newSegArray);
		 #ifdef DEBUG
		 if (newSeg != NULL)
		 {
		     util_dprintf(1, 
			 "DEBUG: Function name changed: %s -> %s\n",
					"main", mainFuncAlternateName);
		 }
		 #endif
	    }
	}
    }

epilogue:
    return newSeg;
}


static CSeg
match_named_segment(
			STRING 		segName, 
			CSegArray	segArray
)
{
    int		i = 0;
    CSeg	seg = NULL;

    for (i = 0; i < segArray->numSegs; ++i)
    {
        if (util_streq(segName, segArray->segs[i].name))
        {
	    seg = &(segArray->segs[i]);
	    break;
        }
    }
    return seg;
}


#ifdef MAIN
int
main (int argc, char *argv[])
{
    int		rc = 0;			/* return code */
    STRING	newFileName = NULL;
    File	newFile = NULL;
    STRING	oldFileName = NULL;
    File	oldFile = NULL;
    File	mergedFile = NULL;
    File	deltaFile = NULL;
    int		c = 0;

    util_init(&argc, &argv);
    if (argc != 3)
    {
	fprintf(stderr, "Usage: %s <old-file> <new-file>\n", argv[0]);
	exit(1);
    }
    oldFileName = argv[1];
    newFileName = argv[2];

    /*
     * Open the files
     */
    oldFile = util_fopen_locked(oldFileName, "r");
    if (oldFile == NULL)
    {
	perror(oldFileName);
	exit(1);
    }
    newFile = util_fopen_locked(newFileName, "r");
    if (newFile == NULL)
    {
	perror(newFileName);
	exit(1);
    }

    /*
     * Merge 'em!
     */
    rc = abmfP_merge_c_files(
			oldFile, oldFileName, FALSE,
			newFile, newFileName, FALSE,
			&mergedFile,
			&deltaFile);

    if ((rc >= 0) && (mergedFile == NULL))
    {
	printf("No changes found.\n");
    }

    if (deltaFile != NULL)
    {
	char	name[1024];
	File	f = NULL;
	sprintf(name, "%s.delta", oldFileName);
	printf("writing %s\n", name);
	unlink(name);
	f = util_fopen_locked(name, "w");
	if (f == NULL)
	{
	    perror(name);
	    exit(1);
	}
	rewind(deltaFile);
	while ((c = fgetc(deltaFile)) != EOF)
	{
	    fputc(c, f);
	}
	util_fclose(deltaFile);
	util_fclose(f);
    }

    if (mergedFile != NULL)
    {
	char	name[1024];
	File	f = NULL;
	sprintf(name, "%s.merge", oldFileName);
	printf("writing %s\n", name);
	unlink(name);
	f = util_fopen_locked(name, "w");
	if (f == NULL)
	{
	    perror(name);
	    exit(1);
	}
	rewind(mergedFile);
	while ((c = fgetc(mergedFile)) != EOF)
	{
	    fputc(c, f);
	}
	util_fclose(mergedFile);
	util_fclose(f);
    }

    util_fclose(oldFile);
    util_fclose(newFile);
    exit(0);
    return 0;
}
#endif /* MAIN */

