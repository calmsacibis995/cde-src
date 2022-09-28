#include <stdio.h>
#include <locale.h>

#include "lcl.h"

static char *load_file(char *);

int
main(int argc, char **argv)
{
    char *locale;
    char *header;
    LCLd lcld;
    LCTd lctd;
    LclError *ret;
    LclCharsetSegmentSet *segs;

    setlocale(LC_ALL, "");

    if ((locale = setlocale(LC_CTYPE, NULL)) == NULL) {
	fprintf(stderr, "Locale is not set correctly\n");
	return -1;
    }
    else{
	fprintf(stderr, "Locale is %s\n", locale);
    }

    if(argc != 2){
	fprintf(stderr, "usage: %s header_file\n", argv[0]);
	return -1;
    }

    if((header = load_file(argv[1])) == (char *)NULL){
	fprintf(stderr, "file read error : %s\n", argv[1]);
	return -1;
    }

    lcld = lcl_open(locale);
    if (!lcld) {
	fprintf(stderr, "Failed: lcl_open()\n");
	return -1;
    }

    lctd = lct_create(lcld, 
		      LctNSourceType, LctNMsgText, header, LctNNone,
		      LctNSourceForm, LctNInComingStreamForm,
		      LctNKeepReference, LctNKeepByReference,
		      NULL);
    if (!lctd) {
	fprintf(stderr, "Failed: lcl_create()\n");
	return -1;
    }

    ret = lct_getvalues(lctd,
			LctNDisplayForm,
			LctNHeaderSegment, &segs,
			NULL);
    if (ret) {
	fprintf(stderr, "Failed: lcl_getvalues()\n");
	if((ret->error_code == LctErrorIconvNonIdenticalConversion) || (ret->error_code == LctErrorIconvHalfDone))
	    lcl_destroy_segment_set(segs);
	lcl_destroy_error(ret);
	return -1;
    }

    fprintf(stdout, "Charset: %s\n", segs->seg[0].charset);
    fprintf(stdout, 
	    "Header content:\n"
	    "%s\n", 
	    segs->seg[0].segment);

    lcl_destroy_segment_set(segs);

    lct_destroy(lctd);

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
