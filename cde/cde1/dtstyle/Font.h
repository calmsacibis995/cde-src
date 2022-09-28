/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        Font.h
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
#ifndef _font_h
#define _font_h

/* typedef statements */

typedef struct {
  XmFontList sysFont;
  XmFontList userFont;
  XmFontList userBoldFont;
  String     sysStr;
  String     userStr;
  String     userBoldStr;
  XmString   pointSize;
} Fontset;

#define XLFD_COUNT      15
#define DT_FONT_GROUP_DIR  "/.dt/sdtfonts/"
#define DT_TMP_DIR "/.dt/tmp/"


typedef struct _fontInfo{
char *alias_name;
char *font_defn;
char *alias_hrn;
char *font_hrn;
char *size;
struct _fontInfo *next;
} fontInfo;
 
typedef struct _FgroupRec {
    char      *fgroup_name;
    char      *family_name;
    char      *foundry;
    char      *weight;
    char      *location;
    struct _FgroupRec * next;
    fontInfo  *mono_fonts; 
    fontInfo  *prop_fonts; 
    fontInfo  *bold_fonts; 
}FgroupRec;

typedef enum {
    MONO_FONT,     /* 0 */
    PROP_FONT,     /* 1 */
    BOLD_FONT      /* 2 */
} list_type;

typedef enum {
    xxs,       /* 0 */
    xs,        /* 1 */
    s,         /* 2 */
    m,         /* 3 */
    l,         /* 4 */
    xl,        /* 5 */
    xxl,       /* 6 */ 
    Scalable,  /* 7 */
    ScaleMax    /* 8 */
} ScaleType;


/* External Interface */

#ifdef _NO_PROTO

extern void popup_fontBB() ;
extern void restoreFonts() ;
extern void saveFonts() ;

#else

extern void popup_fontBB( Widget shell) ;
extern void restoreFonts( Widget shell, XrmDatabase db) ;
extern void saveFonts( int fd) ;

#endif /* _NO_PROTO */

#endif /* _font_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
