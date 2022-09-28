#include <stdio.h>
#include <locale.h>

#include "lcl.h"

static char *load_file(char *);

void
print_charset_list(LclCharsetList *list)
{
	int	i;

	for(i = 0; i < list->num; i++)
		fprintf(stderr, "charset[%d] : %s\n", i, list->charset[i]);
}

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
    int i;
    LclCharsetList *tmp_list;

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

    ret = lct_getvalues(lclt, 
			LctNInComingStreamForm, 
			    LctNBodyPossibleCharsetList, &tmp_list, 
			NULL);

    if (ret) {
	fprintf(stderr, "Failed: lcl_getvalues()\n");
	lcl_destroy_error(ret);
	return 1;
    }

    if(tmp_list) {
	print_charset_list(tmp_list);
	for(i = 0; i < tmp_list->num; i++){
	    lct_setvalues(lclt, 
			  LctNDisplayForm, 
			      LctNBodyCharset, tmp_list->charset[i], 
			  NULL);
	    lct_getvalues(lclt, 
			  LctNDisplayForm, 
			      LctNBodySegment, &segs, 
			  NULL);
	    if(segs){
		fprintf(stdout, "Body content:\n%s\n", segs->seg[0].segment);
		lcl_destroy_segment_set(segs);
	    }
	}
	lcl_destroy_charset_list(tmp_list);
    }

    fprintf(stdout, "Charset: %s\n", segs->seg[0].charset);
    fprintf(stdout, 
	    "Body content:\n"
	    "%s\n", 
	    segs->seg[0].segment);

    lct_destroy(lclt);

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
