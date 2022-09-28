/* $XConsortium: ColorMain.h /main/cde1_maint/3 1995/10/23 11:11:09 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        ColorMain.h
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
#ifndef _colormain_h
#define _colormain_h

#include <Xm/ColorObj.h>

/*  #defines  */
#define PALETTE_SUFFIX       ".dp"

/* defines for foreground colors */
#define DYNAMIC    0
#define BLACK      1
#define WHITE      2

/* selections */
#define GET_TYPE_MONITOR         1
#define GET_PALETTE_NAME         2
#define GET_UPDATE               3


/* typedef statements */

/* ColorSet data structure */

typedef struct {
    XColor fg;
    XColor bg;
    XColor ts;
    XColor bs;
    XColor sc;
} ColorSet;


/*  Palette info */

typedef struct _palette {
      char *name;
      char *desc;
      char *directory;
      int item_position;
      int num_of_colors;
      ColorSet color[MAX_NUM_COLORS];
      struct _palette *next;
      short active;
      short inactive;
      short primary;
      short secondary;
} palette;


/* External variable definitions */

/*  Palettes exist in a linked list  */

extern palette *pHeadPalette;
extern palette *pCurrentPalette;
extern palette *pOldPalette;

/* atoms used in selection communication with color server */

extern Atom     XA_CUSTOMIZE;
extern Atom     XA_PALETTE_NAME;
extern Atom     XA_TYPE_MONITOR;
extern Atom     XA_UPDATE;

extern Widget   modifyColorButton;
extern int      TypeOfMonitor;
extern Bool     UsePixmaps;
extern int	FgColor;
extern Widget   paletteList;
extern char     *defaultName;
extern Bool     WaitSelection;

extern int NumOfPalettes;
extern XColor saved_color[2];


/* External Interface */

#ifdef _NO_PROTO

extern void Customize() ;
extern void CreatePaletteButtons() ;
extern Boolean InitializePaletteList() ;
extern void DeletePaletteFromLinkList() ;
extern void CopyPixel() ;
extern void SaveOrgPalette() ;
extern void RestoreOrgPalette() ;
extern void UpdateDefaultPalette() ;
extern void show_selection() ;
extern void restoreColor() ;
extern void saveColor() ;
extern void SameName() ;
extern void InitializeAtoms() ;
extern void GetDefaultPal() ;
extern void CreateDialogBoxD() ;
extern void AddToDialogBox() ;
extern void CreateTopColor1() ;
extern void CreateTopColor2() ;
extern void CreateBottomColor() ;

#else

extern void _DtTurnOnHourGlass( Widget );
extern void _DtTurnOffHourGlass( Widget );
extern void Customize( Widget shell) ;
extern void CreatePaletteButtons( Widget parent) ;
extern Boolean InitializePaletteList( 
                        Widget shell,
                        Widget list,
#if NeedWidePrototypes
                        int startup) ;
#else
                        Boolean startup) ;
#endif
extern void DeletePaletteFromLinkList( Widget list) ;
extern void CopyPixel( ColorSet srcPixels[MAX_NUM_COLORS],
                      ColorSet dstPixels[MAX_NUM_COLORS],
		      int numOfColors) ;
extern void SaveOrgPalette( void ) ;
extern void RestoreOrgPalette( void ) ;
extern void UpdateDefaultPalette( void ) ;
extern void show_selection( 
                        Widget w,
                        XtPointer client_data,
                        Atom *selection,
                        Atom *type,
                        XtPointer value,
                        unsigned long *length,
                        int *format) ;
extern void restoreColor( Widget shell, XrmDatabase db) ;
extern void saveColor( int fd) ;
extern void SameName( 
                        Widget w,
                        struct _palette *tmpPalette,
                        char *name) ;
extern void InitializeAtoms( void ) ;
extern void GetDefaultPal( Widget shell) ;
extern void CreateDialogBoxD( Widget parent) ;
extern void AddToDialogBox( void ) ;
extern void CreateTopColor1( void ) ;
extern void CreateTopColor2( void ) ;
extern void CreateBottomColor( void ) ;

#endif /* _NO_PROTO */

#endif /* _colormain_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
