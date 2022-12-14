/*** DTB_USER_CODE_START vvv Add file header below vvv ***/
/*** DTB_USER_CODE_END   ^^^ Add file header above ^^^ ***/

/*
 * File: sdt_firstlogin.h
 * Contains: object data structures and callback declarations
 *
 * This file was generated by dtcodegen, from project sdt_firstlogin
 *
 * Any text may be added between the DTB_USER_CODE_START and
 * DTB_USER_CODE_END comments (even non-C code). Descriptive comments
 * are provided only as an aid.
 *
 *  ** EDIT ONLY WITHIN SECTIONS MARKED WITH DTB_USER_CODE COMMENTS.  **
 *  ** ALL OTHER MODIFICATIONS WILL BE OVERWRITTEN. DO NOT MODIFY OR  **
 *  ** DELETE THE GENERATED COMMENTS!                                 **
 */
#ifndef _SDT_FIRSTLOGIN_H_
#define _SDT_FIRSTLOGIN_H_

#include <stdlib.h>
#include <X11/Intrinsic.h>
#include <nl_types.h>
#define DTB_PROJECT_CATALOG	"sdt_firstlogin"
/* Handle for standard message catalog for the project */
extern nl_catd	Dtb_project_catd;

/*
 * Structure to store values for Application Resources
 */
typedef struct {
    char	*session_file;
    
    /*** DTB_USER_CODE_START vvv Add structure fields below vvv ***/
    char	*in_file; 
    char	*out_file; 
    char	*label_font; 
    /*** DTB_USER_CODE_END   ^^^ Add structure fields above ^^^ ***/
} DtbAppResourceRec;


extern DtbAppResourceRec	dtb_app_resource_rec;


/**************************************************************************
 *** DTB_USER_CODE_START
 ***
 *** Add types, macros, and externs here
 ***/

#define OW_SESSION_FILE "/usr/dt/config/Xsession.ow"
#define OW_INIT_FILE "/.openwin-init"

/*** DTB_USER_CODE_END
 ***
 *** End of user code section
 ***
 **************************************************************************/


#endif /* _SDT_FIRSTLOGIN_H_ */
