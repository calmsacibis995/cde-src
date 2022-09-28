/**---------------------------------------------------------------------
***	
***    (c)Copyright 1991 Hewlett-Packard Co.
***    
***                             RESTRICTED RIGHTS LEGEND
***    Use, duplication, or disclosure by the U.S. Government is subject to
***    restrictions as set forth in sub-paragraph (c)(1)(ii) of the Rights in
***    Technical Data and Computer Software clause in DFARS 252.227-7013.
***                             Hewlett-Packard Company
***                             3000 Hanover Street
***                             Palo Alto, CA 94304 U.S.A.
***    Rights for non-DOD U.S. Government Departments and Agencies are as set
***    forth in FAR 52.227-19(c)(1,2).
***
***-------------------------------------------------------------------*/

    /*  imageutil - Utility to use the Image Library (IL),
        including TIFF file read/write capabilities.
        See imageutil.doc for details.
    */

    /*  Version string - please update !!!!! */
char versionName[] = "@(#)imageutil version 2.3.0";

#include "ilu.h"
#include "ilerrors.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif

    /*  NOTE: this code is not much of an example of using the IL.
        See ilu.c for a better IL example.
        This code is a reasonable example of using ilu.c functions.
    */

#define ALL_PAGES       -1

    /*  Elements of a linked list of input files. */
typedef struct FileInfoStruct {
    struct FileInfoStruct *pNext;
    char                typeName [IL_EFS_MAX_NAME_CHARS];
    char                *fileName;
    long                page;   /* page # to read, or ALL_PAGES = all pages */
    } FileInfo, *FileInfoPtr;

    /*  Options local to this program.
    */
typedef struct {
    ilContext       context;
    FileInfoPtr     pInFilesHead, pInFilesTail;
    char           *outFileName;
    char            outTypeName [IL_EFS_MAX_NAME_CHARS];
    char           *srcDisplayName;
    char           *dstDisplayName;
    ilBool          haveSleepTime;
    int             sleepTime;
    int             pauseTime;
    long            repeatCount;
    ilBool          doTime, doListEFS, doListFile, doListPage;
    } OptionRec, *OptionPtr;

    /*  One element of list given to FindInList() 
    */
typedef struct {
    long                     value;     /* corresponding value */
    char                    *name;      /* name to match */
    } MatchListData, *MatchListDataPtr;

#define STRING_EQUAL(str, cmpstr) (strcmp ((str), (cmpstr)) == 0)

    /*  Spot for return of checksum from iluChecksumPipe() */
static iluChecksum pipeChecksum;


    /*  Functions in ilulist.c : */
extern void iluPrintEFSFileTypeInfo (
    ilContext       context
    );

extern void iluPrintEFSFileInfo (
    ilEFSFile       file
    );

extern void iluPrintEFSPageInfo (
    ilEFSFile       file
    );



        /*  -------------------------- FindInList ------------------------ */
        /*  Find given string (str) in given list (pList) of length "nInList";
            return associated int to "pValue", else return FALSE.
        */
int FindInList (
    char           *str,
    MatchListData  *pList,
    int             nInList,
    long           *pValue)
{
int                 i;

for (i = 0; i < nInList; i++)
    if (STRING_EQUAL (str, pList[i].name)) {
        *pValue = pList[i].value;
        return TRUE;
        }

return FALSE;
}

        /*  ------------------------- GetLong ----------------------------- */
        /*  GetLong: convert *str to a long and return its value, or
            print error message including "*optionName" and exit program.
        */
long GetLong (
    char            *str,
    char            *optionName
    )
{
    long            aLong;
    ilBool          isHex;

    isHex = (strlen(str) >= 2) && (str[0] == '0') && (str[1] == 'x');
    if (isHex)
        str += 2;                       /* skip "0x" */
    if (!sscanf (str, (isHex) ? "%lx" : "%ld", &aLong)) {
        fprintf (stderr, "? Non-numeric argument to '%s' option.\n", optionName);
        exit (1);
        }
    return aLong;
}

        /*  ------------------------- GetDouble ----------------------------- */
        /*  GetDouble: convert *str to a double and return its value, or
            print error message including "*optionName" and exit program.
        */
double GetDouble (
    char            *str,
    char            *optionName
    )
{
    double          aDouble;

    if (!sscanf (str, "%lf", &aDouble)) {
        fprintf (stderr, "? Non-real argument to '%s' option.\n", optionName);
        exit (1);
        }
    return aDouble;
}


    /*  ------------------------- ParseCommandLine ------------------------- */
    /*  Parse the command line (argc, argv) and fill in the (local) option rec
        and the ILU request rec.
    */
static void ParseCommandLine (
    int             argc, 
    char           *argv[],
    OptionPtr       pO,
    iluRequestPtr   pR
    )
{
char               *pArg, *pOpt;
FileInfoPtr         pFile;
long                option, i;
static ilJPEGEncodeControl JPEGData, compJPEGData;
char                inTypeName [IL_EFS_MAX_NAME_CHARS];
ilBool              needTIFFOutput;
long                page;
static long         g3Options, g4Options, pipeTime;

    /*  "-" options to program and their defines.
    */
typedef enum {
    optInTIFF, optInJFIF, optInFile, optInFileType, optNoInFileType, 
    optInRoot, optInWindow, optInDisplay, optInCrop, optInAllPages, optInPage,
    optOutFile, optOutCreateWindow, optOutWindow, 
    optOutRoot, optOutNull, optOutImage,
    optOutFileType, optOutDitherMethod, optOutWindowPos, optOutNoWM,
    optOutCrop, optOutDisplay, optOutAppend, optOutPage, optOutRes, 
    optOutComp, optOutCompQ, optOutStrip,
    optOutDstCoord, optOutDefaultVisual,
    optOutPrivateColormap, optOutAlloc,
    optCrop, optScale, optScaleMethod, optScaleThreshold, 
    optConvert, optConvertThreshold, optConvertSubsample, optConvertPalette,
    optGamma, optMirror, optRotate90, optInvert,
    optComp, optCompQ, optDecomp, optChecksum,
    optClear, optRepeat, optPause, optSleep, optImage,
    optTime, 
    optListEFS, optListFile, optListPage
    } options;

static MatchListData optionNames [] = {
    {optInTIFF, "-itiff"}, {optInJFIF, "-ijfif"}, {optInFile, "-if"}, 
    {optInFileType, "-ift"}, {optNoInFileType, "-ifnotype"},
    {optInAllPages, "-iallpages"}, {optInPage, "-ipage"},
    {optInRoot, "-irw"}, {optInWindow, "-iwid"}, 
    {optInDisplay, "-idisplay"}, {optInCrop, "-icrop"}, 
    {optOutFile, "-of"}, {optOutFileType, "-oft"}, {optOutCreateWindow, "-ow"},
    {optOutDstCoord, "-owxy"}, {optOutDefaultVisual, "-odefaultvisual"}, 
    {optOutWindow, "-owid"}, {optOutRoot, "-orw"}, {optOutNull, "-onull"}, 
    {optOutPrivateColormap, "-opcmap"}, {optOutAlloc, "-oalloc"},
    {optOutImage, "-oimage"},
    {optOutDitherMethod, "-odmethod"}, 
    {optOutWindowPos, "-owpos"}, {optOutNoWM, "-onowm"}, 
    {optOutCrop, "-ocrop"}, 
    {optOutDisplay, "-odisplay"}, {optOutDisplay, "-display"}, 
    {optOutAppend, "-oappend"}, {optOutPage, "-opage"}, {optOutRes, "-ores"},
    {optOutComp, "-ocomp"}, {optOutCompQ, "-ocompq"}, {optOutStrip, "-ostrip"},
    {optCrop, "-crop"}, {optScale, "-s"}, {optScaleMethod, "-smethod"}, 
    {optScaleThreshold, "-sthreshold"}, 
    {optConvert, "-convert"}, {optConvertThreshold, "-cthreshold"}, 
    {optConvertSubsample, "-csubsample"}, {optConvertPalette, "-cpalette"}, 
    {optGamma, "-gamma"}, {optMirror, "-mirror"}, {optRotate90, "-rotate90"},
    {optInvert, "-invert"},
    {optComp, "-comp"}, {optCompQ, "-compq"}, {optDecomp, "-decomp"},
    {optChecksum, "-checksum"},
    {optClear,  "-clear"}, {optRepeat, "-repeat"}, {optPause, "-pause"}, 
    {optSleep, "-sleep"}, {optImage, "-image"},
    {optTime, "-time"}, 
    {optListEFS, "-lfiletypes"}, {optListFile, "-lfile"}, {optListPage, "-lpage"}
    };
#define OPTION_NAMES_SIZE (sizeof (optionNames) / sizeof (MatchListData))

    /*  Arguments to -ocomp */
typedef enum {
    compUncompressed, compG3TIFF, compLZW, compPackBits, compJPEG, compCurrent,
    compG3, compG4, compG4LSB, compG32D
    } comps;
static MatchListData compNames [] = {
    {compUncompressed, "uncompressed"}, {compG3TIFF, "g3tiff"}, 
    {compLZW , "lzw"}, {compPackBits, "packbits"}, 
    {compJPEG, "jpeg"}, {compJPEG, "JPEG"}, {compCurrent, "current"}, 
    {compG3, "g3"}, {compG3, "G3"}, 
    {compG4, "g4"}, {compG4, "G4"}, {compG4LSB, "g4lsb"}, {compG32D, "g32d"}
    };
#define COMP_NAMES_SIZE (sizeof (compNames) / sizeof (MatchListData))

    /*  Arguments to -smethod */
static MatchListData smNames [] = {
    {IL_SCALE_SIMPLE, "simple"}, 
    {IL_SCALE_SAMPLE, "sample"}, 
    {IL_SCALE_BITONAL_TO_GRAY, "gray"}, {IL_SCALE_BITONAL_TO_GRAY, "grey"}
    }; 
#define SM_NAMES_SIZE (sizeof (smNames) / sizeof (MatchListData))

    /*  Arguments to -convert */
static MatchListData cNames [] = {
    {IL_BITONAL, "bitonal"}, {IL_GRAY, "gray"}, {IL_GRAY, "grey"}, 
    {IL_RGB, "rgb"}, {IL_YCBCR, "ycbcr"}, {IL_YCBCR, "yuv"},
    {IL_PALETTE, "palette"}
    }; 
#define C_NAMES_SIZE (sizeof (cNames) / sizeof (MatchListData))

    /*  Arguments to -cpalette */
static MatchListData cpNames [] = {
    {IL_AREA_DITHER, "dither"}, 
    {IL_DIFFUSION, "diffusion"},
    {IL_QUICK_DIFFUSION, "quickdiffusion"},
    {IL_CHOOSE_COLORS, "choose"}
    }; 
#define CP_NAMES_SIZE (sizeof (cpNames) / sizeof (MatchListData))

    /*  Arguments to -mirror */
static MatchListData mirrorNames [] = {
    {IL_MIRRORX, "x"}, {IL_MIRRORY, "y"}
    }; 
#define MIRROR_NAMES_SIZE (sizeof (mirrorNames) / sizeof (MatchListData))

    /*  Macro to get the next argument or exit with error message.
    */
static char nextArgErrorMsg[] = "? Too few arguments to the '%s' option.\n";
#define NEXT_ARG {                               \
    if (argc-- <= 0) {                           \
        fprintf (stderr, nextArgErrorMsg, pOpt); \
        exit (1);                                \
        }                                        \
    pArg = *argv++;                              \
    }

        /*  Default the option rec. */
    pO->pInFilesHead = pO->pInFilesTail = (FileInfoPtr)NULL;
    pO->outFileName = "";
    pO->srcDisplayName = pO->srcDisplayName = "";
    pO->haveSleepTime = FALSE;
    pO->sleepTime = 0;
    pO->pauseTime = 0;
    pO->repeatCount = 1;
    pO->doTime = pO->doListEFS = pO->doListFile = pO->doListPage = FALSE;

    strcpy (inTypeName, "");          /* dflt input to no type name */
    strcpy (pO->outTypeName, "TIFF"); /* dflt output type name to TIFF */

    needTIFFOutput = FALSE;         /* true if direct IL TIFF support needed */
    page = 0;                       /* page to read or ALL_PAGES */

        /*  Parse the command line, fill in option and request recs. */
    argc--;                         /* skip first arg: program name */
    argv++;

    while (argc-- > 0) {
        pArg = *argv++;

            /*  If string does not begin with "-", assume it is input file.
            */
        if (pArg[0] != '-') {
            pFile = (FileInfoPtr)malloc (sizeof (FileInfo));
            if (!pFile) {
                fprintf (stderr, "? Too many input files.\n");
                exit (1);
                }

            pFile->pNext = (struct FileInfoStruct *)NULL;
            strcpy (pFile->typeName, inTypeName);
            pFile->fileName = pArg;
            pFile->page = page;

            if (!pO->pInFilesHead)          /* list empty */
                pO->pInFilesHead = pO->pInFilesTail = pFile;
            else {
                pO->pInFilesTail->pNext = pFile;
                pO->pInFilesTail = pFile;
                }
            }
        else {
                /*  String begins with "-": find in list and handle each.
                */
            if (!FindInList (pArg, optionNames, OPTION_NAMES_SIZE, &option)) {
                fprintf (stderr, "? '%s' is not a valid option.\n", pArg);
                exit (1);
                }
            pOpt = pArg;

            switch (option) {

              case optInFile:
                pR->read.rw.type = ILU_RW_EFS;
                break;

              case optNoInFileType:
                strcpy (inTypeName, "");
                break;

              case optInFileType: {
                int i;
                NEXT_ARG
                i = strlen (pArg);
                if (i > (IL_EFS_MAX_NAME_CHARS - 1)) {
                    i = IL_EFS_MAX_NAME_CHARS - 1;
                    bcopy (pArg, inTypeName, i);
                    inTypeName[i] = 0;
                    }
                else strcpy (inTypeName, pArg);
                }
                break;

              case optInTIFF: 
                strcpy (inTypeName, "TIFF");
                break;

              case optInJFIF:
                strcpy (inTypeName, "JFIF");
                break;

              case optInPage:
                NEXT_ARG
                page = GetLong (pArg, pOpt);
                if (page < 0)
                    page = 0;
                break;

              case optInAllPages:
                page = ALL_PAGES;
                break;

              case optInRoot: 
                pR->read.rw.type = ILU_RW_X;
                pR->read.rw.windowUsage = ILU_WINDOW_ROOT;
                break;

              case optInWindow: 
                pR->read.rw.type = ILU_RW_X;
                pR->read.rw.windowUsage = ILU_WINDOW_ID;
                NEXT_ARG
                pR->read.rw.windowID = GetLong (pArg, pOpt);
                break;

              case optInDisplay: 
                NEXT_ARG
                pO->srcDisplayName = pArg;
                break;

              case optInCrop: 
                pR->read.rw.doCrop = TRUE;
                NEXT_ARG
                pR->read.rw.cropRect.x = GetLong (pArg, pOpt);
                NEXT_ARG
                pR->read.rw.cropRect.y = GetLong (pArg, pOpt);
                NEXT_ARG
                pR->read.rw.cropRect.width = GetLong (pArg, pOpt);
                NEXT_ARG
                pR->read.rw.cropRect.height = GetLong (pArg, pOpt);
                break;

              case optOutFile: 
                pR->write.rw.type = ILU_RW_EFS; /* may become "ILU_WRITE_TIFF" */
                NEXT_ARG
                pO->outFileName = pArg;
                break;

              case optOutFileType: {
                int i;
                NEXT_ARG
                i = strlen (pArg);
                if (i > (IL_EFS_MAX_NAME_CHARS - 1)) {
                    i = IL_EFS_MAX_NAME_CHARS - 1;
                    bcopy (pArg, pO->outTypeName, i);
                    pO->outTypeName[i] = 0;
                    }
                else strcpy (pO->outTypeName, pArg);
                }
                break;

              case optOutRoot:
                pR->write.rw.type = ILU_RW_X;
                pR->write.rw.windowUsage = ILU_WINDOW_ROOT;
                break;

              case optOutCreateWindow: 
                pR->write.rw.type = ILU_RW_X;
                pR->write.rw.windowUsage = ILU_WINDOW_CREATE;
                break;

              case optOutWindow: 
                pR->write.rw.type = ILU_RW_X;
                pR->write.rw.windowUsage = ILU_WINDOW_ID;
                NEXT_ARG
                pR->write.rw.windowID = GetLong (pArg, pOpt);
                break;

              case optOutNull:
                pR->write.rw.type = ILU_WRITE_NULL;
                break;

              case optOutImage:
                pR->write.rw.type = ILU_WRITE_IMAGE;
                break;

              case optOutNoWM:
                pR->write.noWM = TRUE;
                break;

              case optOutWindowPos:
                NEXT_ARG
                pR->write.windowX = GetLong (pArg, pOpt);
                NEXT_ARG
                pR->write.windowY = GetLong (pArg, pOpt);
                break;

              case optOutDstCoord:
                NEXT_ARG
                pR->write.dstX = GetLong (pArg, pOpt);
                NEXT_ARG
                pR->write.dstY = GetLong (pArg, pOpt);
                break;

              case optOutDefaultVisual:
                pR->write.useDefaultVisual = TRUE;
                break;

              case optOutPrivateColormap:
                pR->write.usePrivateColormap = TRUE;
                break;

              case optOutAlloc:
                pR->write.useAllColors = TRUE;
                break;

              case optOutDitherMethod:
                NEXT_ARG
                if (!FindInList (pArg, cpNames, CP_NAMES_SIZE, &i)) {
                    fprintf (stderr, "? '%s' not valid for '-odmethod'.\n", pArg);
                    exit (1);
                    }
                pR->write.ditherMethod = i;
                break;

              case optOutCrop: 
                pR->write.rw.doCrop = TRUE;
                NEXT_ARG
                pR->write.rw.cropRect.x = GetLong (pArg, pOpt);
                NEXT_ARG
                pR->write.rw.cropRect.y = GetLong (pArg, pOpt);
                NEXT_ARG
                pR->write.rw.cropRect.width = GetLong (pArg, pOpt);
                NEXT_ARG
                pR->write.rw.cropRect.height = GetLong (pArg, pOpt);
                break;

              case optOutDisplay: 
                NEXT_ARG
                pO->dstDisplayName = pArg;
                break;

              case optOutAppend: 
                needTIFFOutput = TRUE;
                pR->write.doAppend = TRUE;
                break;

              case optOutPage: 
                needTIFFOutput = TRUE;
                NEXT_ARG
                pR->write.page = GetLong (pArg, pOpt);
                NEXT_ARG
                pR->write.nPages = GetLong (pArg, pOpt);
                break;

              case optOutRes:
                NEXT_ARG
                pR->write.xRes = GetLong (pArg, pOpt);
                NEXT_ARG
                pR->write.yRes = GetLong (pArg, pOpt);
                break;

              case optOutComp:
                needTIFFOutput = TRUE;
                NEXT_ARG
                if (!FindInList (pArg, compNames, COMP_NAMES_SIZE, &i)) {
                    fprintf (stderr, "? '%s' not valid for '-ocomp'.\n", pArg);
                    exit (1);
                    }
                pR->write.pCompData = (ilPtr)NULL;
                switch (i) {
                  case compUncompressed:
                    pR->write.compression = IL_UNCOMPRESSED;
                    break;

                    /*  G3: same as G3 TIFF, except point pCompData to
                        mask of bits indicating TIFF class F compatibility.
                    */
                  case compG3:
                    g3Options = IL_G3M_CLASS_F;
                    pR->write.pCompData = (ilPtr)&g3Options;
                    pR->write.compression = IL_G3;
                    break;
                  case compG32D:
                    g3Options = IL_G3M_2D | IL_G3M_EOL_MARKERS;
                    pR->write.pCompData = (ilPtr)&g3Options;
                    pR->write.compression = IL_G3;
                    break;
                  case compG3TIFF:
                    pR->write.compression = IL_G3;
                    break;

                    /*  G4: for backwards-compatibility with IL version 2.0 and
                        earlier, allow "g4lsb" option which writes the file
                        in LSB first bit order.  The "-ostrip" option must also
                        be used with a strip size equal to the image height or
                        larger, e.g. "-ostrip 1000000".  The resulting file can
                        be read by the new and old versions of the IL.  The
                        reason this is not the IL default is that files with
                        strips can perform better, and some other TIFF packages
                        assume MSB first order and the resulting file may not
                        be readable by those packages.  Define the IL v2.1 bit
                        def in case compiling with 2.0 header files.
                    */
#ifndef IL_G4M_LSB_FIRST
#define IL_G4M_LSB_FIRST     (1<<0)
#endif
                  case compG4LSB:
                    g4Options = IL_G4M_LSB_FIRST;
                    pR->write.pCompData = (ilPtr)&g4Options;
                  case compG4:
                    pR->write.compression = IL_G4;
                    break;
                  case compLZW:
                    pR->write.compression = IL_LZW;
                    break;
                  case compPackBits:
                    pR->write.compression = IL_PACKBITS;
                    break;
                  case compJPEG:
                    pR->write.compression = IL_JPEG;
                    break;
                  case compCurrent:
                    pR->write.compression = IL_WRITE_CURRENT_COMPRESSION;
                    break;
                    }
                break;

              case optOutCompQ:
                needTIFFOutput = TRUE;
                NEXT_ARG
                JPEGData.Q = GetLong (pArg, pOpt);
                JPEGData.mustbezero = 0;
                pR->write.pCompData = (ilPtr)&JPEGData;
                break;

              case optOutStrip:
                needTIFFOutput = TRUE;
                NEXT_ARG
                pR->write.stripHeight = GetLong (pArg, pOpt);
                break;

              case optCrop: 
                pR->crop.enabled = TRUE;
                NEXT_ARG
                pR->crop.rect.x = GetLong (pArg, pOpt);
                NEXT_ARG
                pR->crop.rect.y = GetLong (pArg, pOpt);
                NEXT_ARG
                pR->crop.rect.width = GetLong (pArg, pOpt);
                NEXT_ARG
                pR->crop.rect.height = GetLong (pArg, pOpt);
                break;

              case optScale:
                pR->scale.enabled = TRUE;
                NEXT_ARG
                pR->scale.width = GetLong (pArg, pOpt);
                NEXT_ARG
                pR->scale.height = GetLong (pArg, pOpt);
                break;

              case optScaleMethod:
                NEXT_ARG
                if (!FindInList (pArg, smNames, SM_NAMES_SIZE, &i)) {
                    fprintf (stderr, "? '%s' not valid for '-smethod'.\n", pArg);
                    exit (1);
                    }
                pR->scale.method = i;
                break;

              case optScaleThreshold:
                NEXT_ARG
                pR->scale.biThreshold = GetLong (pArg, pOpt);
                break;

              case optConvert:
                pR->convert.enabled = TRUE;
                NEXT_ARG
                if (!FindInList (pArg, cNames, C_NAMES_SIZE, &i)) {
                    fprintf (stderr, "? '%s' not valid for '-convert'.\n", pArg);
                    exit (1);
                    }
                pR->convert.type = i;
                break;

              case optConvertThreshold:
                pR->convert.option = IL_CONVERT_THRESHOLD;
                NEXT_ARG
                pR->convert.biThreshold = GetLong (pArg, pOpt);
                break;

              case optConvertSubsample:
                NEXT_ARG
                i = GetLong (pArg, pOpt);
                if ((i != 1) && (i != 2) && (i != 4)) {
                    fprintf (stderr, "? '-csubsample' value not 1, 2 or 4.\n");
                    exit (1);
                    }
                pR->convert.subsample = i;
                break;

              case optConvertPalette:
              { int     nLevels;
                pR->convert.option = IL_CONVERT_TO_PALETTE;
                NEXT_ARG
                if (!FindInList (pArg, cpNames, CP_NAMES_SIZE, &i)) {
                    fprintf (stderr, "? '%s' not valid for '-cpalette'.\n", pArg);
                    exit (1);
                    }
                nLevels = (i == IL_CHOOSE_COLORS) ? 1 : 3;
                pR->convert.palette.method = i;
                pR->convert.palette.kernelSize = 8;
                for (i = 0; i < nLevels; i++) {
                    NEXT_ARG
                    pR->convert.palette.levels[i] = GetLong (pArg, pOpt);
                    }
                break;
              }

              case optGamma:
                pR->gamma.enabled = TRUE;
                NEXT_ARG
                pR->gamma.factor = GetDouble (pArg, pOpt);
                break;

              case optMirror:
                pR->mirror.enabled = TRUE;
                NEXT_ARG
                if (!FindInList (pArg, mirrorNames, MIRROR_NAMES_SIZE, &i)) {
                    fprintf (stderr, "? '%s' not valid for '-mirror'.\n", pArg);
                    exit (1);
                    }
                pR->mirror.direction = i;
                break;

              case optRotate90:
                pR->rotate90.enabled = TRUE;
                NEXT_ARG
                pR->rotate90.factor = GetLong (pArg, pOpt);
                break;

              case optInvert:
                pR->invert.enabled = TRUE;
                break;

              case optComp:
                NEXT_ARG
                if (!FindInList (pArg, compNames, COMP_NAMES_SIZE, &i)) {
                    fprintf (stderr, "? '%s' not valid for '-comp'.\n", pArg);
                    exit (1);
                    }
                pR->compress.enabled = TRUE;
                pR->compress.pCompData = (ilPtr)NULL;
                switch (i) {
                  case compUncompressed:
                    pR->compress.compression = IL_UNCOMPRESSED;
                    break;

                    /*  G3: same as G3 TIFF, except point pCompData to
                        mask of bits indicating TIFF class F compatibility.
                    */
                  case compG3:
                    g3Options = IL_G3M_CLASS_F;
                    pR->compress.pCompData = (ilPtr)&g3Options;
                  case compG3TIFF:
                    pR->compress.compression = IL_G3;
                    break;
                  case compG32D:
                    g3Options = IL_G3M_2D | IL_G3M_EOL_MARKERS;
                    pR->compress.pCompData = (ilPtr)&g3Options;
                    pR->compress.compression = IL_G3;
                    break;

                    /*  G4: see comments above for -ocomp */
                  case compG4LSB:
                    g4Options = IL_G4M_LSB_FIRST;
                    pR->compress.pCompData = (ilPtr)&g4Options;
                  case compG4:
                    pR->compress.compression = IL_G4;
                    break;
                  case compLZW:
                    pR->compress.compression = IL_LZW;
                    break;
                  case compPackBits:
                    pR->compress.compression = IL_PACKBITS;
                    break;
                  case compJPEG:
                    pR->compress.compression = IL_JPEG;
                    break;
                  case compCurrent:
                    pR->compress.compression = IL_WRITE_CURRENT_COMPRESSION;
                    break;
                    }
                break;

              case optCompQ:
                NEXT_ARG
                compJPEGData.Q = GetLong (pArg, pOpt);
                compJPEGData.mustbezero = 0;
                pR->compress.pCompData = (ilPtr)&compJPEGData;
                break;

              case optDecomp:
                pR->decompress.enabled = TRUE;
                break;

              case optChecksum:
                pR->checksum.enabled = TRUE;
                pR->checksum.pChecksum = &pipeChecksum;
                break;

              case optTime:
                pO->doTime = TRUE;
                break;

              case optListEFS:
                pO->doListEFS = TRUE;
                break;

              case optListFile:
                pO->doListFile = TRUE;
                break;

              case optListPage:
                pO->doListPage = TRUE;
                break;

              case optClear:
                pR->write.clearWindow = TRUE;
                break;

              case optRepeat:
                NEXT_ARG
                pO->repeatCount = GetLong (pArg, pOpt);
                break;

              case optPause:
                NEXT_ARG
                pO->pauseTime = GetLong (pArg, pOpt);
                break;

              case optSleep:
                pO->haveSleepTime = TRUE;
                NEXT_ARG
                pO->sleepTime = GetLong (pArg, pOpt);
                break;

              case optImage:
                pR->general.useImage = TRUE;
                break;

              default:
                fprintf (stderr, "? Internal error: '%s' not handled.\n", pArg);
                exit (1);
                }   /* END switch option */
            }       /* END an option */
        }           /* END while argc */

        /*  If outFileType is TIFF, change out type from EFS to TIFF if needed:
            if append mode, or a specific compression requested.
        */
    if ((pR->write.rw.type == ILU_RW_EFS)
     && needTIFFOutput
     && (STRING_EQUAL (pO->outTypeName, "TIFF")))
        pR->write.rw.type = ILU_WRITE_TIFF;
}


    /*  ------------------------- Execute ---------------------------------- */
    /*  Execute the options.
    */
static void Execute (
    register OptionPtr      pO,
    register iluRequestPtr  pR
    )
{
    iluError        error;
    iluControl      control;
    FileInfoPtr     pInFile;
    ilBool          haveInFiles, multiWrites, multiPage;
    long            sleepTime, i;
    int             nFilesOpened = 0;
    ilEFSFileInfo   fileInfo;
    ilEFSPageInfo   pageInfo;
    long            startTime, endTime, elapsedTime;


        /*  If enabled list EFS file types and relevant info */
    if (pO->doListEFS)
        iluPrintEFSFileTypeInfo (pO->context);

        /*  If read file option selected and no input files specified:
            error if not list efs; exit.
        */
    if ((pR->read.rw.type == ILU_RW_EFS) && !pO->pInFilesHead) {
        if (pO->doListEFS)
            exit (0);
        else {
            fprintf (stderr, "? No input specified.\n");
            exit (1);
            }
        }

        /*  Open output file or X display depending on output.
            Set "multiWrites" false if outputting single-page EFS file type.
        */
    switch (pR->write.rw.type) {
      case ILU_RW_X:
        multiWrites = TRUE;
        pR->write.rw.display = XOpenDisplay (pO->dstDisplayName);
        if (!pR->write.rw.display) {
            fprintf (stderr, "? Error opening (output) X display.\n");
            exit (1);
            }
        break;

        /*  For TIFF, open in read/write mode if -oappend, else write mode */
      case ILU_WRITE_TIFF:
        multiWrites = TRUE;
        pR->write.rw.stream = fopen (pO->outFileName, 
                                    (pR->write.doAppend) ? "r+" : "w+");
        if (!pR->write.rw.stream) {
            fprintf (stderr, "? Error writing file '%s'.\n", pO->outFileName);
            exit (1);
            }
        break;

      case ILU_RW_EFS:
        pR->write.rw.EFSFile = ilEFSOpen (pO->context, pO->outFileName,
            IL_EFS_WRITE, IL_EFS_BY_TYPE_NAME, pO->outTypeName, NULL);
        if (!pR->write.rw.EFSFile) {
            fprintf (stderr, "? Error '%d' from ilEFSOpen(), writing file '%s'.\n", 
                             pO->context->error, pO->outFileName);
            exit (1);
            }
        if (!ilEFSGetFileInfo (pR->write.rw.EFSFile, &fileInfo)) {
            fprintf (stderr, "? Error '%d' querying EFS file '%s'.\n", 
                             pO->context->error, pO->outFileName);
            exit (1);
            }
        multiWrites = (fileInfo.attributes & IL_EFS_MULTI_PAGE_WRITES) != 0;
        break;
        }   /* END switch type of write */

        /*  Open src display if X input */
    if (pR->read.rw.type == ILU_RW_X) {
        pR->read.rw.display = XOpenDisplay (pO->srcDisplayName);
        if (!pR->read.rw.display) {
            fprintf (stderr, "? Error opening (input) X display.\n");
            exit (1);
            }
        }

        /*  If reading from file(s): if writing to a file, set pipe repeatCount
            to 1.  If repeatCount > 1, open in random mode, else sequential.
        */
    if (pR->read.rw.type == ILU_RW_EFS) {
        haveInFiles = TRUE;
        pInFile = pO->pInFilesHead;
        if (pO->repeatCount > 1)
            if ((pR->write.rw.type != ILU_RW_X) 
             && (pR->write.rw.type != ILU_WRITE_IMAGE)
             && (pR->write.rw.type != ILU_WRITE_NULL))
                pO->repeatCount = 1;
        }
    else {
        haveInFiles = FALSE;
        pInFile = (FileInfoPtr)NULL;
        }

        /*  Loop until no more files to do, or only once if !multiWrites.  
            For each loop, build a pipe and execute it repeatCount times.
        */
    pR->read.rw.EFSFile = (ilEFSFile)NULL;      /* no file open yet */
    do {
            /*  If input files: If a file already open: read more from it
                unless single-page file or no more pages available.
            */
        if (haveInFiles) {
            if (pR->read.rw.EFSFile) {
                if (multiPage) {
                    if (!ilEFSGetPageInfo (pR->read.rw.EFSFile, &pageInfo)) {
                        fprintf (stderr, "? Error '%d' querying file '%s'.\n", 
                                 pO->context->error, pInFile->fileName);
                        exit (1);
                        }
                    if (pageInfo.attributes & IL_EFS_EOF)
                        multiPage = FALSE;
                    }
                if (!multiPage) {
                    ilDestroyObject (pR->read.rw.EFSFile);  /* close file */
                    pR->read.rw.EFSFile = (ilEFSFile)NULL;
                    pInFile = pInFile->pNext;
                    if (!pInFile)
                        break;                  /* no more files; quit */
                    }
                }

                /*  If no file open, open one, find out if multiPage.
                    Use type name only if specified in cmd line (non-null).
                */
            if (!pR->read.rw.EFSFile) {
                unsigned long   searchOptions = IL_EFS_READ_SEARCH;
                unsigned int    openMode;
                char           *pTypeName;

                if ((pO->repeatCount > 1) || (pInFile->page > 0))
                    openMode = IL_EFS_READ;
                else openMode = IL_EFS_READ_SEQUENTIAL;

                if (pInFile->typeName[0]) {     /* non-null type name */
                    searchOptions |= IL_EFS_BY_TYPE_NAME;
                    pTypeName = pInFile->typeName;
                    }
                else pTypeName = (char *)NULL;

                pR->read.rw.EFSFile = ilEFSOpen (pO->context, pInFile->fileName,
                    openMode, searchOptions, pTypeName, NULL);
                if (!pR->read.rw.EFSFile) {
                    fprintf (stderr, "? Error '%d' opening file '%s'.\n", 
                             pO->context->error, pInFile->fileName);
                    pInFile = pInFile->pNext;   /* continue if more files */
                    if (pInFile)
                         continue;
                    else break;                 /* no more files; quit */
                    }
                if (!ilEFSGetFileInfo (pR->read.rw.EFSFile, &fileInfo)) {
                    fprintf (stderr, "? Error '%d' querying file '%s'.\n", 
                                     pO->context->error, pInFile->fileName);
                    exit (1);
                    }
                if (pO->doListFile || pO->doListPage)
                    printf ("\n **** File: '%s' ***\n", pInFile->fileName);
                if (pO->doListFile)
                    iluPrintEFSFileInfo (pR->read.rw.EFSFile);

                    /*  If -ipage given: seek to given page #, or to last
                        page if page number (+1) > # pages.
                    */
                if (pInFile->page > 0) {
                    long page = pInFile->page;
                    if (page >= fileInfo.nPages)
                        page = fileInfo.nPages - 1;
                    if (!ilEFSSeek (pR->read.rw.EFSFile, page, NULL)) {
                        fprintf (stderr, "? Error '%d' seeking file '%s'.\n", 
                                        pO->context->error, pInFile->fileName);
                        exit (1);
                        }
                    }

                    /*  Set multiPage only if multipage file and -iallpages */
                multiPage = (fileInfo.attributes & IL_EFS_MULTI_PAGE_READS)
                            && (pInFile->page == ALL_PAGES);
                nFilesOpened++;
                }   /* END open file */

                /*  Positioned at page to read; print if enabled. */
            if (pO->doListPage)
                iluPrintEFSPageInfo (pR->read.rw.EFSFile);
            }       /* END have input files */

        error = iluCreateControl (pR, &control);
        if (error) {
            fprintf (stderr, "? Error #%d creating control.\n", error);
            exit (1);
            }

            /*  Execute the pipe -repeat times; pause -pause seconds after. */
        if (pO->doTime)
            startTime = clock();
        for (i = 0; i < pO->repeatCount; i++) {
            error = iluExecuteControl (control);
            if (error) {
                if (error == IL_ERROR_PIPE_ELEMENT)
                    fprintf (stderr, "? Error #%d from pipe element.\n",
                                      pO->context->errorInfo);
                else
                    fprintf (stderr, "? Error #%d executing control.\n", error);
                exit (1);
                }
            if (pR->checksum.enabled)
                printf ("Checksum = %d\n", pipeChecksum);
            if (pO->pauseTime > 0)
                sleep (pO->pauseTime);
            }

        if (pO->doTime) {
            endTime = clock();
            elapsedTime = endTime - startTime;
            printf ("    %f seconds per repeat; %f total\n",
                    (float)elapsedTime / pO->repeatCount / 1000000,
                    (float)elapsedTime / 1000000);
            }

            /*  If more input files, switch to append mode and increment
                page # if start page specified.
            */
        iluDestroyControl (control);
        pR->write.doAppend = TRUE;
        if (pR->write.page >= 0)
            pR->write.page++;
        } while (multiWrites && pInFile);

        /*  If the output option was to create a window, then sleep "forever",
            or the given sleep time if one given. If an output file, close it.
        */
    sleepTime = pO->sleepTime;

    switch (pR->write.rw.type) {
      case ILU_RW_X:
       if ((pR->write.rw.windowUsage == ILU_WINDOW_CREATE)
        && !pO->haveSleepTime
        && !(haveInFiles && (nFilesOpened == 0)))
        sleepTime = 65535;
        break;
      case ILU_WRITE_TIFF:
        fclose (pR->write.rw.stream);
        break;
      case ILU_RW_EFS:
        ilDestroyObject (pR->write.rw.EFSFile);
        break;
        }

    if (sleepTime > 0)
        sleep (sleepTime);
}


    /*  --------------------------- main ---------------------------------- */
    /*  Mainline for imageutil.  Create/init an ilu request block, parse the 
        command line and fill in the request block, then execute.
    */
main (
    int         argc, 
    char       *argv[]
    )
{
    OptionPtr       pO;
    iluRequestPtr   pR;
    iluControl      control;
    ilContext       context;
    ilError         error;

    error = IL_CREATE_CONTEXT (&context, 0);
    if (error) {
        fprintf (stderr, "? Error #%d initializing Image Library.\n", error);
        exit (1);
        }

    pR = iluCreateRequest (context);
    pO = (OptionPtr)malloc (sizeof (OptionRec));
    if (!pO || !pR) {
        fprintf (stderr, "? Failure to allocate memory.\n");
        exit (1);
        }

    pO->context = context;
    ParseCommandLine (argc, argv, pO, pR);
    Execute (pO, pR);

    ilDestroyContext (context);
    
    exit (0);
}


