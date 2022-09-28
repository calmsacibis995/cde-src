/* $XConsortium: cmnrtns.h /main/cde1_maint/2 1995/10/02 18:01:20 lehors $ */
#include <Dt/IconFile.h>
/***************************************************************************/
/*                                                                         */
/*  cmnrtns.h                                                              */
/*                                                                         */
/***************************************************************************/

#ifndef _CMNRTNS_H_INCLUDED
#define _CMNRTNS_H_INCLUDED

#ifndef STORAGECLASS
#ifdef  NOEXTERN
#define STORAGECLASS
#else
#define STORAGECLASS extern
#endif
#endif

/***************************************************************************/
/*                                                                         */
/*  Macro Declarations                                                     */
/*                                                                         */
/***************************************************************************/
#define SET_ICONGADGET_ICON_AND_EXT(widIconGadget,pszIconFileName, pszBuffer) {\
          sprintf(pszBuffer, "%s%s", pszIconFileName, bShowPixmaps ? PIXMAP_EXT : BITMAP_EXT); \
          XtVaSetValues (widIconGadget,                                     \
                         XmNimageName, "",                                  \
                         NULL);                                             \
        /*printf("setting icon to = '%s'\n", pszBuffer);*/                  \
          XtVaSetValues (widIconGadget,                                     \
                         XmNimageName, pszBuffer,                           \
                         NULL);                                             \
}

#define SET_ICONGADGET_ICON(widIconGadget,pszIconFileName) {                \
          /*printf("setting icon to = '%s'\n", pszIconFileName);*/          \
          XtVaSetValues (widIconGadget,                                     \
                         XmNimageName, "",                                  \
                         NULL);                                             \
          XtVaSetValues (widIconGadget,                                     \
                         XmNimageName, pszIconFileName,                     \
                         NULL);                                             \
}

#define FIND_ICONGADGET_ICON(pszIcon,pszFile,size) {                        \
          pszFile = (char *)NULL;                                           \
          pszFile = _DtGetIconFileName(XtScreen(CreateActionAppShell),       \
                                      NULL,                                 \
                                      pszIcon,                              \
                                      NULL,                                 \
                                      size);                                \
}

#define CHANGE_ICONGADGET_ICON(widIconGadget,buffer,name,size,type) {       \
          sprintf(buffer, "%s.%s.%s", name, size, type);                    \
          if (check_file_exists(buffer)) {                                  \
            SET_ICONGADGET_ICON(widIconGadget, buffer);                     \
          } else {                                                          \
            SET_ICONGADGET_ICON(widIconGadget, "");                         \
          }                                                                 \
}

#define SET_TOGGLEBUTTON(widToggleButton,bState) {                          \
          XtVaSetValues (widToggleButton,                                   \
                         XmNset, bState,                                    \
                         NULL);                                             \
}

/***************************************************************************/
/*                                                                         */
/*  Prototypes for functions                                               */
/*                                                                         */
/***************************************************************************/
#ifdef _NO_PROTO

ushort WriteDefinitionFile();
ushort WriteActionFile();
void   Change_IconGadget_IconType();
void   load_icons ();
void   GetWidgetTextString();
void   PutWidgetTextString ();
char * GetCoreName();
char **GetIconSearchPathList();
void   FreeIconSearchPathList();
void   TurnOnHourGlassAllWindows();
void   TurnOffHourGlassAllWindows();
char * ReplaceSpaces();
void   SetIconData();
char * GetCorrectIconType();
char * CreateMaskName();
IconData * GetIconDataFromWid();

#else /* _NO_PROTO */

ushort WriteDefinitionFile(char *, ActionData *);
ushort WriteActionFile(ActionData *);
void   Change_IconGadget_IconType(Widget, char *);
void   load_icons ( Widget wid, XtPointer client_data,
		 XmFileSelectionBoxCallbackStruct *cbs);
void   GetWidgetTextString(Widget, char **);
void   PutWidgetTextString (Widget wid, char *pszText);
char * GetCoreName(char *pszFullName);
char **GetIconSearchPathList(void);
void   FreeIconSearchPathList(char **);
void   TurnOnHourGlassAllWindows();
void   TurnOffHourGlassAllWindows();
char * ReplaceSpaces(char *pszName);
void   SetIconData(Widget, char *, enum icon_size_range);
char * GetCorrectIconType(char *);
char * CreateMaskName(char *);
IconData * GetIconDataFromWid(Widget);

#endif /* _NO_PROTO */

#endif /* _CMNRTNS_H_INCLUDED */
