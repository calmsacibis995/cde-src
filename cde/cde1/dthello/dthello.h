/* $XConsortium: dthello.h /main/cde1_maint/3 1995/10/09 13:49:10 pascale $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#define  DTHELLO_ID	0
#define  DTCOLOR_ID	1

#define  DTCOLOR_PROGNAME	"dtcolor"
#define  DTHELLO_PROGNAME	"dthello"

#define  DTHELLO_RES_NAME  	"dthello"
#define  DTHELLO_CLASS_NAME  	"Dthello"
#define  DTHELLO_CLASS	        "Dthello"

struct globalStruct {
        char resname [80];
        char classname [80];
};

/*
 * Defintions and Macros
 */
#if 0
#define DEFAULT_FILE	        "/etc/copyright"
#endif

#ifdef __osf__
#define DEFAULT_FONT_LARGE    "-dt-interface system-*-*-*-*-24-*-*-*-*-*-*-*"
#define DEFAULT_FONT_MEDIUM   "-dt-interface system-*-*-*-*-19-*-*-*-*-*-*-*"
#define DEFAULT_FONT_SMALL    "-dt-interface system-*-*-*-*-14-*-*-*-*-*-*-*"
#else
#define DEFAULT_FONT_LARGE	"-dt-interface system-medium-r-normal-xl*-*-*-*-*-*-*-*-*"
#define DEFAULT_FONT_MEDIUM	"-dt-interface system-medium-r-normal-l*-*-*-*-*-*-*-*-*"
#define DEFAULT_FONT_SMALL	"-dt-interface system-medium-r-normal-m*-*-*-*-*-*-*-*-*"
#endif
 
#define FIXED_FONT	"fixed"
#define DEFAULT_BG	"white"
#define DEFAULT_LOW_BG	"white"
#define DEFAULT_FG	"dim gray"
#define DEFAULT_LOW_FG	"black"
#define DEFAULT_TIME	"240"

#define DEFAULT_XOFFSET_SMALL	 25
#define DEFAULT_XOFFSET_MEDIUM	 50
#define DEFAULT_XOFFSET_LARGE	 125

#define BOX_LINE_WIDTH_SMALL	 4
#define BOX_LINE_WIDTH_MEDIUM	 (BOX_LINE_WIDTH_SMALL + 2)
#define BOX_LINE_WIDTH_LARGE	 (BOX_LINE_WIDTH_SMALL * 2)

#define MAX_FILES	5
#define MAX_LINES	100
#define MAX_COLUMNS	160


/*
 * My resources
 */
#define vNfont		"vfont"
#define vCFont		"Vfont"
#define vNxoffset	"xoffset"
#define vCXoffset	"Xoffset"
#define vNbackground	"vbackground"
#define vCBackground	"Vbackground"
#define vNforeground	"vforeground"
#define vCForeground	"Vforeground"
#define vNfile		"file"
#define vCFile		"File"
#define vNstring	"string"
#define vCString	"String"
#define vNtimeout	"timeout"
#define vCTimeout	"Timeout"

#ifndef True
#define True 1
#endif
#ifndef False
#define False 0
#endif


/* 
 * Data definitions
 */
typedef struct 
{
    char *name;
    int  len;
} ArgSpec;

typedef struct
{
    char	*resname;
    char	*resclass;
    char	**ppvalue;
    int		*size;

} VhResourceEntry;


/********    Public Function Declarations    ********/
#ifdef _NO_PROTO

extern int main() ;
extern int ArgMatch() ;
extern char * GetMessage( );
extern unsigned char * SkipWhitespace() ;
extern void KillNewlines() ;
extern void ReadInTextLines() ;
extern void SeparateTextLines() ;
extern void CatchAlarm() ;
extern Cursor GetHourGlass() ;
extern void VhGetResources() ;
extern void PaintText() ;
extern void DrawBox() ;

#else

extern int main( 
                        int argc,
                        char **argv) ;
extern int ArgMatch( 
                        char *pch,
                        int arn) ;
extern char * GetMessage( int set, int n, char *s);
extern unsigned char * SkipWhitespace( 
                        unsigned char *pch) ;
extern void KillNewlines( 
                        unsigned char *pch) ;
extern void ReadInTextLines( 
                        FILE *fp,
                        XFontSet fontset,
                        unsigned int *pMaxWidth) ;
extern void SeparateTextLines( 
                        unsigned char *pchIn) ;
extern void CatchAlarm( 
                        int sig) ;
extern Cursor GetHourGlass( 
                        Display *dpy) ;
extern void VhGetResources( 
                        Display *dpy,
                        char *name,
                        char *class,
                        VhResourceEntry *res,
                        int num) ;

extern void PaintText( void ) ;

extern void DrawBox( void ) ;
#endif /* _NO_PROTO */
