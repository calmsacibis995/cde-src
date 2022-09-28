/* $XConsortium: spell.c /main/cde1_maint/1 1995/07/17 16:40:11 drk $ */
static char sccsid[] = "@(#)spell.c	1.11  @(#)spell.c	1.11 01/20/96 15:41:38";
/*
 * COMPONENT_NAME: (PIITOOLS) AIX PII Tools
 *
 * FUNCTIONS: spell
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
#include <memory.h>
#include "spell.h"

#define ENTER  10 
#define DEL    127 
#define ESC    27 
#define TAB    9
#define ERROR -1
#define NORM   0
#define MXKEYS 500
#define LINE_LEN 256
char *malloc();

spelload(sptbl,file,nwds)
SPLT **sptbl;
char file[];
int *nwds;
{
	FILE *f;
	char t[4000],
	*p;
	int nwords = 0;

 	spellfree(sptbl,nwds);

	f = fopen(file,"r");
	if (f == NULL)
		return(ERROR);

	while (!feof(f)) {
		for (p = fgets(t,2000,f); p && *p ; p++) {
			if (*p == '\\') {
				p++;
				if (!*p)
					break;
			}
			else if (*p == ';')
				nwords++;
		}
	}

	*sptbl = (SPLT *)malloc((nwords + 1) * sizeof(SPLT));
	if ( *sptbl == NULL ) {
		exit_err("Unable to allocate memory for spell table.");
	} else {
		null( *sptbl,(nwords + 1) * sizeof(SPLT));
	}
	rewind(f);

	mk_sptbl(*sptbl,f);
	fclose(f);
	*nwds = nwords;
	return( 0 );
}
mk_sptbl(sptbl,f)
SPLT *sptbl;
FILE *f;
{
	int i;
	int word,
	phrase;
	char c;

	/*--------------------------------------------------------------------------------
  --------------------------------------------------------------------------------
 while not end of file {
  get word until colon
  for phrase = 0 phrase less than 30 and phrase++ {
   malloc phrase entry
   get replacement phrase until comma
  }
 }
  --------------------------------------------------------------------------------
  --------------------------------------------------------------------------------*/

	for (word = 0 ; !feof(f) ; word++) {
		c = getc(f);
		for (i = 0 ; c != ':' && i < 30 && !feof(f) ; ) {
			if (c == '\\' || c == ENTER)
				;
			else
				sptbl[word].bwd[i++] = c;
			c = getc(f);
		}
		if (feof(f))
			continue;

		if (c != ':') {
			exit_err("Syntax error in spell table.  Aborting");
		}

		for (phrase = 0 ; phrase < 30 && c != ';' && !feof(f) ; phrase++) {
			sptbl[word].syn[phrase] = all_syn();
			c = getc(f);
			for (i = 0 ; i < SYNLEN - 1 && c != ',' && c != ';' && !feof(f) ; ) {
				if (c == '\\' || c == ENTER)
					;
				else
					sptbl[word].syn[phrase][i++] = c;
				c = getc(f);
			}
			if (c != ',' && c != ';') {
				exit_err("Syntax error in spell table.  Aborting");
			}
		}
		if (c != ';' && !feof(f)) {
			exit_err("Syntax error in spell table.  Aborting");
		}
	}
}
char *all_syn()
{
	char *tm;

	tm = malloc(SYNLEN);
	if (tm == NULL) {
		exit_err("Unable to allocate memory for synonym table.");
	}
	null(tm,SYNLEN);
	return(tm);
}
/*----------------------------------------------------------------------------------------------------
  ----------------------------------------------------------------------------------------------------
  Spell check routine:  
    pass it a string and a spell table, and it will return a pointer to the first 
    word in that string which exists in the spell table. 
  ----------------------------------------------------------------------------------------------------
  ----------------------------------------------------------------------------------------------------*/

char *spell_check(str,sptbl,nwords,idx)
char str[];
SPLT sptbl[];
int nwords;
int *idx;
{
	char *p,*p1,*p2;
	char tword[LINE_LEN + 1];
	char base[LINE_LEN + 1];

	if (!sptbl)
		return(NULL);
	p = str;
	while (*p != NULL) {
		while (!isalpha(*p) && *p != NULL)
			p++;
		if (*p == NULL)
			continue;
		p1 = tword;
		p2 = p;
		while (isalpha(*p2) && *p2 != NULL)
			*p1++ = *p2++;
		*p1 = NULL;
		decap(tword);
		if (spellchk(tword,sptbl,nwords,idx) == ERROR)
			return(p);
		while (strip(tword,base) != ERROR) {
			if (spellchk(base,sptbl,nwords,idx) == ERROR)
				return(p);
			strcpy(tword,base);
		}
		p = p2;
	}
	return(NULL);
}
isalpha(c)
char c;
{
	if (c >= 'A' && c <= 'Z')
		return(1);
	if (c >= 'a' && c <= 'z')
		return(1);
	if (c == '_' || c == '-')
		return(1);
	return(0);
}
spellchk(word,sptbl,nwords,idx)
char word[];
SPLT sptbl[];
int nwords;
int *idx;
{
	register int low = 0;
	register int up  = nwords;
	register int i;
	register int iold = -1;
	register int stat;

	for (i = up / 2 ;  iold != i ; i = (up + low) / 2) {
		iold = i;
		if (!(stat = strcmp(word,sptbl[i].bwd))) {
			*idx = i;
			return(ERROR);
		}
		else if (stat > 0)
			low = i;
		else 
			up = i;
	}
	return(NORM);
}
strip(word,base)
char word[];
char base[];
{

	strcpy(base,word);

	if (strpwd(base,"ing") == NORM)
		return(NORM);
	if (strpwd(base,"s") == NORM)
		return(NORM);
	if (strpwd(base,"ly") == NORM)
		return(NORM);
	if (strpwd(base,"ely") == NORM)
		return(NORM);
	if (strpwd(base,"ive") == NORM)
		return(NORM);
	if (strpwd(base,"tion") == NORM)
		return(NORM);
	if (strpwd(base,"ion") == NORM)
		return(NORM);
	if (strpwd(base,"ity") == NORM)
		return(NORM);
	if (strpwd(base,"ed") == NORM)
		return(NORM);
	return(ERROR);

}
strpwd(base,sfx)
char base[];
char sfx[];
{
	register int l;
	register int l1;

	l = strlen(base);
	l1 = strlen(sfx);
	if (!strcmp(&base[l - l1],sfx)) {
		null(&base[l - l1],l1);
		return(NORM);
	}
	return(ERROR);
}

spellfree(sptbl,spwds)
SPLT **sptbl;
int *spwds;
{
	int i,j;

	if (*sptbl != NULL) {
		for (i = 0 ; i < *spwds ; i++) {
			for (j = 0 ; j < 30 && (*sptbl)[i].syn[j]!= NULL; j++) {
				free((*sptbl)[i].syn[j]);
			}
		}
		free(*sptbl);
	}
	*spwds = 0;
	*sptbl = NULL;
}
