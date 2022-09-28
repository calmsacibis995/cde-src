/* lcl07.c - LctNSeparatedTaggedText type send attachment sample */

#include <stdio.h>
#include <locale.h>

#include "lcl.h"

static char *load_file(char *);
static void print_segment(LclCharsetSegmentSet *segs);

int
main(int argc, char **argv)
{
    char *locale;
    char *body;
    char *header;
    LCLd lcld;
    LCTd lclt;
    LclCharsetSegmentSet *segs;
    LclError	*ret;

    setlocale(LC_ALL, "");

    if ((locale = setlocale(LC_CTYPE, NULL)) == NULL) {
	fprintf(stderr, "Locale is not set correctly.\n");
	return 1;
    }

    header = load_file("attach_header.euc");
    body = load_file("attach_body.euc");

    /* open lcld */
    lcld = lcl_open(locale);
    if (!lcld) {
	fprintf(stderr, "Failed: lcl_open()\n");
	return 1;
    }

    /* create lclt from attachment header and body */
    lclt = lct_create(lcld, 
		      LctNSourceType, 
		          LctNSeparatedTaggedText, header, body,
		      LctNSourceForm, LctNDisplayForm,
		      LctNKeepReference, LctNKeepByReference,
		      NULL);
    
    if (!lclt) {
	fprintf(stderr, "Failed: lcl_create()\n");
	return 1;
    }
    
    /* get converted attachment header */
    ret = lct_getvalues(lclt,
			LctNOutGoingStreamForm,
			LctNHeaderSegment, &segs,
			NULL);
    
    if (ret) {
	fprintf(stderr, "Failed: lcl_getvalues()\n");
	lcl_destroy_error(ret);
	return 1;
    }
    print_segment(segs);
    
    /* get converted attachment body */
    ret = lct_getvalues(lclt,
			LctNOutGoingStreamForm,
			LctNBodySegment, &segs,
			NULL);
    
    if (ret) {
	fprintf(stderr, "Failed: lcl_getvalues()\n");
	lcl_destroy_error(ret);
	return 1;
    }
    print_segment(segs);
    
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
