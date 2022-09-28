/* static char rcsid[] = 
	"$XConsortium: View.c /main/cde1_maint/1 1995/07/14 20:45:10 drk $";
*/
/**---------------------------------------------------------------------
***	
***	file:		View.c
***
***	project:	MotifPlus Widgets
***
***	description:	Source code for DtView class.
***	
***
***			(c) Copyright 1990 by Hewlett-Packard Company.
***
***
***-------------------------------------------------------------------*/


/*-------------------------------------------------------------
**	Include Files
*/

#include <stdio.h>
#include <Xm/ColorObj.h>
#include <Xm/DialogS.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/SeparatoG.h>
#include <Xm/TextF.h>
#include <Dt/Control.h>
#include <Dt/ViewP.h>


/*-------------------------------------------------------------
**	Public Interface
**-------------------------------------------------------------
*/

WidgetClass	dtViewWidgetClass;

Widget		_DtCreateView ();



/*-------------------------------------------------------------
**	Forward Declarations
*/

#define OFFSET		12
#define XmCR_SELECT	XmCR_SINGLE_SELECT

/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern Widget _DtCreateView() ;

#else

extern Widget _DtCreateView( 
                        Widget parent,
                        String name,
                        ArgList arglist,
                        Cardinal argcount) ;

#endif /* _NO_PROTO */
/********    End Public Function Declarations    ********/

/********    Static Function Declarations    ********/
#ifdef _NO_PROTO


#else


#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/


/*-------------------------------------------------------------
**	Translations and Actions
*/



/*-------------------------------------------------------------
**	Resource List
*/

/*	Define offset macros.
*/
#define R_Offset(field) \
	XtOffset (DtViewWidget, view.field)

static XtResource resources[] = 
{
	{
		XmNleftInset,
		XmCSpacing, XmRDimension, sizeof (Dimension),
		R_Offset (left_inset), XmRImmediate, (caddr_t) 3
	},
	{
		XmNrightInset,
		XmCSpacing, XmRDimension, sizeof (Dimension),
		R_Offset (right_inset), XmRImmediate, (caddr_t) 3
	},
	{
		XmNtopInset,
		XmCSpacing, XmRDimension, sizeof (Dimension),
		R_Offset (top_inset), XmRImmediate, (caddr_t) 3
	},
	{
		XmNbottomInset,
		XmCSpacing, XmRDimension, sizeof (Dimension),
		R_Offset (bottom_inset), XmRImmediate, (caddr_t) 3
	},
	{
		XmNwidthIncrement,
		XmCSpacing, XmRDimension, sizeof (Dimension),
		R_Offset (width_increment), XmRImmediate, (caddr_t) 2
	},
	{
		XmNheightIncrement,
		XmCSpacing, XmRDimension, sizeof (Dimension),
		R_Offset (height_increment), XmRImmediate, (caddr_t) 2
	},
	{
		XmNboxType,
		XmCBoxType, XmRBoxType, sizeof (unsigned char),
		R_Offset (box_type),
		XmRImmediate, (XtPointer) XmBOX_NONE
	},
	{
		XmNsubpanelUnpostOnSelect,
		XmCSubpanelUnpostOnSelect, XmRBoolean, sizeof (Boolean),
		R_Offset (subpanel_unpost_on_select),
		XmRImmediate, (XtPointer) TRUE
	},
	{
		XmNsubpanelTorn,
		XmCSubpanelTorn, XmRBoolean, sizeof (Boolean),
		R_Offset (subpanel_torn), XmRImmediate, (XtPointer) FALSE
	}
};



/*-------------------------------------------------------------
**	Class Record
*/
DtViewClassRec dtViewClassRec =
{
/*	Core Part
*/
	{	
		(WidgetClass) &xmFormClassRec,	/* superclass		*/
		"Box",				/* class_name		*/
		sizeof (DtViewRec),		/* widget_size		*/
		NULL,				/* class_initialize	*/
		NULL,				/* class_part_initialize*/
		False,				/* class_inited		*/
		(XtInitProc) NULL,		/* initialize		*/
		NULL,				/* initialize_hook	*/
		XtInheritRealize,		/* realize		*/
		NULL,				/* actions		*/
		0,				/* num_actions		*/
		resources,			/* resources		*/
		XtNumber (resources),		/* num_resources	*/
		NULLQUARK,			/* xrm_class		*/
		True,				/* compress_motion	*/
		XtExposeCompressMaximal,	/* compress_exposure	*/
		True,				/* compress_enterleave	*/
		False,				/* visible_interest	*/	
		NULL,			 	/* destroy		*/	
		XtInheritResize,		/* resize		*/
		XtInheritExpose,		/* expose		*/	
		(XtSetValuesFunc) NULL,		/* set_values		*/	
		NULL,				/* set_values_hook	*/
		XtInheritSetValuesAlmost,	/* set_values_almost	*/
		NULL,				/* get_values_hook	*/
		NULL,				/* accept_focus		*/	
		XtVersion,			/* version		*/
		NULL,				/* callback private	*/
		XtInheritTranslations,		/* tm_table		*/
		XtInheritQueryGeometry,		/* query_geometry	*/
		NULL,				/* display_accelerator	*/
		NULL,				/* extension		*/
	},

/*	Composite Part
*/
	{
		XtInheritGeometryManager,	/* geometry_manager	*/
		XtInheritChangeManaged,		/* change_managed	*/
		XtInheritInsertChild,		/* insert_child		*/
		XtInheritDeleteChild,		/* delete_child		*/
		NULL,				/* extension		*/
	},

/*	Constraint Part
*/
	{
		NULL,				/* constraint_resources	*/
		0,				/* num_constraint_resources */
		sizeof (DtViewConstraintRec),/* constraint_record	*/
		NULL,				/* constraint_initialize */
		NULL,				/* constraint_destroy	*/
		NULL,				/* constraint_set_values */
		NULL,				/* extension		*/
	},

/*	XmManager Part
*/
	{
		XtInheritTranslations,		/* default_translations	*/
		NULL,				/* syn_resources	*/
		0,				/* num_syn_resources	*/
		NULL,				/* syn_cont_resources	*/
		0,				/* num_syn_cont_resources */
		XmInheritParentProcess,		/* parent_process	*/
		NULL,				/* extension		*/
	},

/*	XmBulletinBoard Part
*/
	{
		False,				/* always_install_accelerators*/
		NULL,				/* geo_matrix_create	*/
		XmInheritFocusMovedProc,	/* focus_moved_proc	*/
		NULL,				/* extension		*/
	},

/*	XmForm Part
*/
	{
		NULL,				/* extension		*/
	},

/*	DtView Part
*/
	{
		NULL,				/* extension		*/
	}

};

WidgetClass dtViewWidgetClass = (WidgetClass) &dtViewClassRec;



/*-------------------------------------------------------------
**	Private Procs
**-------------------------------------------------------------
*/

/*-------------------------------------------------------------
**	Action Procs
**-------------------------------------------------------------
*/



/*-------------------------------------------------------------
**	Core Procs
**-------------------------------------------------------------
*/

/*-------------------------------------------------------------
**	Initialize
**		Initialize a new widget instance.
*/
#if 0
/* ARGSUSED */
static void 
#ifdef _NO_PROTO
Initialize( request_w, new_w )
        Widget request_w ;
        Widget new_w ;
#else
Initialize(
        Widget request_w,
        Widget new_w )
#endif /* _NO_PROTO */
{
DtViewWidget	 request = (DtViewWidget) request_w,
		 new = (DtViewWidget) new_w;
Pixmap		 pix;

/*	Validate box type.
 */
if (M_BoxType (new) != XmBOX_NONE &&
    M_BoxType (new) != XmBOX_PRIMARY &&
    M_BoxType (new) != XmBOX_SECONDARY &&
    M_BoxType (new) != XmBOX_SUBPANEL)
    {
    M_BoxType (new) == XmBOX_NONE;
    }
}
#endif /* 0 */


/*-------------------------------------------------------------
**	SetValues
**		Handle changes in resource data.
*/
#if 0
static Boolean 
#ifdef _NO_PROTO
SetValues( current_w, request_w, new_w )
        Widget current_w ;
        Widget request_w ;
        Widget new_w ;
#else
SetValues(
        Widget current_w,
        Widget request_w,
        Widget new_w )
#endif /* _NO_PROTO */
{
DtViewWidget	current = (DtViewWidget) current_w,
		request = (DtViewWidget) request_w,
		new = (DtViewWidget) new_w;
Boolean		redraw_flag = False;

/*	Can't change box type.
*/
if (M_BoxType (new) != M_BoxType (current))
    {
    M_BoxType (new) = M_BoxType (current);
    }

return (redraw_flag);
}
#endif /* 0 */


/*-------------------------------------------------------------
**	Composite Procs
**-------------------------------------------------------------
*/


/*-------------------------------------------------------------
**	Manager Procs
**-------------------------------------------------------------
*/

/*	All inherited from superclass.
 */


/*-------------------------------------------------------------
**	View Procs
**-------------------------------------------------------------
*/




/*-------------------------------------------------------------
**	Public Entry Points
**-------------------------------------------------------------
*/

/*-------------------------------------------------------------
**      _DtCreateView
**		Create a new DtView instance.
**-------------------------------------------------------------
*/
Widget 
#ifdef _NO_PROTO
_DtCreateView( parent, name, arglist, argcount )
        Widget parent ;
        String name ;
        ArgList arglist ;
        Cardinal argcount ;
#else
_DtCreateView(
        Widget parent,
        String name,
        ArgList arglist,
        Cardinal argcount )
#endif /* _NO_PROTO */
{
	return (XtCreateWidget (name, dtViewWidgetClass, 
			parent, arglist, argcount));
}
	
