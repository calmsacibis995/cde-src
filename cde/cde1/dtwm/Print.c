/* $XConsortium: Print.c /main/cde1_maint/2 1995/08/26 23:02:38 barstow $ */
/*****************************************************************************
 *
 *   File:         Print.c
 *
 *   Project:	    DT
 *
 *   Description:  This file contains the function which prints out the
 *                 front panel contents.  It is used as a client for this
 *                 purpose.
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of Novell, Inc.
 *
 ****************************************************************************/

#include <Dt/DtP.h>                /* required for DtDirPaths type */
#include <Dt/DbReader.h>            /* required for DtDbPathId type */
#include <Dt/WsmM.h>
#include <Dt/IconFile.h>
#include "DataBaseLoad.h"
#include "Parse.h"

#ifdef __STDC__
const char *szWM_TOOL_CLASS = DtWM_TOOL_CLASS;
#else
char *szWM_TOOL_CLASS = DtWM_TOOL_CLASS;
#endif /* __STDC__ */



String unpost_arrow_image;
String unpost_monitor_arrow_image;

/************************************************************************
 *
 *  CheckOtherMonitorsOn
 *      Due to a dependancy, CheckOtherMonitorsOn must be defined.  This
 *      is a stubbed procedure that ensures that the procedure is defined.
 *
 ************************************************************************/
Boolean
#ifdef _NO_PROTO
CheckOtherMonitorsOn(subpanel_data)
SubpanelData * subpanel_data;
#else
CheckOtherMonitorsOn(SubpanelData * subpanel_data)
#endif /* _NO_PROTO */
{
   return False;
}


/************************************************************************
 *
 *  ToggleDefaultControl
 *      Due to a dependancy, ToggleDefaultControl must be defined.  This
 *      is a stubbed procedure that ensures that the procedure is defined.
 *
 ************************************************************************/

#ifdef _NO_PROTO

void
ToggleDefaultControl (main_control_data, subpanel_data, control_data)
ControlData  * main_control_data;
SubpanelData * subpanel_data;
ControlData  * control_data;

#else

void
ToggleDefaultControl (ControlData  * main_control_data,
                      SubpanelData * subpanel_data,
                      ControlData  * control_data)

#endif /* _NO_PROTO */

{
}




/************************************************************************
 *
 *  GetIconName
 *      Get the file name for an icon by extracting the panel resolution
 *      and then looking up the image name.
 *
 ************************************************************************/

#ifdef _NO_PROTO

String
GetIconName (image_name, icon_size)
String       image_name;
unsigned int icon_size;

#else /* _NO_PROTO */

String
GetIconName (String       image_name, 
             unsigned int icon_size)

#endif /* _NO_PROTO */

{
   String       return_name;
   Screen     * screen = XtScreen (panel.shell);

   /*  Get name.  */

   return_name = _DtGetIconFileName (screen, image_name, NULL, NULL, icon_size);

   if (return_name == NULL)
       return_name = _DtGetIconFileName (screen, image_name, NULL, NULL,
		                         DtUNSPECIFIED);
   if (return_name == NULL)
       return_name = XtNewString (image_name);


   /*  Return value to be freed by caller.  */

   return (return_name);
}




/************************************************************************
 *
 *  PrintFrontPanelContents
 *      Using the information from the loading of the database
 *      print out the contents of the database.
 *
 ************************************************************************/

void
#ifdef _NO_PROTO
PrintFrontPanelContents()
#else

PrintFrontPanelContents(void)
#endif /* _NO_PROTO */

{
   BoxData * box_data;
   SwitchData * switch_data;
   ControlData * control_data, * switch_control_data;
   SubpanelData * subpanel_data;
   int switch_position = POSITION_FIRST;
   int i,j, k;


   /*  print out the component tree  */
   
   printf ("PANEL	%s\n", (char *) panel.element_values[0].parsed_value);

   for (i = 0; i < panel.box_data_count; i++)
   {
      box_data = panel.box_data[i];

      if (box_data->switch_data != NULL)
      {
         switch_data = box_data->switch_data;
         switch_position = (int) box_data->switch_data->element_values[SWITCH_POSITION_HINTS].parsed_value;
      }
      else
         switch_data = NULL;


      printf ("   BOX	%s\n", 
              (char *) box_data->element_values[0].parsed_value);

      for (j = 0; j < box_data->control_data_count; j++)
      {
         control_data = box_data->control_data[j];

         if (switch_data != NULL &&
             switch_position < (int) (control_data->element_values[CONTROL_POSITION_HINTS].parsed_value))
         {
            printf ("      SWITCH	%s\n",
                 (char *) switch_data->element_values[0].parsed_value);

            switch_position = POSITION_LAST;

            for (k = 0; k < switch_data->control_data_count; k++)
            {
		switch_control_data = switch_data->control_data[k];

                printf ("         CONTROL	%s\n", (char *)
			switch_control_data->element_values[0].parsed_value);

            }
         }

	 printf ("      CONTROL	%s\n",
                 (char *) control_data->element_values[0].parsed_value);

         if (control_data->subpanel_data != NULL)
	 {
            subpanel_data = control_data->subpanel_data;

            printf("         SUBPANEL	%s\n", (char *)
		    subpanel_data->element_values[0].parsed_value);
      
            for (k = 0; k < subpanel_data->control_data_count; k++)
            {
                control_data = subpanel_data->control_data[k];

                printf ("               CONTROL	%s\n", (char *)
			control_data->element_values[0].parsed_value);
            }
	 }
      }
   }
}




/************************************************************************
 *
 *  The main program for the fron panel print function.
 *
 ************************************************************************/

void
#ifdef _NO_PROTO
main (argc, argv)
      int argc;
      char         **argv;
#else
main (int argc,
      char         **argv)

#endif /* _NO_PROTO */

{
    XtAppContext    appContext;
    Widget		widget;

   /* This call is required to have values to pass to DtAppInitialize */
    widget = XtAppInitialize( &appContext, "Dtfplist",
                              NULL, 0, &argc, argv, NULL, NULL, 0);

   /* This is required initialization so that FrontPanelReadDatabases()
    * procedure complete successfully.
    */
    DtAppInitialize( appContext, XtDisplay(widget), widget,
		     argv[0], (char *)szWM_TOOL_CLASS);

    
   /* Load the database for use in printing. If it is able to load print
    * contents of front panel.
    */
    DtDbLoad();

    panel.app_name = strdup(argv[0]);
    if (FrontPanelReadDatabases ())
    {
      /* Print out the contents of the .fp database */
       PrintFrontPanelContents ();
    }
    else
    {
      /* NEEDS TO BE LOCALIZED */
       printf ("PANEL not found. Error in reading database.\n");
    }

}
