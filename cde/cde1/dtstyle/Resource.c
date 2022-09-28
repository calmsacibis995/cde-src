/* $XConsortium: Resource.c /main/cde1_maint/2 1995/10/23 11:25:06 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Resource.c
 **
 **   Project:     DT 3.0
 **
 **   Description: Controls the Dtstyle resources
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990.  
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
/*+++++++++++++++++++++++++++++++++++++++*/
/* include files                         */
/*+++++++++++++++++++++++++++++++++++++++*/

#include <Xm/Xm.h>
#include <Xm/XmP.h>

#include "Main.h"

/*+++++++++++++++++++++++++++++++++++++++*/
/* include extern functions              */
/*+++++++++++++++++++++++++++++++++++++++*/
#include "Resource.h"

/*+++++++++++++++++++++++++++++++++++++++*/
/* Local #defines                        */
/*+++++++++++++++++++++++++++++++++++++++*/
#define DEF_FONT "Fixed"
#define DEF_GROUP "Default"

#ifndef CDE_INSTALLATION_TOP
#define CDE_INSTALLATION_TOP "/opt/dt"
#endif
/*+++++++++++++++++++++++++++++++++++++++*/
/* Internal Functions                    */
/*+++++++++++++++++++++++++++++++++++++++*/
#ifdef _NO_PROTO

#else

#endif /* _NO_PROTO */

/*+++++++++++++++++++++++++++++++++++++++*/
/* Internal Variables                    */
/*+++++++++++++++++++++++++++++++++++++++*/


/*++++++++++++++++++++++++++++++++++++++*/
/* Application Resources                */
/*++++++++++++++++++++++++++++++++++++++*/

XtResource sysFont_resources[] = {

  {"systemFont1", "SystemFont1", XmRFontList, sizeof (XmFontList), 
      XtOffset(ApplicationDataPtr, fontChoice[0].sysFont), XmRString, 
      "-adobe-helvetica-medium-r-normal--10-*-iso8859-1"
  },
  {"systemFont2", "SystemFont2", XmRFontList, sizeof (XmFontList), 
      XtOffset(ApplicationDataPtr, fontChoice[1].sysFont), XmRString, 
      "-adobe-helvetica-medium-r-normal--12-*-iso8859-1"
  },
  {"systemFont3", "SystemFont3", XmRFontList, sizeof (XmFontList), 
      XtOffset(ApplicationDataPtr, fontChoice[2].sysFont), XmRString,
      "-adobe-helvetica-medium-r-normal--14-*-iso8859-1"
  },
  {"systemFont4", "SystemFont4", XmRFontList, sizeof (XmFontList), 
      XtOffset(ApplicationDataPtr, fontChoice[3].sysFont), XmRString,
      "-adobe-helvetica-medium-r-normal--17-*-iso8859-1"
  },
  {"systemFont5", "SystemFont5", XmRFontList, sizeof (XmFontList), 
      XtOffset(ApplicationDataPtr, fontChoice[4].sysFont), XmRString,
      "-adobe-helvetica-medium-r-normal--18-*-iso8859-1"
  },
  {"systemFont6", "SystemFont6", XmRFontList, sizeof (XmFontList), 
      XtOffset(ApplicationDataPtr, fontChoice[5].sysFont), XmRString,
      "-adobe-helvetica-medium-r-normal--20-*-iso8859-1"
  },
  {"systemFont7", "SystemFont7", XmRFontList, sizeof (XmFontList), 
      XtOffset(ApplicationDataPtr, fontChoice[6].sysFont), XmRString,
      "-adobe-helvetica-medium-r-normal--24-*-iso8859-1"
  },
};

XtResource userFont_resources[] = {

  {"userFont1", "UserFont1", XmRFontList, sizeof (XmFontList), 
      XtOffset(ApplicationDataPtr, fontChoice[0].userFont), XmRString, 
      "-adobe-courier-medium-r-normal--10-*-iso8859-1"
  },
  {"userFont2", "UserFont2", XmRFontList, sizeof (XmFontList), 
      XtOffset(ApplicationDataPtr, fontChoice[1].userFont), XmRString,
      "-adobe-courier-medium-r-normal--12-*-iso8859-1"
  },
  {"userFont3", "UserFont3", XmRFontList, sizeof (XmFontList), 
      XtOffset(ApplicationDataPtr, fontChoice[2].userFont), XmRString,
      "-adobe-courier-medium-r-normal--14-*-iso8859-1"
  },
  {"userFont4", "UserFont4", XmRFontList, sizeof (XmFontList), 
      XtOffset(ApplicationDataPtr, fontChoice[3].userFont), XmRString,
      "-adobe-courier-medium-r-normal--17-*-iso8859-1"
  },
  {"userFont5", "UserFont5", XmRFontList, sizeof (XmFontList), 
      XtOffset(ApplicationDataPtr, fontChoice[4].userFont), XmRString,
      "-adobe-courier-medium-r-normal--18-*-iso8859-1"
  },
  {"userFont6", "UserFont6", XmRFontList, sizeof (XmFontList), 
      XtOffset(ApplicationDataPtr, fontChoice[5].userFont), XmRString,
      "-adobe-courier-medium-r-normal--20-*-iso8859-1"
  },
  {"userFont7", "UserFont7", XmRFontList, sizeof (XmFontList), 
      XtOffset(ApplicationDataPtr, fontChoice[6].userFont), XmRString,
      "-adobe-courier-medium-r-normal--24-*-iso8859-1"
  },
};

XtResource sysStr_resources[] = {

  {"systemFont1", "SystemFont1", XmRString, sizeof (XmString), 
      XtOffset(ApplicationDataPtr, fontChoice[0].sysStr), XmRString,
      "-adobe-helvetica-medium-r-normal--10-*-iso8859-1"
  },
  {"systemFont2", "SystemFont2", XmRString, sizeof (XmString), 
      XtOffset(ApplicationDataPtr, fontChoice[1].sysStr), XmRString,
      "-adobe-helvetica-medium-r-normal--12-*-iso8859-1"
  },
  {"systemFont3", "SystemFont3", XmRString, sizeof (XmString), 
      XtOffset(ApplicationDataPtr, fontChoice[2].sysStr), XmRString,
      "-adobe-helvetica-medium-r-normal--14-*-iso8859-1"
  },
  {"systemFont4", "SystemFont4", XmRString, sizeof (XmString), 
      XtOffset(ApplicationDataPtr, fontChoice[3].sysStr), XmRString,
      "-adobe-helvetica-medium-r-normal--17-*-iso8859-1"
  },
  {"systemFont5", "SystemFont5", XmRString, sizeof (XmString), 
      XtOffset(ApplicationDataPtr, fontChoice[4].sysStr), XmRString,
      "-adobe-helvetica-medium-r-normal--18-*-iso8859-1"
  },
  {"systemFont6", "SystemFont6", XmRString, sizeof (XmString), 
      XtOffset(ApplicationDataPtr, fontChoice[5].sysStr), XmRString,
      "-adobe-helvetica-medium-r-normal--20-*-iso8859-1"
  },
  {"systemFont7", "SystemFont7", XmRString, sizeof (XmString), 
      XtOffset(ApplicationDataPtr, fontChoice[6].sysStr), XmRString,
      "-adobe-helvetica-medium-r-normal--24-*-iso8859-1"
  },
};

XtResource userStr_resources[] = {

  {"userFont1", "UserFont1", XmRString, sizeof (XmString), 
      XtOffset(ApplicationDataPtr, fontChoice[0].userStr), XmRString,
      "-adobe-courier-medium-r-normal--10-*-iso8859-1"
  },
  {"userFont2", "UserFont2", XmRString, sizeof (XmString), 
      XtOffset(ApplicationDataPtr, fontChoice[1].userStr), XmRString,
      "-adobe-courier-medium-r-normal--12-*-iso8859-1"
  },
  {"userFont3", "UserFont3", XmRString, sizeof (XmString), 
      XtOffset(ApplicationDataPtr, fontChoice[2].userStr), XmRString,
      "-adobe-courier-medium-r-normal--14-*-iso8859-1"
  },
  {"userFont4", "UserFont4", XmRString, sizeof (XmString), 
      XtOffset(ApplicationDataPtr, fontChoice[3].userStr), XmRString,
      "-adobe-courier-medium-r-normal--17-*-iso8859-1"
  },
  {"userFont5", "UserFont5", XmRString, sizeof (XmString), 
      XtOffset(ApplicationDataPtr, fontChoice[4].userStr), XmRString,
      "-adobe-courier-medium-r-normal--18-*-iso8859-1"
  },
  {"userFont6", "UserFont6", XmRString, sizeof (XmString), 
      XtOffset(ApplicationDataPtr, fontChoice[5].userStr), XmRString,
      "-adobe-courier-medium-r-normal--20-*-iso8859-1"
  },
  {"userFont7", "UserFont7", XmRString, sizeof (XmString), 
      XtOffset(ApplicationDataPtr, fontChoice[6].userStr), XmRString,
      "-adobe-courier-medium-r-normal--24-*-iso8859-1"
  },
};

XtResource userBoldFont_resources[] = {
 
  {"userBoldFont1", "UserBoldFont1", XmRFontList, sizeof (XmFontList),
      XtOffset(ApplicationDataPtr, fontChoice[0].userBoldFont), XmRString,
      "-dt-interface user-bold-r-normal-xxs*-*-*-*-*-*-*-*-*:"
  },
  {"userBoldFont2", "UserBoldFont2", XmRFontList, sizeof (XmFontList),
      XtOffset(ApplicationDataPtr, fontChoice[1].userBoldFont), XmRString,
      "-dt-interface user-bold-r-normal-xs*-*-*-*-*-*-*-*-*:"
  },
  {"userBoldFont3", "UserBoldFont3", XmRFontList, sizeof (XmFontList),
      XtOffset(ApplicationDataPtr, fontChoice[2].userBoldFont), XmRString,
      "-dt-interface user-bold-r-normal-s*-*-*-*-*-*-*-*-*:"
  },
  {"userBoldFont4", "UserBoldFont4", XmRFontList, sizeof (XmFontList),
      XtOffset(ApplicationDataPtr, fontChoice[3].userBoldFont), XmRString,
      "-dt-interface user-bold-r-normal-m*-*-*-*-*-*-*-*-*:"
  },
  {"userBoldFont5", "UserBoldFont5", XmRFontList, sizeof (XmFontList),
      XtOffset(ApplicationDataPtr, fontChoice[4].userBoldFont), XmRString,
      "-dt-interface user-bold-r-normal-l*-*-*-*-*-*-*-*-*:"
  },
  {"userBoldFont6", "UserBoldFont6", XmRFontList, sizeof (XmFontList),
      XtOffset(ApplicationDataPtr, fontChoice[5].userBoldFont), XmRString,
      "-dt-interface user-bold-r-normal-xl*-*-*-*-*-*-*-*-*:"
  },
  {"userBoldFont7", "UserBoldFont7", XmRFontList, sizeof (XmFontList),
      XtOffset(ApplicationDataPtr, fontChoice[6].userBoldFont), XmRString,
      "-dt-interface user-bold-r-normal-xxl*-*-*-*-*-*-*-*-*:"
  },
};

XtResource userBoldStr_resources[] = {
 
  {"userBoldFont1", "UserBoldFont1", XmRString, sizeof (XmString),
      XtOffset(ApplicationDataPtr, fontChoice[0].userBoldStr), XmRString,
      "-dt-interface user-bold-r-normal-xxs*-*-*-*-*-*-*-*-*:"
  },
  {"userBoldFont2", "UserBoldFont2", XmRString, sizeof (XmString),
      XtOffset(ApplicationDataPtr, fontChoice[1].userBoldStr), XmRString,
      "-dt-interface user-bold-r-normal-xs*-*-*-*-*-*-*-*-*:"
  },
  {"userBoldFont3", "UserBoldFont3", XmRString, sizeof (XmString),
      XtOffset(ApplicationDataPtr, fontChoice[2].userBoldStr), XmRString,
      "-dt-interface user-bold-r-normal-s*-*-*-*-*-*-*-*-*:"
  },
  {"userBoldFont4", "UserBoldFont4", XmRString, sizeof (XmString),
      XtOffset(ApplicationDataPtr, fontChoice[3].userBoldStr), XmRString,
      "-dt-interface user-bold-r-normal-m*-*-*-*-*-*-*-*-*:"
  },
  {"userBoldFont5", "UserBoldFont5", XmRString, sizeof (XmString),
      XtOffset(ApplicationDataPtr, fontChoice[4].userBoldStr), XmRString,
      "-dt-interface user-bold-r-normal-l*-*-*-*-*-*-*-*-*:"
  },
  {"userBoldFont6", "UserBoldFont6", XmRString, sizeof (XmString),
      XtOffset(ApplicationDataPtr, fontChoice[5].userBoldStr), XmRString,
      "-dt-interface user-bold-r-normal-xl*-*-*-*-*-*-*-*-*:"
  },
  {"userBoldFont7", "UserBoldFont7", XmRString, sizeof (XmString),
      XtOffset(ApplicationDataPtr, fontChoice[6].userBoldStr), XmRString,
      "-dt-interface user-bold-r-normal-xxl*-*-*-*-*-*-*-*-*:"
  },
};


XtResource resources[] = {

  {"numFonts", "NumFonts", XmRInt, sizeof (int), 
      XtOffset(ApplicationDataPtr, numFonts), XmRImmediate, (caddr_t) 7
  },
  {"systemFont", "SystemFont", XmRFontList, sizeof (XmFontList), 
      XtOffset(ApplicationDataPtr, systemFont), XmRString, "Fixed"
  },
  {"userFont", "UserFont", XmRFontList, sizeof (XmFontList), 
      XtOffset(ApplicationDataPtr, userFont), XmRString, "Fixed"
  },
  {"systemFont", "SystemFont", XmRString, sizeof (XmString), 
      XtOffset(ApplicationDataPtr, systemFontStr), XmRString, DEF_FONT
  },
  {"userFont", "UserFont", XmRString, sizeof (XmString), 
      XtOffset(ApplicationDataPtr, userFontStr), XmRString, DEF_FONT
  },

  {"session", "Session", XmRString, sizeof (char *),
      XtOffset (ApplicationDataPtr, session), XmRImmediate, (XtPointer)NULL,
  },
  {"backdropDirectories", "BackdropDirectories", XmRString, sizeof(char *),
        XtOffset(ApplicationDataPtr, backdropDir), XmRString, NULL 
  },
  {"paletteDirectories", "PaletteDirectories", XmRString, sizeof(char *),
        XtOffset(ApplicationDataPtr, paletteDir), XmRString, NULL
  },
  {"timeoutScale", "TimeoutScale", XmRString, sizeof (String),
        XtOffset(ApplicationDataPtr, timeoutScale), XmRString, "15" 
  },
  {"lockoutScale", "LockoutScale", XmRString, sizeof (String),
        XtOffset(ApplicationDataPtr, lockoutScale), XmRString, "30" 
  },
  {"writeXrdbImmediate", "WriteXrdbImmediate", XmRBoolean, sizeof(Boolean) ,
        XtOffset(ApplicationDataPtr, writeXrdbImmediate), XmRImmediate, (XtPointer)True
  },
  {"writeXrdbColors", "WriteXrdbColors", XmRBoolean, sizeof(Boolean) ,
        XtOffset(ApplicationDataPtr, writeXrdbColors), XmRImmediate, (XtPointer)True
  },
  {"fontGroup", "FontGroup", XmRString, sizeof(XmString),
        XtOffset(ApplicationDataPtr, fontGroup), XmRString, DEF_GROUP 
  },
  {"componentList", "ComponentList", XtRString, sizeof(String) ,
        XtOffset(ApplicationDataPtr, componentList), XmRImmediate, 
        "Color Font Backdrop Keyboard Mouse Audio Screen Dtwm Startup"
  },
};

/************************************************************************
 * GetSysFontResources
 *
 *  Description:
 *  -----------
 *  This function is used to retrieve the Dtstyle System Font resources 
 ************************************************************************/
void
#ifdef _NO_PROTO
GetSysFontResource(i)
    int i;
#else
GetSysFontResource(int i)
#endif /* _NO_PROTO */
{
    XtGetApplicationResources(style.shell, &style.xrdb,
                            &sysFont_resources[i],
                              1, NULL, 0);
}

/************************************************************************
 * GetUserFontResources
 *
 *  Description:
 *  -----------
 *  This function is used to retrieve the Dtstyle User Font resources 
 ************************************************************************/
void
#ifdef _NO_PROTO
GetUserFontResource(i)
    int i;
#else
GetUserFontResource(int i)
#endif /* _NO_PROTO */
{
    XtGetApplicationResources(style.shell, &style.xrdb,
                            &userFont_resources[i],
                              1, NULL, 0);
}

/************************************************************************
 * GetUserBoldFontResources
 *
 *  Description:
 *  -----------
 *  This function is used to retrieve the Dtstyle User Bold Font resources
 ************************************************************************/
void
#ifdef _NO_PROTO
GetUserBoldFontResource(i)
    int i;
#else
GetUserBoldFontResource(int i)
#endif /* _NO_PROTO */
{
    XtGetApplicationResources(style.shell, &style.xrdb,
                            &userBoldFont_resources[i],
                              1, NULL, 0);
}

/************************************************************************
 * GetFontStrResources
 *
 *  Description:
 *  -----------
 ************************************************************************/
static void
#ifdef _NO_PROTO
GetFontStrResources( )
#else
GetFontStrResources( void )
#endif /* _NO_PROTO */
{
    int i;

    XtGetApplicationResources(style.shell, &style.xrdb, sysStr_resources,
                              XtNumber(sysStr_resources), NULL, 0);
    XtGetApplicationResources(style.shell, &style.xrdb, userStr_resources,
                              XtNumber(userStr_resources), NULL, 0);
    XtGetApplicationResources(style.shell, &style.xrdb, userBoldStr_resources,
                              XtNumber(userBoldStr_resources), NULL, 0);
    for (i=0; i<style.xrdb.numFonts; i++) {
      style.xrdb.fontChoice[i].userFont = NULL;
      style.xrdb.fontChoice[i].sysFont = NULL;
      style.xrdb.fontChoice[i].userBoldFont = NULL;
    }
}

/************************************************************************
 * GetApplicationResources
 *
 *  Description:
 *  -----------
 *  This function is used to retrieve Dtstyle resources that are 
 * not component-specific.
 ************************************************************************/
void 
#ifdef _NO_PROTO
GetApplicationResources( )
#else
GetApplicationResources( void )
#endif /* _NO_PROTO */
{
    XtGetApplicationResources(style.shell, &style.xrdb, resources,
                              XtNumber(resources), NULL, 0);
    GetFontStrResources();

}



