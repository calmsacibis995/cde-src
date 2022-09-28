
/*
 *      $XConsortium: pal_termp.c /main/cde1_maint/2 1995/10/16 13:56:09 rcs $
 *
 * @(#)pal_termp.c	1.14 15 Feb 1994      cde_app_builder/src/ab
 *
 *      RESTRICTED CONFIDENTIAL INFORMATION:
 *
 *      The information in this document is subject to special
 *      restrictions in a confidential disclosure agreement between
 *      HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *      document outside HP, IBM, Sun, USL, SCO, or Univel without
 *      Sun's specific written approval.  This document and all copies
 *      and derivative works thereof must be returned or destroyed at
 *      Sun's request.
 *
 *      Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 */


/*
 * pal_termp.c - Implements Palette TermPane object functionality
 */
#include <stdio.h>
#include <Xm/Xm.h>
#include <Dt/TermPrim.h>
#include <Dt/Term.h>
#include <ab_private/util.h>
#include <ab_private/ab.h>
#include <ab_private/pal.h>
#include <ab_private/prop.h>
#include <ab_private/abobj.h>
#include <ab_private/abobj_set.h>
#include <ab_private/objxm.h>
#include <ab_private/ui_util.h>
#include "termp_ui.h"

const    int    termp_init_height = 80;
const    int    termp_init_width  = 80;

typedef struct  PROP_TERMP_SETTINGS
{
    Widget                      prop_sheet;
    PropFieldSettingRec         name;
    PropRadioSettingRec         scrolling;
    PropGeometrySettingRec      pos;
    PropGeometrySettingRec      size;
    PropOptionsSettingRec       size_metric;
    PropFieldSettingRec         pstring;
    PropCheckboxSettingRec      init_state;
    PropOptionsSettingRec       frame;
    PropColorSettingRec         bg_color;
    PropColorSettingRec         fg_color;
    PropMenunameSettingRec      menuname;
    PropFieldSettingRec         menu_title;
    ABObj                       current_obj;
} PropTermpSettingsRec, *PropTermpSettings;

/*************************************************************************
**                                                                      **
**       Private Function Declarations                                  **
**                                                                      **
**************************************************************************/
/*
 * Methods
 */
static int	termp_initialize(
                    ABObj   obj
                );
static Widget   termp_prop_init(
                    Widget  parent,
                    AB_PROP_TYPE type
                );
static int	termp_prop_activate(
		    AB_PROP_TYPE type,
		    BOOL 	 active
		);
static int	termp_prop_clear(
                    AB_PROP_TYPE type
                );
static int      termp_prop_load(
                    ABObj   	 obj,
                    AB_PROP_TYPE type,
		    unsigned long loadkey
                );
static int	termp_prop_apply(
		    AB_PROP_TYPE type
		);
static BOOL	termp_prop_pending(
		    AB_PROP_TYPE type
		);

static BOOL	verify_props(
		    AB_PROP_TYPE type
		);
static void	turnoff_changebars(
		    AB_PROP_TYPE type
		);


/*************************************************************************
**                                                                      **
**       Data 		                                                **
**                                                                      **
**************************************************************************/
PalItemInfo termp_palitem_rec = {

    /* type             */  AB_TYPE_TERM_PANE,
    /* name             */  "Term Pane",
    /* animation pixmaps*/  NULL,
    /* number of pixmaps*/  0,
    /* rev_prop_frame   */  NULL,
    /* fix_prop_dialog  */  NULL,
    /* initialize       */  termp_initialize,
    /* is_a_test        */  obj_is_term_pane,
    /* prop_initialize  */  termp_prop_init,
    /* prop_activate	*/  termp_prop_activate,
    /* prop_clear	*/  termp_prop_clear,
    /* prop_load        */  termp_prop_load,
    /* prop_apply	*/  termp_prop_apply,
    /* prop_pending	*/  termp_prop_pending

};

PalItemInfo *ab_termp_palitem = &termp_palitem_rec;
PropTermpSettingsRec prop_termp_settings_rec[AB_PROP_TYPE_NUM_VALUES];


/*************************************************************************
**                                                                      **
**       Function Definitions                                           **
**                                                                      **
**************************************************************************/

static int
termp_initialize(
    ABObj     obj
)
{
    ABObj	parent = obj_get_root(obj_get_parent(obj));

    obj_set_unique_name(obj, "termpane");

    if (obj_is_layers(parent)) /* Size already defined in pixels */
    {
	obj_set_num_rows(obj, -1);
	obj_set_num_columns(obj, -1);
    }
    else /* Default Size should be in chars */
    {
    	obj_set_num_rows(obj, 6);
    	obj_set_num_columns(obj, 12);
    }
    obj_set_process_string(obj, "/bin/csh");

    abobj_init_pane_position(obj);

    return OK;
}

static Widget
termp_prop_init(
    Widget parent,
    AB_PROP_TYPE type
)
{
    DtbTermpPropDialogInfoRec	rev_termp_prop_dialog; /* Revolving Props */
    DtbTermpPropDialogInfo	cgen = &dtb_termp_prop_dialog; /* Codegen structure */
    DtbRevolvPropDialogInfo	rpd = &(dtb_revolv_prop_dialog);
    PropTermpSettingsRec    	*pts = &(prop_termp_settings_rec[type]);
    Widget			item[10];
    int				item_val[10];
    int				n;

    if (type == AB_PROP_REVOLVING)
    {
	/* Cloning Trick:
	 * Only the Attributes ControlPanel needs to be created within
	 * the existing Revolving Prop dialog, so fill out all other
	 * fields with the Revolving Prop dialog equivelents, so the
	 * dtb initialize proc will skip those non-NULL fields...
	 */
        dtbTermpPropDialogInfo_clear(&rev_termp_prop_dialog);

	cgen = &(rev_termp_prop_dialog);
	cgen->prop_dialog = rpd->prop_dialog;
        cgen->prop_dialog_shellform = rpd->prop_dialog_shellform; 
	cgen->prop_dialog_panedwin = rpd->prop_dialog_panedwin;
	cgen->prop_dialog_form = rpd->prop_dialog_form;
	cgen->objlist_panel = rpd->objlist_panel;
	cgen->objlist_label = rpd->objlist_label2;
	cgen->objlist_scrolledwin = rpd->objlist_scrolledwin;
	cgen->objlist = rpd->objlist;
	cgen->attrs_ctrlpanel_frame = rpd->attrs_ctrlpanel_frame;
	cgen->activate_panel = rpd->activate_panel;
	cgen->apply_button = rpd->apply_button; 
        cgen->ok_button = rpd->ok_button;
        cgen->cancel_button = rpd->cancel_button;
	cgen->reset_button = rpd->reset_button; 
	cgen->help_button = rpd->help_button; 

    }
    else /* AB_PROP_FIXED */
    	cgen = &dtb_termp_prop_dialog; 

    if (dtb_termp_prop_dialog_initialize(cgen, parent) == 0)
    {
        pts->prop_sheet = cgen->attrs_ctrlpanel;
        pts->current_obj = NULL;

	if (type == AB_PROP_REVOLVING)
        	XtVaSetValues(parent,
                        XmNuserData, pts->current_obj,
                        NULL);

        /* Dialog/Object List */
	if (type == AB_PROP_FIXED)
	{
            prop_fixed_dialog_init(ab_termp_palitem,
			cgen->prop_dialog_shellform, cgen->objlist);
            prop_activate_panel_init(type, ab_termp_palitem, 
                        cgen->ok_button, cgen->apply_button, 
                        cgen->reset_button, cgen->cancel_button,
                        cgen->help_button);
	}

	/* Alternate Editor Buttons */
	prop_editors_panel_init(type, ab_termp_palitem,
		    cgen->attach_button, cgen->conn_button, cgen->helptxt_button);

	/*
	  * Prop Sheet Settings....
	 */

	/* Name */
	prop_field_init(&(pts->name), cgen->name_field_label,
			    cgen->name_field, cgen->name_cb);

	/* Scrolling */
        n = 0;
        item[n] = cgen->scroll_rbox_items.Never_item;
        item_val[n] = AB_SCROLLBAR_NEVER; n++;
        item[n] = cgen->scroll_rbox_items.Always_item;
        item_val[n] = AB_SCROLLBAR_ALWAYS; n++;
        prop_radiobox_init(&(pts->scrolling), cgen->scroll_rbox_label,
                cgen->scroll_rbox, n, item, item_val,
                cgen->scroll_cb);

        /* Border Frame */
        n = 0;
        item[n] = cgen->bframe_opmenu_items.None_item;
        item_val[n] = AB_LINE_NONE; n++;
        item[n] = cgen->bframe_opmenu_items.Shadow_Out_item;
        item_val[n] = AB_LINE_SHADOW_OUT; n++;
        item[n] = cgen->bframe_opmenu_items.Shadow_In_item;
        item_val[n] = AB_LINE_SHADOW_IN; n++;
        item[n] = cgen->bframe_opmenu_items.Etched_Out_item;
        item_val[n] = AB_LINE_ETCHED_OUT; n++;
        item[n] = cgen->bframe_opmenu_items.Etched_In_item;
        item_val[n] = AB_LINE_ETCHED_IN; n++;
        prop_options_init(&(pts->frame), cgen->bframe_opmenu_label,
                            cgen->bframe_opmenu, cgen->bframe_opmenu_menu,
                            n, item, item_val,
                            cgen->bframe_cb);
	/* Menu Title */
        prop_field_init(&(pts->menu_title), cgen->menutitle_field_label,
                            cgen->menutitle_field, cgen->menutitle_cb);

        /* Menu Name */
        prop_menuname_init(&(pts->menuname), type, cgen->menu_label,
			cgen->menu_mbutton,
			cgen->menu_field,
                        cgen->name_field, cgen->menu_cb,
			&(pts->menu_title),
                        &(pts->current_obj), True);

        /* Process String */
        prop_field_init(&(pts->pstring), cgen->pstring_field_label,
                            cgen->pstring_field, cgen->pstring_cb);

	/* Position */
	prop_geomfield_init(&(pts->pos), cgen->pos_label,
			    cgen->x_field_label, cgen->x_field,
			    cgen->y_field_label, cgen->y_field,
			    NULL, NULL, NULL, NULL,
			    cgen->pos_cb);

	/* Size */
        prop_geomfield_init(&(pts->size), cgen->size_opmenu_label,
			    NULL, NULL, NULL, NULL,
                            cgen->width_field_label, cgen->width_field,
                            cgen->height_field_label, cgen->height_field,
                            cgen->size_cb);

	n = 0;
	item[n] = cgen->size_opmenu_items.Characters_item;
	item_val[n] = SIZE_IN_CHARS_KEY; n++;
	item[n] = cgen->size_opmenu_items.Pixels_item;
	item_val[n] = SIZE_IN_PIXELS_KEY; n++;
	prop_options_init(&(pts->size_metric), cgen->size_opmenu_label,
			   cgen->size_opmenu, 
			   cgen->size_opmenu_menu, 
			   n, item, item_val,	
			   cgen->size_cb);

	/* Initial State */
	n = 0;
	item[n] = cgen->istate_ckbox_items.Visible_item;
	item_val[n] = AB_STATE_VISIBLE; n++;
	item[n] = cgen->istate_ckbox_items.Active_item;
	item_val[n] = AB_STATE_ACTIVE; n++;
	prop_checkbox_init(&(pts->init_state), cgen->istate_ckbox_label,
			    cgen->istate_ckbox, n, item, item_val,
			    cgen->istate_cb);

	/* Color */
	prop_colorfield_init(&(pts->bg_color), cgen->bg_mbutton,
                cgen->bg_mbutton_bg_mbutton_menu_items.None_item,
                cgen->bg_mbutton_bg_mbutton_menu_items.Color_Chooser_item,
		cgen->bg_swatch, cgen->bg_field, cgen->bg_cb);

	prop_colorfield_init(&(pts->fg_color), cgen->fg_mbutton, 
                cgen->fg_mbutton_fg_mbutton_menu_items.None_item, 
                cgen->fg_mbutton_fg_mbutton_menu_items.Color_Chooser_item, 
                cgen->fg_swatch, cgen->fg_field, cgen->fg_cb); 

	prop_changebars_cleared(pts->prop_sheet);

	return (cgen->prop_dialog_shellform);
    }
    else
	return NULL;
}

static int
termp_prop_activate(
    AB_PROP_TYPE type,
    BOOL	 active
)
{
    ui_set_active(prop_termp_settings_rec[type].prop_sheet, active);

    return OK;
}

static int
termp_prop_clear(
    AB_PROP_TYPE type
)
{
    PropTermpSettingsRec	*pts = &(prop_termp_settings_rec[type]);

    if (pts->current_obj == NULL)
	return OK;

    /* Clear Name */
    prop_field_set_value(&(pts->name), "", False);

    /* Clear Scrolling */
    prop_radiobox_set_value(&(pts->scrolling), AB_SCROLLBAR_ALWAYS, False); 

    /* Clear Border Frame */
    prop_options_set_value(&(pts->frame), AB_LINE_NONE, False);

    /* Clear Menu Name/Title */
    prop_menuname_set_value(&(pts->menuname), "", False);
    prop_field_set_value(&(pts->menu_title), "", False);

    /* Clear Process String */
    prop_field_set_value(&(pts->pstring), "", False);

    /* Clear Position */
    prop_geomfield_clear(&(pts->pos), GEOM_X);
    prop_geomfield_clear(&(pts->pos), GEOM_Y);

    /* Clear Size */
    prop_geomfield_clear(&(pts->size), GEOM_WIDTH);
    prop_geomfield_clear(&(pts->size), GEOM_HEIGHT);

    /* Clear Initial State */
    prop_checkbox_set_value(&(pts->init_state), AB_STATE_VISIBLE, True, False);
    prop_checkbox_set_value(&(pts->init_state), AB_STATE_ACTIVE, True, False);

    /* Clear Color */
    prop_colorfield_set_value(&(pts->bg_color), "", False);
    prop_colorfield_set_value(&(pts->fg_color), "", False);

    pts->current_obj = NULL;

    turnoff_changebars(type);
 
    return OK;

}


static int
termp_prop_load(
    ABObj    	 obj,
    AB_PROP_TYPE type,
    unsigned long loadkey
)
{
    PropTermpSettingsRec 	*pts = &(prop_termp_settings_rec[type]);
    int				width, height;
    BOOL			load_all = (loadkey & LoadAll);
 
    if (obj == NULL)
    {
        if (pts->current_obj != NULL)
            obj = pts->current_obj;
        else
            return ERROR;
    }
    else if (!obj_is_term_pane(obj))
        return ERROR;
    else
        pts->current_obj = obj;

    /* Load Name */
    if (load_all || loadkey & LoadName)
    	prop_field_set_value(&(pts->name), obj_get_name(obj), False);

    if (load_all)
    {
	/* Load Scrolling */
	prop_radiobox_set_value(&(pts->scrolling), 
		obj_get_vscrollbar_policy(obj), False);

	/* Load Border Frame */
	prop_options_set_value(&(pts->frame), obj_get_border_frame(obj), False);

	/* Load Menu Name/Title */
	prop_menuname_set_value(&(pts->menuname), obj_get_menu_name(obj), False);
	prop_field_set_value(&(pts->menu_title), obj_get_menu_title(obj), False);

	/* Load Process String */
	prop_field_set_value(&(pts->pstring), obj_get_process_string(obj), False);

	/* Load Initial State */
	prop_checkbox_set_value(&(pts->init_state), AB_STATE_VISIBLE,
		obj_is_initially_visible(obj), False);
    	prop_checkbox_set_value(&(pts->init_state), AB_STATE_ACTIVE,
		obj_is_initially_active(obj), False);

    	/* Load Color */
    	prop_colorfield_set_value(&(pts->bg_color), obj_get_bg_color(obj), False); 
    	prop_colorfield_set_value(&(pts->fg_color), obj_get_fg_color(obj), False);

    	turnoff_changebars(type);
    }

    /* Load Position */
    if (load_all || loadkey & LoadPosition)
        prop_load_obj_position(obj, &(pts->pos)); 

    /* Load Size */
    if (load_all || loadkey & LoadSize)
    {
        if (obj_get_num_columns(obj) == -1) /* Size in Pixels */
        {
            prop_options_set_value(&(pts->size_metric), SIZE_IN_PIXELS_KEY, False);
            width = abobj_get_comp_width(obj);
            height = abobj_get_comp_height(obj);
        }
        else /* Size in Chars */
        {
            prop_options_set_value(&(pts->size_metric), SIZE_IN_CHARS_KEY, False);
            width = obj_get_num_columns(obj);
            height = obj_get_num_rows(obj);
        }
        prop_geomfield_set_value(&(pts->size), GEOM_WIDTH, width, False);
        prop_geomfield_set_value(&(pts->size), GEOM_HEIGHT, height, False);
    }

    return OK;
}

int
termp_prop_apply(
    AB_PROP_TYPE   type
)
{
    PropTermpSettingsRec 	*pts = &(prop_termp_settings_rec[type]);
    ABObj			chg_root = pts->current_obj;
    ABObj			pobj;
    STRING			value;
    int				border, metric, new_w, new_h;
    BOOL			size_chg = False;
    BOOL			reset_bg = False;
    BOOL			reset_fg = False;

    if (!verify_props(type))
        return ERROR;

    if (prop_changed(pts->name.changebar))
    {
        value = prop_field_get_value(&(pts->name));
        abobj_set_name(pts->current_obj, value);
        util_free(value);
    }
    if (prop_changed(pts->scrolling.changebar))
    {
        abobj_set_scrollbar_state(pts->current_obj,
            (AB_SCROLLBAR_POLICY)prop_radiobox_get_value(&(pts->scrolling)));

        size_chg = True;
    }

    pobj = obj_get_root(obj_get_parent(pts->current_obj));

    if (prop_changed(pts->pos.changebar))
    {
	if (obj_is_layers(pobj))
	    chg_root = pobj;

	if (abobj_is_movable(pts->current_obj))
	    abobj_set_xy(chg_root,
	     	prop_geomfield_get_value(&(pts->pos), GEOM_X),
	     	prop_geomfield_get_value(&(pts->pos), GEOM_Y));
    }
    if (prop_changed(pts->size.changebar))
    {
	metric = prop_options_get_value(&(pts->size_metric));
	new_w = prop_geomfield_get_value(&(pts->size), GEOM_WIDTH);
	new_h = prop_geomfield_get_value(&(pts->size), GEOM_HEIGHT);

        border = 
	    ((prop_options_get_value(&(pts->frame)) != AB_LINE_NONE)? 4 : 0);

        if (obj_is_layers(pobj))
        {
            if (metric == SIZE_IN_PIXELS_KEY) 
                abobj_layer_set_size(pobj, new_w, new_h);
            chg_root = pobj;
        }
        else 
        {
            abobj_set_num_rows(pts->current_obj,
                metric == SIZE_IN_CHARS_KEY? new_h : -1);
            abobj_set_num_columns(pts->current_obj,
                metric == SIZE_IN_CHARS_KEY? new_w : -1);
 
            abobj_set_pixel_width(pts->current_obj,
                metric == SIZE_IN_PIXELS_KEY? new_w : -1, border);
            abobj_set_pixel_height(pts->current_obj,
                metric == SIZE_IN_PIXELS_KEY? new_h : -1, border);
        }
        size_chg = True;
    }
    if (prop_changed(pts->pstring.changebar))
    {
        ABObj o_obj = objxm_comp_get_subobj(pts->current_obj, AB_CFG_OBJECT_OBJ);
 
        value = prop_field_get_value(&(pts->pstring)); 
        obj_set_process_string(pts->current_obj, value);
        objxm_obj_set_ui_arg(o_obj, AB_ARG_STRING, DtNsubprocessCmd, value);
        obj_set_flag(o_obj, AttrChangedFlag);
        obj_set_flag(obj_get_module(pts->current_obj), SaveNeededFlag);
        util_free(value);
    }
    if (prop_changed(pts->init_state.changebar))
    {
        abobj_set_visible(pts->current_obj, 
		prop_checkbox_get_value(&(pts->init_state), AB_STATE_VISIBLE));
        abobj_set_active(pts->current_obj,
		prop_checkbox_get_value(&(pts->init_state), AB_STATE_ACTIVE));
    }
    if (prop_changed(pts->frame.changebar))
    {
        abobj_set_border_frame(pts->current_obj,
                (AB_LINE_TYPE)prop_options_get_value(&(pts->frame)));
    }
    if (prop_changed(pts->fg_color.changebar))
    {
        value = prop_colorfield_get_value(&(pts->fg_color)); 
        abobj_set_foreground_color(pts->current_obj, value);
	if (util_strempty(value))
	    reset_fg = True;
        util_free(value);
        prop_colorfield_set_value(&(pts->fg_color), obj_get_fg_color(pts->current_obj), False);
    }
    if (prop_changed(pts->bg_color.changebar))
    {
        value = prop_colorfield_get_value(&(pts->bg_color)); 
        abobj_set_background_color(pts->current_obj, value);
	if (util_strempty(value))
	    reset_bg = True;
        util_free(value);
        prop_colorfield_set_value(&(pts->bg_color), obj_get_bg_color(pts->current_obj), False);
    }
    if (prop_changed(pts->menuname.changebar))
    {
        value = prop_menuname_get_value(&(pts->menuname)); 
        abobj_set_menu_name(pts->current_obj, value);
        util_free(value);
        prop_menuname_set_value(&(pts->menuname), obj_get_menu_name(pts->current_obj), False);
    }   
    if (prop_changed(pts->menu_title.changebar))
    {
        value = prop_field_get_value(&(pts->menu_title));
        abobj_set_menu_title(pts->current_obj, value);
        util_free(value);
    }
    abobj_instantiate_changes(pts->current_obj);
    if (chg_root != pts->current_obj) 
        abobj_tree_instantiate_changes(chg_root);

    if (reset_bg || reset_fg) /* Set back to No Color */ 
        abobj_reset_colors(pts->current_obj, reset_bg, reset_fg); 

    turnoff_changebars(type);

    return OK;
}


static BOOL
termp_prop_pending(
    AB_PROP_TYPE type
)
{
    return(prop_changebars_pending(prop_termp_settings_rec[type].prop_sheet));
}

static BOOL
verify_props(
    AB_PROP_TYPE type
)
{
    PropTermpSettingsRec *pts = &(prop_termp_settings_rec[type]);

    if (prop_changed(pts->name.changebar) && 
	!prop_name_ok(pts->current_obj, pts->name.field))
        return False;

    if (prop_changed(pts->pos.changebar) &&
        (!prop_number_ok(pts->pos.x_field, (STRING)XFieldStr, -SHRT_MAX, SHRT_MAX) ||
         !prop_number_ok(pts->pos.y_field, (STRING)YFieldStr, -SHRT_MAX, SHRT_MAX)))
        return False;

    if (prop_changed(pts->size.changebar) &&
        (!prop_number_ok(pts->size.w_field, (STRING)WFieldStr, 1, SHRT_MAX) ||
         !prop_number_ok(pts->size.h_field, (STRING)HFieldStr, 1, SHRT_MAX)))
        return False;
 
    if (prop_changed(pts->fg_color.changebar) && !prop_color_ok(pts->fg_color.field))
        return False;
 
    if (prop_changed(pts->bg_color.changebar) && !prop_color_ok(pts->bg_color.field))
        return False;

    if (prop_changed(pts->menuname.changebar) && !prop_obj_name_ok(pts->menuname.field,
			obj_get_module(pts->current_obj), AB_TYPE_MENU, "Menu"))
	return False;

    return True;
}


static void
turnoff_changebars(
    AB_PROP_TYPE type
)
{
    PropTermpSettingsRec *pts = &(prop_termp_settings_rec[type]);

    prop_set_changebar(pts->name.changebar,    	PROP_CB_OFF);
    prop_set_changebar(pts->scrolling.changebar,PROP_CB_OFF);
    prop_set_changebar(pts->pos.changebar,    	PROP_CB_OFF);
    prop_set_changebar(pts->size.changebar,    	PROP_CB_OFF);
    prop_set_changebar(pts->pstring.changebar, 	PROP_CB_OFF);
    prop_set_changebar(pts->init_state.changebar,PROP_CB_OFF);
    prop_set_changebar(pts->frame.changebar,	PROP_CB_OFF);
    prop_set_changebar(pts->bg_color.changebar, PROP_CB_OFF);
    prop_set_changebar(pts->fg_color.changebar, PROP_CB_OFF);
    prop_set_changebar(pts->menuname.changebar, PROP_CB_OFF);
    prop_set_changebar(pts->menu_title.changebar, PROP_CB_OFF);

    prop_changebars_cleared(pts->prop_sheet);

}
