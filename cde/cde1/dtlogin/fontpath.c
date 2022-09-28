/* $XConsortium: fontpath.c /main/cde1_maint/2 1995/10/23 09:19:22 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*
**  fontpath.c - font path modification routines
**
**  $fontpath.c,v 1.1 93/06/24 14:52:10 bill Exp $
**
**  Copyright 1993 Hewlett-Packard Company
*/

# include "dm.h"
# include <X11/Xatom.h>
# include <setjmp.h>
# include <utmp.h>
# include "vgproto.h"

/* 
**  Prototypes
*/

#ifdef _NO_PROTO

void ApplyFontPathMods();
void GetSysParms();
static int PathInPList();
static int PathInZList();
static int SeparateParts();

#else

void ApplyFontPathMods( struct display *d, Display *dpy );
void GetSysParms( char **tzpp, char **fhpp, char **ftpp );
static int PathInPList(char *path, char **fplist, int listlen);
static int PathInZList(char *path, char *fplist, int listlen);
static int SeparateParts( char **path );

#endif /* _NO_PROTO */

static
#ifdef _NO_PROTO
DebugFontPath(note,path,nelems)
    char *note, **path;
    int nelems;
#else
DebugFontPath(char *note, char **path, int nelems)
#endif /* _NO_PROTO */
{
    int i;
    Debug("  %s: %d elements\n",note,nelems);
    for (i=0; i<nelems; i++)
        Debug("    %s\n",path[i]);
}

static int 
#ifdef _NO_PROTO
ErrorHandler( dpy, event )
    Display *dpy ;
    XErrorEvent *event ;
#else
ErrorHandler(Display *dpy, XErrorEvent *event)
#endif /* _NO_PROTO */
{
    XmuPrintDefaultErrorMessage(dpy, event, stderr);
    return 0; /* nonfatal */
}

/* ________________________________________________________________
**|                                                                |
**| ApplyFontPathMods(d) -                                         |
**|                                                                |
**|     If new font path mods are mandated, fabricate and apply    |
**|     an appropriate new complete X server font path.            |
**|								   |
**| Specify: d = pointer to display structure			   |
**|								   |
**| Returns: nothing						   |
**|________________________________________________________________|
*/

void
#ifdef _NO_PROTO
ApplyFontPathMods( d, dpy ) 
    struct display *d;
    Display *dpy;
#else
ApplyFontPathMods( struct display *d, Display *dpy )
#endif /* _NO_PROTO */
{
    FILE *fin;
    char *s,*t;
    char *fph = NULL;
    char *fpt = NULL;
    char **fontPath,**newList;
    int numHeads = 0;
    int numTails = 0;
    int i,j,k,numPaths;

    Debug("ApplyFontPathMods() for %s\n",d->name);

    if (d->displayType.location == Foreign) {
        Debug("  Foreign display\n");
        return;
    }

    if (!(fontPath=XGetFontPath(dpy, &numPaths))) {
        Debug("  Can't get font path\n");
        return;
    }

    /*
    **  Font path mods can come from (in priority order):
    **
    **    1. FONT_PATH_HEAD/TAIL definitions in /etc/src.sh
    **    2. fontPathHead/Tail resources
    */

    GetSysParms(0,&fph,&fpt);

    if (fph)          { s = "sys parm file"; }
    else if (fpHead)  { s = "resource"; fph = strdup(fpHead); }
    if (fph && !*fph) { free(fph); fph = NULL; }
    if (fph)          Debug("  +fp (%s) %s\n",s,fph);

    if (fpt)          { s = "sys parm file"; }
    else if (fpTail)  { s = "resource"; fpt = strdup(fpTail); }
    if (fpt && !*fpt) { free(fpt); fpt = NULL; }
    if (fpt)          Debug("  fp+ (%s) %s\n",s,fpt);

    /* 
    **  Break up fph and fpt into constituent parts and 
    **  then reconstruct the complete, modified font path.
    **  During reconstruction we also eliminate redundancies.
    */

    numHeads = SeparateParts(&fph);
    numTails = SeparateParts(&fpt);
    if (numHeads || numTails) {
        newList = (char **) malloc((numHeads+numPaths+numTails)
            * sizeof(char *));
        if (newList) {
            for (s=fph, i=j=0; j<numHeads; j++) {
                if (!PathInPList(s,newList,i))
                    newList[i++] = s;
                while (*s) s++; s++;
            }
            for (j=0; j<numPaths; j++) {
                if (!PathInPList(fontPath[j],newList,i) &&
                    !PathInZList(fontPath[j],fpt,numTails)) {
                    newList[i++] = fontPath[j];
                }
            }
            for (s=fpt, j=0; j<numTails; j++) {
                if (!PathInPList(s,newList,i))
                    newList[i++] = s;   
                while (*s) s++; s++;
            }
            if (debugLevel > 0)
                DebugFontPath("Request (XSetFontPath)",newList,i);

            /*
            **  Tell X server to set new font path now.  Log failure,
            **  but don't let it be fatal.  (Note that caller should
            **  reset error handler to elsewhere when we return.)
            */

            (void)XSetErrorHandler(ErrorHandler);
            XSetFontPath(dpy, newList, i);
            XSync(dpy, True);
            free(newList);
            if (debugLevel > 0) {
                newList = XGetFontPath(dpy, &i);
                DebugFontPath("Confirm (XGetFontPath)",newList,i);
                XFreeFontPath(newList);
            }
        }
    }

    if (fph) free(fph);
    if (fpt) free(fpt);
    XFreeFontPath(fontPath);
}

/* ___________________________________________________________________
**|                                                                   |
**| PathInPList(path,fplist,listlen) -                                |
**|								      |
**|     Determine if a specific fontpath element is in a list,        |
**|     taking into account that identical elements may be formed     |
**|     differently (with multiple embedded and trailing slashes).    |
**|                                                                   |
**| Specify: (char *)path = the single element to be tested           |
**|          (char **)fplist = list of ptrs to asciz elements         |
**|          (int)listlen = number of pointers in the list            |
**|                                                                   |
**| Returns: TRUE if element is in the list                           |
**|___________________________________________________________________|
*/

static int
#ifdef _NO_PROTO
PathInPList(path,fplist,listlen)
char *path;
char **fplist;
int listlen;
#else
PathInPList(char *path, char **fplist, int listlen)
#endif /* _NO_PROTO */
{
    char *s,*t;
    while (listlen-- > 0) {
        for (s=path, t=fplist[listlen]; *s && (*s == *t); ) {
            t++; while (*t == '/') t++;
            s++; while (*s == '/') s++;
        }
        if (!*s && !*t) return 1;
    }
    return 0;    
}

/* ___________________________________________________________________
**|                                                                   |
**| PathInZList(path,fplist,listlen) -                                |
**|								      |
**|     Determine if a specific fontpath element is in a list,        |
**|     taking into account that identical elements may be formed     |
**|     differently (with multiple embedded and trailing slashes).    |
**|                                                                   |
**| Specify: (char *)path = the single element to be tested           |
**|          (char *)fplist = list of concatenated asciz elements     |
**|          (int)listlen = number of elements in the list            |
**|                                                                   |
**| Returns: TRUE if element is in the list                           |
**|___________________________________________________________________|
*/

static int
#ifdef _NO_PROTO
PathInZList(path,fplist,listlen)
char *path,*fplist;
int listlen;
#else
PathInZList(char *path, char *fplist, int listlen)
#endif /* _NO_PROTO */
{
    char *s,*t;
    for (t=fplist; listlen > 0; listlen--) {
        for (s=path; *s && (*s == *t); ) {
            t++; while (*t == '/') t++;
            s++; while (*s == '/') s++;
        }
        if (!*s && !*t) return 1;
        while (*t) t++; t++;
    }
    return 0;    
}

/* ________________________________________________________________
**|                                                                |
**| SeparateParts(path)					           |
**|								   |
**|     Break a comma-delimited asciz path string into its	   |
**|     separate asciz constituent parts.			   |
**|								   |
**| Specify: path = ptr to asciz path string (e.g., "as,df,jk")	   |
**|								   |
**| Returns: number of constituent parts, with path string         |
**|          converted into as many sequential asciz strings       |
**|          (e.g., "as\0df\0jk").				   |
**|________________________________________________________________|
*/

static int
#ifdef _NO_PROTO
SeparateParts(path)
char **path;
#else
SeparateParts( char **path )
#endif /* _NO_PROTO */
{
    char *t,*s;
    int nparts = 0;
    if (path && *path)
	for (s=*path; t=strtok(s,","); s=NULL, nparts++);
    return nparts;
}

/* ___________________________________________________________________
**|                                                                   |
**| GetSysParms(tzpp,fhpp,ftpp) -				      |
**|								      |
**|     Extract TZ, FONT_PATH_HEAD, and FONT_PATH_TAIL definitions    |
**|     from the sys parms file (typically /etc/src.sh).              |
**|                                                                   |
**| Specify: (char **)tzpp = where to put ptr to TZ string            |
**|          (char **)fhpp = where to put ptr to FONT_PATH_HEAD       |
**|          (char **)ftpp = where to put ptr to FONT_PATH_TAIL       |
**|                                                                   |
**|     Specify a NULL (char **) for any undesired strings.           |
**|                                                                   |
**| Returns: Appropriate pointers to malloc'ed strings (NULL pointer  |
**|          if a string is neither requested nor defined).           |
**|___________________________________________________________________|
*/

void
#ifdef _NO_PROTO
GetSysParms(tzpp, fhpp, ftpp)
char **tzpp, **fhpp, **ftpp;
#else
GetSysParms( char **tzpp, char **fhpp, char **ftpp )
#endif /* _NO_PROTO */
{
    FILE *fin;
    char *s,*t,buf[256];

    if (tzpp) *tzpp = NULL;
    if (fhpp) *fhpp = NULL;
    if (ftpp) *ftpp = NULL;
    if ((*sysParmsFile != '/') || !(fin=fopen(sysParmsFile,"r"))) {
        Debug("(GetSysParms) Can't open sys parms file\n");
        return;
    }
    Debug("(GetSysParms) Reading sys parms file\n");
    while (fgets(buf,255,fin)) {
        for (t=buf; *t && *t<=' '; t++); /* t -> EOS or nonblank */
        if (!*t || *t=='#') continue;    /* ignore comment lines */
        while (*t && *t!='\n') t++;      /* t -> EOS or newline  */
        if (*t) *t = '\0';               /* discard newline char */
        if (tzpp && !*tzpp)
            if ((s=strstr(buf,"TZ=")) && (t=strtok(s+3,"; \t")))
                *tzpp = (char *) strdup(t);
        if (fhpp && !*fhpp)
            if ((s=strstr(buf,"FONT_PATH_HEAD=")) && (t=strtok(s+15,"; \t")))
                *fhpp = (char *) strdup(t);
        if (ftpp && !*ftpp)
            if ((s=strstr(buf,"FONT_PATH_TAIL=")) && (t=strtok(s+15,"; \t")))
                *ftpp = (char *) strdup(t);
    }
    fclose(fin);
}    

