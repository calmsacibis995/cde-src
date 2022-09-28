/* $XConsortium: ControlP.h /main/cde1_maint/1 1995/07/17 18:26:55 drk $ */
/**---------------------------------------------------------------------
***	
***	file:		ControlP.h
***
***	project:	MotifPlus Widgets
***
***	description:	Private include file for DtControl class.
***	
***	
***			(c) Copyright 1992 by Hewlett-Packard Company.
***
***
***-------------------------------------------------------------------*/


#ifndef _DtControlP_h
#define _DtControlP_h

#include <Xm/XmP.h>
#include <Xm/ColorObj.h>
#include <Dt/IconP.h>
#include <Dt/Control.h>

#ifdef SUN_CDE_PERF
#include <thread.h>
#endif

typedef struct _DtControlCacheObjClassPart
{
    XtPointer dummy;
} DtControlCacheObjClassPart;


typedef struct _DtControlCacheObjClassRec     /* label cache class record */
{
    ObjectClassPart                     object_class;
    XmExtClassPart                      ext_class;
    DtControlCacheObjClassPart           control_class_cache;
} DtControlCacheObjClassRec;

externalref DtControlCacheObjClassRec dtControlCacheObjClassRec;


/*  The Control Gadget Cache instance record  */

typedef struct _DtControlCacheObjPart
{
    XtPointer dummy;
} DtControlCacheObjPart;

typedef struct _DtControlCacheObjRec
{
    ObjectPart               object;
    XmExtPart                ext;
    DtControlCacheObjPart   control_cache;
} DtControlCacheObjRec;

/*-------------------------------------------------------------
**	Class Structure
*/

/*	Class Part
*/
typedef struct _DtControlClassPart
{
	XmCacheClassPartPtr	cache_part;
	caddr_t			extension;
} DtControlClassPart;

/*	Full Class Record
*/
typedef struct _DtControlClassRec
{
	RectObjClassPart	rect_class;
	XmGadgetClassPart	gadget_class;
	DtIconClassPart	icon_class;
	DtControlClassPart	control_class;
} DtControlClassRec;

/*	Actual Class
*/
extern DtControlClassRec dtControlClassRec;



/*-------------------------------------------------------------
**	Instance Structure
*/

/*	Instance Part
*/
typedef struct _DtControlPart
{
	XtPointer	push_function;
	XtPointer	push_argument;

	String		push_action;
	int		num_push_images;
	int		max_push_images;
	int		push_image_position;
	int		*push_delays;
	Pixmap		*push_pixmaps;
	Pixmap		*push_masks;

	String		drop_action;
	int		num_drop_images;
	int		max_drop_images;
	int		drop_image_position;
	int		*drop_delays;
	Pixmap		*drop_pixmaps;
	Pixmap		*drop_masks;

	Widget		subpanel;

	Pixmap		alt_pix;
	Pixmap		alt_mask;
	String		alt_image;

	_XmString	alt_string;
	Dimension	alt_string_width;
	Dimension	alt_string_height;

	String		format;
#ifdef AIX_ILS
        String          format_jp;
#endif /* AIX_ILS */
	String		file_name;
	off_t		file_size;
	XtIntervalId	monitor_timer;
	int		monitor_time;
	time_t		modified_time;
	Boolean		chime;
	Boolean		file_changed;

	Boolean		_do_update;
	Boolean		use_embossed_text;
	Boolean		use_label_adj;

	unsigned char	control_type;

	GC		top_shadow_gc;
	GC		bottom_shadow_gc;

	XtIntervalId	click_timer;
	int		click_time;

	short		busy;
	int		blink_elapsed;
	int		max_blink_time;
	int		blink_time;
	XtIntervalId	blink_timer;

	XtIntervalId	date_timer;
	XtIntervalId	push_animation_timer;
	XtIntervalId	drop_animation_timer;

#ifdef SUN_CDE_PERF
	int		pipe_fds[2];	/* communication chanel between thread and gadget */
	thread_t	tid;
	sema_t*		sema;
	XtInputId	iid;
#endif
} DtControlPart;


/*	Full Instance Record
*/
typedef struct _DtControlRec
{
	ObjectPart	object;
	RectObjPart	rectangle;
	XmGadgetPart	gadget;
	DtIconPart	icon;
	DtControlPart	control;
} DtControlRec;


/*-------------------------------------------------------------
**	Class and Instance Macros
*/

/*	DtControl Class Macros
*/	

/*	DtControl Instance Macros
*/
#define G_Format(g)		(g -> control.format)
#define G_TopShadowGC(g)	(g -> control.top_shadow_gc)
#define G_BottomShadowGC(g)	(g -> control.bottom_shadow_gc)
#define G_Busy(g)		(g -> control.busy)
#define G_PushImagePosition(g)	(g -> control.push_image_position)
#define G_NumPushImages(g)	(g -> control.num_push_images)
#define G_MaxPushImages(g)	(g -> control.max_push_images)
#define G_PushDelays(g)		(g -> control.push_delays)
#define G_PushPixmaps(g)	(g -> control.push_pixmaps)
#define G_PushMasks(g)		(g -> control.push_masks)
#define G_DropImagePosition(g)	(g -> control.drop_image_position)
#define G_NumDropImages(g)	(g -> control.num_drop_images)
#define G_MaxDropImages(g)	(g -> control.max_drop_images)
#define G_DropDelays(g)		(g -> control.drop_delays)
#define G_DropPixmaps(g)	(g -> control.drop_pixmaps)
#define G_DropMasks(g)		(g -> control.drop_masks)
#define G_DropAction(g)		(g -> control.drop_action)
#define G_PushAction(g)		(g -> control.push_action)
#define G__DoUpdate(g)		(g -> control._do_update)
#define G_PushFunction(g)	(g -> control.push_function)
#define G_PushArgument(g)	(g -> control.push_argument)
#define G_Subpanel(g)		(g -> control.subpanel)
#define G_AltMask(g)		(g -> control.alt_mask)
#define G_AltPix(g)		(g -> control.alt_pix)
#define G_AltString(g)		(g -> control.alt_string)
#define G_AltStringWidth(g)	(g -> control.alt_string_width)
#define G_AltStringHeight(g)	(g -> control.alt_string_height)
#define G_ControlType(g)	(g -> control.control_type)
#define G_AltImage(g)		(g -> control.alt_image)
#define G_FileChanged(g)	(g -> control.file_changed)
#define G_FileName(g)		(g -> control.file_name)
#define G_FileSize(g)		(g -> control.file_size)
#define G_Chime(g)		(g -> control.chime)
#define G_ModifiedTime(g)	(g -> control.modified_time)
#define G_MonitorTimer(g)	(g -> control.monitor_timer)
#define G_MonitorTime(g)	(g -> control.monitor_time)
#define G_ClickTime(g)		(g -> control.click_time)
#define G_ClickTimer(g)		(g -> control.click_timer)
#define G_DateTimer(g)		(g -> control.date_timer)
#define G_PushAnimationTimer(g)	(g -> control.push_animation_timer)
#define G_DropAnimationTimer(g)	(g -> control.drop_animation_timer)
#define G_BlinkCount(g)		(g -> control.blink_count)
#define G_BlinkElapsed(g)	(g -> control.blink_elapsed)
#define G_MaxBlinkTime(g)	(g -> control.max_blink_time)
#define G_BlinkTime(g)		(g -> control.blink_time)
#define G_BlinkTimer(g)		(g -> control.blink_timer)
#define G_BlinkTime(g)		(g -> control.blink_time)
#define G_UseEmbossedText(g)	(g -> control.use_embossed_text)
#define G_UseLabelAdjustment(g)	(g -> control.use_label_adj)

#ifdef SUN_CDE_PERF
#define G_MonitorThread(g)	(g -> control.tid)
#define G_Sema(g)		(g -> control.sema)
#define G_FilePipe(g)		(g -> control.pipe_fds)
#define G_FileRPipe(g)		(g -> control.pipe_fds[0])
#define G_FileWPipe(g)		(g -> control.pipe_fds[1])
#define G_ThreadInputId(g)	(g -> control.iid)
#endif

#endif /* _DtControlP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
