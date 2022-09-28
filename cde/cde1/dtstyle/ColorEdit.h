/* $XConsortium: ColorEdit.h /main/cde1_maint/2 1995/10/23 11:09:17 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        ColorEdit.h
 **
 **   Project:     DT 3.0
 **
 **  This file contains function definitions for the corresponding .c
 **  file
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990.  
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
#ifndef _coloredit_h
#define _coloredit_h

/*  Global structure for Color Editor */

typedef struct _EditData {
  Widget      chip_selected;
  Widget      DialogShell;
  Widget      main_form;
  Widget      sliderForm;
  Widget      oldButton;
  Widget      newButton;
  Widget      grabColor;
  Widget      redLabel;
  Widget      greenLabel;
  Widget      blueLabel;
  Widget      hueLabel;
  Widget      satLabel;
  Widget      valLabel;
  Widget      redScale;
  Widget      greenScale;
  Widget      blueScale;
  Widget      hueScale;
  Widget      satScale;
  Widget      valScale;
  int         current_scale;
  ColorSet    *color_set;
  ColorSet    oldButtonColor;
  XmColorProc calcRGB;
  Pixmap      pixmap25;
  Pixmap      pixmap75;

} EditData;

extern EditData edit;

/* External Interface */

#ifdef _NO_PROTO

extern void ColorEditor() ;
extern void RGBtoHSV() ;
extern void restoreColorEdit() ;
extern void saveColorEdit() ;

#else

extern void ColorEditor( 
                        Widget parent,
                        ColorSet *color_set) ;
extern void RGBtoHSV( 
#if NeedWidePrototypes
                        unsigned int r,
                        unsigned int g,
                        unsigned int b,
#else
                        unsigned short r,
                        unsigned short g,
                        unsigned short b,
#endif
                        int *h,
                        int *s,
                        int *v) ;
extern void restoreColorEdit( 
                        Widget shell,
                        XrmDatabase db) ;
extern void saveColorEdit( 
                        int fd) ;

#endif /* _NO_PROTO */


#endif /* _coloredit_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
