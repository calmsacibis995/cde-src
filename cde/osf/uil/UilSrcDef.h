/* 
 * (c) Copyright 1989, 1990, 1991, 1992 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2
*/ 
/*   $XConsortium: UilSrcDef.h /main/cde1_maint/1 1995/07/14 20:56:52 drk $ */

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
**      This include file defines the interface to the UIL source
**	management procedures in the UIL compiler.
**
**--
**/

#ifndef UilSrcDef_h
#define UilSrcDef_h


/*
**  Interface to src_open_source
*/

#define src_k_open_normal   1	    /* open succeeded */
#define src_k_open_error    0	    /* open was unsuccessful */

/*
** Close return statuses
*/

#define src_k_close_normal	1	/* close succeeded */
#define src_k_close_error	0	/* close was unsuccessful */

/*
**  Interface to src_get_source_line
*/

#define src_k_read_truncated	3   /* record truncated */
#define src_k_read_error    	2   /* error during read */
#define src_k_read_normal   	1   /* record read normally */
#define src_k_end_source    	0   /* end of source */

/*
**  Source records describe the lines of the source program.  The are
**  used to retrieve the source file for the listing and diagnostics.
**  Diagnostics randomly access these data structures by maintaining
**  the address of the corresponding source record in tokens, the parse
**  stack and symbol table entries.  The listing file walks the source
**  records via the linked list provide by ar_next_source_record.
*/






#define _src_null_access_key( _key ) (_key.l_key == EOF)
#define src_k_key_length 4

typedef struct
{
    unsigned long	l_key;
} z_key;

typedef struct  _src_message_item_type
{
    struct _src_message_item_type   *az_next_message;
    status			    l_message_number;
    unsigned char		    b_source_pos;
    char			    c_text[ 1 ];
} src_message_item_type;

#define src_message_item_type_size \
    (sizeof(src_message_item_type) -\
     sizeof( struct _src_message_item_type.c_text))

typedef struct  _src_machine_code_type
{
    struct _src_machine_code_type   *az_next_machine_code;
    unsigned short int		    w_offset;
    unsigned short int		    w_code_len;
    char			    c_data[ 1 ];
} src_machine_code_type;

#define src_machine_code_type_size \
    (sizeof(src_machine_code_type) -\
     sizeof( struct _src_machine_code_type.c_text))

/*
**  Mask for bits in b_flags of the source record
*/

#define src_m_form_feed			(1<<0)
#define src_m_unprintable_chars		(1<<1)

typedef struct  _src_source_record_type
{
    struct _src_source_record_type  *az_next_source_record;
    src_message_item_type	    *az_message_list;    
    unsigned short		    w_line_number;
    unsigned char		    b_file_number;
    unsigned char		    b_flags;
    z_key			    z_access_key;
    src_machine_code_type	    *az_machine_code_list;    
    unsigned short		    w_machine_code_cnt;
} src_source_record_type;

/*
**  Source buffers describe the lines of the source program that are
**  currently being SCAN.  Include files and macros a can cause a program
**  to have more than 1 source buffer.  These buffers are managed as a
**  stack, implemented via a linked list.
*/

typedef struct	_src_source_buffer_type
{
    struct _src_source_buffer_type  *az_prior_source_buffer;
    unsigned short		    w_current_line_number;
    unsigned short		    w_current_position;
    char			    b_file_number;
    char			    c_text[ src_k_max_source_line_length+1];
} src_source_buffer_type;

#endif /* UilSrcDef_h */
/* DON'T ADD STUFF AFTER THIS #endif */
