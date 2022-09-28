/*** DTB_USER_CODE_START vvv Add file header below vvv ***/
#include <sys/param.h>
#include <Xm/ToggleB.h>
#include <unistd.h>
/*** DTB_USER_CODE_END   ^^^ Add file header above ^^^ ***/

/*
 * File: sdt_firstlogin_stubs.c
 * Contains: Module callbacks and connection functions
 *
 * This file was generated by dtcodegen, from module sdt_firstlogin
 *
 * Any text may be added between the DTB_USER_CODE_START and
 * DTB_USER_CODE_END comments (even non-C code). Descriptive comments
 * are provided only as an aid.
 *
 *  ** EDIT ONLY WITHIN SECTIONS MARKED WITH DTB_USER_CODE COMMENTS.  **
 *  ** ALL OTHER MODIFICATIONS WILL BE OVERWRITTEN. DO NOT MODIFY OR  **
 *  ** DELETE THE GENERATED COMMENTS!                                 **
 */

#include <stdio.h>
#include <Xm/Xm.h>
#include "dtb_utils.h"
#include "sdt_firstlogin.h"
#include "sdt_firstlogin_ui.h"


/**************************************************************************
 *** DTB_USER_CODE_START
 ***
 *** All necessary header files have been included.
 ***
 *** Add include files, types, macros, externs, and user functions here.
 ***/

#include <errno.h>
int errno;

char *session;

void
Item_Toggled(
    Widget widget,
    XtPointer clientData,
    XmToggleButtonCallbackStruct *state
)
{
    if (state->set) {
       /* save selected session command */ 
       XtVaGetValues(widget, XmNuserData, &session, NULL);
    }
}

/*** DTB_USER_CODE_END
 ***
 *** End of user code section
 ***
 **************************************************************************/



void 
sdt_firstlogin_button2_CB1(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    	/* Cancel button has been called, return a non-zero exit code */

	exit(2);/*** DTB_USER_CODE_START vvv Add C code below vvv ***/
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
FL_Select_OK_Button(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    FILE *fp;
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/

    /* OK Button selected.  Record last session & start it */ 

    if ( session && dtb_app_resource_rec.out_file ) { 

        _DtCreateDtDirs( XtDisplay(widget) );
	fp = fopen(dtb_app_resource_rec.out_file, "w");

	if (fp) {
	   fputs(session, fp);
	   fclose(fp);
	}
    }

    if (execl(session, session, (char *) 0) == -1) {
	char* err = strerror(errno);

	fp = fopen(mktemp("/tmp/sdt_firstlogin_errorXXXXXX"), "w");

	if (fp) {
	    if (session) {
	        fputs(session, fp); 
	    }

	    fputs(" execl failed in firstlogin\n", fp);
	    fputs(err, fp);

	    if (fp) {
	        fclose(fp);
	    }
	}
	exit(-1);
    }

    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
FL_Create_Choices(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    DtbSdtFirstloginMainwindowInfo	dtbSource = (DtbSdtFirstloginMainwindowInfo)callData;
    
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    FILE *fp;
    char buffer1[MAXPATHLEN];
    char buffer2[MAXPATHLEN];
    char ow_init_file[MAXPATHLEN];
    XmString label_xmstring=NULL;
    Arg	args[22];	
    int first=True;
    int i;
    Widget button;
    Widget first_button;
    char *pcmd;
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/

    if ( dtb_app_resource_rec.in_file) { 

        /* Open data file specified by -infile command line option */ 

	if (! (fp=fopen(dtb_app_resource_rec.in_file, "r"))) {
	    exit(0);
	} 

	XtDestroyWidget(dtbSource->radiobox_items.Item1_item);

	while (fgets(buffer1, MAXPATHLEN, fp) != NULL) { 

	    /* Create radio box toggle buttons */

	    i=0;
	    if (fgets(buffer2, MAXPATHLEN, fp) != NULL) {
	        i++;

	 	/* strip trailing new line */
		buffer2[ strlen(buffer2) - 1] = 0;

	        label_xmstring = XmStringCreateLocalized(buffer1);
	        button = XtVaCreateManagedWidget(
			buffer1, xmToggleButtonWidgetClass, widget, 
			XmNset, first==True, 
			XmNlabelString, label_xmstring, 
			XmNfontList, dtb_app_resource_rec.label_font,
			XmNuserData, pcmd=strdup(buffer2), NULL);
                XtAddCallback(button, XmNvalueChangedCallback, 
				(XtCallbackProc) Item_Toggled, (XtPointer) i);
	        XmStringFree(label_xmstring);
	        label_xmstring = NULL;

		if (first) {
		    /* First button is pre-selected for users
		     * with no previous desktop preferences.
		     */
		    first=False;
		    first_button = button;
		    session = pcmd;
		} else {
		    if (strcmp(pcmd, OW_SESSION_FILE) == 0) {

			strcpy(ow_init_file, getenv("HOME"));
			strcat(ow_init_file, OW_INIT_FILE);

			if (access(ow_init_file, R_OK) == 0) {
		           /* User saved an OpenWindows layout once,
			    * pre-select OW button.
			    */

			    XtSetArg(args[0], XmNset, False);
			    XtSetValues(first_button, args, 1);

			    XtSetArg(args[0], XmNset, True);
			    XtSetValues(button, args, 1);

		    	    session = pcmd;
			}
		    }
		}
	    }
	}

	fclose(fp);
	unlink(dtb_app_resource_rec.in_file);
    } else {
	exit(0);
    }
    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
FL_Create_OK_Button(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    DtbSdtFirstloginMainwindowInfo	dtbSource = (DtbSdtFirstloginMainwindowInfo)callData;
    
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    Arg	args[22];	
    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/

    XtSetArg(args[0], XmNdefaultButton, widget);
    XtSetValues(dtbSource->mainwindow_form, args, 1);

    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
}


void 
FL_Main_Window_Created(
    Widget widget,
    XtPointer clientData,
    XtPointer callData
)
{
    DtbSdtFirstloginMainwindowInfo	dtbSource = (DtbSdtFirstloginMainwindowInfo)callData;
    
    /*** DTB_USER_CODE_START vvv Add C variables and code below vvv ***/
    Arg argt[22];
    int cont_width, cont_height;

    Display *disp = XtDisplay(dtbSource->mainwindow);
    int scr = DefaultScreen(disp);
    int scr_width = DisplayWidth(disp, scr);
    int scr_height = DisplayHeight(disp, scr); 

    /*** DTB_USER_CODE_END   ^^^ Add C variables and code above ^^^ ***/
    
    /*** DTB_USER_CODE_START vvv Add C code below vvv ***/

    /* Size window to match display width and height */

    XtVaSetValues(dtbSource->mainwindow, 
		  XmNwidth, scr_width, XmNheight, scr_height, NULL);

    /*** DTB_USER_CODE_END   ^^^ Add C code above ^^^ ***/
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

