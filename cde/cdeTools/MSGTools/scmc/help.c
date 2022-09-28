/* $XConsortium: help.c /main/cde1_maint/1 1995/07/17 16:38:00 drk $ */
static char sccsid[] = "@(#)help.c	1.13  @(#)help.c	1.13 01/20/96 15:41:19";
/*
 * COMPONENT_NAME: (PIITOOLS) AIX PII Tools
 *
 * FUNCTIONS: help
 *
 * ORIGINS: 27
 *
 * IBM CONFIDENTIAL -- (IBM Confidential Restricted when
 * combined with the aggregated modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1988
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */                                                                   
#include <stdio.h>
#include <cur01.h>
#include <cur02.h>
#include <cur03.h>

#define ERROR -1               /* I hate the extended curses sometimes 
(extended curses redefines ERR to 0)   */
#define ENTER 10
#define RETURN 10
#define ESC 27
#define CREATE_DELETE 0
#define CREATE_RETURN 1
#define USE_RETURN    2 
#define USE_DELETE    3
#define THE_UNIVERSE_EXISTS 1  /* infinite while loop def. */
#define prerr(s)  mvaddstr(23,0,s)
WINDOW *helpbox;
WINDOW *helpwin;













help(set,msg,file)
int set,
msg;
char file[];
{
	int cur_msg = msg;
	int status;

	if (cur_msg == -1)
		cur_msg = 0;

	if (file == NULL || file[0] == NULL) {
		prerr("No help available on this topic.");
		return;
	}
	status = draw_helpwin(set,cur_msg,file);
	if (status == ERROR) {
		prerr("No help available on this topic.");
		touchwin(stdscr);
		refresh();
		return;
	}
	while (THE_UNIVERSE_EXISTS) {
		status = xgetch(stdscr);
		if (msg == -1) {
			if (status == KEY_DOWN) {
				status = draw_helpwin(set,cur_msg + 1 , file);
				if (status == ERROR) {
					prerr("No more help on this topic.");
				}
				else {
					cur_msg++;
				}
			}
			if (status == KEY_UP) {
				if (cur_msg > 0) {
					status = draw_helpwin(set,cur_msg - 1 , file);
					if (status == ERROR) {
						prerr("No more help on this topic.");
					}
					else {
						cur_msg--;
					}
				}
			}
			else if (status == ESC || status == RETURN) {
				touchwin(stdscr);
				refresh();
				return;
			}
		}
		else {
			touchwin(stdscr);
			refresh();
			return;
		}
	}
}











draw_helpwin(set,msg,file)
int set,
msg;
char file[];
{
	char *tm;
	char *p;
	char *NLgetamsg();
	int lines,
	i;

	tm = NLgetamsg(file,set,msg,NULL);
	if (tm == NULL)
		return(ERROR);

	for (lines = 1 , p = tm ; *p != NULL ; p++)
		if (*p == '\n')
			lines++;
	if (lines > 21) {
		prerr("Help message to long.");
		lines = 21;
	}



	helpbox = newwin(lines + 2, 80, 23 - (lines + 2), 0);
	helpwin = newwin(lines, 78, 23 - (lines + 1), 1);

	werase(helpwin);
	draw_box(helpbox);
	mvwaddstr(helpwin,0,0,tm);
	mvwaddstr(helpbox,lines + 1,29,"Press return to exit.");

	for (i = 0 ; i < lines  + 2; i++)
		mvwchgat(helpbox,i,0,80,REVERSE);

	for (i = 0 ; i < lines ; i++)
		mvwchgat(helpwin,i,0,78,REVERSE);

	overwrite(helpwin,helpbox);
	wrefresh(helpbox);

	werase(helpwin);
	werase(helpbox);
	delwin(helpwin);
	delwin(helpbox);
	return( 0 );
}










