
/*
 *	$XConsortium: obj_namesP.h /main/cde1_maint/2 1995/09/06 07:48:22 lehors $
 *
 *	@(#)obj_namesP.h	1.21 01 May 1995	cde_app_builder/src/abmf
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
 * obj_namesP.h - C/Widget/other names for objects
 */
#ifndef _ABMF_OBJ_NAMES__H_
#define _ABMF_OBJ_NAMESP_H_

#include "write_codeP.h"

/*
 * Objects
 */
/*
 * abmfP_get_c_name is *real* smart.  It determines the name
 * based on the state of code generation.  Use it a lot!
 */
STRING	abmfP_get_action_name(ABObj action);
STRING	abmfP_get_c_array_of_selected_bools_name(ABObj obj);
STRING	abmfP_get_c_array_of_widgets_name(ABObj obj);
STRING	abmfP_get_c_array_of_names_name(ABObj obj);
STRING	abmfP_get_c_array_of_strings_name(ABObj obj);
STRING	abmfP_get_c_array_of_mnemonics_name(ABObj obj);
STRING	abmfP_get_c_array_of_xmstrings_name(ABObj obj);
STRING	abmfP_get_c_name(GenCodeInfo genCodeInfo, ABObj obj);
STRING	abmfP_get_c_name_global(ABObj obj);
STRING	abmfP_get_c_name_in_inst(ABObj obj);
STRING	abmfP_get_c_name_in_subinst(ABObj obj);
STRING	abmfP_get_c_field_name(ABObj obj);

/*
 * Structs
 */
STRING	abmfP_get_c_struct_type_name(ABObj obj);
STRING	abmfP_get_c_struct_ptr_type_name(ABObj obj);
STRING	abmfP_get_c_struct_global_name(ABObj obj);
STRING	abmfP_get_c_struct_name(GenCodeInfo, ABObj obj);
STRING	abmfP_get_c_struct_ptr_name(GenCodeInfo, ABObj obj);

/*
 * Substructs
 */
STRING	abmfP_get_c_substruct_global_name(ABObj);
STRING	abmfP_get_c_substruct_field_name(ABObj obj);
STRING	abmfP_get_c_substruct_type_name(ABObj obj);
STRING	abmfP_get_c_substruct_ptr_type_name(ABObj obj);
STRING	abmfP_get_c_substruct_name(GenCodeInfo, ABObj obj);

/*
 * Procedures
 */
STRING	abmfP_get_project_init_proc_name(ABObj obj);
STRING	abmfP_get_clear_proc_name(ABObj obj);
STRING 	abmfP_get_msg_clear_proc_name(ABObj module);
STRING	abmfP_get_create_proc_name(ABObj obj);
STRING	abmfP_get_init_proc_name(ABObj obj);

/*
 * Files
 */
STRING	abmfP_get_project_c_file_name(ABObj obj);
STRING	abmfP_get_project_header_file_name(ABObj obj);
STRING	abmfP_get_utils_c_file_name(ABObj obj);
STRING	abmfP_get_utils_header_file_name(ABObj obj);
STRING	abmfP_get_stubs_c_file_name(ABObj obj);
STRING	abmfP_get_ui_c_file_name(ABObj obj);
STRING	abmfP_get_ui_header_file_name(ABObj obj);
STRING	abmfP_get_exe_file_name(ABObj obj);

/*
 * Widgets
 */
STRING	abmfP_get_app_class_name(ABObj obj);
STRING	abmfP_get_widget_name(ABObj obj);
STRING	abmfP_get_widget_name_for_res_file(ABObj obj);

/*
 * Miscellaneous
 */
STRING	abmfP_get_control_imm_parent_name(ABObj obj);
STRING	abmfP_get_c_app_root_win_name(ABObj obj);

#endif /* _ABMF_OBJ_NAMESP_H_ */
