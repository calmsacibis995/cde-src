/* $XConsortium: broadcast.c /main/cde1_maint/2 1995/10/04 12:46:49 gtsang $ */

/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company	
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 */

/* broadcast - dynamic pattern and procedural notification example */

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <string.h>
#include <Xm/XmAll.h>
#include <Tt/tt_c.h>

static Widget toplevel, base_frame, controls, slider, gauge, button ;
static char *my_procid;

static void	broadcast_value();
static void	receive_tt_message();
static void	create_ui_components();


void
main(argc, argv)
	int		argc;
	char		**argv;
{
	int ttfd;
	Tt_pattern pat;
	XtAppContext app;

	/*
	 * Initialize Motif and create ui components
	 */
	toplevel = XtVaAppInitialize(&app, "Broadcast", NULL, 0,
					&argc, argv, NULL, NULL);
	XtVaSetValues(toplevel, XmNtitle, "ToolTalk Broadcast", 
         XmNmwmDecorations, MWM_DECOR_BORDER | MWM_DECOR_TITLE | 
                            MWM_DECOR_MENU | MWM_DECOR_MINIMIZE, 0);

	create_ui_components();

	/*
	 * Initialize ToolTalk, using the initial default session, and
	 * obtain the file descriptor that will become active whenever
	 * ToolTalk has a message for this process.
	 */

	my_procid = tt_open();
	ttfd = tt_fd();

	/*
	 * Arrange for Motif to call receive_tt_message when the ToolTalk
	 * file descriptor becomes active.
	 */

	XtAppAddInput(app, ttfd, (XtPointer) XtInputReadMask,
                      receive_tt_message, 0);

	/*
	 * Create and register a pattern so ToolTalk knows we are interested
	 * in "ttsample1_value" messages within the session we join.
	 */

	pat = tt_pattern_create();
	tt_pattern_category_set(pat, TT_OBSERVE);
	tt_pattern_scope_add(pat, TT_SESSION);
	tt_pattern_op_add(pat, "ttsample1_value");
	tt_pattern_register(pat);

	/*
	 * Join the default session
	 */

	tt_session_join(tt_default_session());

	/*
	 * Turn control over to Motif.
	 */
	XtRealizeWidget(toplevel);
	XtAppMainLoop(app);
	
	/*
	 * Before leaving, allow ToolTalk to clean up.
	 */
	tt_close();

	exit(0);
}


/*
 * When the button is pressed, broadcast the new slider value.
 */
static void
broadcast_value(widget, client_data, call_data)
	Widget widget;
	XtPointer client_data, call_data;
{
	int slider_value;
	Tt_message msg_out;

	/*
	 * Create and send a ToolTalk notice message
	 * ttsample1_value(in int <new value)
	 */

	XtVaGetValues(slider, XmNvalue, &slider_value, 0);
	msg_out = tt_pnotice_create(TT_SESSION, "ttsample1_value");
	tt_message_arg_add(msg_out, TT_IN, "integer", NULL);
	tt_message_arg_ival_set(msg_out, 0, slider_value);
	tt_message_send(msg_out);

	/*
	 * Since this message is a notice, we don't expect a reply, so
	 * there's no reason to keep a handle for the message.
	 */

	tt_message_destroy(msg_out);
}

/*
 * When a ToolTalk message is available, receive it; if it's a
 * ttsample1_value message, update the gauge with the new value.
 */
static void
receive_tt_message(client_data, fid, id)
XtPointer client_data;
int *fid;
XtInputId *id;
{
	Tt_message msg_in;
	int mark;
	int val_in;
	char *op;
	Tt_status err;

	msg_in = tt_message_receive();

	/*
	 * It's possible that the file descriptor would become active
	 * even though ToolTalk doesn't really have a message for us.
	 * The returned message handle is NULL in this case.
	 */

	if (msg_in == NULL) return;


	/*
	 * Get a storage mark so we can easily free all the data
	 * ToolTalk returns to us.
	 */

	mark = tt_mark();

	op = tt_message_op(msg_in);
	err = tt_ptr_error(op);
	if (err > TT_WRN_LAST) {
		printf( "tt_message_op(): %s\n", tt_status_message(err));
	} else if (op != 0) {
		if (0==strcmp("ttsample1_value", tt_message_op(msg_in))) {
			tt_message_arg_ival(msg_in, 0, &val_in);
			XtVaSetValues(gauge, XmNvalue, val_in, 0);
		}
	}

	tt_message_destroy(msg_in);
	tt_release(mark);
	return;
}
	    
/*
 * Straight Motif calls for creating the ui elements.  No
 * ToolTalk-specific code here.
 */
static void
create_ui_components()
{
	int decor;
	Widget glabel, slabel;
	XmString label;

	base_frame =
            XtVaCreateManagedWidget("base_frame",
                xmMainWindowWidgetClass,
                toplevel, 
                NULL);

	controls =
            XtVaCreateManagedWidget("controls",
                xmFormWidgetClass,
                base_frame,
                XmNresizePolicy, XmRESIZE_GROW,
                XmNmarginHeight, 25,
                XmNmarginWidth, 10,
                XmNy, 0,
                XmNx, 0,
                NULL);

	slider =
            XtVaCreateManagedWidget("slider",
                xmScaleWidgetClass,
                controls,
                XmNshowValue, False,
                XmNvalue, 50,
                XmNdecimalPoints, 0,
                XmNscaleMultiple, 1,
                XmNmaximum, 100,
                XmNminimum, 0,
                XmNprocessingDirection, XmMAX_ON_RIGHT,
                XmNorientation, XmHORIZONTAL,
                "slidingMode", False,
                XmNheight, 27,
                XmNwidth, 181,
                XmNtopAttachment, XmATTACH_FORM,
                XmNleftPosition, 40,
                XmNleftAttachment, XmATTACH_POSITION,
                XmNrightAttachment, XmATTACH_FORM,  
                NULL);

        label = XmStringCreateLocalized("Send:  ");
        slabel =
            XtVaCreateManagedWidget("slabel",
                xmLabelWidgetClass,
                controls,
                XmNlabelType, XmSTRING,
                XmNlabelString, label,
                XmNtopAttachment, XmATTACH_FORM,
                XmNrightOffset, 5, 
                XmNrightAttachment, XmATTACH_WIDGET,
                XmNrightWidget, slider,   
                NULL);
        XmStringFree(label);

        label = XmStringCreateLocalized("Received:  ");
        glabel =
            XtVaCreateManagedWidget("glabel",
                xmLabelWidgetClass,
                controls,
                XmNlabelType, XmSTRING,
                XmNlabelString, label,
                XmNtopOffset, 20,
                XmNtopAttachment, XmATTACH_WIDGET,
                XmNtopWidget, slabel,
                XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
                XmNrightWidget, slabel,
                NULL);
        XmStringFree(label);
        label = NULL;

	gauge =
            XtVaCreateManagedWidget("gauge",
                xmScaleWidgetClass,
                controls,
                XmNshowValue, True,
                XmNvalue, 50,
                XmNdecimalPoints, 0,
                XmNscaleMultiple, 1,
                XmNmaximum, 100,
                XmNminimum, 0,
                XmNprocessingDirection, XmMAX_ON_RIGHT,
                XmNorientation, XmHORIZONTAL,
                "slidingMode", False,
                XmNheight, 43,
                XmNwidth, 181,
                XmNtopOffset, 5,
                XmNtopAttachment, XmATTACH_WIDGET,
                XmNtopWidget, slider,
                XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
                XmNleftWidget, slider,
                NULL);

        label = XmStringCreateLocalized(" broadcast ");
	button =
            XtVaCreateManagedWidget("button",
                xmPushButtonWidgetClass,
                controls,
                XmNleftPosition, 30,
                XmNleftAttachment, XmATTACH_POSITION,
                XmNtopOffset, 25,
                XmNtopAttachment, XmATTACH_WIDGET,
 		XmNtopWidget, gauge,
                XmNbottomAttachment, XmATTACH_FORM,
                XmNrecomputeSize, True,
                XmNalignment, XmALIGNMENT_CENTER,
                XmNlabelType, XmSTRING,
                XmNlabelString, label,
                NULL);

	XmStringFree(label);
	XtAddCallback(button, XmNactivateCallback, broadcast_value, 0);
}
