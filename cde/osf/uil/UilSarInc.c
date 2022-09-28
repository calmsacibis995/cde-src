/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.1
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: UilSarInc.c /main/cde1_maint/1 1995/07/14 20:54:55 drk $"
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
**      This module contain the routines for processing the INCLUDE construct.
**
**--
**/


/*
**
**  INCLUDE FILES
**
**/

#include <Xm/Xm.h>
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



/*
**
**  OWN VARIABLE DECLARATIONS
**
**/


/*
**++
**  FUNCTIONAL DESCRIPTION:
**
**      This routine handles the include directive.
**
**  FORMAL PARAMETERS:
**
**      file_frame	parser stack frame that contains the name of the
**			file to be included.
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

void	sar_include_file ( file_frame, include_frame, semi_frame )

yystype	    * file_frame;
yystype	    * include_frame;
yystype	    * semi_frame;

{
    sym_value_entry_type	* value_entry;
    sym_include_file_entry_type	* include_entry;
    sym_section_entry_type	* section_entry;
    sym_section_entry_type	* section_tail_entry;
    char			* buffer;
    unsigned char		  tmp1;
    int				  i,j;

    value_entry =
	(sym_value_entry_type *) file_frame -> value . az_symbol_entry;

/*
 * If the direction is RtoL then reverse the include file.
 */
    if (value_entry->b_direction == XmSTRING_DIRECTION_R_TO_L) 
	{
	/*
	**  Just reverse the bytes from the first to last
	*/
	for (i=0, j=value_entry->w_length-1;  
	     i < (value_entry->w_length>>1);  
	     i++,j--)
	    {
	    tmp1 = value_entry->value.c_value[ i ];
	    value_entry->value.c_value[ i ] = 
		value_entry->value.c_value[ j ];
	    value_entry->value.c_value[ j ] = tmp1;
	    }
	}

    _assert (value_entry -> header . b_tag == sym_k_value_entry,
	     "missing value entry for include");

    /*
    ** Allocate an INCLUDE entry
    */

    include_entry = (sym_include_file_entry_type *) sem_allocate_node
			( sym_k_include_file_entry, sym_k_include_file_entry_size );

    /*
    ** Open the include file.  Set up a null-terminated name string.
    */

    buffer = (char *) _get_memory (value_entry -> w_length + 1);
    _move (buffer, value_entry -> value . c_value,
		   value_entry -> w_length);
    buffer [value_entry -> w_length] = 0 ;

    src_open_file (buffer, include_entry->full_file_name);

    /*
    **  Allocate a section entry for this include "section".
    **  Store the source info in it
    */

    section_entry = (sym_section_entry_type *) sem_allocate_node
			( sym_k_section_entry, sym_k_section_entry_size );

    section_entry->header.b_type = sym_k_include_section;

    _sar_save_source_info ( &section_entry->header , include_frame , semi_frame );

    /*
    ** Link this section into the current section list.
    */

    section_entry->next = (sym_entry_type *) sym_az_current_section_entry;
    section_entry->prev_section = sym_az_current_section_entry->prev_section;
    sym_az_current_section_entry = section_entry;

    /*
    ** Link the include intry we created earlier onto the section list
    */

    section_entry->entries = (sym_entry_type *)include_entry;

    /*
    **  Save the file name
    */

    _move (include_entry->file_name, buffer, value_entry->w_length);
    include_entry->file_name [value_entry->w_length] =  0;

    /*
    ** Since this is an INCLUDE we want included sections to hang off
    ** of it so we create a new place holder entry (tail) and change the
    ** "current" section list to it.
    */

    section_tail_entry = (sym_section_entry_type *) sem_allocate_node
			    ( sym_k_section_entry, sym_k_section_entry_size );

    section_tail_entry->header.b_type = sym_k_section_tail;

    include_entry->sections = section_tail_entry;

    section_tail_entry->prev_section = sym_az_current_section_entry;
    sym_az_current_section_entry = section_tail_entry;

    _free_memory (buffer);

}
