/* $XConsortium: scmc.h /main/cde1_maint/1 1995/07/17 16:38:40 drk $ */
/* @(#)scmc.h	1.11  @(#)scmc.h	1.11 01/20/96 15:41:28 */
/*
 * COMPONENT_NAME: (PIITOOLS) AIX PII Tools
 *
 * FUNCTIONS: Standard header file for scmc
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
#include <errno.h>
#include <memory.h>
#include <string.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>
#include "select.h"
#include "spell.h"
#include "scmc_msg.h"
#include <NLctype.h>

#define MACRO
#ifdef MACRO
#define mv_ch(text,ln,ch) !text[*(ln)] ? ERROR : !text[*(ln)][++(*(ch))] ? ( (*(ch) = 0) , \
                          (text[++(*(ln))] ? NORM : ERROR) ) : (text[*(ln)] ? NORM : ERROR)
#endif

#define skip_to_white(s) 	while (*s && !isspace(*s)) s++
#define skip_to_nwhite(s) 	while (*s && isspace(*s)) s++

#define append( s1, s2 ) strcpy( &( s1 )[strlen( s1 )], ( s2 ) )
#define FREE_MSG( m )   ( freemsg( m ), m = NULL )
#define wperror( s )    prerr( s )
#define prmsg( s )      prerr( s )

#define MSG_LEN NL_TEXTMAX
#define LINE_LEN 256
#define MX_MSG  500
#define MXFILES 300
#define NLTEXTMAX 1024
#define MX_LINES 70
#define MX_SET	100

#define CNTRL_B 2
#define CNTRL_D 4
#define CNTRL_F 6
#define CNTRL_H 8




/*-----------------------------------------------------------------*/
/*--------keyword list table structure definition------------------*/
/*-----------------------------------------------------------------*/
#define CLN 30
#define PGSIZE 16
#define MXLNS 5000
#define THE_UNIVERSE_EXISTS 1

struct comment {
   int set;
   char *comm[MX_LINES];
};

struct message {
   int set;
   char set_name[CLN];
   int msg;
   char msg_name[CLN];
   int status;
   char *line[MX_LINES];
   char *comm[MX_LINES];
   int lenmax;
};

struct constant_table {
   char name[CLN * sizeof( char )];
   char value[CLN * sizeof( char )];
};

typedef struct {
   char key[CLN];
   int lvl;
   int prm;
   char *ls[CLN];
   char *ds[CLN];
   char pfx[CLN];
   char set[CLN];
   char cvm[CLN];
} KEYTBL;

typedef struct {
   char name[CLN];
   char format[256];
   char catd[CLN];
   char file[PATH_MAX];
} CVNTBL;

struct {
   int batch;
   int query_names;
   char cfig[PATH_MAX];
   char file[PATH_MAX];
   int  lvl;
} args;

struct lang_des {
   int cpos;
   char s_com[CLN];
   char e_com[CLN];
   char qt[CLN];
   char s_nests[CLN];
   char e_nests[CLN];
   char p_start[CLN];
   char p_end[CLN];
   char mta;
} ld;

struct file_list {
   char file[PATH_MAX];
   struct message *msg[MX_MSG];
   struct comment *com[MX_SET];
   struct constant_table *ctbl[MX_MSG];
   int changed;
};



void add_msg( ); 
void advance_char( ); 
void clr_err( ); 
void cp_param( ); 
void cpywd( ); 
void decap( ); 
void delete_line( ); 
void destroy_win( ); 
void display_text( ); 
void do_sub( ); 
void draw_check_sub( ); 
void draw_get_name( ); 
void draw_str( ); 
void examine_quotes( ); 
void exit_eot( ); 
void exit_err( ); 
void exit_signal( ); 
void find_key( ); 
void fr_msglist( ); 
void freemsg( ); 
void get_catd_nme( ); 
void get_lang_dat( );
void get_msg_nme( ); 
void get_params( ); 
void get_param_pos( ); 
void get_set_nme( ); 
void gt_ln_dline( ); 
void gt_msg_nm( ); 
void insert_octals( );
void l_edit( ); 
void l_edraw( ); 
void lsubstitute( ); 
void make_cat_defs( );
void mk_ed_ar( );
void mv_end_param( );
void mvwaddchars( );
void nstrcpy( );
void null( ); 
void parse_args( ); 
void place_new_prm( ); 
void pre_process( ); 
void prerr( ); 
void process_file( );
void parse_ct_line( );
void parse_kt_line( );
void remove_ext( ); 
void remove_qts( );
void remove_old_prm( );
void save_updates( );
void sort_table( );
void str_move( );
void strmove( );
void substitute( );
void write_file( );
void wait_msg( );

int build_file( );
int build_msg( );
int check_sub( );
int find_msg( );
int get_cvm( );
int get_flsfd( );
int get_msg_no( ); 
int get_name( );
int get_set_no( );
int line_exists( );
int make_cat( );
int mk_const_table( );
int mk_msg_list( );
int mk_tbls( );
#ifndef MACRO
int mv_ch( );
#endif
int mvwgetchars( );
int parse_line( );
int replace_bwd( );
int main( );
int set_last( );
int set_nest_level( );
int set_quote_level( );
int val_name( );
int val_sub( );
int val_tbls( );

char *rmalloc(  );
KEYTBL *all_kt(  );
CVNTBL *all_ct(  );
struct constant_table *all_c_tbl(  );
struct message *all_msg(  );
char *all_CLN(  );
char* find_num( );

#define ENTER  10 
#define DEL    127 
#define ESC    27 
#define TAB    9
#define ERROR -1
#define NORM   0
#define MXKEYS 500

#define isnum( c ) ( ( ( c ) >= '0' && ( c ) <= '9' ) ? 1 : 0 )
#define max( a, b ) ( ( ( a ) > ( b ) ) ? a : b )
#define adjust(s)		((s = realloc(s,strlen(s) + 1)) ? s : (exit_err("Unable to realloc memory."), (char *)0))
#define isoct(c)	(c >= '0' && c <= '9')

extern VID_MODES svi;
extern SPLT *sptbl;
extern int nbwds;
extern char *params[CLN];
