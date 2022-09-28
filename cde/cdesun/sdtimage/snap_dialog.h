#ifndef _SNAP_DLOG_UI_H_
#define _SNAP_DLOG_UI_H_

#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/Intrinsic.h>
#include "dtb_utils.h"

#define NUM_DELAYS           7
#define DEFAULT_SNAP_DELAY   8

typedef struct
{
    Boolean  created;
    Boolean  snap_needs_save;
    int            actual_delay;
    SnapRegionType snap_type;
    Boolean        delay_manually_set;
    Boolean        snap_in_progress;
    Boolean        snap_cancelled;

    Widget   dialog;
    Widget   MainForm;
    Widget   SnapTypeRadio;
    Widget   SnapDelay;
    Widget   BeepDuringCountDown;
    Widget   HideDuringCapture;
    Widget   OkSnapButton;
    Widget   CancelSnapButton;
    Widget   HelpSnapButton;
    Widget   StatusReport;
} DtbSnapDlogDialogInfoRec, *DtbSnapDlogDialogInfo;


extern DtbSnapDlogDialogInfoRec dtb_snap_dlog_dialog;

void BeepDuringCountDounCB(Widget w, XtPointer clientData, XtPointer callData);
void HideDuringCaptureCB(Widget w, XtPointer clientData, XtPointer callData);
void SnapOkCB(Widget w, XtPointer clientData, XtPointer callData);
void SnapCancelCB(Widget w, XtPointer clientData, XtPointer callData);
void DisplayStatusReport(Widget Label, char *Report);
void SnapDelayValueChangedCB(Widget w, XtPointer clientData, XtPointer callData);

void SnapKeyPress(Widget, XtPointer, XEvent*, Boolean*);
void SnapButtonPress(Widget, XtPointer, XEvent*, Boolean*);
void SnapButtonRelease(Widget, XtPointer, XEvent*, Boolean*);
void SnapButtonMotion(Widget, XtPointer, XEvent*, Boolean*);
void CheckDelayTime(Widget widget, XtPointer, XtPointer);

void SnapTypeToggledCB(Widget w, XtPointer clientData, XtPointer callData);

void DelayTheSnap(XtPointer, XtIntervalId *);
void ReallySnap(XtPointer, XtIntervalId *);
void mapWindow();
void unmapWindow();
void SnapOnCrosshairCursor(Widget);
void SnapOffCrosshairCursor(Widget);
int  OpenNewImage();

int dtbSnapDlogDialogInfo_clear(DtbSnapDlogDialogInfo snapDialogInfo);

void SdtFixShellSize(Widget SdtDialogShell, XtPointer data, XConfigureEvent *event);
void SdtBindDeleteToCancelCallback(Widget SomeVendorShell,
                                   void (*CancelFunction)(), XtPointer Closure);

Boolean SaveUnsavedSnapshot();

#endif
