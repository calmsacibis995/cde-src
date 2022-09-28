/*** DTB_USER_CODE_START vvv Add file header below vvv ***/
/*** DTB_USER_CODE_END   ^^^ Add file header above ^^^ ***/

/*
 * File: sdtactrl.c
 * Contains: main() and cross-module connections
 *
 * This file was generated by dtcodegen, from project sdtactrl
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
#include "audio_control_ui.h"
#include "sdtactrl.h"
#include "dtb_utils.h"


/**************************************************************************
 *** DTB_USER_CODE_START
 ***
 *** All necessary header files have been included.
 ***
 *** Add include files, types, macros, externs, and user functions here.
 ***/
#include "audio_util.h"

#define MessageCatalog "sdtaudio.cat"

nl_catd         msgCatalog;

/*** DTB_USER_CODE_END
 ***
 *** End of user code section
 ***
 **************************************************************************/



/* Workaround for XPG4 API compatibility */
#if !defined(NL_CAT_LOCALE)
#define NL_CAT_LOCALE 0
#endif

/* Handle for standard message catalog for the project */
nl_catd	Dtb_project_catd = (nl_catd)-1;

/*
 * command line options...
 */
static XrmOptionDescRec optionDescList[] = {
    {"-session", "*session", XrmoptionSepArg, (XPointer)NULL}
    
    /*** DTB_USER_CODE_START vvv Add structure fields below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add structure fields above ^^^ ***/
};

DtbAppResourceRec	dtb_app_resource_rec;


/*
 * main for application sdtactrl
 */
int 
main(
    int argc,
    char **argv
)
{
    Widget		toplevel = (Widget)NULL;
    XtAppContext	app = (XtAppContext)NULL;
    String		*fallback_resources = (String*)NULL;
    ArgList		init_args = (ArgList)NULL;
    Cardinal		num_init_args = (Cardinal)0;
    ArgList		get_resources_args = (ArgList)NULL;
    Cardinal		num_get_resources_args = (Cardinal)0;
    XtLanguageProc	language_proc = (XtLanguageProc)NULL;
    XtPointer		language_proc_client_data = (XtPointer)NULL;
    
    
    /**************************************************************************
     *** DTB_USER_CODE_START
     ***
     ***  No initialization has been done.
     ***
     *** Add local variables and code.
     ***/

        /* Initialize message catalog. */
        msgCatalog = catopen(MessageCatalog, NL_CAT_LOCALE);

    /*** DTB_USER_CODE_END
     ***
     *** End of user code section
     ***
     **************************************************************************/
    

    /* NULL language_proc installs the default Xt language procedure */
    XtSetLanguageProc((XtAppContext)NULL,
        language_proc, language_proc_client_data);
    
    toplevel = XtAppInitialize(&app, "Sdtactrl",
        optionDescList, XtNumber(optionDescList),
        &argc, argv, fallback_resources,
        init_args, num_init_args);

    /*
     * Get display and verify initialization was successful.
     */
    if (toplevel == NULL)
    {
        fprintf(stderr, "Could not open display.\n");
        exit(1);
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
    dtbAudioControlMainWindowInfo_clear(&dtb_audio_control_main_window);
    
    /*
     * Set up the application's root (primary) window.
     */
    dtb_audio_control_main_window.main_window = toplevel;
    XtVaSetValues(dtb_audio_control_main_window.main_window,
        XmNallowShellResize, True,
        XmNtitle, catgets(Dtb_project_catd, 2, 1, "Desktop Audio Control Panel"),
        XmNinitialState, NormalState,
        XmNbackground, dtb_cvt_string_to_pixel(dtb_audio_control_main_window.main_window, "white"),
        XmNmwmDecorations, MWM_DECOR_ALL | MWM_DECOR_RESIZEH,
        XmNmwmFunctions, MWM_FUNC_ALL | MWM_FUNC_RESIZE,
        NULL);
    
    Initialize();
    dtb_audio_control_main_window_initialize(&(dtb_audio_control_main_window), dtb_get_toplevel_widget());
    
    
    
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
    

    XtRealizeWidget(toplevel);

    
    /**************************************************************************
     *** DTB_USER_CODE_START
     ***
     ***      All automatic initialization is complete. Add any additional
     *** code that should be executed before the Xt main loop is entered.
     ***/
    SysUpdateWidgets();
    SysSetSigPollHandler();
    
    /*** DTB_USER_CODE_END
     ***
     *** End of user code section
     ***
     **************************************************************************/
    

    /*
     * Enter event loop
     */
    XtAppMainLoop(app);
    return 0;
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

