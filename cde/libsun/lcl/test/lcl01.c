/* lcl01 - LctNMsgText type (header and body) displaying mail sample */

#include <stdio.h>
#include <locale.h>

#include "lcl.h"

static char *load_file(char *);
static void print_segment(LclCharsetSegmentSet *segs);
static void print_charset_list(LclCharsetList *list);

int
main(int argc, char **argv)
{
    char *locale;
    char *body;
    char *header;
    LCLd lcld;
    LCTd lclt;
    LclError *ret;
    LclCharsetSegmentSet *segs;
    LclCharsetList	*list;
    int	n;

    setlocale(LC_ALL, "");

    if ((locale = setlocale(LC_CTYPE, NULL)) == NULL) {
	fprintf(stderr, "Locale is not set correctly.\n");
	return 1;
    }

    header = load_file("header.iso-2022-jp-b");
    body = load_file("body.iso-2022-jp");

    /* open lcld */
    lcld = lcl_open(locale);
    if (!lcld) {
	fprintf(stderr, "Failed: lcl_open()\n");
	return 1;
    }

    /* create lclt from mail header and body */
    lclt = lct_create(lcld, 
		      LctNSourceType, LctNMsgText, header, body,
		      LctNSourceForm, LctNInComingStreamForm,
		      LctNKeepReference, LctNKeepByReference,
		      NULL);

    if (!lclt) {
	fprintf(stderr, "Failed: lcl_create()\n");
	return 1;
    }

    /* get converted header */
    ret = lct_getvalues(lclt,
			LctNDisplayForm,
			LctNHeaderSegment, &segs,
			NULL);

    if (ret) {
	fprintf(stderr, "Failed: lcl_getvalues()\n");
	lcl_destroy_error(ret);
	return 1;
    }
    print_segment(segs);

    /* get converted body */
    ret = lct_getvalues(lclt,
			LctNDisplayForm,
			LctNBodySegment, &segs,
			NULL);

    if (ret) {
	fprintf(stderr, "Failed: lcl_getvalues()\n");
	lcl_destroy_error(ret);
	return 1;
    }
    print_segment(segs);


    /*** optional - for the case of displayed wrongly ***/

    /* get other charset candidates (shown to user) */
    ret = lct_getvalues(lclt,
                        LctNInComingStreamForm,
                            LctNBodyPossibleCharsetList, &list,
                        NULL);
    if (ret) {
	fprintf(stderr, "Failed: lcl_getvalues()\n");
	lcl_destroy_error(ret);
	return 1;
    }
    print_charset_list(list);

    /* set the one of candidates (chosen by user)*/
    lct_setvalues(lclt,
                  LctNInComingStreamForm,
                  LctNBodyCharset, list->charset[0],
                  NULL);
    lcl_destroy_charset_list(list);

    /* get newly-converted body */
    ret = lct_getvalues(lclt,
			LctNDisplayForm,
			LctNBodySegment, &segs,
			NULL);

    if (ret) {
	fprintf(stderr, "Failed: lcl_getvalues()\n");
	lcl_destroy_error(ret);
	return 1;
    }
    print_segment(segs);

    /*** end of optional ***/

    /* destroy */
    lct_destroy(lclt);

    /* close */
    lcl_close(lcld);

    return 0;
}


#include <sys/stat.h>

static char *
load_file(char *filename)
{
    struct stat	fs;
    char *buf = NULL;
    FILE *fp;

    if (!filename || !*filename)
	return NULL;

    if(stat(filename, &fs))
	return NULL;

    if ((fp = fopen(filename, "r")) == (FILE *) NULL)
	return NULL;

    if ((buf = (char *) malloc(fs.st_size + 1)) == (char *) NULL)
	return NULL;

    if (fread(buf, 1, (fs.st_size + 1), fp) == 0)
	return NULL;

    buf[fs.st_size] = (char) 0;

    fclose(fp);

    return buf;
}

void
print_segment(LclCharsetSegmentSet *segs)
{
        int     i;
        char    *buf;

        for(i = 0; i < segs->num; i++){
                fprintf(stderr, "seg[%d].charset = %s\n", i, segs->seg[i].charset);
                buf = (char *)malloc(segs->seg[i].size + 1);
                if(buf == (char *)NULL)
                        return;
                strncpy(buf, segs->seg[i].segment, segs->seg[i].size);
                buf[segs->seg[i].size] = (char)0;
                fprintf(stderr, "seg[%d].segment = %s\n", i, buf);
                free(buf);
        }
}

static void
print_charset_list(LclCharsetList *list)
{
        int     i;

        for(i = 0; i < list->num; i++)
                fprintf(stderr, "charset[%d] : %s\n", i, list->charset[i]);
}
