/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.3
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: UilSymStor.c /main/cde1_maint/1 1995/07/14 20:58:03 drk $"
#endif
#endif

/*
*  (c) Copyright 1989, 1990, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS. */

/*
**++
**  FACILITY:
**
**      User Interface Language Compiler (UIL)
**
**  ABSTRACT:
**
**      This module contains the procedures for managing memory for
**	the compiler. 
**
**--
**/


/*
**
**  INCLUDE FILES
**
**/

#include <Xm/Xm.h>

#include <ctype.h>
#include "UilDefI.h"

/*
**
**  DEFINE and MACRO DEFINITIONS
**
**/


/*
**
**  EXTERNAL VARIABLE DECLARATIONS
**
**/


/*
**
**  GLOBAL VARIABLE DECLARATIONS
**
**/

externaldef(uil_comp_glbl) sym_name_entry_type
	*sym_az_hash_table[ sym_k_hash_table_limit];
externaldef(uil_comp_glbl) sym_value_entry_type
	*sym_az_error_value_entry = NULL;
externaldef(uil_comp_glbl) sym_external_def_entry_type
	*sym_az_external_def_chain;
externaldef(uil_comp_glbl) sym_forward_ref_entry_type
	*sym_az_forward_ref_chain;
externaldef(uil_comp_glbl) sym_val_forward_ref_entry_type
	*sym_az_val_forward_ref_chain;
externaldef(uil_comp_glbl) sym_module_entry_type
	*sym_az_module_entry;
externaldef(uil_comp_glbl) sym_root_entry_type
	*sym_az_root_entry;
externaldef(uil_comp_glbl) sym_section_entry_type
	*sym_az_current_section_entry;
externaldef(uil_comp_glbl) sym_entry_type
	*sym_az_entry_list_header;

/*
 * These lists save the allocated and freed symbol table nodes.
 */
externaldef(uil_comp_glbl) URMPointerListPtr	sym_az_allocated_nodes;
externaldef(uil_comp_glbl) URMPointerListPtr	sym_az_freed_nodes;


/*
**
**  OWN VARIABLE DECLARATIONS
**
**/



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This routine intializes the compiler's memory allocation system.
**
**  FORMAL PARAMETERS:
**
**      none
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**	sym_az_last_location		ptr to last byte avail for allocation
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      first symbol table buffer is allocated
**
**--
**/

void	sym_initialize_storage()
{
int		i;


/*
 * Initialize the name hash table
 */
for (i=0; i<sym_k_hash_table_limit; i++)
    sym_az_hash_table[ i ] = NULL;

/* 
 * Set forward reference, external definition, and symbol table header
 * chains to null.
 */
sym_az_forward_ref_chain = NULL;
sym_az_val_forward_ref_chain = NULL;
sym_az_external_def_chain = NULL;
sym_az_entry_list_header = NULL;
sym_az_module_entry = NULL;

/*
 * Acquire pointer lists to access allocated and freed nodes
 */
UrmPlistInit (1000, &sym_az_allocated_nodes);
UrmPlistInit (100, &sym_az_freed_nodes);

/*
 * Allocate a value entry for an error value and give it a name.  Giving it
 * name assures that it is not freed.  The name used is one that will not
 * conflict with a user name.
 */

if ( sym_az_error_value_entry == NULL )
    sym_az_error_value_entry = (sym_value_entry_type *)
	sem_allocate_node (sym_k_value_entry, sym_k_value_entry_size);

sym_az_error_value_entry->b_type = sym_k_error_value;
sym_az_error_value_entry->obj_header.b_flags = 
    (sym_m_private | sym_m_builtin);

sym_az_error_value_entry->obj_header.az_name =
    sym_insert_name( 9, "#error...");

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This routine cleans up the compiler's memory allocation system, and
**	frees all memory used by the symbol table.
**
**  FORMAL PARAMETERS:
**
**      freealloc		TRUE if allocated nodes are to be freed.
**				Otherwise, free only freed nodes.
**
**  IMPLICIT INPUTS:
**
**      sym_az_entry_list_header	ptr to list of symbol entries
**
**  IMPLICIT OUTPUTS:
**
**      sym_az_entry_list_header	ptr to list of symbol entries
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**	all symbol table memory is freed.
**
**--
**/

void	Uil_sym_cleanup_storage (freealloc)
    boolean		freealloc;

{
if ( freealloc )
    if ( sym_az_allocated_nodes != NULL )
	UrmPlistFreeContents (sym_az_allocated_nodes);
else
    if ( sym_az_freed_nodes != NULL )
	UrmPlistFreeContents (sym_az_freed_nodes);
if ( sym_az_allocated_nodes != NULL )
    UrmPlistFree (sym_az_allocated_nodes);
if ( sym_az_freed_nodes != NULL )
    UrmPlistFree (sym_az_freed_nodes);
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This routine adds an object to the external definition chain.
**
**  FORMAL PARAMETERS:
**
**      az_name		name of object to be placed on the chain
**
**  IMPLICIT INPUTS:
**
**      sym_az_external_def_chain	head of the external definition chain
**
**  IMPLICIT OUTPUTS:
**
**      sym_az_external_def_chain	head of the external definition chain
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

void	    sym_make_external_def( az_name )

XmConst sym_name_entry_type *az_name;

{

    sym_external_def_entry_type   *external_entry;

    /* allocate an external definition entry */

    external_entry = (sym_external_def_entry_type *)
	sem_allocate_node (sym_k_external_def_entry,
			   sym_k_external_def_entry_size);

    /* fill in the entry */

    external_entry->az_name = (sym_name_entry_type *)az_name;

    /* place on the front of the chain */

    external_entry->az_next_object = sym_az_external_def_chain;
    sym_az_external_def_chain = external_entry;

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This routine adds a reference to the forward reference chain.
**	This routine is used for widget and gadget forward references only.
**	
**
**  FORMAL PARAMETERS:
**
**      az_id_frame	parse stack frame for id being referenced
**	l_widget_type	type of object being referenced
**      az_object	ptr to the location that needs to be updated
**			when the object is resolved
**
**  IMPLICIT INPUTS:
**
**      sym_az_forward_ref_chain    head of the forward reference chain
**
**  IMPLICIT OUTPUTS:
**
**      sym_az_forward_ref_chain    head of the forward reference chain
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

void    sym_make_forward_ref( az_id_frame, l_widget_type, a_location )

XmConst yystype       *az_id_frame;
XmConst int           l_widget_type;
XmConst char	    *a_location;

{

    sym_forward_ref_entry_type   *fwd_ref_entry;

    _assert( (az_id_frame->b_tag == sar_k_token_frame) &&
	     (az_id_frame->value.az_symbol_entry->header.b_tag == 
		sym_k_name_entry), "arg1 not an id frame" );

    /* allocate an external definition entry */

    fwd_ref_entry = (sym_forward_ref_entry_type *)
	sem_allocate_node (sym_k_forward_ref_entry,
			   sym_k_forward_ref_entry_size);

    /* fill in the entry */

    _sar_save_source_pos (&fwd_ref_entry->header, az_id_frame);

    fwd_ref_entry->header.b_type = l_widget_type;
    fwd_ref_entry->az_name =
	(sym_name_entry_type *) az_id_frame->value.az_symbol_entry;
    fwd_ref_entry->a_update_location = (char *)a_location;

    /* place on the front of the chain */

    fwd_ref_entry->az_next_ref = sym_az_forward_ref_chain;
    sym_az_forward_ref_chain = fwd_ref_entry;

}

/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This routine adds a reference to the values forward reference chain.
**	This routine is used for value forward references only.
**	
**
**  FORMAL PARAMETERS:
**
**      az_id_frame	parse stack frame for id being referenced
**      az_object	ptr to the location that needs to be updated
**			when the object is resolved
**
**  IMPLICIT INPUTS:
**
**      sym_az_val_forward_ref_chain    head of the forward reference chain
**
**  IMPLICIT OUTPUTS:
**
**      sym_az_val_forward_ref_chain    head of the forward reference chain
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

void    sym_make_value_forward_ref

#ifndef _NO_PROTO
	(XmConst yystype  *az_value_frame, XmConst char *a_location,
		XmConst unsigned char fwd_ref_flags )
#else
	(az_value_frame, a_location, fwd_ref_flags)

XmConst yystype       *az_value_frame;
XmConst char	    *a_location;
XmConst unsigned char fwd_ref_flags;

#endif

{

    sym_val_forward_ref_entry_type   *fwd_ref_entry;

    /* allocate an external definition entry */

    fwd_ref_entry = (sym_val_forward_ref_entry_type *)
	sem_allocate_node (sym_k_val_forward_ref_entry,
			   sym_k_val_forward_ref_entry_size);

    /* fill in the entry */

    _sar_save_source_pos (&fwd_ref_entry->header, az_value_frame);

    switch (fwd_ref_flags)
	{
	case sym_k_patch_add:
	    fwd_ref_entry->az_name =
		((sym_value_entry_type *)
		 az_value_frame->value.az_symbol_entry)->obj_header.az_name;
	    break;
	case sym_k_patch_list_add:
	    fwd_ref_entry->az_name = 
		(sym_name_entry_type *)az_value_frame->value.az_symbol_entry;
	    break;
	default:
	    _assert(FALSE, "Illegal forward reference");
	};
	
    fwd_ref_entry->a_update_location = (char *)a_location;
    fwd_ref_entry->fwd_ref_flags = fwd_ref_flags;

    /* place on the front of the chain */

    fwd_ref_entry->az_next_ref = sym_az_val_forward_ref_chain;
    sym_az_val_forward_ref_chain = fwd_ref_entry;

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This procedure recursively goes through the symbol table, dumping
**	each node accessible from the root node.
**
**  FORMAL PARAMETERS:
**
**      none
**
**  IMPLICIT INPUTS:
**
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      symbol table is dump with the debug output
**
**--
**/

void	UilDumpSymbolTable (node_entry)
    sym_entry_type		*node_entry;

{

sym_value_entry_type		*val_node;
sym_widget_entry_type		*widget_node;
sym_module_entry_type		*module_node;
sym_list_entry_type		*list_node;
sym_obj_entry_type		*list_entry;
sym_root_entry_type		*root_node;
sym_include_file_entry_type	*ifile_node;
sym_section_entry_type		*section_node;


/*
 * No action on null node. Else dump and processing is based on the kind
 * of the current node.
 */
if ( node_entry == NULL ) return;
sym_dump_symbol (node_entry);
switch ( node_entry->header.b_tag )
    {
    case sym_k_value_entry:
        val_node = (sym_value_entry_type *) node_entry;
	UilDumpSymbolTable ((sym_entry_type *)val_node->az_charset_value);
	UilDumpSymbolTable ((sym_entry_type *)val_node->az_first_table_value);
	UilDumpSymbolTable ((sym_entry_type *)val_node->az_next_table_value);
	UilDumpSymbolTable ((sym_entry_type *)val_node->az_exp_op1);
	UilDumpSymbolTable ((sym_entry_type *)val_node->az_exp_op2);
	break;
    case sym_k_widget_entry:
    case sym_k_gadget_entry:
    case sym_k_child_entry:
	widget_node = (sym_widget_entry_type *) node_entry;
	UilDumpSymbolTable ((sym_entry_type *)widget_node->az_callbacks);
	UilDumpSymbolTable ((sym_entry_type *)widget_node->az_arguments);
	UilDumpSymbolTable ((sym_entry_type *)widget_node->az_controls);
	UilDumpSymbolTable ((sym_entry_type *)widget_node->az_create_proc);
	break;
    case sym_k_module_entry:
	module_node = (sym_module_entry_type *) node_entry;
	UilDumpSymbolTable ((sym_entry_type *)module_node->az_version);
	UilDumpSymbolTable ((sym_entry_type *)module_node->az_character_set);
	UilDumpSymbolTable ((sym_entry_type *)module_node->az_case_sense);
	UilDumpSymbolTable ((sym_entry_type *)module_node->az_def_obj);
	break;
    case sym_k_list_entry:
	/*
	 * Sublists (nested lists) are not processed recursively to
	 * pick up definitions, since all named lists are picked up
	 * in their list sections. We assume no list of interest to
	 * us can possibly be encountered only in a nested list reference.
	 */
	list_node = (sym_list_entry_type *) node_entry;
	for (list_entry=(sym_obj_entry_type *)
	         list_node->obj_header.az_next;
	     list_entry!=NULL;
	     list_entry=(sym_obj_entry_type *)
	         list_entry->obj_header.az_next)
	    UilDumpSymbolTable ((sym_entry_type *)list_entry);
	break;
    case sym_k_root_entry:
	root_node = (sym_root_entry_type *) node_entry;
	UilDumpSymbolTable ((sym_entry_type *)root_node->module_hdr);
	UilDumpSymbolTable ((sym_entry_type *)root_node->sections);
	break;
    case sym_k_include_file_entry:
	ifile_node = (sym_include_file_entry_type *) node_entry;
	UilDumpSymbolTable ((sym_entry_type *)ifile_node->sections);
	break;
    case sym_k_section_entry:
	section_node = (sym_section_entry_type *) node_entry;
	switch ( section_node->header.b_type )
	    {
	    case sym_k_section_tail:
		break;
	    default:
		UilDumpSymbolTable ((sym_entry_type *)section_node->next);
		UilDumpSymbolTable ((sym_entry_type *)section_node->entries);
		break;
	    }
	break;
    }

}



/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This procedure dumps the symbol table.
**
**  FORMAL PARAMETERS:
**
**      none
**
**  IMPLICIT INPUTS:
**
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      symbol table is dump with the debug output
**
**--
**/

void	sym_dump_symbols()
{

int				ndx;
sym_entry_type			*cur_entry;


/*
 * Loop over all entries which have been allocated. They are in
 * allocation order (this will include deleted entries).
 */
_debug_output( "\nSymbol Table Dump: \n\n" );
for ( ndx=0 ; ndx<UrmPlistNum(sym_az_allocated_nodes) ; ndx++ )
    {
    cur_entry = (sym_entry_type *) UrmPlistPtrN (sym_az_allocated_nodes, ndx);
    sym_dump_symbol (cur_entry);
    }
_debug_output ("\n\n");

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This procedure dumps a symbol node.
**
**  FORMAL PARAMETERS:
**
**      az_symbol_entry			symbol node to be dumped
**
**  IMPLICIT INPUTS:
**
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      symbol is dumped to the debug output
**
**--
**/

void	sym_dump_symbol (az_symbol_entry)

sym_entry_type	*az_symbol_entry;

{

    switch (az_symbol_entry->header.b_tag) {

	case sym_k_name_entry:
		sym_dump_name((sym_name_entry_type *) az_symbol_entry );
		break;

	case sym_k_module_entry:
		sym_dump_module((sym_module_entry_type *) az_symbol_entry );
		break;

	case sym_k_value_entry:
		sym_dump_value((sym_value_entry_type *) az_symbol_entry );
		break;

	case sym_k_widget_entry:
	case sym_k_gadget_entry:
	case sym_k_child_entry:
		sym_dump_widget((sym_widget_entry_type *) az_symbol_entry );
		break;

	case sym_k_forward_ref_entry:
		sym_dump_forward_ref((sym_forward_ref_entry_type *) az_symbol_entry );
		break;

	case sym_k_external_def_entry:
		sym_dump_external_def((sym_external_def_entry_type *) az_symbol_entry );
		break;

	case sym_k_proc_def_entry:
		sym_dump_proc_def((sym_proc_def_entry_type *) az_symbol_entry );
		break;

	case sym_k_proc_ref_entry:
		sym_dump_proc_ref((sym_proc_ref_entry_type *) az_symbol_entry );
		break;

	case sym_k_control_entry:
		sym_dump_control((sym_control_entry_type *) az_symbol_entry );
		break;

	case sym_k_argument_entry:
		sym_dump_argument((sym_argument_entry_type *) az_symbol_entry );
		break;

	case sym_k_callback_entry:
		sym_dump_callback((sym_callback_entry_type *) az_symbol_entry );
		break;

	case sym_k_list_entry:
		sym_dump_list((sym_list_entry_type *) az_symbol_entry );
		break;

	case sym_k_color_item_entry:
		sym_dump_color_item((sym_color_item_entry_type *) az_symbol_entry );
		break;

	case sym_k_parent_list_entry:
		sym_dump_parent_list_item((sym_parent_list_type *) az_symbol_entry );
		break;

	case sym_k_include_file_entry:
		sym_dump_include_file ((sym_include_file_entry_type *)az_symbol_entry);
		break;

	case sym_k_section_entry:
		sym_dump_section ((sym_section_entry_type *)az_symbol_entry);
		break;

	case sym_k_def_obj_entry:
		sym_dump_object_variant ((sym_def_obj_entry_type *)az_symbol_entry);
		break;

	case sym_k_root_entry:
		sym_dump_root_entry ((sym_root_entry_type *)az_symbol_entry);
		break;

	default:
	    {
		int	    *l_array;
		int	    i;

		_debug_output("%x  unknown type: %d  size: %d  byte: %x\n", 
			      (unsigned) az_symbol_entry,
			      az_symbol_entry->header.b_tag,
			      az_symbol_entry->header.w_node_size,
			      az_symbol_entry->header.b_type );

		l_array = (int *) az_symbol_entry->b_value;

		for (i=0;  i<(az_symbol_entry->header.w_node_size-1);  i++)
		    _debug_output( "\t%x", l_array[ i ] );

		_debug_output("\n");
		break;
	    }
	}

    sym_dump_source_info(( sym_entry_header_type *)az_symbol_entry);
    _debug_output("\n");

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function dumps an object entry in the symbol table
**
**  FORMAL PARAMETERS:
**
**      az_widget_entry	    pointer to the object
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      symbolic representation of the object appears as part of the
**	debug output
**
**--
**/

void	sym_dump_widget(az_widget_entry)

XmConst sym_widget_entry_type	*az_widget_entry;
{

    sym_dump_obj_header ((sym_obj_entry_type *)az_widget_entry);

    _debug_output (
	"  %s %s  controls: %x  callbacks: %x  arguments: %x  parent_list: %x\n", 
	    diag_object_text( az_widget_entry->header.b_type),
	    diag_tag_text( az_widget_entry->header.b_tag ),
	    (unsigned) az_widget_entry->az_controls,
	    (unsigned) az_widget_entry->az_callbacks,
	    (unsigned) az_widget_entry->az_arguments,
            (unsigned) az_widget_entry->parent_list);

    if (az_widget_entry->az_create_proc != NULL) {
	_debug_output ("  create proc: %x\n",
		       (unsigned) az_widget_entry->az_create_proc);
    }
    /* preserve comments */
    _debug_output ("\n Comment: %s\n", az_widget_entry->obj_header.az_comment);

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function dumps an argument entry in the symbol table
**
**  FORMAL PARAMETERS:
**
**      az_argument_entry	pointer to the argument
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      symbolic representation of the name appears as part of the
**	debug output
**
**--
**/

void	sym_dump_argument(az_argument_entry)

XmConst sym_argument_entry_type	*az_argument_entry;

{

    sym_dump_obj_header ((sym_obj_entry_type *)az_argument_entry);

    _debug_output ( "  arg name: %x  arg value: %x\n", 
	(unsigned) az_argument_entry->az_arg_name,
	(unsigned) az_argument_entry->az_arg_value );

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function dumps a control entry in the symbol table
**
**  FORMAL PARAMETERS:
**
**      az_control_entry	pointer to the control
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      symbolic representation of the name appears as part of the
**	debug output
**
**--
**/

void	sym_dump_control(az_control_entry)

XmConst sym_control_entry_type	*az_control_entry;

{

    sym_dump_obj_header ((sym_obj_entry_type *)az_control_entry);

/*    These are for control objects only.     */

    if (az_control_entry->obj_header.b_flags & sym_m_def_in_progress) {
	_debug_output ("  def in progress");
    }

    if (az_control_entry->obj_header.b_flags & sym_m_managed) {
	_debug_output ("  managed");
    } else {
	_debug_output ("  unmanaged");
    }

    _debug_output ( "  obj: %x\n", 
	(unsigned) az_control_entry->az_con_obj );

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function dumps a callback entry in the symbol table
**
**  FORMAL PARAMETERS:
**
**      az_callback_entry	pointer to the callback
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      symbolic representation of the name appears as part of the
**	debug output
**
**--
**/

void	sym_dump_callback(az_callback_entry)

XmConst sym_callback_entry_type	*az_callback_entry;

{

    sym_dump_obj_header ((sym_obj_entry_type *)az_callback_entry);

    _debug_output ( "  reason name: %x  proc ref: %x  proc ref list: %x\n", 
	(unsigned) az_callback_entry->az_call_reason_name,
	(unsigned) az_callback_entry->az_call_proc_ref,
        (unsigned) az_callback_entry->az_call_proc_ref_list );

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function dumps a  list entry in the symbol table
**
**  FORMAL PARAMETERS:
**
**      az_list_entry	pointer to the list
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      symbolic representation of the name appears as part of the
**	debug output
**
**--
**/

void	sym_dump_list(az_list_entry)

XmConst sym_list_entry_type	*az_list_entry;

{

    sym_dump_obj_header ((sym_obj_entry_type *)az_list_entry);

    _debug_output ( "  type: %s  count: %d  gadget count: %d\n", 
	diag_tag_text( az_list_entry->header.b_type),
	az_list_entry->w_count,
	az_list_entry->w_gadget_count );

    /* preserve comments */
    _debug_output ("\n Comment: %s\n", az_list_entry->obj_header.az_comment);

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function dumps a name entry in the symbol table
**
**  FORMAL PARAMETERS:
**
**      az_name_entry	    pointer to the name
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      symbolic representation of the name appears as part of the
**	debug output
**
**--
**/

void	sym_dump_name(az_name_entry)

XmConst	sym_name_entry_type	*az_name_entry;

{

    _debug_output
	( "%x name size: %d  next name: %x  object: %x",
	  (unsigned) az_name_entry,
	  az_name_entry->header.w_node_size,
	  (unsigned) az_name_entry->az_next_name_entry,
	  (unsigned) az_name_entry->az_object );

    if (az_name_entry->b_flags & sym_m_referenced) {
	_debug_output (" referenced");
    }

    _debug_output
	( "  name: %s \n", az_name_entry->c_text );

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function dumps a module entry in the symbol table
**
**  FORMAL PARAMETERS:
**
**      az_module_entry	    pointer to the module
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      symbolic representation of the module appears as part of the
**	debug output
**
**--
**/

void	sym_dump_module(az_module_entry)

XmConst sym_module_entry_type	*az_module_entry;

{

    _debug_output
	( "%x module size: %d  name: %x  version: %x \n",
	  (unsigned) az_module_entry,
	  az_module_entry->header.w_node_size,
	  (unsigned) az_module_entry->obj_header.az_name,
	  (unsigned) az_module_entry->az_version );

    /* preserve comments */
    _debug_output ("\n Comment: %s\n", az_module_entry->obj_header.az_comment);

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function dumps a color item entry in the symbol table
**
**  FORMAL PARAMETERS:
**
**      az_color_item_entry	    pointer to the color_item
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      symbolic representation of the color item appears as part of the
**	debug output
**
**--
**/

void	sym_dump_color_item(az_color_item_entry)

XmConst sym_color_item_entry_type	*az_color_item_entry;

{

    _debug_output
	( "%x color_item  size: %d  letter: %c  index: %d  color: %x  next: %x\n",
	  (unsigned) az_color_item_entry,
	  az_color_item_entry->header.w_node_size,
	  az_color_item_entry->b_letter,
	  az_color_item_entry->b_index,
	  (unsigned) az_color_item_entry->az_color,
	  (unsigned) az_color_item_entry->az_next );
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function dumps a parent_list entry in the symbol table
**
**  FORMAL PARAMETERS:
**
**      az_parent_list_item	    pointer to the parent list entry
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      none
**
**--
**/

void	sym_dump_parent_list_item (az_parent_list_item)

XmConst sym_parent_list_type	*az_parent_list_item;

{
    _debug_output
	( "%x parent_list  size: %d  parent: %x  next: %x \n",
	  (unsigned) az_parent_list_item,
          az_parent_list_item->header.w_node_size,
          (unsigned) az_parent_list_item->parent,
          (unsigned) az_parent_list_item->next);
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function dumps an external definition entry in the symbol table
**
**  FORMAL PARAMETERS:
**
**      az_external_def_entry	pointer to the external definition
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      symbolic representation of the name appears as part of the
**	debug output
**
**--
**/

void	sym_dump_external_def(az_external_def_entry)

XmConst sym_external_def_entry_type	*az_external_def_entry;

{

    _debug_output
	( "%x external def  size: %d  next external: %x  object: %x \n",
	  (unsigned) az_external_def_entry,
	  az_external_def_entry->header.w_node_size,
	  (unsigned) az_external_def_entry->az_next_object,
	  (unsigned) az_external_def_entry->az_name );

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function dumps a procedure definition entry in the symbol table
**
**  FORMAL PARAMETERS:
**
**      az_proc_def_entry	pointer to the procedure definition
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      symbolic representation of the procedure definition appears as 
**	part of the debug output
**
**--
**/

void	sym_dump_proc_def(az_proc_def_entry)

XmConst sym_proc_def_entry_type	*az_proc_def_entry;

{

    char    *private_flag;
    char    *imported_flag;
    char    *exported_flag;
    char    *checking_flag;

    private_flag = "";
    imported_flag = "";
    exported_flag = "";
    checking_flag = " no-check";

    if (az_proc_def_entry->v_arg_checking)
	checking_flag = " check";	
    if (az_proc_def_entry->obj_header.b_flags & sym_m_private)
	private_flag = " private";	
    if (az_proc_def_entry->obj_header.b_flags & sym_m_exported)
	exported_flag = " exported";	
    if (az_proc_def_entry->obj_header.b_flags & sym_m_imported)
	imported_flag = " imported";	

    _debug_output
	( "%x proc def  size: %d  name: %x %s%s%s%s  count: %d  %s\n",
	  (unsigned) az_proc_def_entry,
	  az_proc_def_entry->header.w_node_size,
	  (unsigned) az_proc_def_entry->obj_header.az_name,
	  checking_flag,
	  private_flag,
	  exported_flag,
	  imported_flag,
	  az_proc_def_entry->b_arg_count,
	  diag_value_text( az_proc_def_entry->b_arg_type ) );

    /* preserve comments */
    _debug_output ("\nComment: %s\n",az_proc_def_entry->obj_header.az_comment);


}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function dumps a procedure reference entry in the symbol table
**
**  FORMAL PARAMETERS:
**
**      az_proc_ref_entry	pointer to the procedure reference entry
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      symbolic representation of the procedure reference appears as 
**	part of the debug output
**
**--
**/

void	sym_dump_proc_ref(az_proc_ref_entry)

XmConst sym_proc_ref_entry_type	*az_proc_ref_entry;

{

    sym_dump_obj_header ((sym_obj_entry_type *)az_proc_ref_entry);

    _debug_output
	( "%x proc ref  size: %d  proc def: %x  value: %x\n",
	  (unsigned) az_proc_ref_entry,
	  az_proc_ref_entry->header.w_node_size,
	  (unsigned) az_proc_ref_entry->az_proc_def,
	  (unsigned) az_proc_ref_entry->az_arg_value );

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function dumps an forward reference entry in the symbol table
**
**  FORMAL PARAMETERS:
**
**      az_forward_ref_entry	pointer to the forward reference
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      symbolic representation of the name appears as part of the
**	debug output
**
**--
**/

void	sym_dump_forward_ref(az_forward_ref_entry)

XmConst sym_forward_ref_entry_type	*az_forward_ref_entry;

{

    _debug_output
	( "%x forward ref  size: %d  next ref: %x  location: %x  %s  parent: %x\n",
	  (unsigned) az_forward_ref_entry,
	  az_forward_ref_entry->header.w_node_size,
	  (unsigned) az_forward_ref_entry->az_next_ref,
	  (unsigned) az_forward_ref_entry->a_update_location,
	  diag_object_text( az_forward_ref_entry->header.b_type ),
          (unsigned) az_forward_ref_entry->parent );

    _debug_output
	( "  name: %x %s\n",
	  (unsigned) az_forward_ref_entry->az_name,
	  az_forward_ref_entry->az_name->c_text );

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function dumps a value entry in the symbol table
**
**  FORMAL PARAMETERS:
**
**      az_value_entry	    pointer to the value
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      symbolic representation of the value appears as part of the
**	debug output
**
**--
**/

void	sym_dump_value(az_value_entry)

XmConst sym_value_entry_type	*az_value_entry;

{

    char    *private_flag;
    char    *imported_flag;
    char    *exported_flag;
    char    *builtin_flag;
    char    *special_type, *table_type;

    private_flag = "";
    imported_flag = "";
    exported_flag = "";
    builtin_flag = "";

    if (az_value_entry->obj_header.b_flags & sym_m_builtin)
	builtin_flag = " builtin";	
    if (az_value_entry->obj_header.b_flags & sym_m_private)
	private_flag = " private";	
    if (az_value_entry->obj_header.b_flags & sym_m_exported)
	exported_flag = " exported";	
    if (az_value_entry->obj_header.b_flags & sym_m_imported)
	imported_flag = " imported";	

    _debug_output
	( "%x value  size: %d  name: %x  %s%s%s%s",
	  (unsigned) az_value_entry,
	  az_value_entry->header.w_node_size,
	  (unsigned) az_value_entry->obj_header.az_name,
	  builtin_flag, private_flag, exported_flag, imported_flag );

    if (az_value_entry->obj_header.b_flags & sym_m_imported)
    {
	_debug_output( "  %s \n", diag_value_text( az_value_entry->b_type ));
	return;
    }

    switch (az_value_entry->b_type)
    {
    case sym_k_integer_value:
	_debug_output("  integer: %d \n", 
		      az_value_entry->value.l_integer );
	break;

    case sym_k_bool_value:
	_debug_output("  boolean: %d \n", 
		      az_value_entry->value.l_integer );
	break;

    case sym_k_float_value:
	_debug_output("  double: %g \n", 
		      az_value_entry->value.d_real);
	break;

    case sym_k_color_value:
    {
	char	*ptr;

	switch (az_value_entry->b_arg_type)
	{
	case sym_k_background_color:
		ptr = "background";
		break;
	case sym_k_foreground_color:
		ptr = "foreground";
		break;
	case sym_k_unspecified_color:
		ptr = "unspecified";
		break;
	default:
		ptr = "illegal";
	}

	_debug_output("  color  type: %s", ptr );

	output_text( az_value_entry->w_length, 
		     az_value_entry->value.c_value);

	break;
    }

    case sym_k_reason_value:
	special_type = "reason";
	goto common_special_type;

    case sym_k_argument_value:
	special_type = "argument";

common_special_type:

	_debug_output("  %s", special_type );

	if (az_value_entry->obj_header.b_flags & sym_m_builtin)
	    _debug_output("  code: %d \n", az_value_entry->value.l_integer );
	else
	    output_text( az_value_entry->w_length, 
			 az_value_entry->value.c_value);

	break;

    case sym_k_compound_string_value:
	_debug_output("  compound string\n  first component: %x\n", 
		      (unsigned) az_value_entry->az_first_table_value );

	if ( (az_value_entry->b_aux_flags & sym_m_table_entry) != 0 ) {
	    _debug_output("  next table entry: %x",
			  (unsigned) az_value_entry->az_next_table_value);
	}

	break;


    case sym_k_font_value:
    case sym_k_fontset_value:
	if (az_value_entry->b_charset != sym_k_userdefined_charset) 
	    _debug_output("  font  charset: %s", 
			  diag_charset_text( az_value_entry->b_charset ) );
	else
	    _debug_output("  font  charset: userdefined(%x)", 
			  (unsigned) diag_charset_text
			    ( (int)az_value_entry->az_charset_value ) );

	goto check_for_table_value;


    case sym_k_char_8_value:
	if (az_value_entry->b_charset != sym_k_userdefined_charset) 
	    switch ( az_value_entry->b_direction )
		{
		case XmSTRING_DIRECTION_L_TO_R:
		    _debug_output
			("  string length: %d\n  charset: %s  L_TO_R", 
			  az_value_entry->w_length,
			  diag_charset_text(
			      az_value_entry->b_charset ));
		    break;
		case XmSTRING_DIRECTION_R_TO_L:
		    _debug_output
			("  string length: %d\n  charset: %s  R_TO_L", 
			  az_value_entry->w_length,
			  diag_charset_text(
			      az_value_entry->b_charset ));
		    break;
		}
	else
	    switch ( az_value_entry->b_direction )
		{
		case XmSTRING_DIRECTION_L_TO_R:
		    _debug_output
			("  string length: %d\n  charset: userdefined(%x)  L_TO_R", 
			  az_value_entry->w_length,
			  (unsigned) az_value_entry->az_charset_value);
		    break;
		case XmSTRING_DIRECTION_R_TO_L:
		    _debug_output
			("  string length: %d\n  charset: userdefined(%x)  R_TO_L", 
			  az_value_entry->w_length,
			  (unsigned) az_value_entry->az_charset_value);
		    break;
		}

/*	See if this is an entry in a table.	*/
check_for_table_value:

	if ( (az_value_entry->b_aux_flags & sym_m_table_entry) != 0 ) {
	    _debug_output("  next table entry: %x",
			  (unsigned) az_value_entry->az_next_table_value);
	}

	output_text
	    ( az_value_entry->w_length, az_value_entry->value.c_value );

	break;

    case sym_k_identifier_value:
	_debug_output("  identifier: %s", az_value_entry->value.c_value );

	break;

    case sym_k_icon_value:
	_debug_output("  icon  width: %d  height: %d  colors: %x  rows: %x \n", 
		      az_value_entry->value.z_icon->w_width,
		      az_value_entry->value.z_icon->w_height,
		      (unsigned) az_value_entry->value.z_icon->az_color_table,
		      (unsigned) az_value_entry->value.z_icon->az_rows );

	break;

    case sym_k_string_table_value:
	table_type = "string table";
	goto common_table;

    case sym_k_font_table_value:
	table_type = "font table";
	goto common_table;

    case sym_k_trans_table_value:
	table_type = "translation table";

common_table:

	_debug_output("  %s  first table entry: %x\n",
		table_type, (unsigned) az_value_entry->az_first_table_value);

	break;

    case sym_k_color_table_value:
    {
	int	index;

	_debug_output("  color_table  count: %d  max_index: %d \n",
		      az_value_entry->b_table_count,
		      az_value_entry->b_max_index );

        for (index = 0;  index < az_value_entry->b_table_count;  index++)
	{

	    _debug_output( "    letter: %c  index: %d  color: %x\n",
		     az_value_entry->value.z_color[index].b_letter,
		     az_value_entry->value.z_color[index].b_index,
		     (unsigned) az_value_entry->value.z_color[index].az_color );
	}

	break;
    }

    case sym_k_error_value:
	_debug_output("  error \n");

	break;

    default:
	_debug_output(" unknown type: %d \n", az_value_entry->b_type );
    }

    /* preserve comments */
    _debug_output ("\nComment: %s\n",az_value_entry->obj_header.az_comment);


}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This function will output an arbitrarily long amount of text
**	with the debug output.
**
**  FORMAL PARAMETERS:
**
**      length	    length of the text
**	text	    pointer to the text
**
**  IMPLICIT INPUTS:
**
**      none
**
**  IMPLICIT OUTPUTS:
**
**      none
**
**  FUNCTION VALUE:
**
**      void
**
**  SIDE EFFECTS:
**
**      text is placed in the debug output
**
**--
**/

void	output_text(length, text)

XmConst int	length;
XmConst char	*text;
{

    char    		c_buffer[ 71 ];
    XmConst char	*c_ptr;
    int	    		l_length;

    l_length = length;

    _debug_output( "\n" );

    for (c_ptr = text;  

	 l_length > 0;  

	 l_length -= 70,
	 c_ptr += 70)
    {
	int	last;
	int	i;

	last = ( l_length > 70)? 70: l_length;

	_move( c_buffer, c_ptr, last );

	for (i=0;  i<last;  i++)
	{
	    if (iscntrl( c_buffer[ i ] ))
		c_buffer[ i ] = '.';
	}

	c_buffer[ last ] = 0;			
	_debug_output( "    \"%s\"\n", c_buffer );
    }
}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**	This procedure dumps the source information in the header of symbol
**	entries.
**
**  FORMAL PARAMETERS:
**
**
**  IMPLICIT INPUTS:
**
**
**  IMPLICIT OUTPUTS:
**
**
**  SIDE EFFECTS:
**
**
**--
**/
void sym_dump_source_info (hdr)

sym_entry_header_type *hdr;

{
    src_source_record_type *src_rec;

    src_rec = hdr->az_src_rec;

    if (src_rec != NULL)
	_debug_output ("  Source position:	file %d, line %d, columns %d through %d\n",
		src_rec->b_file_number,
		src_rec->w_line_number,
		hdr->b_src_pos,
		hdr->b_end_pos);
    else
	_debug_output ("  Src source record not present.\n");

}


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This procedure dumps the common header of "object" entries.
**
**  FORMAL PARAMETERS:
**
**
**  IMPLICIT INPUTS:
**
**
**  IMPLICIT OUTPUTS:
**
**
**  SIDE EFFECTS:
**
**
**--
**/

void sym_dump_obj_header (az_obj_entry)

XmConst sym_obj_entry_type  *az_obj_entry;

{

    _debug_output
	( "%x %s  size: %d  \n",
	  (unsigned) az_obj_entry,
	  diag_tag_text (az_obj_entry->header.b_tag),
	  az_obj_entry->header.w_node_size);

    if (az_obj_entry->obj_header.az_name != NULL) {
	_debug_output ("  name: %x", 
		       (unsigned) az_obj_entry->obj_header.az_name);
    }

    if (az_obj_entry->obj_header.az_reference != NULL) {
	_debug_output ("  reference: %x",
			(unsigned) az_obj_entry->obj_header.az_reference);
    }

    if (az_obj_entry->obj_header.az_next != NULL) {
	_debug_output ("  next: %x", 
		       (unsigned) az_obj_entry->obj_header.az_next);
    }

    if (az_obj_entry->obj_header.b_flags & sym_m_private) {
	_debug_output (" private");
    }

    if (az_obj_entry->obj_header.b_flags & sym_m_exported) {
	_debug_output (" exported");
    }

    if (az_obj_entry->obj_header.b_flags & sym_m_imported) {
	_debug_output (" imported");
    }

    _debug_output ("\n");
}



void sym_dump_include_file (az_symbol_entry)

sym_include_file_entry_type *az_symbol_entry;
{

    _debug_output ("%x  include file  file name: %s  full file name: %s\n",
	(unsigned) az_symbol_entry,
	az_symbol_entry->file_name, az_symbol_entry->full_file_name);

}



void sym_dump_section (az_symbol_entry)

sym_section_entry_type *az_symbol_entry;
{
    _debug_output ("%x  %s section  prev section : %x  next section: %x  entries: %x\n",
	(unsigned) az_symbol_entry,
	sym_section_text(az_symbol_entry->header.b_type),
	(unsigned) az_symbol_entry->prev_section, 
        (unsigned) az_symbol_entry->next, 
	(unsigned) az_symbol_entry->entries);

}



void sym_dump_object_variant (az_symbol_entry)

sym_def_obj_entry_type * az_symbol_entry;
{
    _debug_output ("%x  default obj var  next: %x  object info: %d, variant_info: %d\n",
	(unsigned) az_symbol_entry,
	(unsigned) az_symbol_entry->next, 
        az_symbol_entry->b_object_info, 
	az_symbol_entry->b_variant_info);
}



void sym_dump_root_entry (az_symbol_entry)

sym_root_entry_type  *az_symbol_entry;
{
    _debug_output (  "%x  root  tag: %d  module: %x  sections: %x\n  module tail: ",
	(unsigned) az_symbol_entry,
	az_symbol_entry->header.b_tag,
	(unsigned) az_symbol_entry->module_hdr,
	(unsigned) az_symbol_entry->sections);
}



char *sym_section_text (b_type)

int b_type;
{
    switch (b_type)
    {
	case 0:
	    return "";
	case sym_k_list_section:
	    return "list";
	case sym_k_procedure_section:
	    return "procedure";
	case sym_k_value_section:
	    return "value";
	case sym_k_identifier_section:
	    return "identifier";
	case sym_k_object_section:
	    return "object";
	case sym_k_include_section:
	    return "include";
	case sym_k_section_tail:
	    return "tail";
	default:
	    return "*unknown*";
    }
}



