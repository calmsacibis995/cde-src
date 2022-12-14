/*** DTB_USER_CODE_START vvv Add file header below vvv ***/
/*** DTB_USER_CODE_END   ^^^ Add file header above ^^^ ***/

/*
 * File: dtmailopts.C
 * Contains: dtb_dtmailopts_initialize and cross-module connections
 *
 * This file was generated by dtcodegen, from project dtmailopts
 *
 * Any text may be added between the DTB_USER_CODE_START and
 * DTB_USER_CODE_END comments (even non-C code). Descriptive comments
 * are provided only as an aid.
 *
 *  ** EDIT ONLY WITHIN SECTIONS MARKED WITH DTB_USER_CODE COMMENTS.  **
 *  ** ALL OTHER MODIFICATIONS WILL BE OVERWRITTEN. DO NOT MODIFY OR  **
 *  ** DELETE THE GENERATED COMMENTS!                                 **
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/Protocols.h>
#include <Dt/Help.h>
#include <Dt/HelpQuickD.h>
#include <Dt/HelpDialog.h>
#include <nl_types.h>
#include "options_ui.h"
#include "dtmailopts.h"
#include "dtb_utils.h"


/**************************************************************************
 *** DTB_USER_CODE_START
 ***
 *** All necessary header files have been included.
 ***
 *** Add include files, types, macros, externs, and user functions here.
 ***/

/*** DTB_USER_CODE_END
 ***
 *** End of user code section
 ***
 **************************************************************************/


/* Handle for standard message catalog for the project */
nl_catd	Dtb_project_catd = (nl_catd)-1;

DtbAppResourceRec	dtb_app_resource_rec;


/*
 * initialization for application dtmailopts
 * This should be called from user-defined main()
 */
int 
dtb_dtmailopts_initialize(
    int *argc_in_out,
    char **argv_in_out[],
    Widget toplevel
)
{
    #define argc (*argc_in_out)
    #define argv (*argv_in_out)
    XtLanguageProc	language_proc = (XtLanguageProc)NULL;
    XtPointer		language_proc_client_data = (XtPointer)NULL;
    
    
    /**************************************************************************
     *** DTB_USER_CODE_START
     ***
     ***  No initialization has been done.
     ***
     *** Add local variables and code.
     ***/
    
    /*** DTB_USER_CODE_END
     ***
     *** End of user code section
     ***
     **************************************************************************/
    

    /*
     * Get display and verify initialization was successful.
     */
    if (toplevel == NULL)
    {
        fprintf(stderr, "\nWARNING: dtb_dtmailopts_initialize() called with invalid widget.\n");
        fprintf(stderr, "Initialization failed.\n\n");
        return -1;
    }
    
    /*
     * Open the standard message catalog for the project.
     */
    Dtb_project_catd = catopen(DTB_PROJECT_CATALOG, NL_CAT_LOCALE);
    if (Dtb_project_catd == (nl_catd)-1)
    {
        fprintf(stderr, "WARNING: Could not open message catalog: %s. Messages will be defaults.\n",
            DTB_PROJECT_CATALOG);
    }
    
    /*
     * Save the toplevel widget so it can be fetched later as needed.
     */
    dtb_save_toplevel_widget(toplevel);
    
    /*
     * Save the command used to invoke the application.
     */
    dtb_save_command(argv[0]);
    
    /**************************************************************************
     *** DTB_USER_CODE_START
     ***
     ***      A connection to the X server has been established, and all
     *** initialization has been done.
     ***
     ***  Add extra initialization code after this comment.
     ***/
    
    /*** DTB_USER_CODE_END
     ***
     *** End of user code section
     ***
     **************************************************************************/
    

    /*
     * Initialize all global variables.
     */
    dtbOptionsDialogInfo_clear(&dtb_options_dialog);
    
    
    /*
     * Map initially-visible windows
     */
    dtb_options_dialog_initialize(&(dtb_options_dialog), dtb_get_toplevel_widget());
    XtManageChild(dtb_options_dialog.dialog_shellform);
    
    
    
    
    
    /**************************************************************************
     *** DTB_USER_CODE_START
     ***
     ***      All initially-mapped widgets have been created, but not
     *** realized. Set resources on widgets, or perform other operations
     *** that must be completed before the toplevel widget is
     *** realized.
     ***/
    
    /*** DTB_USER_CODE_END
     ***
     *** End of user code section
     ***
     **************************************************************************/
    

    
    /**************************************************************************
     *** DTB_USER_CODE_START
     ***
     ***      All automatic initialization is complete. Add any additional
     *** code that should be executed before the Xt main loop is entered.
     ***/
    
    /*** DTB_USER_CODE_END
     ***
     *** End of user code section
     ***
     **************************************************************************/
    

    return 0;
    #undef argc
    #undef argv
}


/**************************************************************************
 *** DTB_USER_CODE_START
 ***
 *** All automatically-generated data and functions have been defined.
 ***
 *** Add new functions here, or at the top of the file.
 ***/

/*** DTB_USER_CODE_END
 ***
 *** End of user code section
 ***
 **************************************************************************/


