/*****************************************************************************
 *****************************************************************************
 **
 **   File:         DropZone.c
 **
 **   Project:      CDE
 **
 **   Description:  This file contains the functions for creating and managing
 **		    drop sites in the Front Panel.
 **
 **
 ****************************************************************************
 *****************************************************************************/

#include <Dt/DtP.h>
#include <Dt/ControlP.h>
#include <Dt/FileDrop.h>
#include "DataBaseLoad.h"

#ifdef _NO_PROTO
extern void DropCB ();
extern void CustomizeDropCB ();
#else
extern void CustomizeDropCB (Widget, XtPointer, XtPointer);
extern void DropCB (Widget, XtPointer, XtPointer);
#endif /* _NO_PROTO */


/************************************************************************
 *
 *  RegisterCustomizeDropZone
 *	Add the callback needed to catch drops on the control install
 *	within a subpanel.
 *	
 *	Inputs
 *	   control_data:  The main panel control from which the subpanel
 *                        is attached.
 *
 ************************************************************************/

#ifdef _NO_PROTO
void
RegisterCustomizeDropZone (control_data)
ControlData * control_data;

#else
void
RegisterCustomizeDropZone (ControlData * control_data)
#endif /* _NO_PROTO */

{
   SubpanelData * subpanel_data = control_data->subpanel_data;
   Widget         dropzone = (Widget) subpanel_data->dropzone;

   Arg    args[1];


   XtSetArg (args[0], XmNdropSiteOperations,
             XmDROP_COPY | XmDROP_MOVE | XmDROP_LINK);

   XtSetValues (dropzone, args, 1);

   XtAddCallback (dropzone, XmNdropCallback, 
                  CustomizeDropCB, (XtPointer) control_data);
}




void
#ifdef _NO_PROTO
RegisterControlDropZone(control_data)
ControlData * control_data;
#else
RegisterControlDropZone(
	ControlData * control_data)
#endif /* _NO_PROTO */
{
   Widget w = (Widget) control_data->control_icon;
   Arg              args[1];
   PanelActionData * drop_action;

   drop_action = (PanelActionData *)
	       (control_data->element_values[CONTROL_DROP_ACTION].parsed_value);

   if (drop_action != NULL)
   {

      XtSetArg (args[0], XmNdropSiteOperations,
                XmDROP_COPY | XmDROP_MOVE | XmDROP_LINK);
      XtSetValues(w, args, 1);

      XtAddCallback (w, XmNdropCallback, DropCB, (XtPointer) drop_action);
   }
}

void
#ifdef _NO_PROTO
RegisterDropZones()
#else
RegisterDropZones(void)
#endif /* _NO_PROTO */
{
   int i, j, k;
   BoxData * box_data;
   ControlData * control_data;
   SwitchData * switch_data;
   SubpanelData * subpanel_data;

   for (i = 0; i < panel.box_data_count; i++)
   {
      box_data = &panel.box_data[i];
  
      for (j = 0; j < box_data->control_data_count; j++)
      {
         control_data = &box_data->control_data[j];

         RegisterControlDropZone(control_data);

         if (control_data->subpanel_data != NULL)
         {
            subpanel_data = control_data->subpanel_data;

            for (k = 0; k < subpanel_data->control_data_count; k++)
            {
               RegisterControlDropZone(&subpanel_data->control_data[k]);
            }

            if (subpanel_data->dropzone)
               RegisterCustomizeDropZone(control_data);
         }
      }

      if (box_data->switch_data != NULL)
      {
         switch_data = box_data->switch_data;

         for (j = 0; j < switch_data->control_data_count; j++)
         {
            control_data = &switch_data->control_data[j];

            RegisterControlDropZone(control_data);

            if (control_data->subpanel_data != NULL)
            {
               subpanel_data = control_data->subpanel_data;

               for (k = 0; k < subpanel_data->control_data_count; k++)
               {
                  RegisterControlDropZone(&subpanel_data->control_data[k]);
               }
            }
         }
      }      
   }
}


