/* $XConsortium: DtDND.h /main/cde1_maint/2 1995/10/23 10:08:10 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef DTDND_H
#define DTDND_H

#include "MotifUI.h"

typedef enum
{
   FILENAME_TRANSFER,
   TEXT_TRANSFER,
   DROP_ON_ROOT,
   BUFFER_TRANSFER,
   CONVERT_DATA,
   CONVERT_DELETE,
   ANIMATE
} DNDProtocol;

typedef void (*DNDCallback) (BaseUI *, char **value, int *len, DNDProtocol);

class DtDND {

   friend void TransferCB(Widget, XtPointer, XtPointer);
   friend void ConvertCB(Widget, XtPointer, XtPointer);
   friend void DragFinishCB(Widget, XtPointer, XtPointer);
   friend void DropOnRootCB(Widget, XtPointer, XtPointer);
   friend void AnimateCB(Widget, XtPointer, XtPointer);
   friend void DragMotionHandler(Widget, XtPointer, XEvent *, Boolean *);

   static void TransferCB(Widget, XtPointer, XtPointer);
   static void ConvertCB(Widget, XtPointer, XtPointer);
   static void DragFinishCB(Widget, XtPointer, XtPointer);
   static void DropOnRootCB(Widget, XtPointer, XtPointer);
   static void AnimateCB(Widget, XtPointer, XtPointer);
   static void DragMotionHandler(Widget, XtPointer, XEvent *, Boolean *);

 private:

   static boolean FirstTime;
   static boolean DoingDrag;
   static XtCallbackRec transferCBRec[];
   static XtCallbackRec convertCBRec[];
   static XtCallbackRec dragFinishCBRec[];
   static XtCallbackRec dropOnRootCBRec[];
   static XtCallbackRec animateCBRec[];
   static GC gc;
   static GC gc_mask;

   MotifUI *obj;
   Widget dragIcon;
   Widget sourceIcon;
   DNDCallback dndCB;
   XRectangle rects[2];
   Pixmap pixmap;
   Pixmap mask;
   char *name;
   char *iconFile;
   Position s_x, s_y;
   Pixel bg, fg;
   unsigned char stringDirection;
   Pixel textSelectColor;
   XmString string;
   Pixel selectColor;
   unsigned char alignment;
   Boolean showSelectedPixmap;
   XmFontList fontList;
   int string_border_width;
   int p_x, p_y;
   int p_w, p_h, s_w, s_h;
   boolean selected;
   boolean can_drop_on_root;
   IconStyle icon_size;

   void GetRects();
   void GetDragPixmaps();
   void DrawString();
   void StartDrag(XEvent *);

 public:

   DtDND(MotifUI *obj, DNDCallback dndCB, boolean can_drop_on_root = false);
   ~DtDND();
   void UpdateActivity(boolean);
   void UpdateRects();

};

#endif // DTDND_H
