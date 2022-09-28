/* $XConsortium: DtSetPref.h /main/cde1_maint/2 1995/10/23 10:02:05 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef DTSETPREF_H
#define DTSETPREF_H

#include "Dialog.h"
#include "Button.h"
#include "ScaleObj.h"

class Group;
class DtMainW;

typedef enum
{
   DETAILS_ON,
   DETAILS_OFF,
   STATUS_LINE_ON,
   STATUS_LINE_OFF,
   STATUS_FLAG_ON,
   STATUS_FLAG_OFF,
   SHOW_ONLY_MINE_ON,
   SHOW_ONLY_MINE_OFF,
   SHOW_DETAILS_LABEL_ON,
   SHOW_DETAILS_LABEL_OFF,
   UPDATE_INTERVAL_CHANGED
} PreferenceRequest;

typedef void (*PreferenceCallback) (void *callback_data, PreferenceRequest,
				    char *value);
class DtSetPref : public Dialog {

   friend void OkCB(void *);
   friend void ApplyCB(void *);
   friend void CancelCB(void *);
   friend void HelpCB(void *);
   friend void RepCB(void *);

 private:

   boolean _has_been_posted;
   IconStyle _previous_style;
   boolean _previous_status_flag;
   boolean _previous_status_line;
   boolean _previous_show_only_mine;
   boolean _previous_show_details_label;
   int _previous_update_interval;
   AnyUI *_container;
   PreferenceCallback _callback;
   void *_callback_data;

   DtMainW *mainw;

   Group *show_jobs;
        Button *only_mine;
        Button *everyones;

   Group *status;
        Button *status_line;
        Button *status_flag;

   Group *view_group;
        Button *by_name;
        Button *by_icon;
        Button *by_smallIcon;
        Button *by_properties;
        Button *show_labels;

   Group *update_group;
        ScaleObj *update_interval;

   // dialog buttons
   Button *ok;
   Button *apply;
   Button *cancel;
   Button *help;

   static void OkCB(void *);
   static void ApplyCB(void *);
   static void CancelCB(void *);
   static void HelpCB(void *);
   static void RepCB(void *);
   void CloseCB();

   boolean SetVisiblity(boolean flag);

   void Cancel();
   void Reset();
   boolean HandleHelpRequest();

 public:

   DtSetPref(AnyUI *parent, char *name, AnyUI *container, 
             PreferenceCallback callback, void *callback_data);
   virtual ~DtSetPref();

   void Apply();
   boolean ShowStatusFlags()  { return _previous_status_flag; }
   boolean ShowStatusLine()   { return _previous_status_line; }
   boolean ShowOnlyMyJobs()   { return _previous_show_only_mine; }
   boolean ShowDetailsLabel() { return _previous_show_details_label; }
   IconStyle Represetation()  { return _previous_style; }
   int UpdateInterval()       { return _previous_update_interval; }
   void Representation(IconStyle);
   void ShowOnlyMyJobs(boolean);
   void ShowStatusFlags(boolean value)  { status_flag->Selected(value); }
   void ShowStatusLine(boolean value)   { status_line->Selected(value); }
   void ShowDetailsLabel(boolean value) { show_labels->Selected(value); }
   void UpdateInterval(int value)       { update_interval->Value(value); }

};

#endif /* DTSETPREF_H */
