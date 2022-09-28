/* $XConsortium: DtMainW.h /main/cde1_maint/2 1995/10/23 09:55:05 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef DTMAINW_H
#define DTMAINW_H

#include "MainWindow.h"
#include "Menu.h"
#include "MenuBar.h"
#include "DtActions.h"
#include "DtSetPref.h"
#include "DtPrinterIcon.h"

class Button;
class DtSetModList;
class DtWorkArea;
class DtFindD;
class LabelObj;
class HelpSystem;
class DtFindSet;

typedef void (*OpenCallback) (void *callback_data, BaseUI *object);

typedef struct 
{
   ActionCallback actionCallback;
   void *callback_data; 
   char *actionReferenceName;
} ActionData;

class DtMenu : public Menu 
{
 public:
   char *location_id;
   DtMenu(MenuBar *parent, char *name, char *mnemonic, char *help_location_id) :
      Menu(parent, name, mnemonic)
       {
	 location_id = help_location_id;
       }
   boolean HandleHelpRequest();
};

class DtMainW : public MainWindow 
{

   friend void SetPrefCB(void *data);
   friend void ExitCB(void *data);
   friend void ActionCB(void *data);
   friend void HelpCB(void *data);
   friend void ReferenceCB(void *data);
   friend void UsingHelpCB(void *data);
   friend void TasksCB(void *data);
   friend void AboutCB(void *data);

 private:

   OpenCallback _openClose;
   PreferenceCallback _prefCB;
   ContainerType _container_type;
   SelectionType _select_type;
   void *_openCallbackData;
   void *_prefCallbackData;
   char *_fileMenuName;
   char *_fileMenuMnemonic;
   ActionData **action_data;
   int n_action_data;
   int working_curs;
   PrinterApplicationMode app_mode;

   static void SetPrefCB(void *data);
   static void ExitCB(void *data);
   static void ActionCB(void *data);
   static void OnItemCB(void *data);
   static void HelpCB(void *data);
   static void ReferenceCB(void *data);
   static void UsingHelpCB(void *data);
   static void TasksCB(void *data);
   static void AboutCB(void *data);

   boolean HandleHelpRequest();

 public:

   DtFindD *     findD;      // Find dialog
   DtSetPref *   setPrefD;   // Set preferences dialog
   DtSetModList *setModList; // Set printer list dialog
   DtWorkArea *  container;  // Work Area
   MenuBar *     mbar;       // Menu Bar
   HelpSystem *  helpSystem;
   DtFindSet *   findSetD;

   // Pulldown menu structure
   DtMenu *fileMenu;
       Button *exitB;
   DtMenu *viewMenu;
       Button *setPref;
   DtActions *actionsMenu;
   DtMenu *helpMenu;
       Button *introduction;
       // ------------------- Separator
       Button *tasks;
       Button *reference;
       Button *onWindow;
       // ------------------- Separator
       Button *usingHelp;
       // ------------------- Separator
       Button *version;

   LabelObj *status_line;
   boolean in_find;

   DtMainW(char *category,
           AnyUI *p,
	   char *name,
	   ContainerType container_type = SCROLLED_WORK_AREA,
	   SelectionType select_type = SINGLE_SELECT,
	   OpenCallback openCB = NULL,
	   void *openCallbackData = NULL,
	   PreferenceCallback prefCB = NULL,
	   void *prefCallbackData = NULL,
	   char *fileMenuName = NULL,
	   char *fileMenuMnemonic = NULL,
           PrinterApplicationMode app_mode = SINGLE_PRINTER);
   ~DtMainW();
   void DtAddAction(char *name, char *category,
                    char *actionReferenceName, ActionCallback callback,
                    void *callback_data, char *mnemonic = NULL,
                    char *acceleratorText = NULL, char *accelerator = NULL);

   void WorkingCursor(boolean);
   PrinterApplicationMode PrinterAppMode() { return app_mode; }
   void DisplayHelp(char *location_id, char *volume = "Printmgr");
   void DtAddSep(char *category);
   void Initialize();
   void OpenClose(BaseUI *obj);

};

#endif /* DTMAINW_H */
