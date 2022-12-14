/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * ttsample1 -- dynamic pattern, procedural notification
 */

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <string.h>
#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/MainW.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>
#include <Xm/Scale.h>
#include <Xm/MwmUtil.h>

#include <Tt/tt_c.h>

Widget toplevel, base_frame, controls, slider, gauge, button;
char *my_procid;

void	broadcast_value();
void	receive_tt_message();
void	create_ui_components();


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
	toplevel = XtVaAppInitialize(&app, "ttsample1", NULL, 0,
					&argc, argv, NULL, NULL);
	XtVaSetValues(toplevel, XmNtitle, "ToolTalk Sample 1", 0);
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
void
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
void
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
void
create_ui_components()
{
	int decor;
	Widget glabel, slabel;
	XmString label;

	base_frame = XtVaCreateManagedWidget("base_frame",
                xmMainWindowWidgetClass, toplevel,
                XmNwidth,                269,
                XmNheight,               104,
                0);
	XtVaGetValues(base_frame, XmNmwmDecorations, &decor, 0);
	decor &= ~MWM_DECOR_RESIZEH;
	XtVaSetValues(base_frame, XmNmwmDecorations, &decor, 0);

	controls = XtVaCreateManagedWidget("controls",
                xmFormWidgetClass, base_frame, NULL, 0, 0);

	slabel = XtVaCreateManagedWidget("Send:",
                xmLabelWidgetClass, controls,
                XmNx,               51,
                XmNy,               16,
                0);
        slider = XtVaCreateManagedWidget("slider",
                xmScaleWidgetClass, controls,
                XmNleftAttachment,  XmATTACH_WIDGET,
                XmNleftWidget,      slabel,
                XmNtopAttachment,   XmATTACH_OPPOSITE_WIDGET,
                XmNtopWidget,       slabel,
                XmNscaleWidth,      120,
                XmNscaleHeight,     18,
                XmNminimum,         0,
                XmNmaximum,         100,
                XmNorientation,     XmHORIZONTAL,
                0);

	glabel = XtVaCreateManagedWidget("Received:",
                xmLabelWidgetClass, controls,
                XmNx,               23,
                XmNy,               42,
                0);
	gauge = XtVaCreateManagedWidget("gauge",
                xmScaleWidgetClass, controls,
                XmNleftAttachment,  XmATTACH_WIDGET,
                XmNleftWidget,      glabel,
                XmNtopAttachment,   XmATTACH_OPPOSITE_WIDGET,
                XmNtopWidget,       glabel,
                XmNorientation,     XmHORIZONTAL,
                XmNscaleWidth,      125,
                XmNscaleHeight,     18,
                XmNminimum,         0,
                XmNmaximum,         100,
                XmNshowValue,       TRUE,
                0);

	label = XmStringCreateSimple("Broadcast");
	button = XtVaCreateManagedWidget("button",
                xmPushButtonWidgetClass, controls,
                XmNx,           96,
                XmNy,           78,
                XmNwidth,       76,
                XmNheight,      19,
                XmNlabelString, label,
                0);
	XmStringFree(label);
	XtAddCallback(button, XmNactivateCallback, broadcast_value, 0);
}
