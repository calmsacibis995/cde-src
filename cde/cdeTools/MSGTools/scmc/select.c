/* $XConsortium: select.c /main/cde1_maint/1 1995/07/17 16:39:27 drk $ */
static char sccsid[] = "@(#)select.c	1.13  @(#)select.c	1.13 01/20/96 15:41:34";
/*
 * COMPONENT_NAME: (PIITOOLS) AIX PII Tools
 *
 * FUNCTIONS: select
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

#include <cur01.h>
#include <cur02.h>
#include <cur03.h>
#include <sys/signal.h>
#include "select.h"
#define ERROR -1
#define ENTER 10
#define RETURN 10
#define ACCEPT 24
#define END 27
#define CREATE_DELETE 0
#define CREATE_RETURN 1
#define USE_RETURN    2
#define USE_DELETE    3
#define MESSAGE_LEN	1024
#define THE_UNIVERSE_EXISTS 1  /* infinite while loop def. */

/* definitions to convert curses attributes to extended curses attributes */
/* End of definitions */

struct select_struct {
	VID_MODES *sv;
	short int top , 
	bottom , 
	left , 
	right , 
	n_elem ,  
	cur_elem ,
	first_elem ,
	last_elem ,
	delta_elem ,
	elem_width ,
	col_width ,
	col_elems ,
	cols ,
	border,
	msg_row,
	msg_col;
	char boxchar ,
	message[MESSAGE_LEN + 1];

	WINDOW    *selwin;
};

extern long time();
static int playback = 0;

set_playback( i )
int i;
{
	playback = i;
}

xputstat(win,fp,status)
WINDOW *win;
FILE *fp;
int status;
{
int  y, x;
long t, l;
	t = time(&l);
	getyx(win,y,x);
	fprintf(fp,"%d %d %d %ld\n",status,y,x,t);
}

int xgetstat(fp)
FILE *fp;
{
static	int st;
static  long stm, tm = 0;
long t, l, tt;
int  y, x;
	if( tm == 0 ) {
		if(fscanf(fp,"%d %d %d %ld",&st,&y,&x,&tm) != EOF)
			stm = time (&l);
		else 	playback = 0;
	}
	else if(fscanf(fp,"%d %d %d %ld",&st,&y,&x,&t) != EOF) {
		while((t-tm) > ((tt=time(&l))-stm));
	stm = tt;
	tm = t;
	}
	else {
		beep( );
		playback = 0;
	}
	return(st);
}

int xgetch(win)
WINDOW *win;
{
static FILE *fp = NULL;
int status;
	if (playback && fp == NULL)
		{
		if((fp = fopen ("playback",playback == 1? "r":"w"))==NULL)
			playback = 0;
		}
	if ( playback == 1 )
		status = xgetstat(fp);
	else {
		signal(SIGQUIT,SIG_IGN);
		status = wgetch(win);
		if( status == KEY_NEWL )
			status = RETURN;
		if ( playback )
			xputstat(win,fp,status);
	}
	return( status );
}

static 	int	selflag = 0;

int sel_flag()
{
	return(selflag);
}

int select( c_vi , options , def , left , right , top , bottom , cols ,
boxchar , message , msg_row , msg_col , winflag , window , 
set , msg ,file)
VID_MODES *c_vi;
char *options[];           /* Array of pointers to strings , terminated by a--
                              null pointer.----------------------------------*/
int def ,                  /* Default option---------------------------------*/
left ,                     /* Leftmost column of the window------------------*/
right ,                    /* rightmost column of the window-----------------*/
top ,                      /* top row of the window--------------------------*/
bottom ,                   /* bottom column of the window--------------------*/
cols;                      /* Number of columns within the window------------*/
char boxchar,              /* Character to use to draw box-------------------*/
*message;             /* Message to be displayed at bottom of box-------*/
int msg_row;               /* row and column of the message to be displayed -*/
int msg_col;               /* -1 row indicates bottm, -1 col -> centered ----*/
int winflag;               /* window Creation/deletion command flag----------*/
WINDOW **window;           /* Address for window to be returned to-----------*/
int set,                   /* Set number of a help message */
msg;                   /* msg number of a help message */
char file[];               /* name of help message cat file. */
/* A null file name 
                                 (file == NULL || file[0] = NULL)
                                 indicates that there is no help */

/* the options will be truncated if there is not---*/
/* enough width to display them in thier full -----*/
/* width within the width of the window------------*/
{
	int status ;             /* return status----------------------------------*/
	struct select_struct sel_str;

	sel_str.sv = c_vi;
	sel_str.boxchar = boxchar;
	if (boxchar == NULL) {
		sel_str.border = 0;
	}
	else {
		sel_str.border = 1;
	}


	sel_str.msg_row = msg_row;
	sel_str.msg_col = msg_col;

	if ( cols == 0 || cols == -1 ) {
		sel_str.cols = 1;      /* may be replaced with a routine to set the most
		                                   appropriate number of columns */
	}
	else {
		sel_str.cols = cols;
	}

	sel_str.top = top;
	sel_str.left = left;
	sel_str.right = right;
	sel_str.bottom = bottom;

	sel_str.col_width = ( right - left + 1 - sel_str.border * 2) / sel_str.cols;
	sel_str.elem_width = sel_str.col_width - 2;

	sel_str.col_elems = ( bottom  - top ) + 1 - (sel_str.border * 2);
	sel_str.n_elem = sel_str.col_elems * sel_str.cols;
	sel_str.delta_elem = sel_str.n_elem;

	if (message == NULL) {
		strncpy(sel_str.message,"Select an option.",MESSAGE_LEN);
		sel_str.message[MESSAGE_LEN] = NULL;
	}
	else {
		strncpy(sel_str.message,message,MESSAGE_LEN);
		sel_str.message[MESSAGE_LEN] = NULL;
	}
	if (strlen(sel_str.message) > sel_str.right - sel_str.left + 1 - (2 * sel_str.border))
		sel_str.message[sel_str.right - sel_str.left + 1 - (2 * sel_str.border)] = NULL;

	for ( sel_str.last_elem = 0 ;
	    options[sel_str.last_elem] != NULL ;
	    sel_str.last_elem++ ) ;
	/*----------------------------------------------------------------------*/
	/*-----------------------------------Set the first element to include --*/
	/*-----------------------------------the def in the current window--*/
	/*----------------------------------------------------------------------*/
	if (def < 0)
		def = 0;
	if ( def >= sel_str.last_elem ) {
		mvprintw( 23, 0 , "Udef not within array of options." );
		return( ERROR );
	}
	else {
		if ( def >= sel_str.n_elem ) {
			sel_str.first_elem = def - sel_str.n_elem / 2;
		}
		else {
			sel_str.first_elem = 0;
		}
	}
	sel_str.cur_elem = def;
	/*----------------------------------------------------------------------*/
	/*-----------------------------------Set up the window------------------*/
	/*-----------------------------------If one is to be created ...--------*/
	if (winflag == CREATE_DELETE || winflag == CREATE_RETURN) {
		sel_str.selwin = newwin( bottom - top + 1 , right - left + 1 , top , left );

		if (sel_str.selwin == NULL) {
			printf("Unable to create newwin. aborting.");
			return(ERROR);
		}

	}
	/*----------------------------------------------------------------------*/
	/*-----------------------------------Else Use the one passed------------*/
	/*----------------------------------------------------------------------*/

	else if (window != NULL) {
		sel_str.selwin = *window;
	}
	else {
		printf("winflag said use the passed window and it was NULL!!!");
	}

	/*----------------------------------------------------------------------*/
	/*-----------------------------------Loop until Enter is hit------------*/
	/*----------------------------------------------------------------------*/

	status = NULL;
	draw_sel_scr( options , &sel_str );
	while ( THE_UNIVERSE_EXISTS ) {

		/*-----------------------------------Possible entries-------------------*/
		wrefresh( sel_str.selwin );
		status = xgetch( sel_str.selwin );
		clr_err();
		if ( status == KEY_UP ) {
			move_up( options , &sel_str );
		}
		else if ( status == KEY_DOWN ) {
			move_down( options , &sel_str );
		}
		else if ( status == KEY_LEFT ) {
			move_left( options , &sel_str );
		}
		else if ( status == KEY_RIGHT ) {
			move_right( options , &sel_str );
		}
		else if ( status == KEY_PPAGE ) {
			page_up( options , &sel_str );
		}
		else if ( status == KEY_NPAGE ) {
			page_down( options , &sel_str );
		}
		else if ( status == '?' ) {
			if (file != NULL && file[0] != NULL) {
				draw_final_sel_scr(options,&sel_str);
				if (msg == ERROR) {
					help(set,sel_str.cur_elem + 1,file);
				}
				else {
					help(set,msg,file);
				}
				draw_sel_scr(options,&sel_str);
			}
		}
		else if ( status == END ) {
			sel_str.cur_elem = ERROR;
			sel_clup( &sel_str , winflag , window , options);
			return( ERROR );
		}
		else if ( status == RETURN ) {
			selflag = 0;
			sel_clup( &sel_str , winflag , window , options);
			return( sel_str.cur_elem );
		}
		else if ( status == ACCEPT ) {
			selflag = 1;
			sel_clup( &sel_str , winflag , window , options);
			return( sel_str.cur_elem );
		}
		else {
			beep();
		}
	}
}





move_up( options , sel_str )
char *options[];
struct select_struct *sel_str;
{


	if ( sel_str->cur_elem > sel_str->first_elem ) {
		/*-----------------------------------Unhilight the current element -----*/
		waddVchars( f_row( sel_str ) , 
		    f_col( sel_str ) , 
		    sel_str->elem_width , 
		    options[sel_str->cur_elem] , 
		    sel_str , sel_str->sv->cwv);

		sel_str->cur_elem--;
		/*------------------------------Highlight the new currrent element ----*/
		waddVchars( f_row( sel_str ) , 
		    f_col( sel_str ) , 
		    sel_str->elem_width , 
		    options[sel_str->cur_elem] , 
		    sel_str , sel_str->sv->cov);
	}
	else if ( sel_str->cur_elem > 0 ) {
		move_f_elem( sel_str , '-' );
		sel_str->cur_elem--;
		draw_sel_scr( options , sel_str );
	}
	else {
		beep();
	}
}

move_down( options , sel_str )
char *options[];
struct select_struct *sel_str;
{

	if ( sel_str->cur_elem + 1< sel_str->first_elem + sel_str->n_elem  &&
	    sel_str->cur_elem + 1 < sel_str->last_elem ) {
		/*-----------------------------------Unhilight the current element -----*/
		waddVchars( f_row( sel_str ) , 
		    f_col( sel_str ) , 
		    sel_str->elem_width , 
		    options[sel_str->cur_elem] , 
		    sel_str ,sel_str->sv->cwv);

		sel_str->cur_elem++;
		/*------------------------------Highlight the new currrent element ----*/
		waddVchars( f_row( sel_str ) , 
		    f_col( sel_str ) , 
		    sel_str->elem_width , 
		    options[sel_str->cur_elem] , 
		    sel_str ,sel_str->sv->cov);
	}
	else if ( options[sel_str->cur_elem + 1] != NULL ) {
		move_f_elem( sel_str , '+' );
		sel_str->cur_elem++;
		draw_sel_scr( options , sel_str );
	}
	else {
		beep();
	}
}




move_right( options , sel_str )
char *options[];
struct select_struct *sel_str;
{

	/*--------------------------------if new element is on the same screen--*/
	if ( sel_str->cur_elem + 1 < sel_str->first_elem + sel_str->n_elem &&
	    sel_str->cur_elem + 1 < sel_str->last_elem )  {
		/*-----------------------------------Unhilight the current element -----*/
		waddVchars( f_row( sel_str ) , 
		    f_col( sel_str ) , 
		    sel_str->elem_width , 
		    options[sel_str->cur_elem] , 
		    sel_str ,sel_str->sv->cwv);

		if ( sel_str->cur_elem + sel_str->col_elems >= sel_str->last_elem ) {
			sel_str->cur_elem = sel_str->last_elem - 1;
			if (sel_str->cur_elem >= sel_str->first_elem + sel_str->n_elem - 1)
				sel_str->cur_elem = sel_str->n_elem - 1;
		}
		else if ( sel_str->cur_elem + sel_str->col_elems >= 
		    sel_str->first_elem + sel_str->n_elem ) {
			sel_str->cur_elem = sel_str->first_elem + sel_str->n_elem - 1;
		}
		else {
			sel_str->cur_elem += sel_str->col_elems;
		}


		/*------------------------------Highlight the new currrent element ----*/
		waddVchars( f_row( sel_str ) , 
		    f_col( sel_str ) , 
		    sel_str->elem_width , 
		    options[sel_str->cur_elem] , 
		    sel_str ,sel_str->sv->cov);
	}
	else if ( sel_str->cur_elem < sel_str->last_elem - 1 ) {
		move_f_elem( sel_str , '+' );
		if ( sel_str->cur_elem + sel_str->col_elems < sel_str->last_elem ) {
			sel_str->cur_elem += sel_str->col_elems;
		}
		else {
			sel_str->cur_elem = sel_str->last_elem - 1;
		}
		draw_sel_scr( options , sel_str );
	}
	else {
		beep();
	}
}



move_left( options , sel_str )
char *options[];
struct select_struct *sel_str;
{

	/*--------------------------------if new element is on the same screen--*/
	if ( sel_str->cur_elem > sel_str->first_elem ) {
		waddVchars( f_row( sel_str ) , 
		    f_col( sel_str ) , 
		    sel_str->elem_width , 
		    options[sel_str->cur_elem]  , 
		    sel_str ,sel_str->sv->cwv);

		if ( sel_str->cur_elem - sel_str->col_elems >= sel_str->first_elem ) {
			sel_str->cur_elem -= sel_str->col_elems;
		}
		else {
			sel_str->cur_elem = sel_str->first_elem;
		}


		/*------------------------------Highlight the new currrent element ----*/
		waddVchars( f_row( sel_str ) , 
		    f_col( sel_str ) , 
		    sel_str->elem_width , 
		    options[sel_str->cur_elem]  , 
		    sel_str ,sel_str->sv->cov);
	}
	else if ( sel_str->cur_elem > 0 ) {
		move_f_elem( sel_str , '-' );
		sel_str->cur_elem = sel_str->first_elem;
		draw_sel_scr( options , sel_str );
	}
	else {
		beep();
	}
}

page_down( options , sel_str )
char *options[];
struct select_struct *sel_str;
{
	if ( sel_str->first_elem + sel_str->n_elem < sel_str->last_elem ) {
		move_f_elem( sel_str , '+' );
		sel_str->cur_elem = sel_str->first_elem;
		draw_sel_scr( options , sel_str );
	}
	else {
		beep();
	}
}


page_up( options , sel_str )
char *options[];
struct select_struct *sel_str;
{
	if ( sel_str->first_elem > 0 ) {
		move_f_elem( sel_str , '-' );
		sel_str->cur_elem = sel_str->first_elem;
		draw_sel_scr( options , sel_str );
	}
	else {
		beep();
	}
}



static  int  super = 0;

set_super( i )
int i;
{
	super = i;
}


draw_box(win)
WINDOW *win;
{
	if ( super )
		superbox( win, 0, 0, win->_maxy, win->_maxx,
		(NLSCHAR)'|', (NLSCHAR)'-',
		(NLSCHAR)'+', (NLSCHAR)'+',
		(NLSCHAR)'+', (NLSCHAR)'+');
	else
		cbox( win ); 
}



draw_sel_scr( options , sel_str )
char *options[];
struct select_struct *sel_str;
{
	int i , j , k;
	int row , 
	col;


	werase(sel_str->selwin);

	if (sel_str->boxchar != NULL)  {
		draw_box( sel_str->selwin ); 
		if (sel_str->msg_row < 0)
			row = sel_str->bottom - sel_str->top;
		else 
			row = sel_str->msg_row;
		if (sel_str->msg_col < 0) {
			col = (sel_str->right - sel_str->left - strlen(sel_str->message) ) / 2;
		}
		else 
			col = sel_str->msg_col;
		mvwaddstr(sel_str->selwin, row, col, sel_str->message);
	}


	for (i = 0 ; i < sel_str->bottom - sel_str->top + 1 ; i++) {
		mvwchgat(sel_str->selwin,i,0,sel_str->right - sel_str->left + 1, sel_str->sv->cwv);
	}


	for ( j = 0  , k = sel_str->first_elem ; j < sel_str->cols ; j++ ) {
		col = j * sel_str->col_width + sel_str->border;
		for ( i = 0 ; 
		    i < sel_str->col_elems && options[k] != NULL ; 
		    i++  , k++ ) {
			row = i + sel_str->border;
			if ( sel_str->cur_elem == k ) {
				waddVchars( row , col , sel_str->elem_width , 
				    options[k] , sel_str ,sel_str->sv->cov);
			}
			else {
				waddVchars( row , col , sel_str->elem_width , 
				    options[k] , sel_str ,sel_str->sv->cwv);
			}
		}
		if ( options[k] == NULL )
			break;
	}
	if ( options[k]!=NULL && options[k+1]!=NULL ) {
		row = sel_str->bottom - sel_str->top;
		col = (sel_str->right - sel_str->left - 4 );
		mvwaddstr(sel_str->selwin, row, col, "MORE");
	}
	wmove(sel_str->selwin , f_row( sel_str ) , f_col( sel_str ));
}
f_row( sel_str )
struct select_struct *sel_str;
{
	return( ( ( sel_str->cur_elem - sel_str->first_elem ) % 
	    sel_str->col_elems ) + sel_str->border );
}
f_col( sel_str )
struct select_struct *sel_str;
{
	return( ( ( ( ( sel_str->cur_elem - sel_str->first_elem ) / 
	    sel_str->col_elems ) ) /* #cols --*/
	    * sel_str->col_width ) + sel_str->border );                        /* col width -*/

}

waddVchars( row , col , len , str , sel_str , v_op)
int row , 
col , 
len;
char str[];
struct select_struct *sel_str;
int v_op;    /* video option for element */
{
	char tstr[256];
	int i;

	for (i = 0 ; i < len && str[i] != NULL ; i++) {
		tstr[i] = str[i];
	}
	for (    ; i < len ; i++) {
		tstr[i] = ' ';
	}
	tstr[i] = NULL;
	mvwaddstr( sel_str->selwin , row , col , tstr );
	mvwchgat( sel_str->selwin , row , col , strlen(tstr) , v_op);
	wmove(sel_str->selwin,row,col);


}

move_f_elem( sel_str , dir )
struct select_struct *sel_str;
char dir;
{
	if ( dir == '+' ) {
		if ( sel_str->first_elem + sel_str->n_elem > sel_str->last_elem ) {
			beep();
			return( ERROR );
		}
		else if ( sel_str->first_elem + sel_str->delta_elem > sel_str->last_elem ) {
			sel_str->first_elem = sel_str->last_elem - sel_str->n_elem;
		}
		else {
			sel_str->first_elem += sel_str->delta_elem;
		}
	}
	else if ( dir == '-' ) {
		if ( sel_str->first_elem == 0 ) {
			beep();
			return( ERROR );
		}
		else if ( sel_str->first_elem - sel_str->delta_elem < 0 ) {
			sel_str->first_elem = 0;
		}
		else {
			sel_str->first_elem -= sel_str->delta_elem;
		}
	}
	return( 0 );
}

sel_clup( sel_str , winflag , window , options)
struct select_struct *sel_str;
int winflag;
WINDOW **window;
char *options[];
{
	if (winflag == CREATE_DELETE || winflag == USE_DELETE) {
		werase( sel_str->selwin );
		wrefresh( sel_str->selwin );
		delwin( sel_str->selwin );
		if (winflag == USE_DELETE) {
			*window = NULL;
		}
	}
	else {
		*window = sel_str->selwin;
		draw_final_sel_scr(options , sel_str);
		overwrite(sel_str->selwin , stdscr);
	}
	refresh();
}

draw_final_sel_scr( options , sel_str )
char *options[];
struct select_struct *sel_str;
{
	int i , j , k;
	int row , 
	col;


	werase(sel_str->selwin);

	if (sel_str->boxchar != NULL)  {
		draw_box( sel_str->selwin ); 
		if (sel_str->msg_row < 0)
			row = sel_str->bottom - sel_str->top;
		else 
			row = sel_str->msg_row;
		if (sel_str->msg_col < 0) {
			col = (sel_str->right - sel_str->left - strlen(sel_str->message) ) / 2;
		}
		else 
			col = sel_str->msg_col;
		mvwaddstr(sel_str->selwin, row, col, sel_str->message);
	}
	for (i = 0 ; i < sel_str->bottom - sel_str->top + 1 ; i++) {
		mvwchgat(sel_str->selwin,i,0,sel_str->right - sel_str->left + 1, sel_str->sv->ewv);
	}

	for ( j = 0  , k = sel_str->first_elem ; j < sel_str->cols ; j++ ) {
		col = j * sel_str->col_width + sel_str->border;
		for ( i = 0 ; 
		    i < sel_str->col_elems && options[k] != NULL ; 
		    i++  , k++ ) {
			row = i + sel_str->border;
			if ( sel_str->cur_elem == k ) {
				waddVchars( row , col , sel_str->elem_width , 
				    options[k] , sel_str ,sel_str->sv->eov);
			}
			else {
				waddVchars( row , col , sel_str->elem_width , 
				    options[k] , sel_str , sel_str->sv->ewv);
			}
		}
		if ( options[k] == NULL )
			break;
	}
	wmove(sel_str->selwin , f_row( sel_str ) , f_col( sel_str ));
	wrefresh(sel_str->selwin);
}
