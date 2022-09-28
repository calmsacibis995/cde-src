/* $XConsortium: MainWindow.h /main/cde1_maint/2 1995/10/23 10:15:36 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "MotifUI.h"

class PopupMenuContainer;
class Button;

class MainWindow : public MotifUI {

   friend void KeyboardPopupMenu(Widget, XEvent *, String *, Cardinal *);
   friend void PopupMenu(Widget, XtPointer, XEvent *, Boolean *);

 protected:

   static XtActionsRec actions[];
   static void KeyboardPopupMenu(Widget, XEvent *, String *, Cardinal *);
   static void PopupMenu(Widget, XtPointer, XEvent *, Boolean *);

   void PostMenu(MotifUI *, XEvent *);
   PopupMenuContainer *popups;
   Widget LastPopupMenu;
   XtTranslations trans;
   char *icon;
   char *icon_name;

   // Derived classes must define Initialize
   virtual void Initialize() = 0;

   void CreateMainWindow(MotifUI *parent, char *name, char *widgetName,
			 char *icon, char *icon_name);

 public:

   MainWindow(MotifUI *parent,
	      char *name,
	      char *widgetName,
	      char *icon = NULL,
	      char *icon_name = NULL);

   MainWindow(char *category,
	      MotifUI *parent,
	      char *name,
	      char *widgetName,
	      char *icon = NULL,
	      char *icon_name = NULL);
   virtual ~MainWindow();

   void IconFile(char *icon);
   void IconName(char *icon_name);
   char *IconFile() { return icon; }
   char *IconName() { return icon_name; }

   Button *AddAction(char *name, char *category, ButtonCallback callback,
		     void *callback_data, char *mnemonic = NULL,
		     char *acceleratorText = NULL, char *accelerator = NULL);
   void RegisterPopup(MotifUI *object);
   void AddSep(char *category);

   void SetWorkWindow(MotifUI *obj);

   BaseUI *PopupObject;
   int PopupObjectUniqueID;

   const UI_Class UIClass()            { return MAIN_WINDOW; }
   const char *const UIClassName()     { return "MainWindow"; }

};

#endif /* MAINWINDOW_H */
