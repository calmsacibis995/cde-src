/* $XConsortium: ps_graphics.c /main/cde1_maint/6 1995/11/14 16:59:38 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)ps_graphics.c	1.67 97/06/09 Sun Microsystems, Inc."

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <ctype.h>
#include <locale.h>
#include <rpc/rpc.h>
#ifdef SVR4
#include <unistd.h>
#endif /* SVR4 */
#include <sys/stat.h>
#include <sys/param.h>
#include <time.h>
#include "util.h"
#include "calendar.h"
#include "timeops.h"
#include "datefield.h"
#include "props.h"
#include "props_pu.h"
#include "ps_graphics.h"
#include "todo.h"
#include "cm_i18n.h"
#include "dayglance.h"
#include "format.h"
#include "deskset.h"
#include "print.h"

#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include <Dt/SpinBox.h>

#define BUFFERSIZE 512
#define MAX_TODO 36
#define MAX_TODO_LP 29

#define PROLOG_LOC_1	"/etc/dt/config/psfonts/%s/prolog.ps"
#define PROLOG_LOC_2	"/usr/dt/config/psfonts/%s/prolog.ps"

int num_hours = 0;
int prolog_found = 0;
extern int debug;
static char ps_file[128];		/* name of output postscript file */
static char sched_bucket[96];		/* use to draw sched blocks */
static char timestamp[BUFFERSIZE];		/* use to print timestamp on printouts */
static void ps_week_sched_update();
static int nonascii_char = 0;
static int already_printed_warning = 0;
static void nonascii_string();

extern void print_std_month();
extern void get_time_str();
/* forward declaration */
char *fix_ps_string();

extern FILE *
ps_open_file (Calendar *c, Print_default print_default)
{
	FILE 		*fp;
	struct stat 	fstatus;
	Props 		*p = (Props *) calendar->properties;
	Tick 		t;
	struct tm 	*tm;
	_DtCmPrintData 	*pd = (_DtCmPrintData *) c->print_data;
	Props_pu        *pu = (Props_pu *) c->properties_pu;
	char            msgbuf[BUFFERSIZE];

	if ((print_default == print_from_dialog) &&
	    (XmToggleButtonGadgetGetState(pd->file_toggle))) {

		char *filename = XmTextGetString(pd->file_text);

		if (*filename == NULL) {
			char *title = XtNewString(catgets(c->DT_catd, 1, 736, 
					      "Calendar : Error - Print"));
			char *text = XtNewString(catgets(c->DT_catd, 1, 774, 
			 "ps_open_file(): print to file requires file name\n"));
			char *ident1 = XtNewString(
					catgets(c->DT_catd, 1, 95, "Continue"));
			dialog_popup(c->frame,
					DIALOG_TITLE, title, 
					DIALOG_TEXT, text,
					BUTTON_IDENT, 1, ident1,
					DIALOG_IMAGE, pu->xm_error_pixmap,
					NULL);
			XtFree(ident1);
			XtFree(text);
			XtFree(title);
			return(NULL);
		}

		sprintf(ps_file,"%s",filename);

	}
	else if (get_int_prop(p, CP_PRINTDEST) == 1) 
	{
		/* redirect output to a file */
		char *dir  = get_char_prop(p, CP_PRINTDIRNAME);
		char *file = get_char_prop(p, CP_PRINTFILENAME);

		if (*dir == NULL || *file == NULL) {
			char *title = XtNewString(catgets(c->DT_catd, 1, 736, 
                                              "Calendar : Error - Print"));
			char *text = XtNewString(catgets(c->DT_catd, 1, 464, 
			      "ps_open_file(): need to specify dir or file\n"));
			char *ident1 = XtNewString(
					catgets(c->DT_catd, 1, 95, "Continue"));
			dialog_popup(c->frame,
                        		DIALOG_TITLE, title, 
                              		DIALOG_TEXT, text,
                              		BUTTON_IDENT, 1, ident1,
                              		DIALOG_IMAGE, pu->xm_error_pixmap,
                              		NULL);
			XtFree(ident1);
			XtFree(text);
			XtFree(title);
			return(NULL);
		}

		stat(dir, &fstatus);
		if ((fstatus.st_mode & S_IFMT) != S_IFDIR) {
			char *title = XtNewString(catgets(c->DT_catd, 1, 736, 
                                              "Calendar : Error - Print"));
			char *text = XtNewString(catgets(c->DT_catd, 1, 465, 
				     "ps_open_file(): Invalid Directory %s\n"));
			char *ident1 = XtNewString(
					catgets(c->DT_catd, 1, 95, "Continue"));
			dialog_popup(c->frame,
                              		DIALOG_TITLE, title, 
                              		DIALOG_TEXT, text,
                              		BUTTON_IDENT, 1, ident1,
                              		DIALOG_IMAGE, pu->xm_error_pixmap,
                              		NULL);
			XtFree(ident1);
			XtFree(text);
			XtFree(title);
			return(NULL);
		}

		sprintf(ps_file,"%s/%s",dir,file);
	}
	else  {
		/* get a temporary filename to use */
		(void) cm_strcpy(ps_file, "/tmp/CMpXXXXXX");
		(void) mktemp(ps_file);
	}
		
	if ((fp = fopen(ps_file, catgets(c->DT_catd, 1, 466, "w+"))) == NULL) {
		char *title = XtNewString(catgets(c->DT_catd, 1, 736, 
                                              "Calendar : Error - Print"));
		char *ident1 = XtNewString(
					catgets(c->DT_catd, 1, 95, "Continue"));
		sprintf (msgbuf, catgets(c->DT_catd, 1, 467, 
			  "ps_open_file(): unable to open file %s\n"), ps_file);
		dialog_popup(c->frame,
                      		DIALOG_TITLE, title, 
                      		DIALOG_TEXT, msgbuf, 
                      		BUTTON_IDENT, 1, ident1,
                      		DIALOG_IMAGE, pu->xm_error_pixmap,
                      		NULL);
		XtFree(ident1);
		XtFree(title);
		return(NULL);
	}

	/* set up time stamp */
	/* STRING_EXTRACTION SUNW_DESKSET_CM_MSG
	 * The following string is the date/time format used in printing out
	 * calendar views.  This is the locale's date and time.  If this string
	 * is to be translated, please refer to the man pages for strftime()
	 * for various format strings.
	 */
	t = now();
	tm = localtime(&t);
	strftime(timestamp, BUFFERSIZE, "%c", tm);

	nonascii_string(timestamp);

	return(fp);
}

extern void
ps_translate(FILE *fp,
    int x, 
    int y)	/* where to move the origin of the coord system */
{
    fprintf (fp, "%d %d translate \t\t%% move coord system \n", x, y);
}

extern void 
ps_scale (FILE *fp,
    float x, 
    float y)  /* x & y dimension scaling factors */ 
{
    fputs ((char *)cm_printf((double)x, 2), fp); fputs (" ", fp);
    fputs ((char *)cm_printf((double)y, 2), fp);
    fputs (" scale \t\t%% move coord system\n", fp);
}

extern void 
ps_rotate (FILE *fp,
    int  angle)  /* rotate the coord axes angle degrees counter-clockwise */ 
{
    fprintf (fp, "%d rotate \t\t%% rotate coord system\n", angle);
}
 
extern void 
ps_landscape_mode (FILE *fp) 	/* set to print in landscape mode */
{
	/* set to landscape by rotating 90 degrees and moving the   */
	/* origin back the width of the paper                       */
	fprintf (fp, "\n\n%% --- set to landscape mode ---\n");
	fprintf (fp, "\ngsave\t\t%% save initial graphics state\n");
	ps_rotate(fp, 90);
	ps_translate(fp, 0, -612);	/* 8.5" * 72 pts per inch */
}

static void
local_dayname(char ** array_place, char *source) {

	char *str;

	nonascii_string(source);
	str = euc_to_octal(source);
	*array_place = (char *)malloc(sizeof(char) * cm_strlen(str) + 1);
	cm_strcpy(*array_place, str);
}

extern void
ps_minimonthdefs( fp )
        FILE *fp;
{
	char *day_of_week[7];
	int k;
	Calendar *c = calendar;
	int malloc_memory = 0;

	fprintf (fp, "\n%% --- init vars for std month ---\n");
	fprintf (fp, "\n/initstdmonthcontext { %% height width => - \n");
	fprintf (fp, "/old_TM TM def\n");
	fprintf (fp, "/old_LM LM def\n");
	fprintf (fp, "/old_RM RM def\n");
	fprintf (fp, "/old_BM BM def\n");
	fprintf (fp, "/old_x1 x1 def\n");
	fprintf (fp, "/old_y1 y1 def\n\n");

	fprintf (fp, "/TM 5 def\n");
	fprintf (fp, "/LM 5 def\n");
	fprintf (fp, "/BM exch def\n");
	fprintf (fp, "/RM exch def\n");
	fprintf (fp, "/Datewidth RM LM sub 7 div def\n");
	fprintf (fp, "/Monwidth  RM LM sub def\n");
	fprintf (fp, "/x1 LM def\n");
	fprintf (fp, "/y1 TM def\n");
	fprintf (fp, "} def\n");

	fprintf (fp, "\n/finishstdmonthcontext { %% - => - \n");
	fprintf (fp, "\n%% --- finish std month - restore old vars  ---\n");
	fprintf (fp, "/TM old_TM def\n");
	fprintf (fp, "/LM old_LM def\n");
	fprintf (fp, "/RM old_RM def\n");
	fprintf (fp, "/BM old_BM def\n");
	fprintf (fp, "/x1 old_x1 def\n");
	fprintf (fp, "/y1 old_y1 def\n");
	fprintf (fp, "} def\n");

	fprintf (fp, "/printstdmonthdates { %% monthlen, firstday => -\n");
	fprintf (fp, "/monthlen exch def /firstday exch def\n");
	fprintf (fp, "\n%% --- print std month dates ---\n");
	fprintf (fp, "DateFont DateFont.scale font_set\n"); 
	fprintf (fp, "/dow firstday def\n"); 
	fprintf (fp, "/date 1 def\n"); 
	fprintf (fp, "/numstr 2 string def\n"); 

	/* be sure to incr x offset by dow */
	fprintf (fp, "/x1 Datewidth dow mul x1 add def\n");

	fprintf (fp, "monthlen { \n");
	fprintf (fp, "    x1 y1 date numstr cvs right_text\n");
	fprintf (fp, "    dow 7 eq\n");
	fprintf (fp, "    { /dow 1 def CR }\t%% start on next line\n");
	fprintf (fp, "    { /dow dow 1 add def } ifelse \n");
	fprintf (fp, "    /x1 x1 Datewidth add def\n");
	fprintf (fp, "    /date date 1 add def\n"); 
	fprintf (fp, "} repeat\n");
	fprintf (fp, "} def\n\n");

	local_dayname(&day_of_week[0], catgets(c->DT_catd, 1, 497, "S"));
	local_dayname(&day_of_week[1], catgets(c->DT_catd, 1, 491, "M"));
	local_dayname(&day_of_week[2], catgets(c->DT_catd, 1, 492, "T"));
	local_dayname(&day_of_week[3], catgets(c->DT_catd, 1, 493, "W"));
	local_dayname(&day_of_week[4], catgets(c->DT_catd, 1, 494, "T"));
	local_dayname(&day_of_week[5], catgets(c->DT_catd, 1, 495, "F"));
	local_dayname(&day_of_week[6], catgets(c->DT_catd, 1, 496, "S"));

	malloc_memory = 1;

	fprintf (fp, "\n%% --- print std month weekdays ---\n");
	fprintf (fp, "/printstdmonthdays { %% - => -\n");
	fprintf (fp, "DayFont DayFont.scale font_set\n"); 
	fprintf (fp, "CR /t%% do a carriage return with new font size\n");
	fprintf (fp, "[(%s) (%s) (%s) (%s) (%s) (%s) (%s)] {\n", day_of_week[0],
			 day_of_week[1], day_of_week[2], day_of_week[3], day_of_week[4],
			 day_of_week[5], day_of_week[6]); 
	fprintf (fp, "    /x1 x1 Datewidth add def\n");
	fprintf (fp, "    x1 y1 3 -1 roll right_text\n");
	fprintf (fp, "} forall CR\n"); 
	fprintf (fp, "} def\n"); 

	if ( malloc_memory ) {
		for ( k = 0;  k < 7;  k++ ) {
			free((char *)day_of_week[k]);
		}
	}

	fprintf (fp, "/printstdmonthname { %% monthname => -\n");
	fprintf (fp, "/monthname exch def\n");
	fprintf (fp, "\n%% --- print month name centered at top ---\n");
	fprintf (fp, "MonthFont MonthFont.scale font_set\n"); 
	fprintf (fp, "/dx font.x 0.3 mul def\n");
	fprintf (fp, "/dy font.y 0.3 mul def\n");
	fprintf (fp, "x1 dx add y1 dy sub Monwidth font.y dy add make_lt_gray\n");
	fprintf (fp, "LM y1 RM monthname ctr_text \n"); 
	fprintf (fp, "} def\n"); 
}

extern void
ps_init_list (FILE *fp)
{    
	fprintf (fp, "\n%% --- init for todo print ---\n");

	fprintf (fp, "/boxw RM LM sub 2 div def\n");
	fprintf (fp, "/boxh LM def\n");
	fprintf (fp, "/x1 LM def\n");
	fprintf (fp, "/y1 TM def\n");
	fprintf (fp, "/x2 x1 boxw add def\n");
	fprintf (fp, "/y2 TM def\n");
	fprintf (fp, "/line1 0 def\n");
	fprintf (fp, "/line2 0 def\n");
	fprintf (fp, "/tab0 0 def\n");
	fprintf (fp, "/tab1 0 def\n");
	fprintf (fp, "/tab2 0 def\n");
	fprintf (fp, "/tab3 0 def\n");

	ps_set_fontsize (fp, "TimeFont",  10);
	ps_set_fontsize (fp, "ApptFont",  10);

	ps_minimonthdefs(fp);

}
 
extern void
ps_init_day (FILE *fp)
{    
	fprintf (fp, "\n%% --- init for day print ---\n");


	fprintf (fp, "/boxw RM LM sub 2 div def\n");
	fprintf (fp, "/boxh LM def\n");
	fprintf (fp, "/x1 LM def\n");
	fprintf (fp, "/y1 TM def\n");
	fprintf (fp, "/x2 x1 boxw add def\n");
	fprintf (fp, "/y2 TM def\n");
	fprintf (fp, "/line1 0 def\n");
	fprintf (fp, "/line2 0 def\n");
	fprintf (fp, "/tab1 0 def\n");
	fprintf (fp, "/tab2 0 def\n");

	ps_set_fontsize (fp, "TimeFont",  10);
	ps_set_fontsize (fp, "ApptFont",  10);

        fprintf (fp, "/printdaytime { %% string => \n");
	fprintf (fp, "/daytimestr exch def\n");
        fprintf (fp, "line1 2 sub y1 gt {\n");
        /* time and text must be same font for multi-pages */
        fprintf (fp, "    ApptFont ApptFont.scale font_set\n");
        fprintf (fp, "    /tab2 tab1 font.x 2 mul add def\n");
        fprintf (fp, "    x1 tab1 add 3 add line1 daytimestr left_text\n");
        fprintf (fp, "    /line1 line1 font.y sub 1 sub def\n");
        fprintf (fp, "} if\n\n");
	fprintf (fp, "} def\n");



	/* set up macro code for the printing of month text */

	fprintf (fp, "/printdaytext { %% string indented => \n");
	fprintf (fp, "/daytextstring exch def /dayindent exch def\n");
        fprintf (fp, "line1 2 sub y1 gt {\n");
        fprintf (fp, "    ApptFont ApptFont.scale font_set\n");
        fprintf (fp, "    gsave\n");
        fprintf (fp, "    x1 y1 boxw font.x 2 div sub boxh rect clip newpath\n");

	fprintf (fp, "dayindent 1 eq { x1 tab2 add line1 daytextstring left_text} { x1 tab1 add 3 add line1 daytextstring left_text} ifelse\n");

        fprintf (fp, "    grestore\n");
        fprintf (fp, "    /line1 line1 font.y sub 1 sub def\n");
        fprintf (fp, "} if\n");


	fprintf (fp, "} def\n");

	fprintf (fp, "/daytimeslot { %% hourstring modstring hournum more => -\n");
	fprintf (fp, "/hourstring exch def /modstring exch def /hournum exch def /more exch def\n");
       /* reset to next col */
	fprintf (fp, "hournum 12 eq {/x1 LM boxw add def  /y1 y2 def } if \n");

    	fprintf (fp, "/y1 y1 boxh sub def\n");

    	fprintf (fp, "x1 y1 boxw boxh line_width draw_box\n");
    	fprintf (fp, "x1 y1 tab1 boxh make_lt_gray\n");
    	fprintf (fp, "x1 y1 tab1 boxh line_width draw_box\n");

    	fprintf (fp, "TimeFont TimeFont.scale font_set\n");
    	fprintf (fp, "x1 y1 boxh 2 center_y x1 tab1 add hourstring ctr_text\n");

    	fprintf (fp, 
        "x1 y1 boxh 2 center_y font.y sub x1 tab1 add modstring ctr_text\n");
	fprintf(fp, "more 1 eq {-9 -12 rmoveto (%s) show } if\n", "*");
    	fprintf (fp, "/line1 y1 boxh add font.y sub 1 sub def\n");
	fprintf (fp, "} def\n");
}


extern void
ps_init_month (FILE *fp)
{    
	fprintf (fp, "\n%% --- init for month view print ---\n");

	fprintf (fp, "/boxw RM LM sub 2 div def\n");
	fprintf (fp, "/boxh LM def\n");
	fprintf (fp, "/x1 LM def\n");
	fprintf (fp, "/y1 TM def\n");
	fprintf (fp, "/x2 x1 boxw add def\n");
	fprintf (fp, "/y2 TM def\n");
	fprintf (fp, "/line1 0 def\n");
	fprintf (fp, "/line2 0 def\n");
	fprintf (fp, "/tab1 0 def\n");
	fprintf (fp, "/tab2 0 def\n");

	if (prolog_found) {
		ps_set_font(fp, "TimeFont", "LC_Helvetica");
		ps_set_font(fp, "DateFont", "LC_Times-Bold");
		ps_set_font(fp, "DayFont",  "LC_Times-Bold");
	}
	else {
		fprintf (fp, "/ISO_fonts {\n");
		ps_set_font(fp, "TimeFont", "Helvetica-ISOLatin1");
		ps_set_font(fp, "DateFont", "Times-Bold-ISOLatin1");
		ps_set_font(fp, "DayFont",  "Times-Bold-ISOLatin1");
		fprintf (fp, "} def\n");
		fprintf (fp, "/Reg_fonts {\n");
		ps_set_font(fp, "TimeFont", "Helvetica");
		ps_set_font(fp, "DateFont", "Times-Bold");
		ps_set_font(fp, "DayFont",  "Times-Bold");
		fprintf (fp, "} def\n");
		fprintf (fp, "%%\n");
		fprintf (fp, "systemdict /ISOLatin1Encoding known\n");
		fprintf (fp, "{ISO_fonts} {Reg_fonts} ifelse\n");
		fprintf (fp, "%%\n");
	}

	ps_set_fontsize (fp, "MonthFont", 30);
	ps_set_fontsize (fp, "DateFont",  20);
	ps_set_fontsize (fp, "DayFont",   14);
	ps_set_fontsize (fp, "TimeFont",   8);
	ps_set_fontsize (fp, "ApptFont",   8);

	/* set up macro code for the printing of the month boxes */

	fprintf (fp, "\n%% --- print daily boxes for appt entries ---\n");

	fprintf (fp, "/printmonthday { %% datestring date dayofweek more => \n");
	fprintf (fp, "/datestring exch def /monthdate exch def /dayofweek exch def /more exch def\n");
 
	/* if weekday==0 && not 1st day of month then start on next line */

	fprintf (fp, "dayofweek 0 eq monthdate 1 ne and { /y1 y1 boxh sub def } if\n");
	fprintf (fp, "/x1 boxw dayofweek mul LM add def\n");
	fprintf (fp, "x1 y1 boxw boxh clear_box\n");
	fprintf (fp, "x1 y1 boxw boxh line_width draw_box\n");
 
	fprintf (fp, "DateFont DateFont.scale font_set\n");
	fprintf (fp, "/tab1 3 def\n");
	fprintf (fp, "/line1 y1 boxh add font.y sub tab1 sub def\n");
	fprintf (fp, "x1 tab1 add line1 datestring left_text\n");
	fprintf (fp, "more 1 eq { 6 0 rmoveto (%s) show } if\n", "*");
	fprintf (fp, "/line1 line1 tab1 sub def\n");
 
	fprintf (fp, "TimeFont TimeFont.scale font_set\n");
	fprintf (fp, "/line1 line1 font.y sub 1 sub def\n");
	fprintf (fp, "/tab2 font.x 3 mul tab1 add def\n");
	fprintf (fp, "} def\n");

        fprintf (fp, "/printmonthtime { %% string => \n");
	fprintf (fp, "/monthtimestr exch def\n");
        fprintf (fp, "line1 2 sub y1 gt {\n");
        /* time and text must be same font for multi-pages */
        fprintf (fp, "    ApptFont ApptFont.scale font_set\n");
        fprintf (fp, "    /tab2 font.x 3 mul tab1 add def\n");
        fprintf (fp, "    x1 tab2 add line1 monthtimestr right_text\n");
        fprintf (fp, "} if\n\n");
	fprintf (fp, "} def\n");

	/* set up macro code for the printing of month text */

	fprintf (fp, "/printmonthtext { %% string indented => \n");
	fprintf (fp, "/monthtextstring exch def /monthindent exch def\n");
        fprintf (fp, "line1 2 sub y1 gt {\n");
        fprintf (fp, "    ApptFont ApptFont.scale font_set\n");
        fprintf (fp, "    gsave\n");
        fprintf (fp, "    x1 y1 boxw font.x 2 div sub boxh rect clip newpath\n")
;
	fprintf (fp, "monthindent 1 eq { x1 tab2 add 1 add line1 monthtextstring left_text } { x1 font.x add line1 monthtextstring left_text } ifelse\n");

        fprintf (fp, "    grestore\n");
        fprintf (fp, "    /line1 line1 font.y sub 1 sub def\n");
        fprintf (fp, "} if\n");


	fprintf (fp, "} def\n");

	ps_minimonthdefs( fp );

}


extern void
ps_finish_printer (FILE *fp)
{
	char *str;

	fprintf (fp, "\n\n%% --- final cmds to printer ---\n");
	fprintf (fp, "showpage\n");

	if ( !prolog_found ) {
		if ( nonascii_char && !already_printed_warning ) {
			already_printed_warning = 1;
			fprintf (fp, "%%\n");
			fprintf (fp, "90 rotate\n");
			fprintf (fp, "0 -612 translate\n");
			fprintf (fp, "/Helvetica-Bold findfont 18 scalefont setfont\n");
			fprintf (fp, "systemdict /ISOLatin1Encoding known\n");
			fprintf (fp, "{ }\n");
			str = euc_to_octal("Calendar Manager warning:");
			fprintf (fp, "{ 72 470 moveto (%s) show\n", str);
			str = euc_to_octal("Your calendar contains characters that");
			fprintf (fp, "  72 435 moveto (%s) show\n", str);
			str = euc_to_octal("your printer does not support.");
			fprintf (fp, "  72 410 moveto (%s) show\n", str);
			str = euc_to_octal("This will result in erroneous characters being printed.");
			fprintf (fp, "  72 385 moveto (%s) show showpage }\n", str);
			fprintf (fp, "ifelse\n");
			fprintf (fp, "%%\n");
		}
	}

	fprintf (fp, "grestore\t\t%% restore initial graphics state\n"); 
}
extern void
ps_print_file (Calendar *c, Print_default print_default)
{
	Props *p = (Props *) calendar->properties;
	char command[BUFSIZ];
	char printer[81];
	char *printer_name, *options, *def_pr=NULL;
	int copies;
	Boolean file_only = False;
	_DtCmPrintData *pd = (_DtCmPrintData *)c->print_data;
#ifdef __osf__
	/* Fix for QAR 27926 */
	FILE            *errfp;
	int             errtmpfd;
	char            tmp_filename[128], errbuf[BUFFERSIZE];
	Props_pu        *pu = (Props_pu *) c->properties_pu;
	int             error_state = FALSE;
#endif


	nonascii_char = 0;
	already_printed_warning = 0;

	/*
	 * check to see if print to printer (prop value of 0) or file
	 * (prop value of 1).  Use state of file_toggle if a dialog operation.
	 */
	if (print_default == print_from_dialog)
		XtVaGetValues(pd->file_toggle, XmNset, &file_only, NULL);
	else
		file_only = (get_int_prop(p, CP_PRINTDEST) == 1);

	if (!file_only)		/* file printing case handled elsewhere */
	{
		if (print_default == print_from_default) {
			/* not printing from dialog */
			if (c->app_data->default_printer)
                		printer_name = c->app_data->default_printer;
        		else {
                		printer_name = get_char_prop(
							(Props *)c->properties, 
                                                        CP_PRINTERNAME);
			}
			copies = atoi(get_char_prop(p, CP_PRINTCOPIES));
			options = get_char_prop(p, CP_PRINTOPTIONS);
		}
		else {
			/* printing from dialog: override defaults */

			printer_name = XmTextGetString(pd->printer_text);
			XtVaGetValues(pd->copies_spin,
				DtNposition, &copies, NULL);
			options = XmTextGetString(pd->args_text);
		}

		if ((int)strlen(printer_name) < 1) {
			def_pr = (char*)cm_def_printer();
#if defined(SVR4) && !defined(__osf__)
			sprintf(printer,"-d%s", def_pr);
#else
			sprintf(printer,"-P%s", def_pr);
#endif
		}
		else
#if defined(SVR4) && !defined(__osf__)
			sprintf(printer,"-d%s", printer_name);
#else
			sprintf(printer,"-P%s", printer_name);
#endif

#ifdef SunOS
		sprintf(command,
			"lp -s -Tpostscript -c %s -n%d %s %s",
			printer, copies, options, ps_file);
#elif __osf__
		/* Fix for QAR 27926 */
		/* get a temporary filename to use for lpr output */
		(void) cm_strcpy(tmp_filename, "/tmp/CMlXXXXXX");
		(void) mktemp(tmp_filename);

		sprintf(command, "lpr -r %s -#%d %s %s 2>%s",
                        printer, copies, options, ps_file, tmp_filename);
#else
		sprintf(command,
			"lp -s -c %s -n%d %s %s",
			printer, copies, options, ps_file);
#endif

		/* Issue the command */
#ifdef __osf__
 
		/* If lpr error, read error file and pop up error dialog */
		if (system(command))  {
		       char *title = XtNewString(catgets(c->DT_catd, 1, 736, 
                                       "Calendar : Error - Print"));
		       char *ident1 = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));

                       errfp = fopen(tmp_filename, "r");
                       fgets (errbuf, BUFFERSIZE, errfp);
                       fclose (errfp);
 
                       /* errbuf has the I18N lpr output which describes the */
                       /* problem (i.e. unknown printer, bad options, etc. */
                       dialog_popup(c->frame,
                               DIALOG_TITLE, title, 
                               DIALOG_TEXT, errbuf,
                               BUTTON_IDENT, 1, ident1,
                               DIALOG_IMAGE, pu->xm_error_pixmap,
                               NULL);
		       XtFree(ident1);
		       XtFree(title);
                       }
		unlink (tmp_filename);
#else
		system(command);
#endif

		if (def_pr != NULL)
			free(def_pr);
#ifdef SVR4
		unlink(ps_file);
#endif
	}
}

extern void
ps_print_header (FILE *fp, char *buf) {
	char *str;

	fprintf (fp, "\n%% --- print header info at top ---\n");

	fprintf (fp, "StampFont StampFont.scale font_set\n"); 

	/* print timestamp at left margin */ 
	fprintf (fp, "LM y1 font.y sub (%s) left_text\n", timestamp); 

	/* print user@host stamp at right margin */ 
	fprintf (fp, "RM y1 font.y sub (%s) right_text\n", calendar->view->current_calendar); 

	fprintf (fp, "MonthFont MonthFont.scale font_set\n"); 

	nonascii_string(buf);
	str = euc_to_octal(buf);
	fprintf (fp, "LM y1 RM (%s) ctr_text \n", str); 

#ifdef NEVER
	/* print sun logo at left margin */ 
	fprintf (fp, "LM 20 add y1 4 Sunlogo\n"); 
#endif

}

extern void
ps_day_header (FILE *fp, int timeslots, int num_page, int total_pages)
{
	char *str, str2[BUFFERSIZE];
	Calendar *c = calendar;

	if (total_pages == 0)
		total_pages++;

	fprintf (fp, "CR CR \n");

	/* print creation notice at bottom */ 
	fprintf (fp, "StampFont StampFont.scale font_set\n"); 

	str = "of";
	nonascii_string(str);
        cm_strcpy(str2, catgets(c->DT_catd, 1, 468, "Page"));
        cm_strcpy(str2, euc_to_octal(str2));
     	fprintf (fp, "LM BM font.y sub (%s %d %s %d) left_text \n",
                        str2, num_page, str, total_pages);

	str = catgets(c->DT_catd, 1, 469, "Day view by Calendar Manager");
	nonascii_string(str);
	str = euc_to_octal(str);
	fprintf (fp, "RM BM font.y sub (%s) right_text \n", str);

	fprintf (fp, "\n%% --- print Morning/Afternoon boxes ---\n");
	fprintf (fp, "DayFont DayFont.scale font_set\n");
	fprintf (fp, "/tab1 font.y 2 mul def\n");
	fprintf (fp, "/boxh font.x 2 mul def\n");
	fprintf (fp, "/y2 y1 def\n");

	fprintf (fp, "x1 y1 boxw boxh make_lt_gray\n");
	fprintf (fp, "x2 y2 boxw boxh make_lt_gray\n");

	fprintf (fp, "x1 y1 tab1 boxh make_gray\n");
	fprintf (fp, "x2 y2 tab1 boxh make_gray\n");

	fprintf (fp, "x1 y1 boxw boxh line_width draw_box\n");
	fprintf (fp, "x2 y2 boxw boxh line_width draw_box\n");

	fprintf (fp, "x1 y1 tab1 boxh line_width draw_box\n");
	fprintf (fp, "x2 y2 tab1 boxh line_width draw_box\n");

	str = catgets(c->DT_catd, 1, 470, "Morning");
	nonascii_string(str);
	fprintf (fp, "x1 tab1 add y1 boxh 1 center_y x2 (%s)   ctr_text\n", str);
	str = catgets(c->DT_catd, 1, 471, "Afternoon");
	nonascii_string(str);
	str = euc_to_octal(str);
	fprintf (fp, "x2 tab1 add y2 boxh 1 center_y RM (%s) ctr_text\n", str);

	fprintf (fp, "/boxh y1 BM sub %d div def\n", timeslots);
	fprintf (fp, "/y2 y1 def\n");	/* save pos for 2nd column */
}

extern void
ps_day_timeslots(FILE *fp, int i, Boolean more)
{
    	char hourbuf[6];
    	char modbuf[6];
	Calendar *c = calendar;
    	
	sprintf (hourbuf, "%d", morning(i) ? i : i==12 ? i : (i-12) );
    	sprintf (modbuf, "%s", morning(i) ? catgets(c->DT_catd, 1, 4, "am") : catgets(c->DT_catd, 1, 3, "pm"));   

	fprintf (fp, "\n%% --- print hourly boxes for appt entries ---\n");


	fprintf (fp, "%d %d ( %s ) ( %s ) daytimeslot\n", more, i, modbuf, hourbuf);
}

extern void 
ps_todo_outline(FILE *fp, CSA_sint32 appt_type) {
	int curr_page = 1, num_pages = 1;
	char *str;
	char str2[BUFFERSIZE];
	Calendar *c = calendar;

	fprintf (fp, "StampFont StampFont.scale font_set\n");
	if (appt_type == CSA_TYPE_TODO)
		str = catgets(c->DT_catd, 1, 473, "To Do List by Calendar Manager");
	else
		str = catgets(c->DT_catd, 1, 474, "Appt List by Calendar Manager");
	nonascii_string(str);
	str = euc_to_octal(str);
	fprintf (fp, "RM BM font.y sub (%s) right_text \n", str);
	cm_strcpy(str2, catgets(c->DT_catd, 1, 475, "Page"));
	str = catgets(c->DT_catd, 1, 476, "of");
	cm_strcpy(str2, euc_to_octal(str2));
	fprintf (fp, "LM BM font.y sub (%s %d %s %d) left_text \n",
			str2, curr_page, str, num_pages);
	curr_page++;
	fprintf (fp, "/line_width 2 def\n");
	fprintf (fp, "/x1 LM def\n");
	fprintf (fp, "/y1 BM def\n");
	fprintf (fp, "/boxw RM LM sub def\n");
	fprintf (fp, "/boxh TM BM sub 15 sub def\n");

	fprintf (fp, "x1 y1 boxw boxh make_lt_gray\n");

	fprintf (fp, "x1 y1 boxw 10 make_gray\n");
	fprintf (fp, "x1 y1 10 boxh make_gray\n");
	fprintf (fp, "x1 y1 boxh add 10 sub boxw 10 make_gray\n");
	fprintf (fp, "x1 boxw add 10 sub y1 10 boxh make_gray\n");

	fprintf (fp, "x1 y1 boxw boxh line_width draw_box\n");
	fprintf (fp, "x1 10 add y1 10 add boxw 20 sub boxh 20 sub line_width draw_box\n");
	/* init for drawing appts */
	fprintf (fp, "/line1 TM 50 sub def\n");
	fprintf (fp, "/tab0 25 def\n");
	fprintf (fp, "/tab1 50 def\n");
	fprintf (fp, "/tab2 70 def\n");
	fprintf (fp, "/tab3 84 def\n");
}

extern void 
ps_month_daynames(FILE *fp, int rows, int num_page, int total_pages)
{ 
	char *str, str2[BUFFERSIZE];
	char *day_of_week[7];
	int k;
	Calendar *c = calendar;

	if (total_pages == 0)
		total_pages++;

	/* print creation notice at bottom */ 

	fprintf (fp, "StampFont StampFont.scale font_set\n"); 
	str = catgets(c->DT_catd, 1, 477, "Month view by Calendar Manager");
	nonascii_string(str);
	str = euc_to_octal(str);
	fprintf (fp, "RM BM font.y sub (%s) right_text \n", str); 
	fprintf (fp, "StampFont StampFont.scale font_set\n"); 
	str = catgets(c->DT_catd, 1, 476, "of");
	nonascii_string(str);
        cm_strcpy(str2, catgets(c->DT_catd, 1, 479, "Page"));
        cm_strcpy(str2, euc_to_octal(str2));
     	fprintf (fp, "LM BM font.y sub (%s %d %s %d) left_text \n",
                        str2, num_page, str, total_pages);

	fprintf (fp, "\n%% --- print days of the week with boxes ---\n"); 
	fprintf (fp, "DayFont DayFont.scale font_set\n");
	fprintf (fp, "/boxh font.x 2 mul def\n"); 
	fprintf (fp, "/boxw RM LM sub 7 div def\n");
	fprintf (fp, "/x1 LM def\n"); 
	fprintf (fp, "/y1 y1 boxh sub 15 sub def\n");

	day_of_week[0] = catgets(c->DT_catd, 1, 480, "Sun");
	nonascii_string(day_of_week[0]);
	day_of_week[1] = catgets(c->DT_catd, 1, 481, "Mon");
	nonascii_string(day_of_week[1]);
	day_of_week[2] = catgets(c->DT_catd, 1, 482, "Tue");
	nonascii_string(day_of_week[2]);
	day_of_week[3] = catgets(c->DT_catd, 1, 483, "Wed");
	nonascii_string(day_of_week[3]);
	day_of_week[4] = catgets(c->DT_catd, 1, 484, "Thu");
	nonascii_string(day_of_week[4]);
	day_of_week[5] = catgets(c->DT_catd, 1, 485, "Fri");
	nonascii_string(day_of_week[5]);
	day_of_week[6] = catgets(c->DT_catd, 1, 486, "Sat");
	nonascii_string(day_of_week[6]);
	for ( k = 0;  k < 7;  k++ ) {
		str = euc_to_octal(day_of_week[k]);	
		day_of_week[k] = (char *)malloc(sizeof(char) * cm_strlen(str) + 1);
		cm_strcpy(day_of_week[k], str);
	}
	fprintf (fp, "[(%s) (%s) (%s) (%s) (%s) (%s) (%s)] {\n", day_of_week[0], 
			 day_of_week[1], day_of_week[2], day_of_week[3], day_of_week[4],
			 day_of_week[5], day_of_week[6]);
	fprintf (fp, "    x1 y1 boxw boxh make_gray\n");
	fprintf (fp, "    x1 y1 boxw boxh line_width draw_box\n");
	fprintf (fp, "    WHITE setgray\n");
	fprintf (fp, "    x1 y1 boxh 1 center_y x1 boxw add\n");
	fprintf (fp, "    4 -1 roll ctr_text\n");
	fprintf (fp, "    BLACK setgray\n"); 
	fprintf (fp, "    /x1 x1 boxw add def\n");
	fprintf (fp, "} forall\n"); 
	fprintf (fp, "/boxh y1 BM sub %d div def\n", rows);
	fprintf (fp, "/y1 y1 boxh sub def\n");

	fprintf (fp, "\n%% --- make monthly grid ---\n\n");
	fprintf (fp, "/y2 y1 def\n"); 
	fprintf (fp, "%d {\n", rows);
	fprintf (fp, "    /x1 LM def\n"); 
	fprintf (fp, "    7 { \n");
	fprintf (fp, "        x1 y1 boxw boxh make_lt_gray\n");
	fprintf (fp, "        x1 y1 boxw boxh line_width draw_box\n");
	fprintf (fp, "        /x1 x1 boxw add def\n");
	fprintf (fp, "    } repeat  \n");
	fprintf (fp, "    /y1 y1 boxh sub def\n");
	fprintf (fp, "} repeat\n");
	fprintf (fp, "/y1 y2 def\n"); 

	for ( k = 0;  k < 7;  k++ ) {
		free((char *)day_of_week[k]);
	}
}

extern void 
ps_month_timeslots(FILE *fp, int date, int dow, Boolean more)
{ 
    fprintf (fp, "\n%d %d %d (%d) printmonthday\n", more, dow, date, date); 

#ifdef NEVER
    /* if weekday==0 && not 1st day of month then start on next line */
    fprintf (fp, "%d 0 eq %d 1 ne and { /y1 y1 boxh sub def } if\n",dow,date);
    fprintf (fp, "/x1 boxw %d mul LM add def\n", dow);
    fprintf (fp, "x1 y1 boxw boxh clear_box\n");
    fprintf (fp, "x1 y1 boxw boxh line_width draw_box\n");

    fprintf (fp, "DateFont DateFont.scale font_set\n");
    fprintf (fp, "/tab1 3 def\n");
    fprintf (fp, "/line1 y1 boxh add font.y sub tab1 sub def\n");
    fprintf (fp, "x1 tab1 add line1 (%d) left_text\n", date);
    if (more)
	fprintf(fp, "6 0 rmoveto (%s) show\n", "*");
    fprintf (fp, "/line1 line1 tab1 sub def\n");

    fprintf (fp, "TimeFont TimeFont.scale font_set\n");
    fprintf (fp, "/line1 line1 font.y sub 1 sub def\n");
    fprintf (fp, "/tab2 font.x 3 mul tab1 add def\n");
#endif
}


extern void
ps_print_time (FILE *fp, char *str, Glance view)
{

	if (view == dayGlance) {
                fprintf (fp, "( %s ) printdaytime\n", str);
        }
	else if (view == weekGlance) {
                fprintf (fp, "( %s ) printweektime\n", str);
	}
	else if (view == monthGlance) {
                fprintf (fp, "( %s ) printmonthtime\n", str);
	}
}


extern void
ps_print_text(FILE *fp, char *str, int indented, Glance view)
{
	nonascii_string(str);
	str = fix_ps_string(str);

	if (view == monthGlance) {

		fprintf (fp, "%d (%s) printmonthtext\n", indented, str);
	}
	else if (view == weekGlance) {

		fprintf (fp, "%d (%s) printweektext\n", indented, str);
	}
	else if (view == dayGlance) {

		fprintf (fp, "%d (%s) printdaytext\n", indented, str);
	}
}

static void
ps_print_todo_text(FILE *fp,
    char *str1,
    CSA_sint32 as, 
    CSA_sint32 appt_type, 
    int items)
{
	char buf[20];
	char *str;

	fprintf (fp, "line1 20 sub y1 gt {\n");
	fprintf (fp, "    ApptFont ApptFont.scale font_set\n");
	fprintf (fp, "    gsave\n");

	fprintf (fp, "    x1 y1 boxw font.x 2 div sub boxh rect clip newpath\n");

        if (items > 9)
		sprintf(buf, "%d.", items);
	else
		sprintf(buf, " %d.", items);
    	fprintf (fp, "TimeFont TimeFont.scale font_set\n");
	fprintf (fp, "    x1 tab0 add line1 (%s) left_text\n", buf);

	fprintf (fp, "    ApptFont ApptFont.scale font_set\n");
	if (appt_type == CSA_TYPE_TODO) {
		/* draw check box */
		fprintf (fp, "    x1 tab1 add line1 10 10 1 draw_box\n");
		/* add check to box */
		if (as == CSA_STATUS_COMPLETED) {
			fprintf (fp, "    x1 tab1 add 3 add line1 6 add moveto\n");
			fprintf (fp, "    x1 tab1 add 6 add line1 1 add lineto\n");
			fprintf (fp, "    x1 tab1 add 14 add line1 10 add lineto\n");
			fprintf (fp, "    1 setlinewidth");
			fprintf (fp, "    stroke");
		}
	}
	else 
		fprintf (fp, "    /tab2 tab1 def\n");
	
	nonascii_string(str1);
	str1 = fix_ps_string(str1);
	str = euc_to_octal(str1);

	fprintf (fp, "    x1 tab2 add line1 (%s) left_text\n", str);
	fprintf (fp, "    grestore\n");
	fprintf (fp, "    /line1 line1 font.y 2 mul sub def\n");
	fprintf (fp, "} if\n");
}

extern void
ps_month_line(FILE *fp)
{
	fprintf (fp, "    2 setlinewidth \n");
	fprintf (fp, "    /yval 213 def\n");
	fprintf (fp, "x1 10 add yval boxw 20 sub 11 2 draw_box\n");
	fprintf (fp, "x1 8 add yval boxw 15 sub 11 make_gray\n");
}

extern void
ps_print_todo_months(FILE *fp)
{
	int m_wid, m_hgt, mon, yr;
	Calendar *c = calendar;

	mon = month(c->view->date);
	yr = year(c->view->date);

	m_wid = (PRINTER_WIDTH-INCH-INCH-20) / 3;
        m_hgt = (PRINTER_HEIGHT-3*INCH-20) / 4;

	ps_translate(fp, INCH+6, 184); 

	if ((mon-1) == 0) 
		print_std_month(fp, 12, yr-1, m_wid, m_hgt);
	else 
		print_std_month(fp, mon-1, yr, m_wid, m_hgt);


	ps_translate(fp, m_wid, 0);
	print_std_month(fp, mon, yr, m_wid, m_hgt);

	ps_translate(fp, m_wid, 0);
	if ((mon+1) > 12) 
		print_std_month(fp, 1, yr+1, m_wid, m_hgt);
	else 
		print_std_month(fp, mon+1, yr, m_wid, m_hgt);

	ps_translate(fp, -(INCH+6+m_wid+m_wid), -184);

	fprintf (fp, "230 213 moveto\n");
	fprintf (fp, "230 82 lineto\n");
	fprintf (fp, "    stroke\n");
	fprintf (fp, "380 213 moveto\n");
	fprintf (fp, "380 82 lineto\n");
	fprintf (fp, "    stroke\n");

}

extern void
ps_init_printer (FILE *fp,
	short  portrait)		/* portrait or landscape mode? */
{    
	char prolog_file[MAXPATHLEN];
	int num_read = 0;
	char prolog_buf[BUFFERSIZE];
	char *locale;
	FILE *prolog_fp;
	double slm, srm, stm, sbm, tmp;
	int lm, rm, tm, bm;
	Props *p = (Props *) calendar->properties;

#ifdef HPUX
	{
		struct locale_data *li;
		li = getlocale(LOCALE_STATUS);
		locale = strdup(li->LC_MESSAGES_D);
	}
#else /* HPUX */
	locale = strdup(setlocale(LC_MESSAGES, NULL)); 
#endif /* HPUX */

	/* setup margins for programs coordinate space */
	lm = INCH;
	rm = PRINTER_WIDTH-INCH;
	bm = INCH;
	tm = PRINTER_HEIGHT-INCH;
	/* 
	 * Get width & height settings from the printer props sheet.
	 * These will be used to scale the output to fit in the 
	 * area described in the printer props sheet.
	 */
	slm = (double)72.0 * (double)get_int_prop(p, CP_PRINTPOSXOFF);
	srm = (double)72.0 * (double)get_int_prop(p, CP_PRINTWIDTH);
	sbm = (double)72.0 * (double)get_int_prop(p, CP_PRINTPOSYOFF);
	stm = (double)72.0 * (double)get_int_prop(p, CP_PRINTHEIGHT);
	
	if (!portrait) {
		rm = PRINTER_HEIGHT-INCH;	/* landscape mode */
		tm = PRINTER_WIDTH-INCH;
		tmp = srm;		/* exchange right & top margin */
		srm = stm;
		stm = tmp;
	}

	/* Use cm_printf to do numeric formatting to avoid locale conversion.
	 * This is to avoid printing (e.g.) 72,00 instead of 72.00 in the
	 * postscript file.
	 */

/*	fprintf (fp, "%%!PS-Adobe-2.0 EPSF-2.0\n"); */
	fprintf (fp, "%%!\n");
	fprintf (fp, "%%BoundingBox ");
	fputs ((char *)(cm_printf(slm, 2)), fp); fputs (" ", fp);
	fputs ((char *)(cm_printf(sbm, 2)), fp); fputs (" ", fp);
	fputs ((char *)(cm_printf(srm, 2)), fp); fputs (" ", fp);
	fputs ((char *)(cm_printf(stm, 2)), fp);
	fprintf (fp, "\n%%Title: %s\n", ps_file); 
	fprintf (fp, "%%Creator: Mike Bundschuh & Nannette Simpson\n"); 
	fprintf (fp, "%%I18N printing: Wendy Mui\n");
	fprintf (fp, "%%EndComments\n"); 

/*  Add locale specific prolog file 
 *  Prolog file = /etc/dt/config/psfonts/<locale>/prolog.ps:
 *		  /ust/dt/config/psfonts/<locale>/prolog.ps
 */
	sprintf(prolog_file, PROLOG_LOC_1, locale); 
	if ( (prolog_fp = fopen(prolog_file, "r")) == NULL ) {
		sprintf(prolog_file, PROLOG_LOC_2, locale);
		if ( (prolog_fp = fopen(prolog_file, "r")) == NULL ) {
			prolog_found = 0;

#ifdef DEBUG
		fprintf(stderr, catgets(c->DT_catd, 1, 487, "ps_init_printer(): cannot open postscript prolog file: %s\n"), prolog_file);
#endif
	/* Need to define LC_<fontname> because prolog.ps is missing */
            	fprintf (fp, "\n%% I18NL4 prolog.ps file not available\n");
            	fprintf (fp, "/LC_Times-Bold\n");
            	fprintf (fp, "    /Times-Bold findfont def\n");
            	fprintf (fp, "/LC_Helvetica\n");
            	fprintf (fp, "    /Helvetica findfont def\n");
            	fprintf (fp, "/LC_Courier\n");
            	fprintf (fp, "  /Courier findfont def\n");
            	fprintf (fp, "/LC_Helvetica-BoldOblique\n");
            	fprintf (fp, "  /Helvetica-BoldOblique findfont def\n");
            	fprintf (fp, "/LC_Helvetica-Bold\n");
            	fprintf (fp, "  /Helvetica-Bold findfont def\n");
            	fprintf (fp, "/LC_Times-Italic\n");
            	fprintf (fp, "  /Times-Italic findfont def\n");
            	fprintf (fp, "/LC_Times-Roman\n");
            	fprintf (fp, "  /Times-Roman findfont def\n\n");
		}
		else
			prolog_found = 1;
	} 
	else {
		prolog_found = 1;
	}

	free (locale);

	/* if there is a matching prolog file for the locale, 
	   write it out to the stream. */
	if ( prolog_found ) {
		while ( (num_read = fread(prolog_buf, 1, BUFFERSIZE, prolog_fp)) ) {
			fwrite(prolog_buf, 1, num_read, fp);
		}
		fclose(prolog_fp);
	}

#ifdef DEBUG
	fprintf (fp, "\n%% --- show rulers (for debugging) ---\n");
	fprintf (fp, "/showrulers { \n");
	fprintf (fp, "    /Times-Roman findfont 20 scalefont setfont\n");
	fprintf (fp, "    /pwid 8.5 72 mul def\n");
	fprintf (fp, "    /phgt 11  72 mul def\n");
	fprintf (fp, "    /gap  18 def\n");
	fprintf (fp, "    /xreps pwid gap div def\n");
	fprintf (fp, "    /yreps phgt gap div def\n");
	fprintf (fp, "    /linelen 20 def\n");
	fprintf (fp, "    /tmpstr 8 string def\n\n");

	fprintf (fp, "    0 0 moveto\n");
	fprintf (fp, "    /offset 0 def\n");
	fprintf (fp, "    xreps {\n");
	fprintf (fp, "        0 linelen rlineto gsave stroke grestore\n");
	fprintf (fp, "        offset 72 mod 0 eq\n");
	fprintf (fp, "        { offset 72 idiv tmpstr cvs gsave show grestore } if\n");
	fprintf (fp, "        gap linelen neg rmoveto\n");
	fprintf (fp, "        /offset gap offset add def\n");
	fprintf (fp, "    } repeat\n\n");

	fprintf (fp, "    0 0 moveto\n");
	fprintf (fp, "    /offset 0 def\n");
	fprintf (fp, "    yreps {\n");
	fprintf (fp, "        linelen 0 rlineto gsave stroke grestore\n");
	fprintf (fp, "        offset 72 mod 0 eq\n");
	fprintf (fp, "        { offset 72 idiv tmpstr cvs gsave show grestore } if\n");
	fprintf (fp, "        linelen neg gap rmoveto\n");
	fprintf (fp, "        /offset gap offset add def\n");
	fprintf (fp, "    } repeat\n");
	fprintf (fp, "} def\n");
	fprintf (fp, "\nshowrulers\t\t\t%% FOR DEBUGGING\n");
#endif

	if (!portrait)
		ps_landscape_mode(fp);
	else
	fprintf (fp, "\ngsave\t\t%% save initial graphics state\n");

	fprintf (fp, "\n%% --- fit into bounding box ---\n");

    fputs ((char *)(cm_printf(slm, 2)), fp); fputs (" ", fp);
    fputs ((char *)(cm_printf((double)lm, 0)), fp);  fputs (" ", fp);
    fputs ((char *)(cm_printf(srm, 2)), fp); fputs (" ", fp);
    fputs ((char *)(cm_printf((double)rm, 0)), fp);
    fputs (" div mul sub ", fp);
    fputs ((char *)(cm_printf(sbm, 2)), fp); fputs (" ", fp);
    fputs ((char *)(cm_printf((double)bm, 0)), fp);  fputs (" ", fp);
    fputs ((char *)(cm_printf(stm, 2)), fp); fputs (" ", fp);
    fputs ((char *)(cm_printf((double)tm, 0)), fp);
    fputs (" div mul sub translate\n", fp);

/*	fprintf (fp, "%.2f %d sub %.2f %d sub translate\n", slm, lm, sbm, bm);  */
/*	fprintf (fp, "%.2f %.2f translate\n", slm, sbm); */

#ifdef NEVER
fprintf (stderr, "slm=%.2f lm=%d sub sbm=%.2f bm=%d sub translate\n", slm, lm, sbm, bm);
#endif

	if (portrait)
		tm += (0.5*INCH);
	else
		rm += (0.5*INCH);

/*	fprintf (fp, "%.2f %.2f sub %d div %.2f %.2f sub %d div scale\n", 
		 srm, slm, rm, stm, sbm, tm); */

    fputs ((char *)(cm_printf(srm, 2)), fp); fputs (" ", fp);
    fputs ((char *)(cm_printf((double)rm, 0)), fp);
    fputs (" div ", fp);
    fputs ((char *)(cm_printf(stm, 2)), fp); fputs (" ", fp);
    fputs ((char *)(cm_printf((double)tm, 0)), fp);
    fputs (" div scale\n", fp);

#ifdef NEVER
fprintf (stderr, "srm=%.2f slm=%.2f sub rm=%d div stm=%.2f sbm=%.2f sub tm=%d div scale\n", 
	 srm, slm, rm, stm, sbm, tm);
#endif

	fprintf (fp, "\n%% --- init vars, fonts, procedures, etc. ---\n");

	fprintf (fp, "/WHITE   1.0  def\n");		/* set print tones */
	fprintf (fp, "/GRAY    0.5  def\n");
	fprintf (fp, "/LT_GRAY 0.95 def\n");
	fprintf (fp, "/BLACK   0.0  def\n");

	fprintf (fp, "/TM %d def\n", tm);		/* set margins */
	fprintf (fp, "/BM %d def\n", bm);
	fprintf (fp, "/LM %d def\n", lm);
	fprintf (fp, "/RM %d def\n", rm);

	fprintf (fp, "/line_width 1 def\n");
	fprintf (fp, "/x1 LM def\n");
	fprintf (fp, "/y1 TM def\n");

	if (prolog_found) {
		fprintf (fp, "%%\n");
		fprintf (fp, "/MonthFont\n");
		fprintf (fp, "    /LC_Times-Bold findfont def\n");
		fprintf (fp, "/DayFont\n");
		fprintf (fp, "    /LC_Times-Bold findfont def\n");
		fprintf (fp, "/DateFont\n");
		fprintf (fp, "    /LC_Helvetica findfont def\n");
		fprintf (fp, "/TimeFont\n");
		fprintf (fp, "    /LC_Helvetica-Bold findfont def\n");
		fprintf (fp, "/ApptFont\n");
		fprintf (fp, "    /LC_Helvetica findfont def\n");
		fprintf (fp, "/StampFont\n");
		fprintf (fp, "    /LC_Times-Bold findfont def\n");
		fprintf (fp, "%%\n");
	}
else {
		fprintf (fp, "%%\n");
		fprintf (fp, "/ISOLatin1_header {\n");
		fprintf (fp, "%% Use ISOLatin1Encoding\n");
		fprintf (fp, "/Helvetica findfont\n");
		fprintf (fp, "\tdup length dict begin\n");
		fprintf (fp, "\t{1 index /FID ne {def} {pop pop} ifelse} forall\n");
		fprintf (fp, "\t/Encoding ISOLatin1Encoding def\n");
		fprintf (fp, "\tcurrentdict\n");
		fprintf (fp, "\tend\n");
		fprintf (fp, "/Helvetica-ISOLatin1 exch definefont pop\n");
		fprintf (fp, "%%\n");
		fprintf (fp, "/Times-Bold findfont\n");
		fprintf (fp, "\tdup length dict begin\n");
		fprintf (fp, "\t{1 index /FID ne {def} {pop pop} ifelse} forall\n");
		fprintf (fp, "\t/Encoding ISOLatin1Encoding def\n");
		fprintf (fp, "\tcurrentdict\n");
		fprintf (fp, "\tend\n");
		fprintf (fp, "/Times-Bold-ISOLatin1 exch definefont pop\n");
		fprintf (fp, "%%\n");

		fprintf (fp, "/MonthFont\n");
		fprintf (fp, "    /Times-Bold-ISOLatin1 findfont def\n");
		fprintf (fp, "/DayFont\n");
		fprintf (fp, "    /Times-Bold-ISOLatin1 findfont def\n");
		fprintf (fp, "/DateFont\n");
		fprintf (fp, "    /Helvetica-ISOLatin1 findfont def\n");
		fprintf (fp, "/TimeFont\n");
		fprintf (fp, "    /Times-Bold-ISOLatin1 findfont def\n");
		fprintf (fp, "/ApptFont\n");
		fprintf (fp, "    /Helvetica-ISOLatin1 findfont def\n");
		fprintf (fp, "/StampFont\n");
		fprintf (fp, "    /Times-Bold-ISOLatin1 findfont def\n");
		fprintf (fp, "} def    %% ISOLatin1_header\n");

		fprintf (fp, "%%\n");
		fprintf (fp, "/regular_header {\n");
		fprintf (fp, "/MonthFont\n");
		fprintf (fp, "    /Times-Bold findfont def\n");
		fprintf (fp, "/DayFont\n");
		fprintf (fp, "    /Times-Bold findfont def\n");
		fprintf (fp, "/DateFont\n");
		fprintf (fp, "    /Helvetica findfont def\n");
		fprintf (fp, "/TimeFont\n");
		fprintf (fp, "    /Times-Bold findfont def\n");
		fprintf (fp, "/ApptFont\n");
		fprintf (fp, "    /Helvetica findfont def\n");
		fprintf (fp, "/StampFont\n");
		fprintf (fp, "    /Times-Bold findfont def\n");
		fprintf (fp, "} def    %% regular_header\n");
		fprintf (fp, "%%\n");
		fprintf (fp, "systemdict /ISOLatin1Encoding known\n");
		fprintf (fp, "{ISOLatin1_header} {regular_header} ifelse\n");
		fprintf (fp, "%%\n");
}
	fprintf (fp, "/MonthFont.scale 16 def\n");
	fprintf (fp, "/DayFont.scale   12 def\n");
	fprintf (fp, "/DateFont.scale  12 def\n");
	fprintf (fp, "/TimeFont.scale  12 def\n");
	fprintf (fp, "/ApptFont.scale  10 def\n");
	fprintf (fp, "/StampFont.scale  9 def\n");

	fprintf (fp, "\n%% --- inch macro ---\n");
	fprintf (fp, "/inch { %% x => -\n");
	fprintf (fp, "    72 mul\n");
	fprintf (fp, "} def\n");

	fprintf (fp, "\n%% --- make a rect path ---\n");
	fprintf (fp, "/rect { %% x y w h => -\n");
	fprintf (fp, "    4 2 roll moveto\n");
	fprintf (fp, "    exch dup 3 1 roll\n");
	fprintf (fp, "    0 rlineto\n");
	fprintf (fp, "    0 exch rlineto\n");
	fprintf (fp, "    neg 0 rlineto\n");
	fprintf (fp, "    closepath\n");
	fprintf (fp, "} def\n");

	fprintf (fp, "\n%% --- fill a square with black ---\n");
	fprintf (fp, "/make_black { %% x y w h => -\n");
	fprintf (fp, "    rect\n");
	fprintf (fp, "    fill \n");
	fprintf (fp, "} def\n");

	fprintf (fp, "\n%% --- fill a square with gray ---\n");
	fprintf (fp, "/make_gray { %% x y w h => -\n");
	fprintf (fp, "    GRAY setgray\n");
	fprintf (fp, "    rect\n");
	fprintf (fp, "    fill \n");
	fprintf (fp, "    BLACK setgray\n");
	fprintf (fp, "} def\n");

	fprintf (fp, "\n%% --- fill a square with light gray ---\n");
	fprintf (fp, "/make_lt_gray { %% x y w h => -\n");
	fprintf (fp, "    LT_GRAY setgray\n");
	fprintf (fp, "    rect\n");
	fprintf (fp, "    fill \n");
	fprintf (fp, "    BLACK setgray\n");
	fprintf (fp, "} def\n");
	  
	fprintf (fp, "\n%% --- draw a box & set line width ---\n");
	fprintf (fp, "/draw_box { %% x y w h linew => - \n");
	fprintf (fp, "    setlinewidth\n");
	fprintf (fp, "    rect\n");
	fprintf (fp, "    stroke\n");
	fprintf (fp, "    1 setlinewidth\n");
	fprintf (fp, "} def\n");
	    
	fprintf (fp, "\n%% --- draw a box with gray lines ---\n");
	fprintf (fp, "/draw_gray_box { %% x y w h linew => - \n");
	fprintf (fp, "    GRAY setgray\n");
	fprintf (fp, "    draw_box\n");
	fprintf (fp, "    BLACK setgray\n");
	fprintf (fp, "} def\n");

	fprintf (fp, "\n%% --- draw a box with white lines ---\n");
	fprintf (fp, "/dissolve_box { %% x y w h linew => - \n");
	fprintf (fp, "    WHITE setgray\n");
	fprintf (fp, "    draw_box\n");
	fprintf (fp, "    BLACK setgray\n");
	fprintf (fp, "} def\n");

	fprintf (fp, "\n%% --- clear the inside of a box ---\n");
	fprintf (fp, "/clear_box { %% x y w h => -\n");
	fprintf (fp, "    WHITE setgray\n");
	fprintf (fp, "    rect\n");
	fprintf (fp, "    fill \n");
	fprintf (fp, "    BLACK setgray\n");
	fprintf (fp, "} def\n");
	      
	fprintf (fp, "\n%% --- fill a square with gray with white edges ---\n");
	fprintf (fp, "/gray_box { %% x y w h => -\n");
	fprintf (fp, "    6 sub 4 1 roll\n");
	fprintf (fp, "    6 sub 4 1 roll\n");
	fprintf (fp, "    3 add 4 1 roll\n");
	fprintf (fp, "    3 add 4 1 roll\n");
	fprintf (fp, "    make_gray\n");
	fprintf (fp, "} def\n");

	fprintf (fp, "\n%% --- print center-justified text ---\n");
	fprintf (fp, "/ctr_text { %% x1 y x2 str => -\n");
	fprintf (fp, "    dup stringwidth pop 2 div\n");
	fprintf (fp, "    5 2 roll\n");
	fprintf (fp, "    3 -1 roll dup 3 1 roll sub\n");
	fprintf (fp, "    2 div add\n");
	fprintf (fp, "    3 -1 roll sub\n");
	fprintf (fp, "    exch moveto\n");
	fprintf (fp, "    show\n");
	fprintf (fp, "} def\n");

	fprintf (fp, "\n%% --- print left-justified text ---\n");
	fprintf (fp, "/left_text { %% x y str => -\n");
	fprintf (fp, "   3 -2 roll moveto\n");
	fprintf (fp, "   show\n");
	fprintf (fp, "} def\n");

	fprintf (fp, "\n%% --- print right-justified text ---\n");
	fprintf (fp, "/right_text { %% x y str => -\n");
	fprintf (fp, "   dup stringwidth pop\n");
	fprintf (fp, "   4 -1 roll exch sub\n");
	fprintf (fp, "   3 -1 roll moveto show\n"); 
	fprintf (fp, "} def\n");

	fprintf (fp, "\n%% --- set font, keep track of font width & height ---\n");
	fprintf (fp, "/font_set { %% font scale => - \n");
	fprintf (fp, "    /font.x exch def\n");
	fprintf (fp, "    font.x scalefont\n");
	fprintf (fp, "    setfont\n");
	fprintf (fp, "   /font.y (M) stringwidth pop def\n");
	fprintf (fp, "} def\n");
		   
	fprintf (fp, "\n%% --- line feed ---\n");
	fprintf (fp, "/LF { %% - => - \n");
	fprintf (fp, "    /y1 y1 font.x sub def\n");
	fprintf (fp, "    x1 y1 moveto\n");
	fprintf (fp, "} def\n");

	fprintf (fp, "\n%% --- carriage return ---\n");
	fprintf (fp, "/CR { %% - => - \n");
	fprintf (fp, "    /y1 y1 font.x sub font.x 4 div sub def\n");
	fprintf (fp, "    /x1 LM def\n");
	fprintf (fp, "    x1 y1 moveto\n");
	fprintf (fp, "} def\n");

	fprintf (fp, "\n%% --- Sun Logo ---\n");
	fprintf (fp, "/Sunlogo { %% x y size = -\n");
	fprintf (fp, "3 1 roll        %% s x y\n");
	fprintf (fp, "gsave translate %% s\n");
	fprintf (fp, "dup scale       %% -\n");
	fprintf (fp, "45 rotate\n");
	fprintf (fp, "    /Uchar {\n");
	fprintf (fp, "        newpath\n");
	fprintf (fp, "        -.1 0 moveto\n");
	fprintf (fp, "        0 0 .1 180 360 arc\n");
	fprintf (fp, "        0 2.9 rlineto\n");
	fprintf (fp, "        .8 0 rlineto\n");
	fprintf (fp, "        0 -2.9 rlineto\n");
	fprintf (fp, "        0 0 .9 0 180 arcn\n");
	fprintf (fp, "        0 2.9 rlineto\n");
	fprintf (fp, "        .8 0 rlineto\n");
	fprintf (fp, "        closepath\n");
	fprintf (fp, "        fill\n");
	fprintf (fp, "        } def\n");
	fprintf (fp, "    /2Uchar { \n");
	fprintf (fp, "        Uchar gsave 4 4 translate Uchar grestore\n");
	fprintf (fp, "        } def\n");
	fprintf (fp, "    4 { 2Uchar 6 0 translate 90 rotate } repeat\n");
	fprintf (fp, "grestore\n");
	fprintf (fp, "} def\n");

	fprintf (fp, "\n%% --- get y coord to center between TM & BM of a rect ---\n");
	fprintf (fp, "/center_y { %% y boxh lines => y_delta\n");
	fprintf (fp, "    font.y mul 2 div 3 1 roll\n");
	fprintf (fp, "    2 div add\n");
	fprintf (fp, "    add font.y sub\n");
	fprintf (fp, "} def\n");
}

extern void
ps_set_font(fp, font_type, font)
    FILE *fp;
    char *font_type, *font;
{
    fprintf (fp, "/%s /%s findfont def\n", font_type, font);
}

extern void 
ps_set_fontsize (fp, font_type, size)
    FILE *fp; 
    char *font_type; 
    int  size;
{ 
    fprintf (fp, "/%s.scale %d def\n", font_type, size);
} 

extern void
ps_print_todo(FILE *fp, CSA_entry_handle *list, int a_total, 
		CSA_sint32 appt_type, Glance glance, int vf) 
{
	int      	i, max_chars = 75, items_per_page = 0, meoval;
	char            buf3[100], buf2[10], buf1[80];
	static		int total_items;
	boolean_t	skip_appt;
	CSA_return_code	stat;
	Dtcm_appointment *appt;
	Calendar	*c = calendar;
	Props		*p = (Props*)c->properties;
	DisplayType	dt = get_int_prop(p, CP_DEFAULTDISP);
	OrderingType	ot = get_int_prop(p, CP_DATEORDERING);
	SeparatorType	st = get_int_prop(p, CP_DATESEPARATOR);
	Tick		start_tick;
 
	meoval = get_int_prop(p, CP_PRINTPRIVACY);
	buf1[1] = buf2[0] = buf3[0] = '\0';

        appt = allocate_appt_struct(appt_read,
				    c->general->version,
				    CSA_ENTRY_ATTR_CLASSIFICATION_I,
				    CSA_ENTRY_ATTR_TYPE_I,
				    CSA_ENTRY_ATTR_STATUS_I,
				    CSA_ENTRY_ATTR_START_DATE_I,
				    CSA_ENTRY_ATTR_SUMMARY_I,
				    CSA_X_DT_ENTRY_ATTR_SHOWTIME_I,
				    CSA_ENTRY_ATTR_END_DATE_I,
				    NULL);
        for (i = 0; i < a_total; i++) {
		stat = query_appt_struct(c->cal_handle, list[i], appt);
		if (stat != CSA_SUCCESS) {
			free_appt_struct(&appt);
			return;
		}
		skip_appt = _B_FALSE;

		if ((privacy_set(appt) == CSA_CLASS_PUBLIC) && !(meoval & PRINT_PUBLIC))
			skip_appt = _B_TRUE;
		else if ((privacy_set(appt) == CSA_CLASS_CONFIDENTIAL) && !(meoval & PRINT_SEMIPRIVATE))
			skip_appt = _B_TRUE;
		else if ((privacy_set(appt) == CSA_CLASS_PRIVATE) && !(meoval & PRINT_PRIVATE))
			skip_appt = _B_TRUE;
		else
			skip_appt = _B_FALSE;

		if ((appt_type == CSA_TYPE_TODO) &&
		    ((appt->type->value->item.sint32_value != CSA_TYPE_TODO)
		    ||
		     ((vf == VIEW_PENDING) && (appt->state->value->item.sint32_value == CSA_STATUS_COMPLETED)) ||
		     ((vf == VIEW_COMPLETED) && (appt->state->value->item.sint32_value == CSA_X_DT_STATUS_ACTIVE))))
			skip_appt = _B_TRUE;

		if ((appt_type == CSA_TYPE_EVENT) && (appt->type->value->item.sint32_value != CSA_TYPE_EVENT))
			skip_appt = _B_TRUE;

		if (skip_appt)
			continue;

		format_maxchars(appt, buf1, max_chars, dt);
		if (glance != dayGlance) {
			_csa_iso8601_to_tick(appt->time->value->item.date_time_value, &start_tick);
			format_date3(start_tick, ot, st, buf2);
			sprintf(buf3, "%s  %s", buf2, buf1);
		} else
			cm_strcpy(buf3, buf1);
		if (buf3 != NULL) {
			total_items++;
			ps_print_todo_text (fp, buf3, appt->state->value->item.sint32_value,
					appt_type, total_items);
			if ((++items_per_page % MAX_TODO) == 0) {
				fprintf (fp, "showpage\n");
				ps_init_printer(fp, PORTRAIT);
				ps_init_list(fp);
				ps_todo_outline(fp, appt_type);
				items_per_page = 0;
			}
		}
	} /* end for stmt */

	free_appt_struct(&appt);

	if (items_per_page > MAX_TODO_LP) {
		fprintf (fp, "showpage\n");
		ps_init_printer(fp, PORTRAIT);
		ps_init_list(fp);
		ps_todo_outline(fp, appt_type);
	}

	ps_month_line(fp);
	ps_print_todo_months(fp);
	total_items = 0;
}

extern Boolean
ps_print_month_appts(FILE *fp,
        CSA_entry_handle *list,
        int a_total,
	int num_page,
	int hi_tm,
	int lines_per_box,
	Glance view)
{
	/*
	 * This routine is used to print appointments for day, week 
	 * and month views.  The parm "view" is used to differentiate
	 * who is printing, and personalize the print for that view.
	 */

	int      	indented, multlines=TRUE;
	Lines		*lines, *lp;
    	char     	buf1[128];
	Calendar	*c = calendar;
	Props		*pr = (Props*)c->properties;
	int		meoval = get_int_prop(pr, CP_PRINTPRIVACY);
	int	        i, start, pos = 1, line_counter = 0;	
	CSA_return_code	stat;
	Dtcm_appointment *appt;
	Tick		start_tick;
 
	start = ((num_page - 1) * lines_per_box) + 1;

        appt = allocate_appt_struct(appt_read,
				    c->general->version,
				    CSA_ENTRY_ATTR_CLASSIFICATION_I,
				    CSA_ENTRY_ATTR_START_DATE_I,
				    CSA_ENTRY_ATTR_SUMMARY_I,
				    CSA_X_DT_ENTRY_ATTR_SHOWTIME_I,
				    NULL);
	for (i = 0; i < a_total; i++) {
		stat = query_appt_struct(c->cal_handle, list[i], appt);
                if (stat != CSA_SUCCESS) {
			free_appt_struct(&appt);
                        return False;
                }

		_csa_iso8601_to_tick(appt->time->value->item.date_time_value, &start_tick);
		if (start_tick >= hi_tm) {
			free_appt_struct(&appt);
			return TRUE;
		}
 
		if ((privacy_set(appt) == CSA_CLASS_PUBLIC) && !(meoval & PRINT_PUBLIC))
			continue;
		else if ((privacy_set(appt) == CSA_CLASS_CONFIDENTIAL) && !(meoval & PRINT_SEMIPRIVATE))
			continue;
		else if ((privacy_set(appt) == CSA_CLASS_PRIVATE) && !(meoval & PRINT_PRIVATE))
			continue;


                if (pos < start) {
                        pos++;
                }
		else {

			get_time_str (appt, buf1);

			indented = (*buf1 != NULL);


                	lp = lines = text_to_lines(appt->what->value->item.string_value, multlines ? 10 : 1);
                	line_counter++;
                	if ((line_counter > lines_per_box) && (lines != NULL)) {
				free_appt_struct(&appt);
                        	return(FALSE);
			}

			/* only print if appt text found */ 
			if (lines != NULL && lines->s != NULL) {
				if (indented)  	/* time found so print it */
					ps_print_time (fp, buf1, view);
				ps_print_text (fp, lines->s, indented, view);

			}
		destroy_lines(lp);
		}
	}
	free_appt_struct(&appt);
	return(TRUE);
}

extern Boolean   
ps_print_multi_appts(FILE *fp,
	CSA_entry_handle *list,
	int a_total,
        int num_page,
        int hi_tm,
        Glance view)
{
        /*
         * This routine is used to print appointments for day, week
         * and month views.  The parm "view" is used to differentiate
         * who is printing, and personalize the print for that view.
         */
 
        int             indented, multlines=TRUE;
        Lines           *lines, *lp;    
        char            buf1[128], buf2[257];
        Calendar        *c = calendar;  
        Props           *pr = (Props*)c->properties;
	int		meoval = get_int_prop(pr, CP_PRINTPRIVACY);
	int		dt = get_int_prop(pr, CP_DEFAULTDISP);
        int             i, j, start, maxlines, pos = 1, line_counter = 0;
	Boolean		new_appt = FALSE;
	CSA_return_code 	stat;
	Dtcm_appointment *appt;
 
        if (view == weekGlance)
                maxlines = 22;
        else if (view == dayGlance)
                maxlines = get_lines_per_page(pr);
 
        start = ((num_page - 1) * maxlines) + 1;
 
        appt = allocate_appt_struct(appt_read,
				    c->general->version,
				    CSA_ENTRY_ATTR_CLASSIFICATION_I,
				    CSA_ENTRY_ATTR_START_DATE_I,
				    CSA_ENTRY_ATTR_SUMMARY_I,
				    CSA_X_DT_ENTRY_ATTR_SHOWTIME_I,
				    CSA_ENTRY_ATTR_END_DATE_I,
				    NULL);
        for (j = 0; j < a_total; j++) {
		stat = query_appt_struct(c->cal_handle, list[j], appt);
		if (stat != CSA_SUCCESS) {
			free_appt_struct(&appt);
			return False;
		}

		if ((privacy_set(appt) == CSA_CLASS_PUBLIC) && !(meoval & PRINT_PUBLIC))
			continue;
		else if ((privacy_set(appt) == CSA_CLASS_CONFIDENTIAL) && !(meoval & PRINT_SEMIPRIVATE))
			continue;
		else if ((privacy_set(appt) == CSA_CLASS_PRIVATE) && !(meoval & PRINT_PRIVATE))
			continue;
 
                lp = lines = text_to_lines(appt->what->value->item.string_value, multlines ? 10 : 1);
		new_appt = TRUE;

                /* skip past lines already printed */
                if (pos < start) {
                        if (showtime_set(appt)) {
				if (new_appt)
					for (i = 1; i <= num_page; i++)
						if (pos == (maxlines * i))
							start--;
				pos++;
			}
                        while ((lines != NULL) && (pos < start)) {
                        	pos++;
				new_appt = FALSE;
                        	lines = lines->next;
                        }
                        if (pos < start) {
                        	continue;
                        }
                }
 
		/* skip last line if it's a time */
		if (showtime_set(appt)) {
			if (line_counter == (maxlines - 1)) {
				free_appt_struct(&appt);
				return(FALSE);
			}
		}

                /* HH:MM xm - HH:MM xm format */
                format_line2(appt, buf1, buf2, dt);

                indented = (*buf1 != NULL);
                /* only print if appt text found */
                if (lines != NULL && lines->s != NULL) {
                        if ((indented) && (new_appt)) {
                                line_counter++;
                                if ((line_counter > maxlines) && (lines != NULL)) {
					free_appt_struct(&appt);
                                	return(FALSE);
				}

                                ps_print_time (fp, buf1, view);
                        }
 
                        ps_print_text (fp, lines->s, indented, view);
                        line_counter++;
                        if ((line_counter > maxlines) && (lines != NULL)) {
				free_appt_struct(&appt);
                                return(FALSE);
			}
 
                        lines = lines->next;
                        while (lines != NULL) {
                        	line_counter++;
                        	if ((line_counter > maxlines) && (lines != NULL)) {
					free_appt_struct(&appt);
                        		return(FALSE);
				}

                        	ps_print_text (fp, lines->s, indented, view);
                        	lines = lines->next;
                        }
                }
                destroy_lines(lp);
                if (view == weekGlance)
                        ps_week_sched_update(appt, pr);
		new_appt = FALSE;
        }
	free_appt_struct(&appt);
        return(TRUE);
}


extern void
ps_print_little_months (FILE *fp, Tick tick) {
	int m = month(tick);		/* current month just printed */
	int y = year(tick);		/* year of month just printed */
	int fday = fdom(tick);		/* first day of month, 0=Sun ... */

	int nm, nmy, pm, pmy;

	int ndays = monthlength(tick);
	int nrows = numwks(tick);
	int inch = 72;
	int dx, dy;

	/* 
	 * Print out miniature prev & next month on month grid.
	 * Check if there is enough room at end;  if not then
	 * print at beguinning of grid.
	 *
	 * This is a massive kludge, as we are eyeballing where to place
	 * the miniatures.  Oh well... redo right later...
	 */
	if (nrows == 4) nrows++;

	fprintf (fp, "\n%% --- print miniature months ---\n");

	if ((fday+ndays+2) <= (nrows*7))
	{
		dx = 564;		/* print at bottom */
		dy = 140;
		if (nrows == 6)
			dy = 130;
	}
	else
	{
		dx = 75;		/* print at top */
		dy = 478;
	}

	if (m == 12)
	{
		nm = 1;
		pm = m - 1;
		nmy = y + 1;
		pmy = y;
	}
	else if (m == 1)
	{
		nm = m + 1;
		pm = 12;
		nmy = y;
		pmy = y - 1;
	}
	else
	{
		nm = m + 1;
		pm = m - 1;
		nmy = pmy = y;
	}

	fprintf (fp, "\ngsave\n\n");
	ps_translate (fp, dx, dy);
	ps_scale (fp, 0.6, 0.6);
	print_std_month(fp, pm, pmy, 2*inch, 2*inch);

	ps_translate (fp, 165, 0);
	print_std_month(fp, nm, nmy, 2*inch, 2*inch);
	fprintf (fp, "\ngrestore\n\n");
}


/* 
 * 	print week routines
 */

extern void
ps_init_week(FILE *fp)
{
	fprintf (fp, "\n%% --- init for week view print ---\n");

	fprintf (fp, "/x1 LM def\n");
	fprintf (fp, "/y1 TM def\n");
	fprintf (fp, "/boxw RM LM sub 5 div def\n");

	ps_set_fontsize (fp, "MonthFont", 16);
	ps_set_fontsize (fp, "DayFont",   12);
	ps_set_fontsize (fp, "TimeFont",   9);
	ps_set_fontsize (fp, "ApptFont",   9);

        fprintf (fp, "/printweektime { %% string => \n");
	fprintf (fp, "/weektimestr exch def\n");
        fprintf (fp, "line1 2 sub y1 gt {\n");
        /* time and text must be same font for multi-pages */
        fprintf (fp, "    ApptFont ApptFont.scale font_set\n");
        fprintf (fp, "    /tab2 tab1 font.x 2 mul add def\n");
        fprintf (fp, "    x1 tab1 add 3 add line1 weektimestr left_text\n");
        fprintf (fp, "    /line1 line1 font.y sub 1 sub def\n");
        fprintf (fp, "} if\n\n");
	fprintf (fp, "} def\n");

	/* set up macro code for the printing of week text */

	fprintf (fp, "/printweektext { %% string indented => \n");
	fprintf (fp, "/weektextstring exch def /weekindent exch def\n");
        fprintf (fp, "line1 2 sub y1 gt {\n");
        fprintf (fp, "    ApptFont ApptFont.scale font_set\n");
        fprintf (fp, "    gsave\n");
        fprintf (fp, "    x1 y1 boxw font.x 2 div sub boxh rect clip newpath\n");

	fprintf (fp, "weekindent 1 eq { /tab2 tab1 3 mul def x1 tab2 add line1 weektextstring left_text } { /tab2 tab1 2 mul def x1 tab1 add line1 weektextstring left_text } ifelse\n");

        fprintf (fp, "    grestore\n");
        fprintf (fp, "    /line1 line1 font.y sub 1 sub def\n");
        fprintf (fp, "} if\n");


	fprintf (fp, "} def\n");

	/* redraw surrounding black box  & horiz lines */
	fprintf (fp, "/tidyweek { %% number_of_hour_boxes => - \n");
	fprintf (fp, "/numhours exch def\n");
	fprintf (fp, "BLACK setgray\n");
	fprintf (fp, "0.25 setlinewidth\n");
	fprintf (fp, "/s_x LM tab add def\n");
	fprintf (fp, "/s_y BM s_boxh add def\n");
	fprintf (fp, "numhours {\n");   
	fprintf (fp, "    s_x s_y moveto\n");
	fprintf (fp, "    s_boxw 7 mul 0 rlineto\n");
	fprintf (fp, "    stroke\n");
	fprintf (fp, "    /s_y s_y s_boxh add def\n");
	fprintf (fp, "} repeat\n");

	fprintf (fp, "/s_x LM tab add def\n");
	fprintf (fp, "/s_y BM def\n");
	fprintf (fp, "/line_width 2 def\n");
	fprintf (fp, "s_x s_y s_boxw 7 mul s_boxh %d mul line_width draw_box\n", num_hours);
	fprintf (fp, "} def\n");
}


extern void
ps_week_header(FILE *fp, char *buf, int num_page, int total_pages)
{
	char *str, str2[BUFFERSIZE];
	Calendar *c = calendar;

	if (total_pages == 0)
		total_pages++;

	nonascii_string(buf);
	fprintf (fp, "\n%% --- print week header centered at top ---\n");
	fprintf (fp, "MonthFont MonthFont.scale font_set CR\n"); 
	str = euc_to_octal(buf);
	fprintf (fp, "LM y1 RM (%s) ctr_text\n", str); 

#ifdef NEVER
	/* print sun logo at left margin */ 
	fprintf (fp, "LM 20 add y1 4 Sunlogo\n", buf); 
#endif

	fprintf (fp, "StampFont StampFont.scale font_set\n"); 

	str = euc_to_octal(timestamp);
	/* print timestamp at left margin */ 
	fprintf (fp, "LM y1 font.y sub (%s) left_text\n", str); 

	/* print user@host stamp at right margin */ 
	fprintf (fp, "RM y1 font.y sub (%s) right_text\n", calendar->view->current_calendar); 

       	str = catgets(c->DT_catd, 1, 476, "of");
        nonascii_string(str);
        cm_strcpy(str2, catgets(c->DT_catd, 1, 489, "   Page"));
        cm_strcpy(str2, euc_to_octal(str2));
        fprintf (fp, "LM BM font.y sub (%s %d %s %d) left_text \n",
                        str2, num_page, str, total_pages);

	/* print creation notice at bottom */ 
	str = catgets(c->DT_catd, 1, 490, "Week view by Calendar Manager");
	nonascii_string(str);
	str = euc_to_octal(str);
	fprintf (fp, "RM BM font.y sub (%s) right_text \n", str); 

	/* save current position of y1 */
	fprintf (fp, "CR\n");
	fprintf (fp, "/off_y y1 2 sub def\n");

	/* determine box height (takes into account size of month font) */
	fprintf (fp, "/boxh off_y BM sub 2 div def\n");
}


extern void
ps_week_appt_boxes(FILE *fp)
{
	fprintf (fp, "\n%% --- print week appt boxes ---\n");

	/* save to place box around weekday string */
	fprintf (fp, "/tab_y DayFont.scale 1.5 mul def\n");

	/* print gray background then bold weekdays box */
	fprintf (fp, "/line_width 2 def\n");
	fprintf (fp, "/x2 LM def\n");
	fprintf (fp, "/y2 off_y boxh sub def\n");
	fprintf (fp, "x2 off_y tab_y sub boxw 5 mul tab_y make_lt_gray\n");
	fprintf (fp, "x2 y2 boxw 5 mul boxh line_width draw_box\n");

	/* print gray background then weekends box */
	fprintf (fp, "/x2 LM boxw 3 mul add def\n");
	fprintf (fp, "/y2 off_y boxh 2 mul sub def\n");
	fprintf (fp, "x2 y2 boxh add tab_y sub boxw 2 mul tab_y make_lt_gray\n");
	fprintf (fp, "x2 y2 boxw 2 mul boxh line_width draw_box\n");

	/* print internal lines for weekday box */
	fprintf (fp, "/x2 LM def\n");
	fprintf (fp, "/y2 off_y boxh sub def\n");
	fprintf (fp, "/line_width 1 def\n");
	fprintf (fp, "5 {\n");
	fprintf (fp, "    x2 y2 boxw boxh line_width draw_box\n");
	fprintf (fp, "    /x2 x2 boxw add def\n");
	fprintf (fp, "} repeat\n");

	/* print horizontal internal line for weekday box */
	fprintf (fp, "LM off_y tab_y sub moveto\n");
	fprintf (fp, "RM off_y tab_y sub lineto\n");
	fprintf (fp, "stroke\n");

	/* print internal lines for weekend box */
	fprintf (fp, "/x2 LM boxw 3 mul add def\n");
	fprintf (fp, "/y2 off_y boxh 2 mul sub def\n");
	fprintf (fp, "2 {\n");
	fprintf (fp, "    x2 y2 boxw boxh line_width draw_box\n");
	fprintf (fp, "    /x2 x2 boxw add def\n");
	fprintf (fp, "} repeat\n");

	/* print horizontal internal line for weekend box */
	fprintf (fp, "LM boxw 3 mul add off_y boxh sub tab_y sub moveto\n");
	fprintf (fp, "RM off_y boxh sub tab_y sub lineto\n");
	fprintf (fp, "stroke\n");
}

extern void
ps_week_sched_boxes(FILE *fp)
{
	char *day_of_week[7];
	int malloc_memory = 0;
	int k;
	Props *p = (Props *)calendar->properties;
	int begin = get_int_prop(p, CP_DAYBEGIN);
	int end = get_int_prop(p, CP_DAYEND);
	DisplayType dt = get_int_prop(p, CP_DEFAULTDISP);
	int reset_font = 0;
	int begin_hr = 0;
	Calendar *c = calendar;

	fprintf (fp, "\n%% --- print week sched boxes ---\n");

	/* figure out number of partitions in sched box */
	num_hours = end - begin;
	/* determine offset of sched box from left margin */
	fprintf (fp, "DayFont DayFont.scale font_set\n");
	fprintf (fp, "/tab (12) stringwidth pop def\n");
	fprintf (fp, "/x2 LM tab add def\n");
	fprintf (fp, "/y2 off_y boxh sub DayFont.scale 2 mul sub def\n");
	fprintf (fp, "/off_y1 y2 5 sub def\n");

	/* divide by number of partitions in schedule box */
	fprintf (fp, "/s_boxh off_y1 BM sub %d div def\n", num_hours);
	fprintf (fp, "/s_boxw 3 boxw mul tab sub tab sub 7 div def\n");	

	local_dayname(&day_of_week[0], catgets(c->DT_catd, 1, 491, "M"));
	local_dayname(&day_of_week[1], catgets(c->DT_catd, 1, 492, "T"));
	local_dayname(&day_of_week[2], catgets(c->DT_catd, 1, 493, "W"));
	local_dayname(&day_of_week[3], catgets(c->DT_catd, 1, 494, "T"));
	local_dayname(&day_of_week[4], catgets(c->DT_catd, 1, 495, "F"));
	local_dayname(&day_of_week[5], catgets(c->DT_catd, 1, 496, "S"));
	local_dayname(&day_of_week[6], catgets(c->DT_catd, 1, 497, "S"));

	malloc_memory = 1;

	/* print abbreviated weekdays on top */
	fprintf (fp, "[(%s) (%s) (%s) (%s) (%s) (%s) (%s)] {\n", day_of_week[0],
			 day_of_week[1], day_of_week[2], day_of_week[3], day_of_week[4],
			 day_of_week[5], day_of_week[6]); 
	fprintf (fp, "    x2 y2 x2 s_boxw add 4 -1 roll ctr_text\n");
	fprintf (fp, "    /x2 x2 s_boxw add def\n");
	fprintf (fp, "} forall\n"); 

	/* print times at left of box */
	fprintf (fp, "/x2 LM tab add 3 sub def\n");
	fprintf (fp, "/y2 off_y1 def\n");
	/* adjust font scale according to number of partitions in sched box */
	if ( num_hours > 12  &&  num_hours <= 15 ) {
		reset_font = 1;
		fprintf (fp, "/DayFont.scale.old DayFont.scale def\n");
		fprintf (fp, "/DayFont.scale 10 def\n");
		fprintf (fp, "DayFont DayFont.scale font_set\n");
	} else if ( num_hours > 15 ) {
		reset_font = 1;
		fprintf (fp, "/DayFont.scale.old DayFont.scale def\n");
		fprintf (fp, "/DayFont.scale 9 def\n");
		fprintf (fp, "DayFont DayFont.scale font_set\n");
	}
	/* set up the hour(s) array */
	fprintf (fp, "[");
	begin_hr = begin;
	for ( k = 0;  k <= num_hours;  k++ ) {
		/* 24 hr clock -- 24th hr = 0 */
		if ( dt == HOUR12  &&  begin_hr > 12 ) {
			begin_hr = 1;
		} else if ( dt == HOUR24  &&  begin_hr > 24 ) {
			begin_hr = 1;
		}
		fprintf (fp, "(%d)", begin_hr++);
	}
	fprintf (fp, "] {\n");
	fprintf (fp, "    x2 y2 3 -1 roll right_text\n");
	fprintf (fp, "    /y2 y2 s_boxh sub def\n");
	fprintf (fp, "} forall\n"); 
	if ( reset_font ) {    /* reset font scale to old value */
		fprintf (fp, "/DayFont.scale DayFont.scale.old def\n");
		fprintf (fp, "DayFont DayFont.scale font_set\n");
		reset_font = 0;
	}

	/* print horizontal internal lines (1/2 linewidth) */
	fprintf (fp, "BLACK setgray\n");
	fprintf (fp, "0.25 setlinewidth\n");
	fprintf (fp, "/x2 LM tab add def\n");
	fprintf (fp, "/y2 BM s_boxh add def\n");
	fprintf (fp, "%d {\n", num_hours - 1);   /* number of lines drawn */
	fprintf (fp, "    x2 y2 moveto\n");
	fprintf (fp, "    s_boxw 7 mul 0 rlineto\n");
	fprintf (fp, "    stroke\n");
	fprintf (fp, "    /y2 y2 s_boxh add def\n");
	fprintf (fp, "} repeat\n");

	/* print vertical internal lines */
	fprintf (fp, "1.0 setlinewidth\n");
	fprintf (fp, "/x2 LM tab add s_boxw add def\n");
	fprintf (fp, "/y2 BM def\n");
	fprintf (fp, "6 {\n");
	fprintf (fp, "    x2 BM moveto\n");
	fprintf (fp, "    x2 off_y1 lineto\n");
	fprintf (fp, "    stroke\n");
	fprintf (fp, "    /x2 x2 s_boxw add def\n");
	fprintf (fp, "} repeat\n");

	/* print bold box */
	fprintf (fp, "/x2 LM tab add def\n");
	fprintf (fp, "/y2 BM def\n");
	fprintf (fp, "/line_width 2 def\n");
	fprintf (fp, "x2 y2 s_boxw 7 mul s_boxh %d mul line_width draw_box\n", num_hours);

	/* set for ps_week_daynames() call (saves repeating code) */
	fprintf (fp, "\n%% --- print weekday strings ---\n");
	fprintf (fp, "/x2 LM def\n");
	fprintf (fp, "/y2 off_y tab_y sub 0.40 DayFont.scale mul add def\n");

	if ( malloc_memory ) {
		for ( k = 0;  k < 7;  k++ ) {
			free((char *)day_of_week[k]);
		}
	}
}

static void
lowercase(char *s) {
    int i, l;
 
    if (s==NULL) return;
    l = cm_strlen(s);
    for(i=0; i<=l; i++) {
        s[i]=isupper((unsigned char)s[i]) ? tolower((unsigned char)s[i]) : s[i];
    }
}

extern void
ps_week_daynames(FILE *fp, char *buf, Boolean more)
{
	char *str;
	char *buf_lower;
	int saturday;
	int sunday;
	Calendar *c = calendar;

	/* STRING_EXTRACTION SUNW_DESKSET_CM_MSG
     * Note that the strings "Sat" and "Sun" begin with a capital letter.
     * Please translate accordingly.  If it is not the local custom to
     * capitalize these strings, then just use lower case.
	 */
    /*  Convert strings to lower case before making comparison so that
     *  the application is not dependent on case sensitive translation.
     */
	buf_lower = (char *)cm_strdup(buf);
	lowercase(buf_lower);
	str = (char *)cm_strdup(catgets(c->DT_catd, 1, 505, "Sat"));
	lowercase(str);
	saturday = (strncmp(buf_lower, str, cm_strlen(str))==0);
	free(str);
	str = (char *)cm_strdup(catgets(c->DT_catd, 1, 506, "Sun"));
	lowercase(str);
	sunday   = (strncmp(buf_lower, str, cm_strlen(str))==0);
	free(buf_lower);
	free(str);

	if (saturday)
	{
		/* reset for weekend line */
		fprintf (fp, "\n%% --- print weekend strings ---\n");
		fprintf (fp, "/x2 LM boxw 3 mul add def\n");
		fprintf (fp, "/y2 y2 boxh sub def\n");
	}

	fprintf (fp, "DayFont DayFont.scale font_set\n");
	str = euc_to_octal(buf);
	fprintf (fp, "x2 y2 x2 boxw add (%s) ctr_text\n", str);
	fprintf (fp, "/x2 x2 boxw add def\n");

	fprintf (fp, "\n%% --- setup for printing appts ---\n");
	fprintf (fp, "/x1 x2 boxw sub def\n");
	if (saturday || sunday) {
		fprintf (fp, "/y1 BM def\n");
		fprintf (fp, "/line1 off_y tab_y sub font.y sub boxh sub def\n");
	}
	else {
		fprintf (fp, "/y1 off_y boxh sub def\n");
		fprintf (fp, "/line1 off_y tab_y sub font.y sub def\n");
	}
    	if (more)
		fprintf(fp, "4 0 rmoveto (%s) show\n", "*");

	fprintf (fp, "/tab1 5 def\n");
	fprintf (fp, "/tab2 tab1 def\n");
}

extern void
ps_week_sched_init()
{
	/* initialize the sched array */
	register int i;

	for (i = 0; i < 96; i++)
		sched_bucket[i] = 0;
}

static void
ps_week_sched_update(appt, p)
	Dtcm_appointment *appt;
	Props *p;
{
	/*
	 * In order to draw appts in the sched box,  all appts for a day are
	 * mapped into a array, and then drawn later.  The array has
	 * 96 elements, and represents the 95,  15 minute segements
	 * available between the day boundaries
	 */
	int begin_t = get_int_prop(p, CP_DAYBEGIN);
	int end_t = get_int_prop(p, CP_DAYEND);
	Tick	tick;
	Tick	end_tick = 0;
	register int i, start, end;
	int end_slider;

	_csa_iso8601_to_tick(appt->time->value->item.date_time_value, &tick);
	if (appt->end_time)
		_csa_iso8601_to_tick(appt->end_time->value->item.\
			date_time_value, &end_tick);

	/*
	 * Determine the last element in the sched_buckets array which
	 * corresponds to this time range.
	 */

	end_slider = (end_t - begin_t) * 4;

	/* 
	 * Figure where the begin and end times should be in the array.
	 */

	start = ((hour(tick) - begin_t) * 4) + (minute(tick) / 15);
	if (end_tick)
		end = start + ((end_tick - tick) * 4 / 3600);
	else
		end = start;

	/*
	 * Make sure that the appointment starts/ends within the visible
	 * time range.
	 */

	if (start < 0) start = 0;
	if (end < 0) end = 0;
	if (start > end_slider) start = end_slider;
	if (end > end_slider) end = end_slider;

	/* 
	 * Only map if some portion of time is between day boundaries.
	 */

	if ((start < 0 && end < 0) || 
	    (start >= end_slider && end >= end_slider))
		return;

	/* 
	 * Mark the blocks of time affected in the array.
	 */

	for (i = start; i < end; i++)
		sched_bucket[i]++;
}

extern void
ps_week_sched_draw(fp, wd)
	FILE *fp;
	int  wd; 	/* 1 = Mon, etc */
{
	/*
	 * Draw the sched appt blocks for the day
	 */
	register int i, last_color=0, curr_color=0, jump;

	fprintf (fp, "\n%% --- filling in sched box for appt ---\n");

	/*
	 * The following calculations assume: 
	 *
	 * 	s_boxw, s_boxh (width and height of sched boxes)
	 * 	off_y1         (y coord of top left corner of * sched box) 
	 *
	 * are defined previously in the program.
	 */

	/*
	 * The way this is done:
         *   Position x,y to the top left corner of the appt grid.
         *   Check sched_buckets to see if we should be drawing anything.
         *   If not (sched_buckets is 0), then just move down one 15 minute
         *     increment on the grid (each element in the sched_buckets
         *     array represents one 15 minute increment).
         *   If sched_buckets is 1 (or 2), then we should begin shading
         *     in the area. Each appt is shaded in, in 15 minute increments.
         *   The shaded area is drawn by having the upper left x,y
         *     coordinate, the width and the height. You'll see in the
         *     postscript that is generated that the height is made negative
         *     since we are drawing from top to bottom, and we always have
         *     the upper left x,y coordinate.
         *   Outline the shaded region just drawn.
         */

	/* determine x coord 
         * NOTE: the 0.5 added to the x coordinate was determined mainly
         * by trial and error. This is definetely not the way this should
         * have been done, but it's too late to do it write and totally
         * rewrite the postscript that is generated.
	 */

	fprintf(fp, "/s_x LM tab add s_boxw %d mul add 0.5 add def\n", wd-1);

	/* determine y coord */
	fprintf(fp, "/s_y off_y1 def\n");

	/* determine width of box (minus borders) 
         * NOTE: Again, the 1.1 subtracted from the box width was
         * determined mainly by trial and error.
	 */
	fprintf(fp, "/s_width s_boxw 1.1 sub def\n");

	/* determine height of 15 minute box */
	fprintf(fp, "/s_height s_boxh 4 div def\n");

	/* 
	 * Fill in appt blocks for the day.  Remember to draw a line if
	 * there is a change in color from the previous draw.  This ends
	 * up in outlining the sched boxes with lines.
	 */
	fprintf (fp, "0.25 setlinewidth\n");
	for (i=0, jump = 0; i < 96; i++, jump++)
	{
		switch (sched_bucket[i])
		{
		case 0 :/* no appts - no appts to draw */
			curr_color = 0;
			break;

		case 1 :/* 1 appt - no time intersections */
			if (jump)
			{
				fprintf(fp, "/s_y s_y s_height %d mul sub def\n", jump);
				jump = 0;
			}
			fprintf(fp, "s_x s_y s_width s_height neg make_gray\n");
			curr_color = 1;
			break;

		default:/* 2 or more appts - time intersects discovered */
			if (jump)
			{
				fprintf(fp, "/s_y s_y s_height %d mul sub def\n", jump);
				jump = 0;
			}
			fprintf(fp, "s_x s_y s_width s_height neg 0.10 setgray rect fill BLACK setgray\n");
			curr_color = 2;
			break;

		}
		if (curr_color != last_color)
		{	/* draw line of demarcation */

			if (jump)
			{
				fprintf(fp, "/s_y s_y s_height %d mul sub def\n", jump);
				jump = 0;
			}
			fprintf (fp, "s_x 1 sub s_y moveto\n");
			fprintf (fp, "s_boxw 0 rlineto\n");
			fprintf (fp, "stroke\n");
			last_color = curr_color;
		}
	}

	fprintf (fp, "%d tidyweek\n", num_hours - 1);
}

/* 
 * 	print standard year routines
 */

extern void
ps_init_std_year (fp)
	FILE *fp;
{

	fprintf (fp, "\n%% --- init for std year ---\n");
	fprintf (fp, "/x1 LM def\n");
	fprintf (fp, "/y1 TM 0.5 inch sub def\n");

	ps_minimonthdefs(fp);

}

extern void
ps_std_year_name (fp, year)
	FILE *fp;
	int  year; 
{
	char *str;
	Calendar *c = calendar;

#ifdef NEVER
	/* print sun logo at left margin */ 
	fprintf (fp, "LM 20 add y1 4 Sunlogo\n"); 
#endif
	fprintf (fp, "StampFont StampFont.scale font_set\n"); 

	/* print timestamp at left margin */ 
	fprintf (fp, "LM 8 add y1 font.y sub (%s) left_text\n", timestamp); 

	/* print user@host stamp at right margin */ 
	fprintf (fp, "RM y1 font.y sub (%s) right_text\n", calendar->view->current_calendar); 

	str = catgets(c->DT_catd, 1, 507, "Year view by Calendar Manager");
	nonascii_string(str);
	str = euc_to_octal(str);

	/* print creation notice at bottom */ 
	fprintf (fp, "RM BM font.y sub (%s) right_text \n", str); 

	/* set scale to 2x, y normal */
	ps_scale(fp, 2.0, 1.0);

	fprintf (fp, "\n%% --- std print month name centered at top ---\n");
	fprintf (fp, "MonthFont MonthFont.scale font_set\n"); 

	/* Since we're scaling 2x in x dim, set LM & RM accordingly */
	fprintf (fp, "LM 2 div y1 RM 2 div (%d) ctr_text \n", year); 
	
	/* Set scaling back */
	ps_scale(fp, 1.0/2.0, 1.0);

	fprintf (fp, "0 font.x 2 div neg translate\n"); 
}

extern void
ps_init_std_month (fp, wid, hgt)
	FILE *fp;
	int  wid, hgt;
{
	fprintf (fp, "\n%d %d initstdmonthcontext\n", wid - 5, hgt - 5);
}

extern void
ps_finish_std_month (FILE *fp)
{
	fprintf (fp, "finishstdmonthcontext\n");
}

extern void
ps_std_month_name (FILE *fp, char *mon) 
{
	char *str;

	nonascii_string(mon);
	str = euc_to_octal(mon);

	fprintf (fp, "(%s) printstdmonthname\n", str);
}

extern void
ps_std_month_weekdays (FILE *fp)
{

	fprintf (fp, "printstdmonthdays\n");
}

extern void
ps_std_month_dates (FILE *fp,
    int  fdom,	/* first day of month, 0 = Sun */
    int  monlen)	/* length of month */
{
	fdom++;
	fprintf (fp, "%d %d printstdmonthdates\n", fdom, monlen);
}



int token_count(char *str)
{
	int count = 0;

	while (*str) {
		if ( *str == '\\' ) {
			switch (*(str+1)) {
				case 'n':
				case 'r':
				case 't':
				case 'b':
				case 'f':
				case '(':
				case ')':
				case '\\':
					count++;	
					break;	
				case '0': case '1': case '2': case '3': case '4': case '5':
				case '6': case '7':
					if (*(str+2) >= '0' && *(str+1) < '8' &&
						*(str+3) >= '0' && *(str+2) < '8' &&
						*(str+3)) {
						count++;
					}
			}
		} else if ( *str == '('  ||  *str == ')' ) {
			count++;
		}
		str++;
	}
	return count;
}


/* Put '\' in front of escape sequences */

char *
fix_ps_string(char *str)
{
	int num_occur;
	char *fixed_str;
	int fixed_str_len = 0;
	int str_len = 0;
	int str_pos = 0;
	int fixed_pos = 0;

	num_occur = token_count(str);
	if (num_occur) {
		str_len = cm_strlen(str);
		fixed_str_len = CHAR_SIZE * str_len + num_occur;
		fixed_str = (char *)malloc(fixed_str_len + 1);
		for (str_pos = 0; str_pos < str_len; str_pos++, fixed_pos++) {
			if ( *(str+str_pos) == '\\' ) {
				switch (*(str+str_pos+1)) { 
					case 'n':
					case 'r':
					case 't':
					case 'b':
					case 'f':
					case '(':
					case ')':
					case '\\':
						*(fixed_str+fixed_pos) = '\\';
						fixed_pos += 1;
						break;	
					case '0': case '1': case '2': case '3': case '4': case '5':
					case '6': case '7':
						if (*(str+str_pos+2) >= '0' && *(str+str_pos+2) < '8' &&
							*(str+str_pos+3) >= '0' && *(str+str_pos+3) < '8' &&
							str_pos + 3 < str_len) {
							*(fixed_str+(fixed_pos++)) = '\\';
							*(fixed_str+(fixed_pos++)) = *(str+(str_pos++));
							*(fixed_str+(fixed_pos++)) = *(str+(str_pos++));
							*(fixed_str+(fixed_pos++)) = *(str+(str_pos++));
						}
						break;
				}
			} else if ( *(str+str_pos) == '(' || *(str+str_pos) == ')' ) {
				/* Prevent mismatch parentheses */
				*(fixed_str+fixed_pos) = '\\';
				fixed_pos += 1;
			}
			*(fixed_str+fixed_pos) = *(str+str_pos);
		}
		*(fixed_str+fixed_pos) = '\0';
		return fixed_str;
	}
	return str;
}


static void
nonascii_string(char *str)
{
	int str_len;
	int k;

	if ( !nonascii_char ) {
		str_len = cm_strlen(str);
		for ( k = 0;  k < str_len;  k++ ) {
			if ( !isascii(str[k]) ) {
				nonascii_char = 1;
				break;
			}
		}
	}
}


extern void
ps_print_list_range(Calendar *c, CSA_sint32 appt_type, int item_data, Tick start_tick, Tick end_tick) {
	char		buf[MAXNAMELEN], buf2[MAXNAMELEN];
	FILE		*fp;
	Props		*p = (Props *)c->properties;
	Glance		glance;
	todo_view_op	todo_view;
	OrderingType	ot = get_int_prop(p, CP_DATEORDERING);
	time_t 		start, end;
        CSA_return_code stat;
        CSA_entry_handle *list;
        CSA_attribute *range_attrs;
	CSA_enum *ops;
        int i;
	CSA_uint32 a_total;
	
	if ((fp = ps_open_file(c, print_from_dialog)) == NULL)
		return;
	if (appt_type ==  CSA_TYPE_TODO) {
		todo_view = (todo_view_op)item_data;
		glance = c->view->glance;
	} else
		glance = (Glance)item_data;

	ps_init_printer(fp, PORTRAIT);
	ps_init_list(fp);

	memset(buf, '\0', MAXNAMELEN);
	memset(buf2, '\0', MAXNAMELEN);

	start = lowerbound(start_tick);
	end = nextday(end_tick) - 1;

	format_date(start + 1, ot, buf2, 1, 0, 0);

	if (appt_type == CSA_TYPE_TODO)
		sprintf(buf, catgets(c->DT_catd, 1, 899, "To Do List Beginning: %s"), buf2);
	else
		sprintf(buf, catgets(c->DT_catd, 1, 900, "Appt List Beginning: %s"), buf2);
	ps_print_header(fp, buf);
	ps_todo_outline(fp, appt_type);

	setup_range(&range_attrs, &ops, &i, start, end, appt_type,
		    NULL, _B_FALSE, c->general->version);
	stat = csa_list_entries(c->cal_handle, i, range_attrs, ops, &a_total, &list, NULL);
	if (stat != CSA_SUCCESS) {
		return;
	}
 
	ps_print_todo(fp, list, a_total, appt_type, glance, todo_view);

	csa_free(list);
	ps_finish_printer (fp);
	fclose(fp);
	ps_print_file(c, print_from_dialog);
}
