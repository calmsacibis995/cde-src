/* $XConsortium: nametbl.h /main/cde1_maint/1 1995/07/17 20:01:56 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _NAMETBL_H
#define _NAMETBL_H

#pragma ident "@(#)nametbl.h	1.8 96/11/12 Sun Microsystems, Inc."

/*
 * Contain the definition for a hashed name table for attribute names.
 * It is intended to be used by both the library and the daemon to
 * build name tables for calendar attribute names and entry attribute
 * names.
 */

#include "ansi_c.h"
#include "csa.h"

/*
 * NOTE: The first element in names is not used.  So a name table
 *	 with a size of 5 actually contains names in element 1 to 5
 *	 of names.
 */
typedef struct {
	void	*tbl;
	int	size;
	char	**names;
} _DtCmNameTable;

extern _DtCmNameTable *_DtCm_make_name_table(int	size,
					     char	**names);

extern void _DtCm_free_name_table(_DtCmNameTable *tbl);

extern CSA_return_code _DtCm_add_name_to_table(_DtCmNameTable	*tbl,
					       int		index,
					       char		*newname);

extern int _DtCm_get_index_from_table(_DtCmNameTable	*tbl,
				      char		*name);

extern CSA_return_code _DtCmExtendNameTable(char	*name,
					    int		index,
					    int		type,
					    _DtCmNameTable	*base,
					    int		basesize,
					    char	**basenames,
					    _DtCmNameTable	**tbl,
					    int		**types);

#endif /* _NAMETBL_H */

