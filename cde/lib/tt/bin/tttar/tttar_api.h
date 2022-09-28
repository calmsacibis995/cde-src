/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * tttar_api.h - Link Service/ToolTalk object archiving interface functions.
 *
 * If LS/TT object archiving were ever to go into the LS/TT API,
 * some variation on these would be the functions to put in.
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 */

#ifndef _TTTAR_API_H
#define _TTTAR_API_H

#include "tttar_string_map.h"

#define CURRENT_ARCHIVE_VERSION 1

typedef enum object_kind {
	NO_KIND,
	VERSION_NUM,
	SPEC,
	ARCHIVE_END,
	SUN_LINK } Object_kind;

bool_t	pathlist_lstt_archive(
			_Tt_string_list_ptr	paths,
		        bool_t			recurse,
		        bool_t			follow_symlinks,
			int			verbosity,
		        XDR		       *xdrs );
bool_t	pathlist_lstt_dearchive(
			_Tt_string_list_ptr	paths_to_extract,
			Lstar_string_map_list_ptr renamings,
			_Tt_string		where_to_dearchive,
			bool_t			preserve__props,
			int			verbosity,
		        XDR		       *xdrs );
bool_t	pathlist_lstt_archive_list(
			_Tt_string_list_ptr	paths_to_extract,
			int			verbosity,
		        XDR		       *xdrs );
bool_t	spec_archive(
			char	       *id,
			char 	       *path,
			int		verbosity,
			XDR	       *xdrs,
			Tt_status      *err );
bool_t	spec_dearchive(
			char	      **old_spec_id_ptr,
			char	      **new_spec_id_ptr,
			char	      **path_as_archived,
			Lstar_string_map_list_ptr renamings,
			char	       *where_to_create,
			bool_t		preserve__props,
			bool_t	      (*dearchive_this_path)(char *, void *),
			void	       *context,
		        int		verbosity,
			XDR 	       *xdrs,
			Tt_status      *err );

#endif /* _TTTAR_API_H */
