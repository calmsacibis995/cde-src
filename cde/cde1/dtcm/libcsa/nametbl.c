/* $XConsortium: nametbl.c /main/cde1_maint/2 1995/09/06 08:23:40 lehors $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)nametbl.c	1.12 96/11/12 Sun Microsystems, Inc."

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nametbl.h"
#include "hash.h"

#define STD_TABLE_SIZE	37

extern _DtCmNameTable *
_DtCm_make_name_table(int size, char **names)
{
	_DtCmNameTable *tbl;
	int i;

	if ((tbl = (_DtCmNameTable *)calloc(1, sizeof(_DtCmNameTable))) == NULL)
		return (NULL);

	if ((tbl->tbl = _DtCmMakeHash(STD_TABLE_SIZE)) == NULL) {
		free(tbl);
		return (NULL);
	}

	if ((tbl->names = (char **)malloc(sizeof(char *) * (size+1))) == NULL) {
		_DtCm_free_name_table(tbl);
		return (NULL);
	}

	for (i = 1; i <= size; i++) {
		if ((tbl->names[i] = strdup(names[i])) == NULL) {
			tbl->size = i;
			_DtCm_free_name_table(tbl);
			return (NULL);
		}

		*(int *)_DtCmGetHash(tbl->tbl, (unsigned char *)names[i]) = i;
	}

	tbl->size = i - 1;

	return (tbl);
}

extern void
_DtCm_free_name_table(_DtCmNameTable *tbl)
{
	int	i;

	if (tbl == NULL) return;

	if (tbl->tbl)
		_DtCmDestroyHash(tbl->tbl, NULL, NULL);

	for (i = 1; i <= tbl->size; i++)
		free(tbl->names[i]);

	if (tbl->names)
		free(tbl->names);

	free(tbl);
}

/*
 * if index <= 0, then add one more element to the table
 * otherwise use the index to add the new name and extend the
 * table by (index - size)
 */
extern CSA_return_code
_DtCm_add_name_to_table(_DtCmNameTable *tbl, int index, char *newname)
{
	int	*ptr;
	char	**newptr;

	if (index > 0 && index <= tbl->size && tbl->names[index])
		return (CSA_E_INVALID_PARAMETER);

	if (index <= 0)
		index = tbl->size + 1;

	/* add new name to table */
	ptr = (int *)_DtCmGetHash(tbl->tbl, (unsigned char *)newname);
	if (ptr)
		*ptr = index;

	if (index > tbl->size) {
		if ((newptr = (char **)realloc(tbl->names,
		    sizeof(char *)*(index + 1))) == NULL) {
			*ptr = -1;
			return (CSA_E_INSUFFICIENT_MEMORY);
		} else {
			tbl->names = newptr;
			memset((void *)&tbl->names[tbl->size+1], NULL,
				sizeof(char *)*(index - tbl->size));
		}
	}

	if ((tbl->names[index] = strdup(newname)) == NULL) {

		*ptr = -1;
		return (CSA_E_INSUFFICIENT_MEMORY);

	} else {
		if (index > tbl->size)
			tbl->size = index;
	}

	return (CSA_SUCCESS);
}

extern int
_DtCm_get_index_from_table(_DtCmNameTable *tbl, char *name)
{
	int *ptr;

	ptr = (int *)_DtCmFindHash(tbl->tbl, (unsigned char *)name);

	if (ptr)
		return (*ptr);
	else
		return (-1);
}

/*
 * if index <= 0, then add one more element to the table
 * other use the index to add the new name and extend the
 * table by (index - size)
 */
extern CSA_return_code
_DtCmExtendNameTable(
	char		*name,
	int		index,
	int		type,
	_DtCmNameTable	*base,
	int		basesize,
	char		**basenames,
	_DtCmNameTable	**tbl,
	int		**types)
{
	_DtCmNameTable	*ntbl;
	int		*newarray;
	int		newindex;

	if (index > 0 && index <= (*tbl)->size)
		return (CSA_E_INVALID_PARAMETER);

	if (*tbl == base) {
		if ((ntbl = _DtCm_make_name_table(basesize, basenames)) == NULL)
		{

			return (CSA_E_INSUFFICIENT_MEMORY);
		} else
			*tbl = ntbl;
	}

	if (types) {
		newindex = (index <= 0) ? (*tbl)->size+1 : index;
		if ((newarray = (int *)realloc(*types,
		    sizeof(int) * (newindex+1))) == NULL) {
			return (CSA_E_INSUFFICIENT_MEMORY);
		} else {
			*types = newarray;
			(*types)[newindex] = type;
		}
	}

	return (_DtCm_add_name_to_table(*tbl, index, name));
}

