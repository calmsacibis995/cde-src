/* $XConsortium: scmc.c /main/cde1_maint/1 1995/07/17 16:38:22 drk $ */
static char sccsid[] = "@(#)scmc.c	1.14  @(#)scmc.c	1.14 01/20/96 15:41:21";
/*
 * COMPONENT_NAME: (PIITOOLS) AIX PII Tools
 *
 * FUNCTIONS: scmc
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
 *                                                                   
 *	LICENSED MATERIAL -- PROGRAM PROPERTY OF IBM
 *	REFER TO COPYRIGHT INSTRUCTIONS G120-2083
 */

/*              include file for message texts          */
#include "scmc_msg.h" 
nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */

/*
 * EXTERNAL PROCEDURES CALLED: select, spell, help
 */


#include "scmc.h"

#define CREATE_DELETE 0
#define CREATE_RETURN 1
#define USE_RETURN    2
#define USE_DELETE    3


/*
 * NAME: main
 *
 * FUNCTION: source code message  convertor main routine
 *
 * RETURN VALUE DESCRIPTION:  returns ERROR if failure
 */

VID_MODES svi;
SPLT *sptbl = NULL;
int nbwds = 0;

long msg_last_t = 0;
int mxlns = 0;


#ifdef	_ANSI
int main(int  argc,char  *argp[],char  *envp[])
#else		/*_ANSI */
int main( argc, argp, envp )
int argc;
char *argp[], 
*envp[];
#endif		/*_ANSI */
{
extern	char* getenv();
	KEYTBL *kt[MXKEYS];
	CVNTBL *ct[MXKEYS];
	char **text;
	int stat;

        scmc_catd = catopen(MF_SCMC,0);

	parse_args( argc, argp );

	if ( !args.batch ) {
		initscr(  );
		signal(2,exit_signal);
		setup_attr(  );
		keypad( TRUE );
		noecho(  );
		extended( TRUE );
		scrollok( stdscr, TRUE );

		svi.eov = UNDERSCORE;
		svi.ewv = NORMAL;
		svi.cov = REVERSE;
		svi.cwv = NORMAL;
	}


	spelload( &sptbl, getenv("BAD_WORDS"), &nbwds );

	stat = mk_tbls( kt, ct );
	if ( stat == ERROR )
		return( ERROR );

	build_file( args.file, &text );

	process_file( text, kt, ct );

	if ( !args.batch ) {
		erase();
		prerr(  catgets(scmc_catd, MS_scmc, E_MSG_0, "Done.                         " ) );
		move( 23, 0 );
		refresh(  );
		endwin(  );
		return( 0 );
	}
	return(NORM);
}






/*
 * NAME:   add_msg
 *
 * FUNCTION:  add a message in the temperary message table
 *
 * RETURN VALUE DESCRIPTION:  void
 */

#ifdef	_ANSI
void add_msg(char  str[],char msg[],char set[],struct file_list  *fls,char  *text[],int  ln)
#else		/*_ANSI */
void add_msg( str, msg, set, fls, text, ln )
char str[], 
msg[], 
set[];
struct file_list *fls;
char *text[];
int ln;
#endif		/*_ANSI */
{
	int i, idx, stat;
	char t[NLTEXTMAX];
	char t1[CLN];
	char *p;

	for ( i = 0 ; i < MX_MSG && fls->msg[i] != NULL ; i++ ) ;
	if ( i == MX_MSG ) {
		exit_err(  catgets(scmc_catd, MS_scmc, E_MSG_1, "Only NLTEXTMAX messages in any one file please" ) );
	}

	fls->msg[i] = all_msg(  );

	strcpy( t, str );
	remove_qts( t );

	/*----------------------------------------------------------------------*/
	/*-------Spell checking routine, Send the output to standard output-----*/
	/*----------------------------------------------------------------------*/
	p = t;
	if (sptbl) {
		while ( p != NULL ) {
			p = spell_check( p, sptbl, nbwds, &idx );
			if ( p != NULL ) {
				if ( args.batch ) {
					cpywd( t1, p );
					fprintf( stderr,  catgets(scmc_catd, MS_scmc, M_MSG_2, "Bad word:  file '%s', set '%s', msg '%s', word '%s'\n") , fls->file, set,
					    msg, t1 );
					while (isalpha(*p))
						p++;
				}
				else {
				stat = replace_bwd( t, p, sptbl, idx, text, ln );
				if (stat == ERROR) {
					while (isalpha(*p))
						p++;
					}
				}
			}
		}
	}


	mk_ed_ar( t, fls->msg[i]->line );

	fls->msg[i]->lenmax = MSG_LEN;
	strcpy( fls->msg[i]->set_name, set );
	fls->msg[i]->set = get_set_no( set, fls->msg );
	strcpy( fls->msg[i]->msg_name, msg );
	fls->msg[i]->msg = get_msg_no( fls->msg );

}







/*
 * NAME:   advance_char 
 *
 * FUNCTION: skip comment goto next character
 *
 * RETURN VALUE DESCRIPTION: void
 */


#ifdef	_ANSI
void advance_char(register char  *text[],register int  *ln,register int  *ch)
#else		/*_ANSI */
void advance_char( text, ln, ch )
register char *text[];
register int *ln;
register int *ch;
#endif		/*_ANSI */
{
	register int i;

	register int skip = 1, 
	stat = NORM;


	while ( skip == 1 && stat == NORM ) {
		if ( text[*ln] != NULL ) {
			skip = 0;
			for ( i = 0 ; i < 5 && ld.qt[i] ; i++ ) {
				if ( text[*ln][*ch] == ld.qt[i] ) {
					stat = mv_ch( text, ln, ch );
					while ( text[*ln][*ch] != ld.qt[i] && stat == NORM )  {
						if ( stat == NORM && text[*ln][*ch] == ld.mta ) {
							stat = mv_ch( text, ln, ch );
						}
						stat = mv_ch( text, ln, ch );
					}
					stat = mv_ch( text, ln, ch );
					skip = 1;
					break;
				}
			}

			if ( memcmp( &text[*ln][*ch], ld.s_com, strlen( ld.s_com ) ) == 0 && stat == NORM ) {
				*ch += strlen( ld.s_com ) - 1;
				stat = mv_ch( text, ln, ch );
				while ( stat == NORM ) {
					if ( strlen( ld.e_com ) <= strlen( &text[*ln][*ch] ) && 
					    memcmp( &text[*ln][*ch], ld.e_com, strlen( ld.e_com ) ) == 0 ) {
						*ch += strlen( ld.e_com ) - 1;
						stat = mv_ch( text, ln, ch );
						break;
					}
					stat = mv_ch( text, ln, ch );
				}
				skip = 1;
			}
		}
	if ( skip == 0 )  stat = mv_ch( text, ln, ch );	
	}
}


/*
 * NAME:  all_CLN
 *
 * FUNCTION: allocate memory for internal tables
 *
 * RETURN VALUE DESCRIPTION: return pointer of address
 */
#ifdef	_ANSI
char *all_CLN(  )
#else		/*_ANSI */
char *all_CLN(  )
#endif		/*_ANSI */
{
	char *tm;
	tm = ( char * )rmalloc( CLN );
	if ( tm == NULL ) {
		exit_err(  catgets(scmc_catd, MS_scmc, E_MSG_3, "Unable to allocate enough memory for internal tables ( rmalloc ), aborting" ) );
	}
	null( tm, sizeof( *tm ) );
	return( tm );
}







/*
 * NAME:  all_c_tbl
 *
 * FUNCTION:  allocate memory for constant_table and initialize space
 *
 * RETURN VALUE DESCRIPTION:  return pointer of address
 */


#ifdef	_ANSI
struct constant_table *all_c_tbl(  )
#else		/*_ANSI */
struct constant_table *all_c_tbl(  )
#endif		/*_ANSI */
{
	struct constant_table *tm;
	tm = ( struct constant_table * ) rmalloc( sizeof( *tm ) );
	if ( tm == NULL ) {
		exit_err(  catgets(scmc_catd, MS_scmc, E_MSG_4, "Unable to allocate memory for constant table, exiting." ) );
	}
	null( tm, sizeof( *tm ) );
	return( tm );
}





/*
 * NAME: all_com()
 *
 * FUNCTION: allocate a block of memory for a comment structure and null
 * the memory just allocated.
 *
 * RETURN VALUE DESCRIPTION:  returns a pointer to the allocated memory  (on failure
 * the routine exits.
 */
#ifdef	_ANSI
struct comment *all_com(  )
#else		/*_ANSI */
struct comment *all_com(  )
#endif		/*_ANSI */
{
	struct comment *tm;
	tm = ( struct comment* ) rmalloc( sizeof( *tm ) );
	return( tm );
}








/*
 * NAME:  all_ct
 *
 * FUNCTION:  allocate memory for convert table and initialize space
 *
 * RETURN VALUE DESCRIPTION: return pointer of address
 */


#ifdef	_ANSI
CVNTBL *all_ct(  )
#else		/*_ANSI */
CVNTBL *all_ct(  )
#endif		/*_ANSI */
{
	CVNTBL *tm;
	tm = ( CVNTBL * )rmalloc( sizeof( CVNTBL ) );
	if ( tm == NULL ) {
		exit_err(  catgets(scmc_catd, MS_scmc, E_MSG_5, "Unable to allocate enough memory for internal tables ( rmalloc ), aborting" ) );
	}
	null( tm, sizeof( *tm ) );
	return( tm );
}








/*
 * NAME:  all_kt
 *
 * FUNCTION:  allocate memory for keywords table and initialize space
 *
 * RETURN VALUE DESCRIPTION:  return pointer of address
 */



#ifdef	_ANSI
KEYTBL *all_kt(  )
#else		/*_ANSI */
KEYTBL *all_kt(  )
#endif		/*_ANSI */
{
	KEYTBL *tm;
	tm = ( KEYTBL * )rmalloc( sizeof( KEYTBL ) );
	if ( tm == NULL ) {
		exit_err(  catgets(scmc_catd, MS_scmc, E_MSG_6, "Unable to allocate enough memory for internal tables ( rmalloc ), aborting" ) );
	}
	null( tm, sizeof( *tm ) );
	return( tm );
}






/*
 * NAME: all_msg
 *
 * FUNCTION: allocate memory for message and initialize space
 *
 * RETURN VALUE DESCRIPTION:  return pointer of address 
 */



#ifdef	_ANSI
struct message *all_msg(  )
#else		/*_ANSI */
struct message *all_msg(  )
#endif		/*_ANSI */
{
	struct message *tm;
	tm = ( struct message * ) rmalloc( sizeof( *tm ) );
	if ( tm == NULL ) {
		exit_err(  catgets(scmc_catd, MS_scmc, E_MSG_7, "Unable to allocate memory for message table, exiting." ) );
	}
	null( (char *)tm, sizeof( *tm ) );
	return( tm );
}












/*
 * NAME:  build_file
 *
 * FUNCTION: load source file into memory text table
 *
 * RETURN VALUE DESCRIPTION: returns ERROR if failure
 */


#ifdef	_ANSI
int build_file(char  *file,char  ***text)
#else		/*_ANSI */
int build_file( file, text )
char *file;
char ***text;
#endif		/*_ANSI */
{
	FILE *f;
	char tln[MSG_LEN];
	int i;

	f = fopen( file, "r" );

	if ( f == NULL ) {
		prerr(  catgets(scmc_catd, MS_scmc, E_MSG_8, "Unable to open input file, aborting" ) );
		return( ERROR );
	}
	
	*text = (char **)rmalloc((mxlns += MSG_LEN) * sizeof(char *));

	for ( i = 0 ; !feof( f ) ; i++ ) {
		if (i == mxlns - 1) {
			*text = (char **)realloc(*text,(mxlns += MSG_LEN) * sizeof(char *));
			if (!*text)
				exit_err( catgets(scmc_catd, MS_scmc, E_MSG_9, "Unable to realloc, file may be to large.") );
		}
		if ( fgets( tln, MSG_LEN, f ) == NULL )
			break;
		(*text)[i] = rmalloc( strlen( tln ) + 1 );
		strcpy( (*text)[i], tln );
	}
	if (!feof(f)) {	
		exit_err( catgets(scmc_catd, MS_scmc, E_MSG_10, "Unable to read input file.") );
	}
	while (i < mxlns)
		(*text)[i++] = NULL;
	fclose( f );
	return( NORM );
}






/*
 * NAME: build a comment 
 *
 * FUNCTION: given a line of text, fill the comment struct with
 *           the appropriate values.
 *
 * RETURN VALUE DESCRIPTION: returns ERROR on failure
 */
#ifdef	_ANSI
int build_comm(char   line[],char  *comm[])
#else		/*_ANSI */
int build_comm( line, comm)
char 	line[];		/*----  the line with macro substitution  ----*/
char	*comm[];	/*----  the comment structure to be filled  ----*/
#endif		/*_ANSI */
{
	char	data[MSG_LEN];	/*----   the temp data array  ----*/
	char	*p;		/*----  pointer into line  ----*/
	int	i;		/*----  Misc counter ----*/

	p = line;
	for ( i = 0 ; *p != NULL ; p++, i++ ) {
		if ( *p == '\\' ) {
			data[i++] = *p++;
			data[i] = *p;
			if (!*p)
				break;
		}
		else {
			data[i] = (*p != '\n' )?*p:' ';
		}
	}
	data[i] = NULL;
	mk_ed_ar( data, comm );
	return( NORM );
}










/*
 * NAME:  build_msg
 *
 * FUNCTION: build message
 *
 * RETURN VALUE DESCRIPTION:  returns ERROR if failure
 */

#ifdef	_ANSI
int build_msg(char  line[],char  line1[],int  set_no,char  set_name[],char  quote,struct message  *msg,int  msg_length)
#else		/*_ANSI */
int build_msg( line, line1, set_no, set_name, quote, msg, msg_length )
char line[];
char line1[];
int set_no;
char set_name[];
char quote;
struct message *msg;
int msg_length;
#endif		/*_ANSI */
{
	char data[MSG_LEN];
	char *p;
	int i;
	int quoted = FALSE;	

	msg->lenmax = msg_length;
	msg->set = set_no;
	msg->msg = atoi( line );
	strncpy( msg->set_name, set_name, CLN );
	for ( i = 0 ; 
	    line1[i] != ' ' && line1[i] != NULL && 
	    line1[i] != TAB && line1[i] != ENTER  && i < CLN ; 
	    i++ )
		msg->msg_name[i] = line1[i];

	p = strpbrk( line, " \t" );

	while ( *p <= ' ' && *p != NULL )
		p++;
	if ( *p == NULL ) {
		return( ERROR );
	}
	else if ( *p == quote ){
		quoted = TRUE;
		p++;
	}

	for ( i = 0 ; i < MSG_LEN && *p != NULL ; p++, i++ ) {
		if (*p == quote && quoted)
			break;
		if ( *p == '\\' ) {
			data[i++] = *p++;
			data[i] = *p;
		}
		else {
			data[i] = *p;
		}
	}
	data[i] = NULL;
	mk_ed_ar( data, msg->line );
	return( NORM );
}











/*
 * NAME:  check_sub
 *
 * FUNCTION: let user conform the substitution  
 *
 * RETURN VALUE DESCRIPTION: returns TRUE if type RETURN key  
 */

#ifdef	_ANSI
int check_sub(char  *text[],KEYTBL  *kt,int  ln,int  ch,char  format[])
#else		/*_ANSI */
int check_sub( text, kt, ln, ch, format )
char *text[];
KEYTBL *kt;
int ln;
int ch;
char format[];
#endif		/*_ANSI */
{
	int st;
	int status = NORM, last;

	last = set_last( text );

	st = ln;

	draw_check_sub( text, st, kt, ln, format );
	while ( status != ERROR ) {
		status = xgetch( stdscr );
		if ( status == '?' ) {
			help( MS_SCMC, MCHKSUB, MF_SCMC );
		}
		if ( status == ESC ) {
			return( ERROR );
		}
		else if ( status == ENTER ) {
			return( NORM );
		}
		else if ( status == KEY_PPAGE ) {
			if ( st >= PGSIZE ) {
				st -= PGSIZE;
			}
			else {
				st = 0;
			}
		}
		else if ( status == KEY_NPAGE ) {
			if ( st + PGSIZE <= last ) {
				st += PGSIZE;
			}
			else {
				st = last;
			}
		}
		else if ( status == KEY_DOWN || status == KEY_RIGHT ) {
			if ( st + 1 <= last ) {
				st++;
			}
		}
		else if ( status == KEY_UP || status == KEY_LEFT ) {
			if ( st > 0 ) {
				st--;
			}
		}
		draw_check_sub( text, st, kt, ln, format );
	}
	return( ERROR );
}










/*
 * NAME:  clr_err
 *
 * FUNCTION:  clear error information on screen
 *
 * RETURN VALUE DESCRIPTION: void
 */



#ifdef	_ANSI
void clr_err(  )
#else		/*_ANSI */
void clr_err(  )
#endif		/*_ANSI */
{
	if ( args.batch )
		return;
	move( 23, 0 );
	clrtoeol(  );
}








/*
 * NAME:  cp_param
 *
 * FUNCTION: find a parameter and copy to parameter list (str)
 *
 * RETURN VALUE DESCRIPTION: void 
 */

#ifdef	_ANSI
void cp_param(char  **base,char  *text[],int  *ln,int  *ch,char  *dlist[],char  p_end[],int  fcn_is_equal)
#else		/*_ANSI */
void cp_param( base, text, ln, ch, dlist, p_end, fcn_is_equal )
char **base;
char *text[];
int *ln, 
*ch;
char *dlist[], 
p_end[];
int	fcn_is_equal;
#endif		/*_ANSI */
{
	int 	i, 
		nlvl[5], 
		qtlvl[5], 
		quote = 0,
		stat = NORM,
		param_size;
	char 	*str;


	for ( i = 0 ; i < 5 ; i++ ) {
		qtlvl[i] = 0;
		nlvl[i] = 0;
	}

	*base = str = rmalloc(NL_TEXTMAX);
	param_size = NLTEXTMAX;


	while ( stat == NORM ) {
		if (str - *base > param_size - 10) {
			int pos = str - *base;
			*base = (char *)realloc(*base,param_size += NL_TEXTMAX);
			str = *base + pos;
		}
		if (text[*ln][*ch] == ld.mta) {
			*( str++ ) = text[*ln][*ch];
			stat = mv_ch( text, ln, ch );
			if (text[*ln][*ch] == ENTER) {
				str--;
			}
			else {
				*( str++ ) = text[*ln][*ch];
			}
		}
		else {
			if (!set_quote_level( qtlvl, &quote, text[*ln][*ch] )) {
				if ( !quote ) {
					if( !set_nest_level( nlvl, text[*ln][*ch] ) ) {
						if ( ( nlvl[0] | nlvl[1] | nlvl[2] | nlvl[3] | nlvl[4] ) == 0 ) {
							for ( i = 0 ; i < CLN && dlist[i] != NULL ; i++ ) {
								if ( strlen( dlist[i] ) <= strlen( &text[*ln][*ch] ) ) {
									if ( memcmp( dlist[i], &text[*ln][*ch], strlen( dlist[i] ) ) == 0 ) {
										break;
									}
								}
							}
							if ( fcn_is_equal || i < CLN && dlist[i] != NULL ) {
								if (!fcn_is_equal) {
									*ch += strlen( dlist[i] ) - 1;
									stat = mv_ch( text, ln, ch );
								}
								*str = NULL;
								break;
							}
							if ( strlen( p_end ) <= strlen( &text[*ln][*ch] ) && 
											!memcmp( &text[*ln][*ch], p_end, strlen( p_end ) ) ) {
								*str = NULL;
								break;
							}
						}
					}
				}
   			}
			*( str++ ) = text[*ln][*ch];
		}
		stat = mv_ch( text, ln, ch );
	}
}








/*
 * NAME:  cpywd
 *
 * FUNCTION:  copy word s2 to word s1
 *
 * RETURN VALUE DESCRIPTION: void 
 */

#ifdef	_ANSI
void cpywd(char  *s1,char   *s2)
#else		/*_ANSI */
void cpywd( s1, s2 )
char *s1, *s2;
#endif		/*_ANSI */
{
	while ( isalpha( *s2 ) )
		*s1++ = *s2++;
	*s1 = NULL;
}








/*
 * NAME:  decap
 *
 * FUNCTION:  decapitalize string s
 *
 * RETURN VALUE DESCRIPTION: void 
 */

#ifdef	_ANSI
void decap(char  *s)
#else		/*_ANSI */
void decap( s )
char *s;
#endif		/*_ANSI */
{
	while( *s ) {
		if( *s >= 'A' && *s <= 'Z' )
			*s += 'a' - 'A';
		s++;
	}
}








/*
 * NAME:  delete_line
 *
 * FUNCTION:  delete a line in text and move the following to forth
 *
 * RETURN VALUE DESCRIPTION:  void 
 */

#ifdef	_ANSI
void delete_line(char  *text[],int  ln)
#else		/*_ANSI */
void delete_line( text, ln )
char *text[];
int ln;
#endif		/*_ANSI */
{
	free( text[ln] );
	while ( ln < mxlns - 1 ) {
		text[ln] = text[ln + 1];
		ln++;
	}
	text[ln] = NULL;
}










/*
 * NAME:  destroy_win
 *
 * FUNCTION: close window and refresh screen 
 *
 * RETURN VALUE DESCRIPTION: void 
 */


#ifdef	_ANSI
void destroy_win(WINDOW  *win)
#else		/*_ANSI */
void destroy_win( win )
WINDOW *win;
#endif		/*_ANSI */
{
	werase( win );
	overwrite( win, stdscr );
	delwin( win );
	refresh(  );
}









/*
 * NAME:  display_text
 *
 * FUNCTION: display text and mark current line
 *
 * RETURN VALUE DESCRIPTION:  void
 */


#ifdef	_ANSI
void display_text(char  *text[],int  ln)
#else		/*_ANSI */
void display_text( text, ln )
char *text[];
int ln;
#endif		/*_ANSI */
{
	int i, j, st;
	int row, col;
	int status = NULL;
	int pglen = PGSIZE + 5;

	for ( j = 0 ; text[j] != NULL ; j++ )
		;
	st = ln;
	if ( st > pglen/2 )
		st = st - pglen/2;
	else 
		st = 0;

	while ( status != ENTER && status != ESC ) {

		erase(  );
		for ( i = st, row = 0 ; row < pglen && text[i] != NULL ; i++ )  {
			for ( col = 0 ; row < pglen && col < strlen( text[i] ) ; col += 80 ) {
				mvwaddchars( stdscr, row, 0, &text[i][col], 80 );
				if ( i == ln ) {
					mvchgat( row, 0, strlen( &text[i][col] ), REVERSE );
				}
				row++;
			}
		}
		mvaddstr( 23, 0,  catgets(scmc_catd, MS_scmc, ECM_MSG_11, "Press Enter to exit." ) );
		refresh(  );
		status = xgetch( stdscr );
		if ( status == KEY_UP ) {
			if ( st > 0 )
				st--;
		}
		else if ( status == KEY_DOWN ) {
			if ( st + pglen + 1 < j )
				st++;
		}
		else if ( status == KEY_NPAGE ) {
			if ( st + pglen + pglen < j )
				st += pglen;
		}
		else if ( status == KEY_PPAGE ) {
			if ( st > pglen )
				st -= pglen;
		}
	}

}









/*
 * NAME:  do_sub
 *
 * FUNCTION: check level and user conformation to do the substitution
 *
 * RETURN VALUE DESCRIPTION:  void
 */

#ifdef	_ANSI
void do_sub(char  *text[],KEYTBL  *kt,CVNTBL  *ct[],int  ln,int ch,struct file_list  fls[])
#else		/*_ANSI */
void do_sub( text, kt, ct, ln, ch, fls )
char *text[];
KEYTBL *kt;
CVNTBL *ct[];
int ln, 
ch;
struct file_list fls[];
#endif		/*_ANSI */
{
	char message[CLN];
	char set[CLN];
	char catd[CLN * 2];
	char tformat[NLTEXTMAX * 2];
	int i, 
	fd, 
	status;

	if (strlen(params[kt->prm]) > NL_TEXTMAX) {
		char t[NL_TEXTMAX];
		sprintf(t,"Parameter to long for substitution at line %d",ln);
		prerr(t);
		return;
	}

	get_set_nme( kt, set );
	i = get_cvm( kt->cvm, ct );
	get_catd_nme( ct[i], catd );
	fd = get_flsfd( ct[i]->file, fls );
	gt_msg_nm( message, set, &fls[fd], kt->pfx );

	strcpy( tformat, ct[i]->format );
	lsubstitute( tformat, "%m", message );
	lsubstitute( tformat, "%s", set );
	lsubstitute( tformat, "%c", catd );
	lsubstitute( tformat, "%p", params[kt->prm] );

	if ( kt->lvl >= args.lvl ) {
		if ( args.batch )  {
			return;
		}
		else {
			status = check_sub( text, kt, ln, ch, tformat );
			if ( status == ERROR )
				return;
		}
	}
	if (!args.batch && args.query_names) {
		do {
			get_name(text,ln,ch,tformat,message);
		} while (val_name(set,message,fls[fd].msg) == ERROR);
		strcpy( tformat, ct[i]->format );
		lsubstitute( tformat, "%m", message );
		lsubstitute( tformat, "%s", set );
		lsubstitute( tformat, "%c", catd );
		lsubstitute( tformat, "%p", params[kt->prm] );
	}

	fls[fd].changed = TRUE;
	add_msg( params[kt->prm], message, set, &fls[fd], text, ln );

	get_param_pos( text, kt, &ln, &ch, kt->prm );
	remove_old_prm( text, kt, ln, ch );
	place_new_prm( text, ln, ch, tformat );


}







/*
 * NAME:  draw_check_sub
 *
 * FUNCTION: draw the screen of checking substitution 
 *
 * RETURN VALUE DESCRIPTION:  void
 */

#ifdef	_ANSI
void draw_check_sub(char  *text[],int  st,KEYTBL  *kt,int  ln,char  format[])
#else		/*_ANSI */
void draw_check_sub( text, st, kt, ln, format )
char *text[];
int st;
KEYTBL *kt;
int ln;
char format[];
#endif		/*_ANSI */
{
	int i;
	int row, col;
	char *p;

	if ( st > PGSIZE/2 )
		st = st - PGSIZE/2;
	else 
		st = 0;

	erase(  );
	for ( i = st, row = 0 ; row < PGSIZE && text[i] != NULL ; i++ )  {
		for ( col = 0 ; row < PGSIZE && col < strlen( text[i] ) ; col += 80 ) {
			mvwaddchars( stdscr, row, 0, &text[i][col], 80 );
			if ( i == ln ) {
				mvchgat( row, 0, strlen( &text[i][col] ), REVERSE );
			}
			row++;
		}
	}

	drawbox( stdscr, PGSIZE, 0, 7, 80 );
	mvprintw( PGSIZE + 1, 1,  catgets(scmc_catd, MS_scmc, ECM_MSG_12, "Old:  ") );
	mvwaddchars( stdscr, PGSIZE+ 1, 6, params[kt->prm], 72 );

	i = strlen( format );
	mvaddstr( PGSIZE + 2, 1,  catgets(scmc_catd, MS_scmc, ECM_MSG_13, "New:  " ) );
	mvwaddchars( stdscr, PGSIZE + 2, 6, format, 72 );
	i -= 72;
	if ( i > 0 ) {
		mvwaddchars( stdscr, PGSIZE + 3, 1, &format[72], 78 );
		i -= 78;
	}
	if ( i > 0 ) {
		mvwaddchars( stdscr, PGSIZE + 4, 1, &format[150], 78 );
	}

	p =  catgets(scmc_catd, MS_scmc, M_MSG_14, "Press Enter to make substitution, ESC to skip.") ;
	mvprintw( PGSIZE + 5, 15, p );
	mvchgat( PGSIZE+5, 15, strlen( p ), REVERSE );
	refresh(  );
}








/*
 * NAME:  draw_get_name
 *
 * FUNCTION: draw the screen of getting message name 
 *
 * RETURN VALUE DESCRIPTION: void 
 */


#ifdef	_ANSI
void draw_get_name(char  *text[],int  st,int  ln,char  format[],char  message[])
#else		/*_ANSI */
void draw_get_name( text, st,  ln, format, message )
char *text[];
int st;
int ln;
char format[];
char message[];
#endif		/*_ANSI */
{
	int i;
	int row, col;
	char *p;

	if ( st > PGSIZE/2 )
		st = st - PGSIZE/2;
	else 
		st = 0;

	erase(  );
	for ( i = st, row = 0 ; row < PGSIZE && text[i] != NULL ; i++ )  {
		for ( col = 0 ; row < PGSIZE && col < strlen( text[i] ) ; col += 80 ) {
			mvwaddchars( stdscr, row, 0, &text[i][col], 80 );
			if ( i == ln ) {
				mvchgat( row, 0, strlen( &text[i][col] ), REVERSE );
			}
			row++;
		}
	}

	drawbox( stdscr, PGSIZE, 0, 7, 80 );

	i = strlen( format );
	mvwaddstr(stdscr,PGSIZE + 1,1, catgets(scmc_catd, MS_scmc, ECM_MSG_15, "Message:") );
	mvwaddstr(stdscr,PGSIZE + 1, 10, message);
	mvwaddchars( stdscr, PGSIZE + 2, 1, format, 78 );
	i -= 78;
	if ( i > 0 ) {
		mvwaddchars( stdscr, PGSIZE + 3, 1, &format[78], 78 );
		i -= 78;
	}
	if ( i > 0 ) {
		mvwaddchars( stdscr, PGSIZE + 4, 1, &format[156], 78 );
	}

	p =  catgets(scmc_catd, MS_scmc, M_MSG_16, "Enter a message_name:") ;
	mvprintw( PGSIZE + 5, 15, p );
	mvchgat( PGSIZE+5, 15, strlen( p ), REVERSE );
	refresh(  );
}








/*
 * NAME:  draw_str
 *
 * FUNCTION:  draw current line and mark illegal word
 *
 * RETURN VALUE DESCRIPTION: void
 */


#ifdef	_ANSI
void draw_str(int  row,int col,char  str[],int  sth,int  nh)
#else		/*_ANSI */
void draw_str( row, col, str, sth, nh )
int row, 
col;
char str[];
int sth;
int nh;
#endif		/*_ANSI */
{
	int i;
	move( row, col );

	for ( i = 0 ; i < sth && str[i] != NULL ; i++ ) {
		addch( str[i] );
		col++;
		if ( col == 79 ) {
			printw( "\n" );
			col = 0;
		}
	}
	standout(  );
	while ( (i < sth + nh) && str[i] != NULL ) {
		addch( str[i++] );
		col++;
		if ( col == 79 ) {
			printw( "\n" );
			col = 0;
		}
	}
	standend(  );
	while ( str[i] != NULL ) {
		addch( str[i++] );
		col++;
		if ( col == 79 ) {
			printw( "\n" );
			col = 0;
		}
	}
}







/*
 * NAME:  examine_quotes
 *
 * FUNCTION:  put a back slash befor the single qoute 
 *
 * RETURN VALUE DESCRIPTION:  void
 */

#ifdef	_ANSI
void examine_quotes(char  *line)
#else		/*_ANSI */
void examine_quotes( line )
char *line;
#endif		/*_ANSI */
{
	int i;

	if ( line == NULL )
		return;
	for ( i = 0 ; i < LINE_LEN && line[i] != NULL ; i++ ) {
		if ( line[i] == '\\' ) {
			if ( i + 1 == LINE_LEN || line[i + 1] == NULL )
				break;
			i++;
			continue;
		}
		else if ( line[i] == '"' ) {
			memmove( &line[i + 1], &line[i], LINE_LEN - ( i + 1 ) );
			line[i] = '\\';
			i++;
		}
	}
}








/*
 * NAME:  exit_eot
 *
 * FUNCTION: exit and show end of text exit information
 *
 * RETURN VALUE DESCRIPTION: void 
 */


#ifdef	_ANSI
void exit_eot(  )
#else		/*_ANSI */
void exit_eot(  )
#endif		/*_ANSI */
{
	exit_err(  catgets(scmc_catd, MS_scmc, E_MSG_20, "An unexpected end of text was encountered, aborting." ) );
}






/*
 * NAME:  exit_err
 *
 * FUNCTION: exit error show and back to shell
 *
 * RETURN VALUE DESCRIPTION: void 
 */

#ifdef	_ANSI
void exit_err(char  str[])
#else		/*_ANSI */
void exit_err( str )
char str[];
#endif		/*_ANSI */
{
	if ( args.batch ) {
		fprintf( stderr, "%s\n", str );
		exit( -1 );
	}
	else {
		prerr( str );
		refresh(  );
		sleep( 2 );
		endwin(  );
		printf( "\n" );
		exit( -1 );
	}
}










/*
 * NAME:  exit_signal
 *
 * FUNCTION: exit and show interrupting exit information
 *
 * RETURN VALUE DESCRIPTION: void 
 */

#ifdef	_ANSI
void exit_signal( )
#else		/*_ANSI */
void exit_signal( )
#endif		/*_ANSI */
{
	exit_err( catgets(scmc_catd, MS_scmc, E_MSG_23, "Signal recieved, aborting") );
}








/*
 * NAME:  find_key
 *
 * FUNCTION: find a keyword in text
 *
 * RETURN VALUE DESCRIPTION:  void
 */

#ifdef	_ANSI
void find_key(register char  *text[],register kEYTBL  *kt[],register int  *kn,register int  *ln,register int  *ch)
#else		/*_ANSI */
void find_key( text, kt, kn, ln, ch )
register char *text[];
register KEYTBL *kt[];
register int *kn;
register int *ln;
register int *ch;
#endif		/*_ANSI */
{
	register int i;
	register char *p;
	register lline;

	if ( !args.batch ) {
		refresh(  );
	}
	wait_msg();
	while ( text[*ln] != NULL ) {
		if (!args.batch && lline != *ln && !(lline % 10)) {
			mvprintw(23,2,"Line:  %d",*ln);
			refresh();
			lline = *ln;
		}
		p = &text[*ln][*ch];
		for ( i = 0 ; kt[i] != NULL ; i++ ) {
			if ( *p == kt[i]->key[0] ) {     /* check first char before spending
			         the time for a memcmp */
				if ( strlen( kt[i]->key ) <= strlen( p ) ) {
					if ( *p == '=' ) {
						if ( !*ch || !strchr("!~^+-*/|&=<>",*(p-1))) {
							*kn = i;
							return;
						}
					}
					else if ( ( !*ch || strchr("({[]});:,!~^+-*/?|&=<>\t ",*(p - 1))) &&
						strchr("({[]});:,!~^+-*/?|&=<>\t ",*(p + strlen(kt[i]->key)))) {
						if ( memcmp( kt[i]->key, p, strlen( kt[i]->key ) ) == 0 ) {
							*kn = i;
							return;
						}
					}
				}
			}
		}
		advance_char( text, ln, ch );
	}
}









/*
 * NAME: find_msg
 *
 * FUNCTION: given set name and message name find index in message table
 *
 * RETURN VALUE DESCRIPTION: if found return index otherwise return ERROR 
 */


#ifdef	_ANSI
int find_msg(struct message  *mtbl[],char  set[],char  msg[])
#else		/*_ANSI */
int find_msg( mtbl, set, msg )
struct message *mtbl[];
char set[];
char msg[];
#endif		/*_ANSI */
{
	int i;
	for ( i = 0 ; i < MX_MSG && mtbl[i] != NULL ; i++ ) {
		if ( !strcmp( mtbl[i]->msg_name, msg ) ) {
			return( i );
		}
	}
	return( ERROR );
}









/*
 * NAME: fr_msglist 
 *
 * FUNCTION:  free message table
 *
 * RETURN VALUE DESCRIPTION: void 
 */



#ifdef	_ANSI
void fr_msglist(struct message  *tbl[])
#else		/*_ANSI */
void fr_msglist( tbl )
struct message *tbl[];
#endif		/*_ANSI */
{
	int i;
	for ( i = 0 ; i < MX_MSG && tbl[i] != NULL ; i++ ) {
		FREE_MSG( tbl[i] );
	}
}









/*
 * NAME:  freemsg
 *
 * FUNCTION: free message structure space 
 *
 * RETURN VALUE DESCRIPTION: void
 */

#ifdef	_ANSI
void freemsg(struct message  *msg)
#else		/*_ANSI */
void freemsg( msg )
struct message *msg;
#endif		/*_ANSI */
{
	int i;
	if ( msg == NULL ) {
		prerr(  catgets(scmc_catd, MS_scmc, E_MSG_25, "Attempt to free a null pointer.  tsk tsk." ) );
		return;
	}
	for ( i = 0 ; i < MX_LINES && msg->line[i] != NULL ; i++ ) {
		free( msg->line[i] );
	}
	free( msg );
}









/*
 * NAME:  get_catd_nme
 *
 * FUNCTION: get the catalog name from convert table  
 *
 * RETURN VALUE DESCRIPTION: void
 */


#ifdef	_ANSI
void get_catd_nme(CVNTBL  *ct,char  catd[])
#else		/*_ANSI */
void get_catd_nme( ct, catd )
CVNTBL *ct;
char catd[];
#endif		/*_ANSI */
{
	strcpy( catd, ct->catd );
}









/*
 * NAME:  get_cvm
 *
 * FUNCTION: compare a conversion method name with covert table
 *
 * RETURN VALUE DESCRIPTION: returns index of convert table 
 */



#ifdef	_ANSI
int get_cvm(char  cvm[],CVNTBL  *ct[])
#else		/*_ANSI */
int get_cvm( cvm, ct )
char cvm[];
CVNTBL *ct[];
#endif		/*_ANSI */
{
	int i;
	for ( i = 0 ; i < MXKEYS  && ct[i] != NULL ; i++ )
		if ( ! strcmp( cvm, ct[i]->name ) )
			return( i );

	exit_err(  catgets(scmc_catd, MS_scmc, E_MSG_26, "Unable to find conversion method name." ) );
	return(ERROR);
}








/*
 * NAME: get_flsfd
 *
 * FUNCTION: find an entry in file list for message file 
 *
 * RETURN VALUE DESCRIPTION: if failure returns ERROR else returns index of
 *                          file list
 */

#ifdef	_ANSI
int get_flsfd(char  file[],struct file_list  fls[])
#else		/*_ANSI */
int get_flsfd( file, fls )
char file[];
struct file_list fls[];
#endif		/*_ANSI */
{
	int i, 
	status;
	char tm[80];
	char t_file[80];

	for ( i = 0 ; i < CLN && fls[i].file[0] != NULL ; i++ ) {
		if ( !strcmp( file, fls[i].file ) ) {
			break;
		}
	}
	if ( i == CLN || fls[i].file[0] == NULL ) {
		if ( i == CLN ) {
			prerr(  catgets(scmc_catd, MS_scmc, E_MSG_27, "Sorry only 30 message files can be updated at one time." ) );
			return( ERROR );
		}
		else {
			strcpy( fls[i].file, file );
			if ( !access( file, F_OK ) ) {   /* if file exists make startoff tables */
				strcpy( tm, file );
				remove_ext( tm );            /* remove extension from file name */
				sprintf( t_file, "%s_msg.h", tm );
				status = mk_const_table( t_file, fls[i].ctbl );
				sprintf( t_file, "%s.msg", tm );
				status = mk_msg_list( t_file, fls[i].msg, fls[i].ctbl, fls[i].com );
				if ( status != ERROR ) {
					fls[i].changed = FALSE;
					return( i );
				}
				else {
					return( ERROR );
				}
			}
			else {
				return( i );
			}
		}
	}
	else {
		return( i );
	}
}










/*
 * NAME:  get_lang_dat
 *
 * FUNCTION: get language discription information
 *
 * RETURN VALUE DESCRIPTION:  void
 */

#ifdef	_ANSI
void get_lang_dat(FILE  *f)
#else		/*_ANSI */
void get_lang_dat( f )
FILE *f;
#endif		/*_ANSI */
{
	char t[NLTEXTMAX];

	gt_ln_dline( f, t );
	sscanf( t, "%d", &ld.cpos );

	gt_ln_dline( f, t );
	strncpy( ld.s_com, t, 20 );

	gt_ln_dline( f, t );
	strncpy( ld.e_com, t, 20 );

	gt_ln_dline( f, t );
	strncpy( ld.qt, t, 20 );

	gt_ln_dline( f, t );
	strncpy( ld.s_nests, t, 20 );

	gt_ln_dline( f, t );
	strncpy( ld.e_nests, t, 20 );

	gt_ln_dline( f, t );
	strncpy( ld.p_start, t, 20 );

	gt_ln_dline( f, t );
	strncpy( ld.p_end, t, 20 );

	gt_ln_dline( f, t );
	ld.mta = t[0];
}







/*
 * NAME:  get_msg_nme
 *
 * FUNCTION: create a message name
 *
 * RETURN VALUE DESCRIPTION: void
 */


#ifdef	_ANSI
void get_msg_nme(char  pfx[],char  message[])
#else		/*_ANSI */
void get_msg_nme( pfx, message )
char pfx[];
char message[];
#endif		/*_ANSI */
{
	static int msg_no;
	sprintf( message, "%sMSG_%d", pfx, msg_no++ );

}










/*
 * NAME: get_msg_no
 *
 * FUNCTION:  get a message number 
 *
 * RETURN VALUE DESCRIPTION: return message number 
 */



#ifdef	_ANSI
int get_msg_no(struct message  *msg[])
#else		/*_ANSI */
int get_msg_no( msg )
struct message *msg[];
#endif		/*_ANSI */
{
	int i, 
	msg_no;

	for ( i = 0, msg_no = 0 ; i < MX_MSG && msg[i] != NULL ; i++ ) {
		if ( msg[i]->msg >= msg_no ) {
			msg_no = msg[i]->msg + 1;
		}
	}
	return( msg_no );
}








/*
 * NAME:  get_name
 *
 * FUNCTION: the control routine of get a message name 
 *
 * RETURN VALUE DESCRIPTION: type RETURN key return NORM 
 *                           type ESC key return ERROR
 */



#ifdef	_ANSI
int get_name(char  *text[],int  ln,int  ch,char  format[],char  message[])
#else		/*_ANSI */
int get_name( text, ln, ch, format, message )
char *text[];
int ln;
int ch;
char format[];
char message[];
#endif		/*_ANSI */
{
	int st;
	int status = ERROR, last;

	last = set_last( text );

	st = ln;

	null(message,21);
	draw_get_name( text, st, ln, format, message );
	while ( status != NORM) {
		status = mvwgetchars(stdscr,PGSIZE + 1, 10, message, 20);
		if ( status == '?' ) {
			help( MS_SCMC, MCHKSUB, MF_SCMC );
		}
		if ( status == ESC ) {
			return( ERROR );
		}
		else if ( status == ENTER ) {
			return( NORM );
		}
		else if ( status == KEY_PPAGE ) {
			if ( st >= PGSIZE ) {
				st -= PGSIZE;
			}
			else {
				st = 0;
			}
		}
		else if ( status == KEY_NPAGE ) {
			if ( st + PGSIZE <= last ) {
				st += PGSIZE;
			}
			else {
				st = last;
			}
		}
		else if ( status == KEY_DOWN || status == KEY_RIGHT ) {
			if ( st + 1 <= last ) {
				st++;
			}
		}
		else if ( status == KEY_UP || status == KEY_LEFT ) {
			if ( st > 0 ) {
				st--;
			}
		}
		draw_get_name( text, st, ln, format, message );
	}
	return( ERROR );
}









/*
 * NAME:  get_params
 *
 * FUNCTION: put each parameter into params  table
 *
 * RETURN VALUE DESCRIPTION: void
 */



char *params[CLN];

#ifdef	_ANSI
void get_params(char  *text[],KEYTBL  *kt,int  ln,int  ch)
#else		/*_ANSI */
void get_params( text, kt, ln, ch )
char *text[];
KEYTBL *kt;
int ln;
int ch;
#endif		/*_ANSI */
{
	int pno, i, stat, fcn_is_equal;


	for ( i = 0 ; i < CLN ; i++)  {
		if (params[i])  {
			free(params[i]);
			params[i] = NULL;
		}
	}

	ch += strlen( kt->key ) - 1;
	stat = mv_ch( text, &ln, &ch );
	if ( stat == ERROR )
		exit_eot(  );

	fcn_is_equal = !strcmp(kt->key,"=");
	if (!fcn_is_equal ) {
		while ( stat != ERROR && memcmp( &text[ln][ch], ld.p_start, strlen( ld.p_start ) ) )
			stat = mv_ch( text, &ln, &ch );
		ch += strlen( ld.p_start ) - 1;
		stat = mv_ch( text, &ln, &ch );
	}


	for ( pno = 0 ; pno < CLN && stat != ERROR; pno++ ) {
		while ( text[ln][ch] <= ' ' )                            /* skip to non blank character */
			stat = mv_ch( text, &ln, &ch );
		cp_param( &params[pno], text, &ln, &ch, kt->ds, ld.p_end, fcn_is_equal );    /* copy paramter until a delim or p_end 
		      update ch & ln also */
		if ( fcn_is_equal || !memcmp( &text[ln][ch], ld.p_end, strlen( ld.p_end ) ) )  /* if at p_end break */
			break;
	}
}











/*
 * NAME: get_param_pos
 *
 * FUNCTION: get parameter position in text
 *
 * RETURN VALUE DESCRIPTION: void
 */


#ifdef	_ANSI
void get_param_pos(char  *text[],KEYTBL  *kt,int  *ln,int  *ch,int  prm)
#else		/*_ANSI */
void get_param_pos( text, kt, ln, ch, prm )
char *text[];
KEYTBL *kt;
int *ln, 
*ch, 
prm;
#endif		/*_ANSI */
{
	char 	*str = NULL;
	int  	pno, 
		stat,
		fcn_is_equal = !strcmp(kt->key,"="); 

	
	*ch += strlen( kt->key ) - 1;
	stat = mv_ch( text, ln, ch );
	if ( stat == ERROR )
		exit_eot(  );
	if (!fcn_is_equal) {
		while ( stat != ERROR && memcmp( &text[*ln][*ch], ld.p_start, strlen( ld.p_start ) ) )
			stat = mv_ch( text, ln, ch );

		*ch += strlen( ld.p_start ) - 1;
		stat = mv_ch( text, ln, ch );
	}

	for ( pno = 0 ; pno < CLN && stat != ERROR ; pno++ ) {
		while ( text[*ln][*ch] <= ' ' )                        /* skip to non blank character */
			stat = mv_ch( text, ln, ch );
		if ( pno == prm )
			break;
		cp_param( &str, text, ln, ch, kt->ds, ld.p_end, fcn_is_equal);    /* copy paramter until a delim or p_end */
		free(str);
		/* update ch & ln also */
		if ( !memcmp( &text[*ln][*ch], ld.p_end, strlen( ld.p_end ) ) )   /* if at p_end break */
			break;
	}
	if ( pno != prm ) {
		printw(  catgets(scmc_catd, MS_scmc, ECM_MSG_31, "Unable to find parameter %d") , prm );
	}
}








/*
 * NAME:  get_set_nme
 *
 * FUNCTION:  get set name 
 *
 * RETURN VALUE DESCRIPTION: void
 */

#ifdef	_ANSI
void get_set_nme(KEYTBL  *kt,char  set[])
#else		/*_ANSI */
void get_set_nme( kt, set )
KEYTBL *kt;
char set[];
#endif		/*_ANSI */
{
	char *p;

	if ( kt->set[0] != NULL )
		strcpy( set, kt->set );
	else {
		sprintf( set, "MS_%s", args.file );
	}


	p = set;
	remove_ext(p);


	p = set;
	while ( *p != NULL ) {
		if (isupper(*p))
			*p = toupper(*p);
		p++;
	}
}








/*
 * NAME: get_set_no 
 *
 * FUNCTION: get set number
 *
 * RETURN VALUE DESCRIPTION:  return set number
 */



#ifdef	_ANSI
int get_set_no(char  set[],struct message  *msg[])
#else		/*_ANSI */
int get_set_no( set, msg )
char set[];
struct message *msg[];
#endif		/*_ANSI */
{
	int i;
	int set_no;

	for ( i = 0 ; i < MX_MSG && msg[i] != NULL ; i++ )
		if ( !strcmp( set, msg[i]->set_name ) )
			break;

	if ( i < MX_MSG && msg[i] != NULL ) {
		return( msg[i]->set );
	}
	else {
		for ( i = 0, set_no = 0 ; i < MX_MSG && msg[i] != NULL ; i++ ) {
			if ( msg[i]->set >= set_no ) {
				set_no = msg[i]->set + 1;
			}
		}
		return( set_no );
	}
}








/*
 * NAME: gt_ln_dline
 *
 * FUNCTION:  get a line from config file ( pass comment line # )
 *
 * RETURN VALUE DESCRIPTION: void
 */




#ifdef	_ANSI
void gt_ln_dline(FILE  *f,char  *t)
#else		/*_ANSI */
void gt_ln_dline( f, t )
FILE *f;
char *t;
#endif		/*_ANSI */
{

	for ( t[0] = '#' ; !feof( f ) && t[0] == '#' ; ) {
		fgets( t, NLTEXTMAX, f );
	}
	if (feof(f)) 
		exit_err( catgets(scmc_catd, MS_scmc, E_MSG_33, "An unexpected eof of cfig file was encountered.") );
	t[strlen( t ) - 1] = NULL;    /* get rid of the line feed */
}










/*
 * NAME: gt_msg_nm
 *
 * FUNCTION: get a unique message name 
 *
 * RETURN VALUE DESCRIPTION:  void
 */

#ifdef	_ANSI
void gt_msg_nm(char  msg[],char set[],struct file_list  *fld,char  pfx[])
#else		/*_ANSI */
void gt_msg_nm( msg, set, fld, pfx )
char msg[], 
set[];
struct file_list *fld;
char pfx[];
#endif		/*_ANSI */
{
	get_msg_nme( pfx, msg );
	while( find_msg( fld->msg, set, msg ) != ERROR )
		get_msg_nme( pfx, msg );
}





/*
 * NAME:  find_num
 *
 * FUNCTION: find a octal number in string  
 *
 * RETURN VALUE DESCRIPTION: return string postion 
 */

#ifdef	_ANSI
char *find_num(char  *s)
#else		/*_ANSI */
char *find_num(s)
char *s;
#endif		/*_ANSI */
{
	while (*s) {
		if (*s == '\\') {
			if (isoct(*(s + 1)))
				return(s);
			else 
				s++;
		}
		s++;
	}
	return(s);
}


/*
 * NAME: insert_octals
 *
 * FUNCTION: if found a octal number in string convert this number into ASCII 
 *
 * RETURN VALUE DESCRIPTION: void
 */

#ifdef	_ANSI
void insert_octals(char  *array[],int  n)
#else		/*_ANSI */
void insert_octals(array,n)
char *array[];
int n;
#endif		/*_ANSI */
{
	int i;
	char *p;
	int val;
	char t_line[LINE_LEN];

	for (i = 0 ; i < n ; i++) {
		p = find_num(array[i]);
		if (!(*p))
			continue;
		sscanf(p + 1,"%o",&val);
		*p = NULL;
		p++;
		while(isoct(*p))
			p++;
		sprintf(t_line,"%s%c%s",array[i],val,p);
		free(array[i]);
		strcpy(array[i] = rmalloc(strlen(t_line) + 1),t_line);
	}
}








/*
 * NAME:  l_edit
 *
 * FUNCTION: one line editor
 *
 * RETURN VALUE DESCRIPTION:  void
 */



#ifdef	_ANSI
void l_edit(int  row,int col,char  str[],int  len,int p)
#else		/*_ANSI */
void l_edit( row, col, str, len, p )
int row, 
col;
char str[];
int len, 
p;
#endif		/*_ANSI */
{
	int ch;

	while ( THE_UNIVERSE_EXISTS ) {
		l_edraw( row, col, str );
		move( row + p / 80, ( p + col > 80 ) ? ( p + col ) % 80 : p + col );
		refresh(  );
		ch = xgetch( stdscr );
		if ( ch == KEY_LEFT && p > 0 ) {
			p--;
		}
		else if ( ch == KEY_RIGHT && p < len - 1 && str[p] != NULL ) {
			p++;
		}
		else if ( ch == KEY_BACKSPACE && p > 0 ) {
			strmove( &str[p - 1], &str[p] );
			p--;
			move( row + p / 80, ( p + col > 80 ) ? ( p + col ) % 80 : p + col );
		}
		else if ( ch >= ' ' && ch <= DEL ) {
			strmove( &str[p + 1], &str[p] );
			str[p++] = ch;
		}
		else if ( ch == ENTER ) {
			return;
		}
		else {
			beep(  );
		}
	}
}









/*
 * NAME:  l_edraw
 *
 * FUNCTION:  one line editor grawing function 
 *
 * RETURN VALUE DESCRIPTION: void 
 */


#ifdef	_ANSI
void l_edraw(int  row,int col,char  *str)
#else		/*_ANSI */
void l_edraw( row, col, str )
int row, 
col;
char *str;
#endif		/*_ANSI */
{

	move( row, col );
	while ( *str != NULL ) {
		addch( *( str++ ) );
		col++;
		if ( col == 79 ) {
			col = 0;
			printw( "\n" );
		}
	}
	addch( ' ' );
}







/*
 * NAME: line_exists
 *
 * FUNCTION:  check line if in text 
 *
 * RETURN VALUE DESCRIPTION: returns TRUE if line is in text already
 */


#ifdef	_ANSI
int line_exists(char  *text[],char  *ln)
#else		/*_ANSI */
int line_exists(text,ln)
char *text[];
char *ln;

#endif		/*_ANSI */
{
	int 	i;
	char	*pat;	/*----  compiled pattern form regcmp  ----*/
	char	*regcmp(),regex();	/*----  standard library functions  ----*/

	pat = regcmp(ln,NULL);
	for (i = 0 ; text[i] ; i++) {
		if (regex(pat,text[i])) 
			break;
	}
	return((int)text[i]);
}









/*
 * NAME:  lsubstitute
 *
 * FUNCTION: substitute value1 in line with value2 
 *
 * RETURN VALUE DESCRIPTION: void
 */

#ifdef	_ANSI
void lsubstitute(char  line[],char  val1[],char  val2[])
#else		/*_ANSI */
void lsubstitute( line, val1, val2 )
char line[];
char val1[];
char val2[];
#endif		/*_ANSI */
{
	int i, j;
	int v1_len, 
	v2_len, 
	ln_len;

	v1_len = strlen( val1 );
	v2_len = strlen( val2 );
	ln_len = strlen( line );

	for ( i = 0 ; i < ln_len ; i++ ) {
		if ( line[i] == val1[0] ) {
			if ( memcmp( &line[i], val1, v1_len ) == 0 ) {
				memmove( &line[i + v2_len], &line[i + v1_len], strlen( &line[i + v1_len] ) + 1 );
				for ( j = 0 ; j < v2_len ; j++ ) {
					line[i + j] = val2[j];
				}
				return;
			}
		}
	}
}








/*
 * NAME: make_cat
 *
 * FUNCTION: output  xxx.msg file 
 *
 * RETURN VALUE DESCRIPTION: returns ERROR if failure 
 */


#ifdef	_ANSI
int make_cat(struct message  *msg[], struct comment *com[], char  file[])
#else		/*_ANSI */
int make_cat( msg, com, file )
struct message *msg[];
struct comment *com[];
char file[];
#endif		/*_ANSI */
{
	long i, j;
	FILE *fp;
	char *ctime(  );
	char *getenv(  );
	int  msg_length = MSG_LEN;
	int  comcount = 0;

	sort_table( msg );

	fp = fopen( file, "w" );
	if ( fp == NULL ) {
		prerr(  catgets(scmc_catd, MS_scmc, E_MSG_36, "Unable to open .msg file for output." ) );
		prerr(  catgets(scmc_catd, MS_scmc, E_MSG_37, "Check for access rights on the file and the directory." ) );
		sleep( 2 );
		return( ERROR );
	}

	time(&i);
	fprintf( fp, "\n$ scmc msg file created %s$ user:  %s\n\n", ctime( &i ), getenv( "LOGNAME" ) );
	fprintf( fp, "$quote \"         define quote character.\n\n" );
	for ( j = 0; j < MX_LINES && com[comcount] != NULL
				  && com[comcount]->comm[j] != NULL; j++ ) {
		if( com[comcount]->comm[j][0] != NULL) {
			fprintf( fp, "$ %s\n", com[comcount]->comm[j]);
		}
	}
	for ( i = 0 ; i < MX_MSG && msg[i] != NULL ; i++ ) {
		if ( i == 0 ) {
			fprintf( fp, "$len \n");
			fprintf( fp, "$set %s        Define initial set#\n\n", msg[i]->set_name );
			comcount++;
			for ( j = 0; j < MX_LINES &&
				     com[comcount] != NULL &&
				     com[comcount]->comm[j] != NULL; j++ ) {
				if( com[comcount]->comm[j][0] != NULL) {
					fprintf( fp, "$ %s\n", com[comcount]->comm[j]);
				}
			}
		}
		else if ( msg[i]->set != msg[i - 1]->set ) {
			fprintf( fp, "\n$set %s        Redefine set#\n\n", msg[i]->set_name );
			comcount++;
			for ( j = 0; j < MX_LINES &&
				   com[comcount] != NULL &&
				   com[comcount]->comm[j] != NULL; j++ ) {
				if( com[comcount]->comm[j][0] != NULL) {
					fprintf( fp, "$ %s\n", com[comcount]->comm[j]);
				}
			}
		}
		if ( msg[i]->lenmax != msg_length ) {
			msg_length = msg[i]->lenmax;
			if ( msg_length == MSG_LEN )
				fprintf ( fp, "$len \n" );
			else
				fprintf ( fp, "$len %d \n",msg_length);
		}
		fprintf( fp, "%-20s \"", msg[i]->msg_name );
		for ( j = 0 ; j < MX_LINES && msg[i]->line[j] != NULL ; j++ ) {
			examine_quotes( msg[i]->line[j] );
			if ( msg[i]->line[j][0] == NULL && 
			    msg[i]->line[j + 1] == NULL && j != 0 )
				continue;
			fprintf( fp, "%s", msg[i]->line[j] );
			if ( j < MX_LINES - 1 && msg[i]->line[j + 1] != NULL ) {
				fprintf( fp, "\\n" );
			}
			if ( msg[i]->line[j + 1] )
			if ( j < MX_LINES - 2 && ( msg[i]->line[j + 1][0] != NULL || msg[i]->line[j + 2] != NULL ) ) {
				fprintf( fp, "\\" );
			}
			if ( j < MX_LINES - 1 && msg[i]->line[j + 1] == NULL )
				fprintf( fp, "\"\n" );
			else if ( j < MX_LINES - 2 && msg[i]->line[j + 1][0] == NULL && msg[i]->line[j+2] == NULL )
				fprintf( fp, "\"\n" );
			else
				fprintf( fp, "\n" );
		}
		for ( j=0; j<MX_LINES && msg[i]->comm[j] != NULL; j++) {
			if( msg[i]->comm[j][0] != NULL ) {
				fprintf( fp, "$ %s\n", msg[i]->comm[j]);
			}
		}
	}
	fclose( fp );
	return( NORM );
}

/*
 * NAME:  mk_const_table
 *
 * FUNCTION: make constance table 
 *
 * RETURN VALUE DESCRIPTION: returns ERROR if failure
 */


#ifdef	_ANSI
int mk_const_table(char  file[],struct constant_table  *tbl[MX_MSG])
#else		/*_ANSI */
int mk_const_table( file, tbl )
char file[];
struct constant_table *tbl[MX_MSG];
#endif		/*_ANSI */
{
	FILE *fp;
	char line[256];
	char *p;
	int cno;
	int i;
	int atoi(  );

	make_defs(file);
	fp = fopen( file, "r" );
	if ( fp == NULL ) {
		sprintf( line,  catgets(scmc_catd, MS_scmc, M_MSG_50, "Warning %s not found.") , file );
		prerr( line );
		tbl[0] = NULL;
		return( ERROR );
	}
	for ( cno = 0 ; cno < MX_MSG && feof( fp ) == 0; cno++ ) {

		line[0] = NULL;

		while( strncmp( "#define", line, 7 ) != 0 && feof( fp ) == 0 ) {
			if ( fgets( line, 255, fp ) == NULL && feof( fp ) == 0 ) {
				prerr(  catgets(scmc_catd, MS_scmc, E_MSG_51, "Unable to read _msg.h file" ) );
				return( ERROR );
			}
		}
		if ( feof( fp ) )
			continue;

		p = strpbrk( line, "    " );
		p++;

		while ( *p == TAB || *p == ' ' )
			p++;

		tbl[cno] = all_c_tbl(  );
		for ( i = 0 ; i < CLN - 1 && *p > ' ' ; i++, p++ )
			tbl[cno]->name[i] = *p;

		tbl[cno]->name[i] = NULL;
		while( *p > ' ' )           /* ==========Advance to the end of the */
			         p++;                     /* ==========word if not there already */

			while ( ( *p <= ' ') && ( *p != NULL ) )   /*==Advance to the beginning of the   */
				            p++;                   /*          ==next word.              */

				if ( *p == NULL ) {     /* ===========if there is no numerical constant */
					free( tbl[cno--] );    /* =========free the memory and decrement cno */
					continue;
				}
		for ( i = 0 ; i < CLN && *p != ENTER && *p != NULL ; i++, p++ ) {
			tbl[cno]->value[i] = *p;
		}
	}
	fclose( fp );
	if ( cno == MX_MSG ) {
		prerr(  catgets(scmc_catd, MS_scmc, E_MSG_52, "Warning Not all constants have been defined. ( too many to define )" ) );
		return( ERROR );
	}
	return( NORM );
}










/*
 * NAME: mk_ed_ar
 *
 * FUNCTION: make array editable
 *
 * RETURN VALUE DESCRIPTION: void
 */


#ifdef	_ANSI
void mk_ed_ar(char  str[],char  *array[])
#else		/*_ANSI */
void mk_ed_ar( str, array )
char str[];
char *array[];
#endif		/*_ANSI */
{
	char *p;
	int row = 0, 
	column = 0;

	while( row < (MX_LINES -1) && array[row] != NULL ) row++;
	array[row] = ( char * ) rmalloc( LINE_LEN * sizeof( char ) );
	if ( array[row] == NULL )
		exit_err(  catgets(scmc_catd, MS_scmc, E_MSG_54, "Unable to get enough memory for edit array." ) );
	null( array[row], LINE_LEN );
	for ( p = str ; *p != NULL && column < LINE_LEN ; p++ ) {
		if ( *p == '\\' && *( p + 1 ) == 'n' ) {
			p++;
			for ( ; column < LINE_LEN ; column++ )
				array[row][column] = NULL;
			column = 0;
			row++;
			if ( row == MX_LINES ) {
				prerr(  catgets(scmc_catd, MS_scmc, E_MSG_55, "The input message had more than 70 lines, and was truncated." ) );
				return;
			}
			array[row] = ( char * )rmalloc( LINE_LEN * sizeof( char ) );
			if ( array[row] == NULL )
				exit_err(  catgets(scmc_catd, MS_scmc, E_MSG_56, "Unable to get enough memory for edit array." ) );
			null( array[row], LINE_LEN );
			continue;
		}
		else if (*p == '\\') {
			array[row][column++] = *p++;
			array[row][column++] = *p;
		}
		else {
			array[row][column++] = *p;
		}

	}

	if ( *p != ENTER ) {
		row++;
	}
	for ( ; row < MX_LINES ; row++ ) {
		array[row] = NULL;
	}
}







/*
 * NAME: mk_msg_list
 *
 * FUNCTION: make message list
 *
 * RETURN VALUE DESCRIPTION: returns ERROR if failure 
 */

#ifdef	_ANSI
int mk_msg_list(char  file[],struct message  *msg[],struct constant_table  *tbl[], struct comment *com[])
#else		/*_ANSI */
int mk_msg_list( file, msg, tbl, com )
char file[];
struct message *msg[];
struct constant_table *tbl[];
struct comment *com[];
#endif		/*_ANSI */
{
	FILE *fp;
	char *line, bline[MSG_LEN], 
	line1[MSG_LEN], 
	set_name[60], 
	quote = NULL, 
	*p;
	int status, 
	com_idx = 0,
	msg_idx = 0, 
	set_no = 0, 
	msg_length = MSG_LEN,
	sm_flag = 0,
	i;


	/*======================================================================
Open the message source file.  Return an error code if non existant.
======================================================================*/
	fp = fopen( file, "r" );
	if ( fp == NULL ) {
		msg[0] = NULL;
		com[0] = NULL;
		return( ERROR );
	}

	com[com_idx] = all_com();
	com[com_idx]->set = set_no;
	while ( feof( fp ) == 0 ) {
		/*======================================================================
Read in a line.
while line terminated in a newline, append next line to end of current.
preprocess the constants into integers.
======================================================================*/
		bline[0] = NULL;
		if ( fgets( bline, MSG_LEN, fp ) == NULL && ( feof( fp ) == 0 ) ) {
			prerr(  catgets(scmc_catd, MS_scmc, E_MSG_57, "Unable to read message file." ) );
			fclose( fp );
			return( ERROR );
		}
		line = &bline[0];
		while ( isspace(*line)) line++;
		while ( strlen( line ) >= 2 && 
		    line[strlen( line ) - 2] == '\\' && 
		    feof( fp ) == 0 ) {
			if ( fgets( &line[strlen( line ) - 2], 
			    MSG_LEN - strlen( line ), fp ) == NULL 
			    && ( feof( fp ) == 0 ) ) {
				prerr(  catgets(scmc_catd, MS_scmc, E_MSG_58, "Unable to read message file." ) );
				fclose( fp );
				return( ERROR );
			}
		}
		strcpy( line1, line );
		pre_process( line, tbl );

		/*======================================================================
Skip comment lines and blank lines.
======================================================================*/
		if ( line[0] == NULL )
			continue;
		else if ( line[0] == '$' && line[1] <= ' ' ) {
			if(sm_flag) 
				build_comm( &line[2], msg[msg_idx-1]->comm);
			else
				build_comm( &line[2],com[com_idx]->comm);

		}

		/*======================================================================
check for kewords ( $quote, $set, $msg, $len, $ comment )
======================================================================*/

		/*======================================================================
$message processing ...
allocate message table entry.
build_msg( line, line1, set, quote, msg[ms], msg_length );
increment current message table index.
======================================================================*/
		if ( line[0] >= '0' && line[0] <= '9' ) {
			msg[msg_idx] = all_msg(  );
			status = build_msg( line, line1, set_no, set_name, quote, 
			    msg[msg_idx], msg_length );
			if ( status == ERROR ) {
				FREE_MSG( msg[msg_idx] );
			}
			else {
				msg_idx++;
				sm_flag = 1;
			}
		}





		/*======================================================================
$len processing
Set message length .
======================================================================*/
		if ( strncmp( "$len", line, 4 ) == 0 ) {
			p = line;
			skip_to_white(p) ;
			skip_to_nwhite(p);
			if ( !*p )
				msg_length = MSG_LEN;
			else {
				if ( atoi( p ) )
					msg_length = atoi( p );
				else {
					prerr( "Invalid length." );
					return (ERROR);
				}
			}
		}

		/*======================================================================
$Quote processing
Set quote character.
======================================================================*/
		if ( strncmp( "$quote", line, 6 ) == 0 ) {
			p = line ;
			skip_to_white(p) ;
			skip_to_nwhite(p);
			if ( !*p ) 
				prerr(  catgets(scmc_catd, MS_scmc, E_MSG_59, "Invalid quote character, ignoring." ) );
			else 
				quote = *p;
		}

		/*======================================================================
$set processing ...
set current set number.
======================================================================*/
		if ( strncmp( "$set", line, 4 ) == 0 ) {
			p = line ;
			skip_to_white(p) ;
			skip_to_nwhite(p);
			if ( !*p ) 
				prerr(  catgets(scmc_catd, MS_scmc, E_MSG_60, "Invalid set number, ignoring." ) );
			else {
				if ( atoi( p ) )
					set_no = atoi( p );
				else {
					prerr( "Invalid set number." );
					return (ERROR);
				}
			}
			p = line1 + 4;
			while ( *p == ' ' || *p == TAB )
				p++;
			if ( *p == NULL || *p == ENTER )
				prerr(  catgets(scmc_catd, MS_scmc, E_MSG_61, "Invalid set number, ignoring." ) );
			for ( i = 0 ; 
			    *p != NULL && *p != ENTER && *p != TAB && *p != ' ' ; 
			    i++, p++ )
				set_name[i] = *p;
			set_name[i] = NULL;
			sm_flag = 0;
			com_idx ++;
			com[com_idx] = all_com();
			com[com_idx]->set = set_no;
		}
	}
	fclose( fp );
	return( NORM );
}











/*
 * NAME: mk_tbls
 *
 * FUNCTION: make key table and coversion table
 *
 * RETURN VALUE DESCRIPTION:  returns ERROR if failure
 */


#ifdef	_ANSI
int mk_tbls(KEYTBL  *kt[],CVNTBL  *ct[])
#else		/*_ANSI */
int mk_tbls( kt, ct )
KEYTBL *kt[];
CVNTBL *ct[];
#endif		/*_ANSI */
{
	int i;
	char *status;
	char line[NLTEXTMAX];
	char *BeginKwd, *BeginSub, *BeginLdf;
	FILE *f;

	BeginKwd = "Begin keywords";
	BeginSub = "Begin sub methods";
	BeginLdf = "Begin language defs";

	f = fopen( args.cfig, "r" );
	if ( f == NULL ) {
		exit_err(  catgets(scmc_catd, MS_scmc, E_MSG_65, "Unable to open config file." ) );
	}

	while ( memcmp( fgets( line, NLTEXTMAX, f ), BeginKwd, strlen( BeginKwd ) ) && !feof( f ) )
		;

	if (feof(f)) {
		exit_err( catgets(scmc_catd, MS_scmc, E_MSG_66, "Enexpected eof of cfig file encountered.") );
	}
	for ( i = 0 ; !feof( f ) && i < MXKEYS ; ) {
		status = fgets( line, NLTEXTMAX, f );
		if ( feof( f ) )
			continue;
		if ( status == NULL ) {
			exit_err(  catgets(scmc_catd, MS_scmc, E_MSG_67, "Unable to read configuration file." ) );
		}

		if ( line[0] == '#' || line[0] == NULL )
			continue;
		if ( !memcmp( line, BeginSub, strlen( BeginSub ) ) )
			break;

		kt[i] = all_kt(  );
		parse_kt_line( kt[i], line );
		i++;
	}
	if (feof(f)) {
		exit_err( catgets(scmc_catd, MS_scmc, E_MSG_68, "Enexpected eof of cfig file encountered.") );
	}
	while (i < MXKEYS)
		kt[i++] = NULL;

	for ( i = 0 ; !feof( f ) && i < MXKEYS ; ) {
		status = fgets( line, NLTEXTMAX, f );
		if ( feof( f ) )
			continue;
		if ( status == NULL ) {
			exit_err(  catgets(scmc_catd, MS_scmc, E_MSG_69, "Unable to read configuration file." ) );
		}

		if ( line[0] == '#' || line[0] == NULL )
			continue;
		if ( !memcmp( line, BeginLdf, strlen( BeginLdf ) ) )
			break;

		ct[i] = all_ct(  );
		parse_ct_line( ct[i], line );
		i++;
	}
	if (feof(f)) {
		exit_err( catgets(scmc_catd, MS_scmc, E_MSG_70, "Enexpected eof of cfig file encountered.") );
	}
	while (i < MXKEYS)
		ct[i++] = NULL;
	get_lang_dat( f );
	i = val_tbls( kt, ct );
	return( i );
}









/*
 * NAME:  mv_ch
 *
 * FUNCTION: move forth current pointer in text 
 *
 * RETURN VALUE DESCRIPTION: returns ERROR if end of text
 */

#ifndef MACRO
#ifdef	_ANSI
int mv_ch(register char  *text[],register int  *ln,register int  *ch)
#else		/*_ANSI */
int mv_ch( text, ln, ch )
register char *text[];
register int *ln;
register int *ch;
#endif		/*_ANSI */
{
	if ( text[*ln] == NULL )
		return( ERROR );
	( *ch )++;
	if ( !text[*ln][*ch] ) {
		if (ld.cpos != -1) {
			while(text[*ln] && 
			    strlen(text[*ln]) >= (ld.cpos + strlen(ld.scom)) &&
			    memcmp(ld.scom,text[*ln][cpos],strlen(ld.scom)) == 0) {
				(*ln)++;
			}
		}

		*ch = 0;
		( *ln )++;
	}
	if ( text[*ln] )
		return( NORM );
	else
		return( ERROR );
}



#endif








/*
 * NAME: mv_end_param
 *
 * FUNCTION: move to end of parameter
 *
 * RETURN VALUE DESCRIPTION:  void
 */

#ifdef	_ANSI
void mv_end_param(char  *text[],int  *ln,int  *ch,char  *dlist[],char  p_end[],int  fcn_is_equal)
#else		/*_ANSI */
void mv_end_param( text, ln, ch, dlist, p_end, fcn_is_equal )
char *text[];
int *ln, 
*ch;
char *dlist[], 
p_end[];
int fcn_is_equal;
#endif		/*_ANSI */
{
	int i;
	int nlvl[5], 
	qtlvl[5], 
	quote = 1, 
	stat = NORM;


	for ( i = 0 ; i < 5 ; i++ ) {
		qtlvl[i] = 0;
		nlvl[i] = 0;
	}

	while ( stat != ERROR ) {
		if (text[*ln][*ch] == ld.mta) {
			stat = mv_ch( text, ln, ch );
		}
		else {
			if ( !set_quote_level( qtlvl, &quote, text[*ln][*ch] )) {
				if ( !quote ) {
					if ( !set_nest_level( nlvl, text[*ln][*ch] ) ) {
						if ( ( nlvl[0] | nlvl[1] | nlvl[2] | nlvl[3] | nlvl[4] ) == 0 ) {
							for ( i = 0 ; i < CLN && dlist[i] != NULL ; i++ )
								if ( strlen( dlist[i] ) <= strlen( &text[*ln][*ch] ) )
									if ( memcmp( dlist[i], &text[*ln][*ch], strlen( dlist[i]) ) == 0 )
										break;
							if ( fcn_is_equal || i < CLN && dlist[i] != NULL ) {
								break;
							}
							if ( !memcmp( &text[*ln][*ch], p_end, strlen( p_end ) ) ) {
								break;
							}
						}
					}
				}
			}
		}
		stat = mv_ch( text, ln, ch );
	}
}










/*
 * NAME:  mvaddchars
 *
 * FUNCTION: draw a string on specified postion
 *
 * RETURN VALUE DESCRIPTION: void
 */

#ifdef	_ANSI
void mvwaddchars(WINDOW  *win,int  row,int  col,char  str[],int  n)
#else		/*_ANSI */
void mvwaddchars( win, row, col, str, n )
WINDOW *win;
int row, col;
char str[];
int n;
#endif		/*_ANSI */
{
	int i;
	wmove( win, row, col );
	for ( i = 0 ; i < n && str[i] != NULL ; i++ )
		if ( str[i] >= ' ' )
			waddch( win, str[i] );
		else if ( str[i] != ENTER )
			waddch( win, '@' );
}









/*
 * NAME: mvwgetchars
 *
 * FUNCTION: line editor
 *
 * RETURN VALUE DESCRIPTION: return NORM if type return
 *			     return ERROR if type ESC
 *			     return control status according control key			
 */

#ifdef	_ANSI
int mvwgetchars(WINDOW  *win,int  line,int col,char  *str,int  len)
#else		/*_ANSI */
int mvwgetchars( win, line, col, str, len )
WINDOW *win;
int line, 
col;
char *str;
int len;
#endif		/*_ANSI */
{
	int status;
	int i = 0;
	wrefresh( win );


	while (str[i] != NULL)
		i++;
	while ( THE_UNIVERSE_EXISTS ) {
		wmove ( win, line, col + i );
		wrefresh( win );
		status = xgetch( win );
		if ( status == ENTER ) {
			return( NORM );
		}
		else if ( status == ESC ) {
			return( ERROR );
		}
		else if ( status == CNTRL_H || status == KEY_BACKSPACE ) {
			if ( i > 0 ) {
				i--;
				mvwaddch( win, line, col + i, ' ' );
				str[i] = NULL;
			}
		}
		else if (status == KEY_UP || status == KEY_DOWN || 
		    status == KEY_RIGHT || status == KEY_LEFT ||
		    status == KEY_NPAGE || status == KEY_PPAGE ||
		    status == '?') {
			return (status);
		}
		else if ( status < ' ' ) {
			beep(  );
		}
		else {
			if ( i < len - 1 ) {
				mvwaddch( win, line, col + i, status );
				str[i++] = status;
				str[i] = NULL;
			}
			else {
				beep(  );
			}
		}
	}
}








/*
 * NAME: nstrcpy 
 *
 * FUNCTION: n bytes string copy
 *
 * RETURN VALUE DESCRIPTION:  void
 */


#ifdef	_ANSI
void nstrcpy(char  *t,char  *s,int  n)
#else		/*_ANSI */
void nstrcpy(t,s,n)
char *t;
char *s;
int n;
#endif		/*_ANSI */
{
	if (strlen(s) < n)
		strcpy(t,s);
	else {
		char t1[256];
		sprintf(t1, catgets(scmc_catd, MS_scmc, M_MSG_71, "String was too long:\n%200s\nExiting.") ,s);
		exit_err(t1);
	}
}










/*
 * NAME: null
 *
 * FUNCTION: fill n NULLs in string 
 *
 * RETURN VALUE DESCRIPTION: void 
 */

#ifdef	_ANSI
void null(register char  s[],register int  n)
#else		/*_ANSI */
void null( s, n )
register char s[];
register int n;
#endif		/*_ANSI */
{
	while ( --n >= 0 )
		s[n] = NULL;
}







/*
 * NAME: parse_args
 *
 * FUNCTION: parse arguments
 *
 * RETURN VALUE DESCRIPTION: void 
 */


#ifdef	_ANSI
void parse_args(int  argc,char  *argp[])
#else		/*_ANSI */
void parse_args( argc, argp )
int argc;
char *argp[];
#endif		/*_ANSI */
{
	int i;
	int batch = FALSE;

	strcpy( args.cfig, "/afs/austin/rs_aix32/usr/lpp/piitools/src/scmc.cfig" );
	
	args.batch = TRUE;
	args.query_names = FALSE;
	args.lvl = 0;
	args.file[0] = NULL;

	for ( i = 1 ; i < argc ; i++ ) {
		if ( strncmp( argp[i], "-f", 2 ) == 0 ) {
			if ( argp[i][2] != NULL )
				strcpy( args.cfig, &argp[i][2] );
			else {
				puts(  catgets(scmc_catd, MS_scmc, M_MSG_73, "-f specified, but no cfig file name followed it." ) );
				break;
			}
		}

		else if ( strcmp( argp[i], "-b" ) == 0 ) {
			batch = TRUE;
		}
		else if ( strcmp( argp[i], "-q" ) == 0 ) {
			args.query_names = TRUE;
		}
		else if ( strncmp( argp[i], "-l", 2 ) == 0 ) {
			if ( isnum( argp[i][2] ) != 0 )
				args.lvl = atol( &argp[i][2] );
			else {
				puts(  catgets(scmc_catd, MS_scmc, M_MSG_74, "A number did not follow the -l<num> command flag" ) );
				break;
			}
		}
		else if (!strcmp(argp[i],"-r")) {
			set_playback ( 1 );
		}
		else if (!strcmp(argp[i],"-w")) {
			set_playback ( -1 );
		}
		else if ( argp[i][0] != '-' ) {
			strcpy( args.file, argp[i] );
		}
		else {
			break;
		}
	}
	if ( i < argc || args.file[0] == NULL ) {
		exit_err(  catgets(scmc_catd, MS_scmc, E_MSG_75, "Usage: scmc [-q] [-f<file>] [-l<num>] [-b] file" ) );
	}
	args.batch = batch;
}



/*
 * NAME:  parse_line
 *
 * FUNCTION:  parsing one line
 *
 * RETURN VALUE DESCRIPTION:  returns ERROR if failure
 */

#include "string.h"
#define iswhite(c)	(isspace(c) |  (c == '\t'))

#ifdef	_ANSI
int parse_line(char  *ln,char  *array[],int  n,char  sep)
#else		/*_ANSI */
int parse_line(ln,array,n,sep)
char *ln;
char *array[];
int n;
char sep;
#endif		/*_ANSI */
{
	int i;
	char *p,*p1;
	char *line;


	line = rmalloc(strlen(ln) + 1);

	strcpy(line,ln);
	

	array[0] = line;
	for (i = 1 , p = line ; *p ; p++) {
		if (*p == '\\') {
			p++;
		}
		else if (*p == sep ) {
			*p = NULL;
			i++;
		}
	}
	if (n == -1) {
		if (i < CLN)
			n = i;
		else {
			free(line);
			return(ERROR);
		}
	}
	if (i != n) {
		free(line);
		return(ERROR);
	}
	else {
		p = line;
		for (i = 0 ; i < n ; i++) {
			while (iswhite(*p))
				p++;
			array[i] = rmalloc(strlen(p) + 1);
			strcpy(array[i],p);
			p += strlen(p) + 1;
			for (p1 = array[i] + strlen(array[i]) - 1 ; iswhite(*p1) && p1 > array[i] ; p1--) 
				*p1 = NULL;

			for (p1 = array[i] ; *p1 ; p1++) {
				if (*p1 == '\\') {
					p1++;
					if (*p1 == sep) {
						strmove(p1 - 1,p1);
					}
				}
			}
			adjust(array[i]);
			
		
		}
		free(line);
		insert_octals(array,n);
		return(n);
	}
}









/*
 * NAME: place_new_prm
 *
 * FUNCTION: place  new parameter in text
 *
 * RETURN VALUE DESCRIPTION: void
 */



#ifdef	_ANSI
void place_new_prm(char  *text[],int  ln,int  ch,char  prm[])
#else		/*_ANSI */
void place_new_prm( text, ln, ch, prm )
char *text[];
int ln;
int ch;
char prm[];
#endif		/*_ANSI */
{
	char ts[MSG_LEN];
	char ts1[MSG_LEN];

	strncpy( ts1, text[ln], ch );
	ts1[ch] = NULL;

	sprintf( ts, "%s %s %s", ts1, prm, &text[ln][ch] );

	free( text[ln] );
	text[ln] = rmalloc( strlen( ts ) + 1 );
	strcpy( text[ln], ts );
}








/*
 * NAME: pre_process
 *
 * FUNCTION: substitute definded name with value
 *
 * RETURN VALUE DESCRIPTION: void 
 */


#ifdef	_ANSI
void pre_process(char  line[],struct constant_table  *tbl[])
#else		/*_ANSI */
void pre_process( line, tbl )
char line[];
struct constant_table *tbl[];
#endif		/*_ANSI */
{
	int i;
	char *p;

	for ( i = 0 ; i < MX_MSG && tbl[i] != NULL ; i++ ) {
		substitute( line, tbl[i]->name, tbl[i]->value );
	}

	for ( p = line ; *p != NULL && *p <= ' ' ; p++ )
		;
	if ( p != line )
		memmove( line, p, strlen( p ) );

}








/*
 * NAME: prerr
 *
 * FUNCTION:  print out error
 *
 * RETURN VALUE DESCRIPTION: void
 */


#ifdef	_ANSI
void prerr(char  *s)
#else		/*_ANSI */
void prerr( s )
char *s;
#endif		/*_ANSI */
{


	if ( args.batch ) {
		fprintf( stderr, "%s\n", s );
	}
	else {
		wait_msg();
		mvaddstr( 23, 0, s );
		refresh(  );
		msg_last_t = time(NULL);
	}
}





/*
 * NAME: wait_msg
 *
 * FUNCTION: wait for a message to be read.
 *
 * RETURN VALUE DESCRIPTION: void
 */
#ifdef	_ANSI
void wait_msg()
#else		/*_ANSI */
void wait_msg()
#endif		/*_ANSI */
{
	if (!args.batch) {
		while (time(NULL) < msg_last_t + 2) {
			sleep(1);
		}
		move(23,0);
		clrtoeol();
	}
}








/*
 * NAME:  process_file 
 *
 * FUNCTION: process text file and save results
 *
 * RETURN VALUE DESCRIPTION: void 
 */



#ifdef	_ANSI
void process_file(char  *text[],KEYTBL  *kt[],CVNTBL  *ct[])
#else		/*_ANSI */
void process_file( text, kt, ct )
char *text[];
KEYTBL *kt[];
CVNTBL *ct[];
#endif		/*_ANSI */
{

	int ln = 0, 
	ch = 0, 
	kn, 
	stat;

	struct file_list fls[CLN];

	null(fls,sizeof(struct file_list) * CLN);

	while ( text[ln] != NULL ) {
		find_key( text, kt, &kn, &ln, &ch );
		if ( text[ln] == NULL )
			continue;

		get_params( text, kt[kn], ln, ch );

		stat = val_sub( text, kt[kn], ln, ch );
		if ( stat == ERROR ) {
			ch++;
			continue;
		}
		do_sub( text, kt[kn], ct, ln, ch, fls );
		ch += strlen( kt[kn]->key );
	}
	save_updates( fls );
	write_file( text, fls );

}






/*
 * NAME: parse_ct_line
 *
 * FUNCTION: parsing line of convert table 
 *
 * RETURN VALUE DESCRIPTION: void 
 */



#ifdef	_ANSI
void parse_ct_line(CVNTBL  *ct,char  line[])
#else		/*_ANSI */
void parse_ct_line( ct, line )
CVNTBL *ct;
char line[];
#endif		/*_ANSI */
{
	char *array[4];
	
	if (parse_line(line,array,4,':') == ERROR) {
		char t[256];
		sprintf(t, catgets(scmc_catd, MS_scmc, M_MSG_78, "Bad conversion table line\n%s\nAborting.") ,line);
		exit_err(line);
	}

	nstrcpy(ct->name,array[0],CLN);
	nstrcpy(ct->format,array[1],256);
	nstrcpy(ct->catd,array[2],CLN);
	if (array[3][0]) {
		nstrcpy(ct->file,array[3],PATH_MAX);
	}
	else {
		char 	t[PATH_MAX];

		if (strlen(args.file) > PATH_MAX)
			exit_err( catgets(scmc_catd, MS_scmc, E_MSG_79, "File name too long.") );
		strcpy(t,args.file);
		remove_ext(t);
		sprintf(ct->file,"%s.msg",t);
	}
}








/*
 * NAME: parse_kt_line
 *
 * FUNCTION: parsing line of key word table
 *
 * RETURN VALUE DESCRIPTION: void 
 */

#ifdef	_ANSI
void parse_kt_line(register kEYTBL  *kt,register char  line[])
#else		/*_ANSI */
void parse_kt_line( kt, line )
register KEYTBL *kt;
register char line[];
#endif		/*_ANSI */
{
	char *array[10];
	char *t_array[CLN];
	register int i, j;

	if (parse_line(line,array,8,':') == ERROR) {
		char t[256];
		sprintf(t, catgets(scmc_catd, MS_scmc, M_MSG_81, "Bad line in key table\n%s\nAborting.\n") ,line);
		exit_err(t);
	}

	strcpy(kt->key,array[0]);
	sscanf(array[1],"%d",&kt->lvl);
	sscanf(array[2],"%d",&kt->prm);

	if ((i = parse_line(array[3],t_array,-1,',')) == ERROR) {
		char t[256];
		sprintf(t, catgets(scmc_catd, MS_scmc, M_MSG_82, "Bad line in key table (too many commas)\n%s\nAborting.\n") ,line);
		exit_err(t);
	}
	for (j = 0 ; j < i ; j++) {
		kt->ls[j] = t_array[j];
	}
	kt->ls[j] = NULL;

	
	if ((i = parse_line(array[4],t_array,-1,',')) == ERROR) {
		char t[256];
		sprintf(t, catgets(scmc_catd, MS_scmc, M_MSG_83, "Bad line in key table (too many commas)\n%s\nAborting.\n") ,line);
		exit_err(t);
	}
	for (j = 0 ; j < i ; j++) {
		kt->ds[j] = t_array[j];
	}
	kt->ds[j] = NULL;

	nstrcpy(kt->pfx,array[5],CLN);
	nstrcpy(kt->set,array[6],CLN);
	nstrcpy(kt->cvm,array[7],CLN);
}






/*
 * NAME: remove_ext
 *
 * FUNCTION: remove extension name
 *
 * RETURN VALUE DESCRIPTION: void 
 */


#ifdef	_ANSI
void remove_ext(char  file[])
#else		/*_ANSI */
void remove_ext( file )
char file[];
#endif		/*_ANSI */
{
	char *p;

	p = file;
	while (*p)
		if (*(p++) == '.')
			break;
	if (*p == NULL)
		return;
	p = file + strlen( file ) - 1;
	while ( p >= file && *p != '.' )
		*( p-- ) = NULL;

	if ( *p == '.' )
		*p = NULL;
}













/*
 * NAME: remove_qts
 *
 * FUNCTION: remove quotations
 *
 * RETURN VALUE DESCRIPTION: void
 */


#ifdef	_ANSI
void remove_qts(char  *s)
#else		/*_ANSI */
void remove_qts( s )
char *s;
#endif		/*_ANSI */
{
	char *p;
	if ( s[0] == '"' ) {
		memmove( s, s+1, strlen( s+1 ) + 1 );

		p = s + strlen( s ) - 1;
		while ( p >= s && *p <= ' ' )
			p--;
		if ( *p == '"' )
			*p = NULL;
	}

}

/*
 * NAME: replace_bwd
 *
 * FUNCTION: replace bad word
 *
 * RETURN VALUE DESCRIPTION: returns ERROR if override
 *			     otherwise returns NORM	
 */
/*______________________________________________________________________
	
Replace bad word [ string, spell table, index to spell table ]

make list of options, 
first n options are the synonyms for the badword, 
option n + 1 is none of the above
option n + 2 is display text
option n + 3 is OVERRIDE
  ______________________________________________________________________*/
#ifdef	_ANSI
int replace_bwd(char  str[],char  *strp,SPLT  spelltbl[],int  idx,char  *text[],int  ln)
#else		/*_ANSI */
int replace_bwd( str, strp, spelltbl, idx, text, ln )
char str[];
char *strp;
SPLT spelltbl[];
int idx;
char *text[];
int ln;
#endif		/*_ANSI */
{
	int n, 
	result, 
	wincmd, 
	len;
	char t[NLTEXTMAX];
	char *os[34];
	WINDOW *win;
	char *p, *p1, 
	tword[80];

	strcpy( t, str );
	p = strp - str + t;
	for  ( p1 = p, len = 0; isalpha( *p1 ) ; p1++ )
		tword[len++] = *p1;
	tword[len] = NULL;

	for ( n = 0 ; spelltbl[idx].syn[n] != NULL ; n++ )
		os[n] = spelltbl[idx].syn[n];
	n--;

	os[n + 1] =  catgets(scmc_catd, MS_scmc, M_MSG_84, "Commands:") ;
	os[n + 2] =  catgets(scmc_catd, MS_scmc, M_MSG_85, "   None of the above") ;
	os[n + 3] =  catgets(scmc_catd, MS_scmc, M_MSG_86, "   Display source code text") ;
	os[n + 4] =  catgets(scmc_catd, MS_scmc, M_MSG_87, "   Override and leave the illegal word unchanged") ;
	os[n + 5] =  catgets(scmc_catd, MS_scmc, M_MSG_88, "   Edit the current version") ;
	os[n + 6] = NULL;

	result = 0;
	wincmd = CREATE_RETURN;
	while ( result != ERROR ) {
		erase(  );
		mvprintw( 0, 0,  catgets(scmc_catd, MS_scmc, ECM_MSG_89, "The highlighted word is illegal:" ) );
		draw_str( 1, 0, t, p - t, len );
		refresh(  );
		result = select( &svi, os, result, 0, 79, 8, 22, 1, 
		    ' ',  catgets(scmc_catd, MS_scmc, M_MSG_90, "Select new phrase or command ( Esc to accept and edit )") , 0, 0, wincmd,
		    &win, 0, 0, MF_SCMC );
		wincmd = USE_RETURN;
		if ( result == ERROR )
		 	break;
		else if ( result <= n ) {
			strcpy( t, str );
			lsubstitute( t, tword, os[result] );
			*p = isupper(tword[0]) ? toupper(*p): tolower(*p);
			len = strlen( os[result] );
		}
		else if ( result == ( n + 1 ) ) {
			beep(  );
		}
		else if ( result == ( n + 2 ) ) {
			strcpy( t, str );
			lsubstitute( t, tword, "" );
			len = 0;
		}
		else if ( result == ( n + 3 ) ) {
			display_text( text, ln );
		}
		else if ( result == ( n + 4 ) ) {
			destroy_win( win );
			return( ERROR );
		}
		else if ( result == ( n + 5 ) ) {
			break;
		}
	}
	l_edit( 1, 0, t, NLTEXTMAX, p - t );
	destroy_win( win );
	strcpy( str, t );
	return( NORM );
}








/*
 * NAME: remove_old_prm 
 *
 * FUNCTION: remove old parameter
 *
 * RETURN VALUE DESCRIPTION: void 
 */



#ifdef	_ANSI
void remove_old_prm(char  *text[],KEYTBL  *kt,int  ln,int ch)
#else		/*_ANSI */
void remove_old_prm( text, kt, ln, ch )
char *text[];
KEYTBL *kt;
int ln, 
ch;
#endif		/*_ANSI */
{

	int lne, che;


	lne = ln;
	che = ch;

	mv_end_param( text, &lne, &che, kt->ds, ld.p_end, !strcmp(kt->key,"=") );

	if ( lne == ln ) {
		str_move( &text[ln][ch], &text[lne][che] );
	}
	else {
		int i;

		null( &text[ln][ch], strlen( &text[ln][ch] ) );

		adjust(text[ln]);

		for ( i = lne - 1  ; i > ln ; i-- ) {
			delete_line( text, i );
			lne--;
		}

		if ( che == strlen( text[lne] ) - 1)
			delete_line( text, lne );
		else {
			str_move( &text[lne][0], &text[lne][che] );
			adjust(text[lne]);
		}
	}
}


/*
 * NAME: rmalloc
 *
 * FUNCTION: allocate memory
 *
 * RETURN VALUE DESCRIPTION: returns pointer of memory address 
 */

#ifdef	_ANSI
char *rmalloc(int  n)
#else		/*_ANSI */
char *rmalloc(n)
int n;
#endif		/*_ANSI */
{
	char *t;

	t = malloc(n);
	if (t)
		null(t,n);
	else
		exit_err( catgets(scmc_catd, MS_scmc, E_MSG_91, "Unable to malloc memory.") );
	return(t);
}
	










/*
 * NAME: save_updates
 *
 * FUNCTION: save updates message
 *
 * RETURN VALUE DESCRIPTION: void 
 */

#ifdef	_ANSI
void save_updates(struct file_list  fls[])
#else		/*_ANSI */
void save_updates( fls )
struct file_list fls[];
#endif		/*_ANSI */
{
	int i;
	char file[PATH_MAX];

	for ( i = 0 ; i < CLN && fls[i].file[0] != NULL ; i++ ) {
		if (fls[i].changed) {
			char t[NLTEXTMAX];
			sprintf(t,"Saving %s",fls[i].file);
			prerr(t);
			sort_table( fls[i].msg );

			strcpy(file,fls[i].file);
			strcpy(strrchr(file,'.'),".msg");
			make_cat( fls[i].msg,fls[i].com ,file);
		}
	}
}






/*
 * NAME: set_last 
 *
 * FUNCTION: find last line in text 
 *
 * RETURN VALUE DESCRIPTION: returns index of last line in text 
 */


#ifdef	_ANSI
int set_last(char  *text[])
#else		/*_ANSI */
int set_last( text )
char *text[];
#endif		/*_ANSI */
{
	int i;
	for ( i = 0 ; i < mxlns && text[i] != NULL ; i++ ) ;
	return( --i );
}







/*
 * NAME: set_nest_level
 *
 * FUNCTION: set nest counting array
 *
 * RETURN VALUE DESCRIPTION: returns 1 if find nest delimeters other 0
 */


#ifdef	_ANSI
int set_nest_level(int  nests[5],char  ch)
#else		/*_ANSI */
int set_nest_level( nests, ch )
int nests[5];
char ch;
#endif		/*_ANSI */
{
	int i;

	for ( i = 0 ; i < 5 && ld.s_nests[i] != NULL ; i++ ) {
		if ( ch == ld.s_nests[i] ) {
			nests[i]++;
			return( 1 );
		}
		else if ( ch == ld.e_nests[i] ) {
			if ( nests[i] > 0 ) {
				nests[i]--;
				return( 1 );
			}
			else {
				return( 0 );
			}
		}
	}
	return( 0 );
}










/*
 * NAME: set_quote_level 
 *
 * FUNCTION: set quotation counting array
 *
 * RETURN VALUE DESCRIPTION: returns 1 if start quote , 0 end quote 
 */


#ifdef	_ANSI
int set_quote_level(register int  qtlvl[],register int *quote,register char  ch)
#else		/*_ANSI */
int set_quote_level( qtlvl, quote, ch )
register int qtlvl[], 
*quote;
register char ch;
#endif		/*_ANSI */
{
	register int i, j;
	for ( i = 0 ; i < 5 && ld.qt[i] != NULL ; i++ )
		if ( ch == ld.qt[i] )
			break;
	if ( i < 5 && ld.qt[i] != NULL ) {
		for ( j = 0 ; j < 5 ; j++ )
			if ( qtlvl[j] )
				break;
		if ( j < 5 ) {
			if ( i == j ) {
				qtlvl[i] = 0;
				*quote = 0;
				return(1);
			}
		}
		else {
			qtlvl[i] = 1;
			*quote = 1;
			return(1);
		}
	}
	return(0);
}








/*
 * NAME:  sort_table
 *
 * FUNCTION:  sort message table
 *
 * RETURN VALUE DESCRIPTION:  void
 */


#ifdef	_ANSI
void sort_table(struct message  *msg[])
#else		/*_ANSI */
void sort_table( msg )
struct message *msg[];
#endif		/*_ANSI */
{
	struct message *tm;
	int i, 
	changed;

	/* ======================================================================
Bubble sort of the messages, msg number within set number, inefficient, but
it shouldn't have to be very fast. ( not that many messages. )
======================================================================*/

	changed = 1;
	while ( changed == 1 ) {
		changed = 0;
		for ( i = 0 ; i < MX_MSG && msg[i] != NULL ; i++ ) {
			if ( i < MX_MSG && msg[i + 1] != NULL ) {
				if ( msg[i]->set > msg[i + 1]->set ) {
					tm = msg[i];
					msg[i] = msg[i + 1];
					msg[i + 1] = tm;
					changed = 1;
				}
				else if ( msg[i]->set == msg[i + 1]->set ) {
					if ( msg[i]->msg > msg[i + 1]->msg ) {
						tm = msg[i];
						msg[i] = msg[i + 1];
						msg[i + 1] = tm;
						changed = 1;
					}
				}
			}
		}
	}
}












/*
 * NAME:  str_move
 *
 * FUNCTION: move string2 to string1  
 *
 * RETURN VALUE DESCRIPTION: void 
 */



#ifdef	_ANSI
void str_move(char  *s1,char  *s2)
#else		/*_ANSI */
void str_move( s1, s2 )
char *s1, 
*s2;
#endif		/*_ANSI */
{
	char *tm;
	tm = rmalloc( strlen( s2 ) + 2 );
	strcpy( tm, s2 );
	strcpy( s1, tm );
	free(tm);
}









/*
 * NAME: strmove 
 *
 * FUNCTION: move string2 to string1 with warning if string too long 
 *
 * RETURN VALUE DESCRIPTION: void
 */


#ifdef	_ANSI
void strmove(char  *s1,char   *s2)
#else		/*_ANSI */
void strmove( s1, s2 )
char *s1, *s2;
#endif		/*_ANSI */
{
	static char tm[NLTEXTMAX * 2];
	if ( strlen( s2 ) > NLTEXTMAX * 2 )
		prerr(  catgets(scmc_catd, MS_scmc, E_MSG_92, "Unable to copy string longer than 2048 bytes" ) );
	strcpy( tm, s2 );
	strcpy( s1, tm );
}










/*
 * NAME: substitute
 *
 * FUNCTION: substitutes val1 in line with val2
 *
 * RETURN VALUE DESCRIPTION: void 
 */


#ifdef	_ANSI
void substitute(char  line[],char  val1[],char  val2[])
#else		/*_ANSI */
void substitute( line, val1, val2 )
char line[];
char val1[];
char val2[];
#endif		/*_ANSI */
{
	int i, j;
	int v1_len, 
	v2_len, 
	ln_len;

	v1_len = strlen( val1 );
	v2_len = strlen( val2 );
	ln_len = strlen( line );

	for ( i = 0 ; i < ln_len ; i++ ) {
		if ( line[i] == val1[0] ) {
			if ( ( i == 0 || ( i > 0 && line[i - 1] <= ' ' ) ) && 
			    line[i + v1_len] <= ' ' ) {
				if ( memcmp( &line[i], val1, v1_len ) == 0 ) {
					memmove( &line[i + v2_len], &line[i + v1_len], strlen( &line[i + v1_len] ) + 1 );
					for ( j = 0 ; j < v2_len ; j++ ) {
						line[i + j] = val2[j];
					}
					return;
				}
			}
		}
	}
}










/*
 * NAME: val_name 
 *
 * FUNCTION: check message name validition and uniquness
 *
 * RETURN VALUE DESCRIPTION: returns ERROR if failure
 */


#ifdef	_ANSI
int val_name(char  set[],char  msg[],struct message  *mtbl)
#else		/*_ANSI */
int val_name(set,msg,mtbl)
char set[];
char msg[];
struct message *mtbl;
#endif		/*_ANSI */
{
	int i;

	for (i = 0 ; i < CLN && msg[i] != NULL ; i++) {
		if(!( (msg[i] >= 'A' && msg[i] <= 'Z') || 
		    (msg[i] >= 'a' && msg[i] <= 'z') ||  
		    (msg[i] >= '0' && msg[i] <= '9') ||
		    msg[i] == '_')) {
			break;
		}
	}

	if (msg[i] != NULL) {
		prerr( catgets(scmc_catd, MS_scmc, E_MSG_93, "The message name contains an invalid character.") );
		return(ERROR);
	}
	if (find_msg(mtbl,set,msg) == ERROR)
		return(NORM);
	else {
		prerr( catgets(scmc_catd, MS_scmc, E_MSG_94, "This message name has already been used.") );
		return(ERROR);
	}
}















/*
 * NAME: val_sub
 *
 * FUNCTION: check parameters
 *
 * RETURN VALUE DESCRIPTION: returns ERROR if failure
 */



#ifdef	_ANSI
int val_sub(char  *text[],KEYTBL  *kt,int  ln,int  ch)
#else		/*_ANSI */
int val_sub( text, kt, ln, ch )
char *text[];
KEYTBL *kt;
int ln;
int ch;
#endif		/*_ANSI */
{
	int i;

	for ( i = 0 ; i < CLN && kt->ls[i] != NULL ; i++ ) {
		if ( params[kt->prm] && strlen( params[kt->prm] ) >= strlen( kt->ls[i] ) ) {
			if ( memcmp( kt->ls[i], params[kt->prm], strlen( kt->ls[i] ) ) == 0 ) {
				return( NORM );
			}
		}
	}
	return( ERROR );
}











/*
 * NAME: val_tbls
 *
 * FUNCTION:  check keyword table and conversion table consistency
 *
 * RETURN VALUE DESCRIPTION: returns ERROR if failure
 */


#ifdef	_ANSI
int val_tbls(KEYTBL  *kt[],CVNTBL  *ct[])
#else		/*_ANSI */
int val_tbls( kt, ct )
KEYTBL *kt[];
CVNTBL *ct[];
#endif		/*_ANSI */
{
	extern struct lang_des ld;

	int i, j;

	for ( i = 0 ; i < MXKEYS && kt[i] != NULL ; i++ )  {
		for ( j = 0 ; j < MXKEYS && ct[j] != NULL ; j++ )  {
			if ( !strcmp( kt[i]->cvm, ct[j]->name ) ) {
				break;
			}
		}
		if ( j == MXKEYS || ct[j] == NULL ) {
			prerr(  catgets(scmc_catd, MS_scmc, E_MSG_95, "Not all conversion methods have been defined." ) );
			return( ERROR );
		}
	}
	return( NORM );
}







/*
 * NAME:  write_file
 *
 * FUNCTION: write out new source file
 *
 * RETURN VALUE DESCRIPTION: void 
 */


#ifdef	_ANSI
void write_file(char  *text[],struct file_list  fls[])
#else		/*_ANSI */
void write_file( text, fls )
char *text[];
struct file_list fls[];
#endif		/*_ANSI */
{
	int message_added = FALSE;
	char tm[128];
	char file[128];
	int i, status;
	FILE *f;

	if ( !args.batch )
		prerr(  catgets(scmc_catd, MS_scmc, E_MSG_96, "Saving files....                    " ) );

	strcpy(file,args.file);
	i = strlen(file);

	if (file[i - 1] != 'n')
		file[i - 1] = 'n';
	else 
		file[i - 1] = 'N';


	for ( i = 0 ; i < CLN && fls[i].file[0] != NULL ; i++ ) {
		if (fls[i].changed) {
			message_added = TRUE;
		}
	}
	if (message_added) {

		f = fopen( file, "w" );
		if ( f == NULL ) {
			prerr(  catgets(scmc_catd, MS_scmc, E_MSG_98, "Unable to open file. Aborting." ) );
			return;
		}

		sprintf(tm,".*include file for message texts.*") ;
		if (!line_exists(text,tm))  
			fputs("/*              include file for message texts          */\n",f) ;

		for ( i = 0 ; i < CLN && fls[i].file[0] != NULL ; i++ ) {
			if (fls[i].changed) {
				strcpy( file, fls[i].file);
				remove_ext ( file );
				sprintf(tm,"#include.*%s_msg.h",file);
				if (!line_exists(text,tm)) {
					fprintf( f, "#include \"%s_msg.h\" \n", file );
				}
			}
		}
		sprintf(tm,"nl_catd  scmc_catd;" ); 
		if (!line_exists(text,tm)) 
			fputs ("nl_catd  scmc_catd;   /* Cat descriptor for scmc conversion */\n", f);

		for ( i = 0 ; i < mxlns && text[i] != NULL ; i++ ) {
			status = fputs( text[i], f );
			if ( status != strlen( text[i] ) ) {
				prerr(  catgets(scmc_catd, MS_scmc, E_MSG_101, "Unable to write to output file." ) );
				break;
			}
		}
		fclose( f );
	}
	else {
		prerr("No messages added to file, no changes made.");
	}
}


#ifdef	_ANSI
make_defs(char  *line)
#else		/*_ANSI */
make_defs(line)
char *line;
#endif		/*_ANSI */
{
	char 	base[PATH_MAX],
		command[PATH_MAX * 2];
	struct stat buf;
	int 	tdef;
	
	strcpy(base,line);
	base[strrchr(line,'_') - line] = NULL;
	
	if (!access(line,F_OK)) {
		if (!stat(line,&buf)) {
			tdef = buf.st_mtime;
		}
		else {
			prerr( catgets(scmc_catd, MS_scmc, E_MSG_38, "Unable to stat '_msg.h' file.") );
			return(ERROR);
		}
		sprintf(command,"%s.msg",base);
		if (!stat(command,&buf)) {
			if (buf.st_mtime <= tdef)
				return(NORM);
		}
		else {
			prerr( catgets(scmc_catd, MS_scmc, E_MSG_40, "Unable to stat '.msg' file.") );
			return(ERROR);
		}
	}


	sprintf(command,"mkcatdefs %s %s.msg > /dev/null 2>&1",base,base);
	prerr( catgets(scmc_catd, MS_scmc, E_MSG_42, "The _msg.h file did not exist for this file, creating it.") );
	system(command);
	return(NORM);
}
	


