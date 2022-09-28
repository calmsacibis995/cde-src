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

#ifndef ILU_H
#define ILU_H

/*
    Image Library Utility (ILU) package - a simplified package for the using 
    the Image Library (IL).
*/


#include "il.h"
#include "ilfile.h"
#include "ilefs.h"
#include "ilX.h"

    /*  iluError: error codes are an integer as follows:
            < 0     an error specific to ILU (see ILU_ERROR_? list below)
            == 0    IL_OK: success
            > 0     an IL error code
    */
typedef ilError iluError;
#define ILU_OK  IL_OK

    /*  Type for checksums calculated by iluChecksumPipe() */
typedef unsigned short iluChecksum;

    /*  No supported images in the input TIFF file(s). */
#define ILU_ERROR_NULL_TIFF_FILE        -1

    /*  Error mallocing memory */
#define ILU_ERROR_MALLOC                -2

    /*  Invalid code for iluRequestRec.read.rw.type */
#define ILU_ERROR_READ_TYPE             -3

    /*  Invalid code for iluRequestRec.write.rw.type */
#define ILU_ERROR_WRITE_TYPE            -4

    /*  Invalid code for iluRequestRec.write.rw.windowUsage */
#define ILU_ERROR_WRITE_WINDOW_USAGE    -5

    /*  Error creating X output GC */
#define ILU_ERROR_WRITE_GC              -6

    /*  Error creating X output window */
#define ILU_ERROR_WRITE_WINDOW          -7

    /*  Invalid code for iluRequestRec.read.rw.windowUsage */
#define ILU_ERROR_READ_WINDOW_USAGE     -8

    /*  Invalid value for iluRequestRec.convert.type */
#define ILU_ERROR_CONVERT_TYPE          -9

    /*  Error reading/writing file */
#define ILU_ERROR_FILE_IO               -10

    /*  Input file is not legal JFIF. */
#define ILU_ERROR_NOT_JFIF              -11

    /*  EOF reached while reading/parsing input file */
#define ILU_ERROR_PREMATURE_EOF         -12

    /*  Version of input file not supported */
#define ILU_ERROR_FILE_VERSION          -13

    /*  Type of JFIF not supported, e.g. unsupported precision */
#define ILU_ERROR_UNSUPPORTED_JFIF


    /*  Options for request.read/write.rw.type */
#define ILU_WRITE_TIFF  0           /* write (only) TIFF file */
#define ILU_RW_EFS      1           /* read/write an EFS file */
#define ILU_RW_X        2           /* read/write an X window */
#define ILU_WRITE_NULL  3           /* write (only) nothing (null consumer) */
#define ILU_WRITE_IMAGE 4           /* write (only) to a newly created image */

    /*  Options for request.read/write.windowUsage */
#define ILU_WINDOW_ROOT     0       /* use DefaultRootWindow */
#define ILU_WINDOW_ID       1       /* use window/pixmap with id "windowID" */
#define ILU_WINDOW_CREATE   2       /* write: create new window (default) */

    /*  Structure which defines reads (producers) and writes (consumers)
        to be done by ILU.  It is the fields "read" and "write" for the
        iluRequestRec defined below.
            "type" defines the read/write operation: ILU_RW_EFS (read/write
        an EFS file) or ILU_RW_X (read/write an X window).
            If "doCrop" is true, a crop to "cropRect" will be done after a
        read, or before a write.
            If "type" is ILU_WRITE_TIFF, then "stream" must be a file stream
        from fopen().  Note that "stream" is a misnomer, since normal streams
        (e.g. stdin/stdout) cannot be used for TIFF files, which require
        random access.  "r" should be used when opening a TIFF file for 
        reading; "w+" when opening a file for writing.
            If "type" is ILU_RW_EFS, then "EFSfile" must be a open EFS file.
            If "type" is ILU_RW_X, then "display" must be the X display
        returned from XOpenDisplay().  "windowUsage" is one of ILU_WINDOW_
        ROOT (read/write default root window of "display"); ID (read/write
        window whose id is "windowID"; or CREATE (write only: create a new
        window the size of the pipe image).
    */
typedef struct {
    int         type;           /* dflt: ILU_RW_ EFS(read), X(write) */
    ilShortBool doCrop;         /* dflt: FALSE */
    ilRect      cropRect;       /* dflt: 0, 0, 1000000, 1000000 */

        /*  Write-only: fields used for type = ILU_WRITE_TIFF. */
    FILE       *stream;         /* no dflt */

        /*  Fields used for type = ILU_RW_EFS. */
    ilEFSFile   EFSFile;        /* no dflt */

        /*  Fields used for type = ILU_X. */
    Display    *display;        /* no dflt */
    int         windowUsage;    /* dflt: ILU_WINDOW_ROOT(read) CREATE(write) */
    Window      windowID;       /* no dflt */
    } iluReadWriteRec, *iluReadWritePtr;


typedef struct {

        /*  General control information.
                "maskEnable": if true, reading (from EFS files that support 
            masks) and writing (to existing window, or to files that support
            masks) of mask images will be done.  A mask image is like a single
            bit alpha value: a 0 disables drawing the corresponding pixel
            (is transparent); a 1 enables drawing.
                "useImage": if true, iluCreateControl() reads from the source
            into a internal image, then starts the pipe with a read from that
            image.  iluExecuteControl() then executes pipe starting with a read
            from the image.
        */
    struct {
        ilBool          masksEnabled;   /* dflt: true */
        ilBool          useImage;       /* dflt: false */
        } general;

        /*  Read (producer) information: see notes for iluReadWriteRec. */
    struct {
        iluReadWriteRec rw;
        } read;

        /*  Filters - default is to add no filters (i.e. all the 
            "<filter>.enabled" booleans are defaulted to false).
        */

        /*  Crop: call ilCrop() immediately after reading from source.
            Note that this is different from read.rw.doCrop.
        */
    struct {
        ilShortBool     enabled;        /* dflt: FALSE */
        ilRect          rect;           /* dflt: 0, 0, 1000000, 1000000 */
        } crop;

        /*  Scale: scale to width, height.  "method" is one of:
                IL_SCALE_BITONAL_TO_GRAY: do scaling to gray,  unless not 
                   bitonal image or scaling up, in which case becomes:
                IL_SCALE_SAMPLE (default): use sample scaling.  If bitonal,
                    biThreshold (dflt: 128) is the threshold when scaling 
                    down bitonal.
                IL_SCALE_SIMPLE: simple decimation/replication
        */
    struct {
        ilShortBool     enabled;        /* dflt: FALSE */
        long            width, height;  /* dflt: 0, 0 */
        int             method;         /* dflt: IL_SCALE_SAMPLE */
        unsigned long   biThreshold;    /* dflt: 128 */
        } scale;

        /*  Convert to "type": one of IL_BITONAL/GRAY/RGB.
                "option" is the option to ilConvert(), e.g. IL_CONVERT_THRESHOLD
            (use threshold "biThreshold" when converting to bitonal - 
            default, option 0, is to use dithering).
                "subsample" is for conversions to YCbCr only.  It is the 
            hori/vert subsampling factor for Cb and Cr components: 1 (none),
            2 (1/4 the pixels - 1/2 each dimension) or 4 (1/16 the pixels).
                "palette" is pData to ilConvert for "option" equal to
            IL_CONVERT_TO_PALETTE.
        */
    struct {
        ilShortBool     enabled;        /* dflt: FALSE */
        int             type;           /* dflt: IL_BITONAL */
        int             option;         /* dflt: 0 */
        unsigned long   biThreshold;    /* dflt: 128 */
        int             subsample;      /* dflt: 1 (no subsampling) */
        ilConvertToPaletteInfo palette; /* dflt: IL_DIFFUSION, 484, 8, IL_PALETTE, NULL */
        } convert;

        /*  Gamma: used for RGB and bitonal images only - but note
            that convert is done before gamma.  A "factor" of 1.0 (default)
            is a noop; values < 1.0 darken the image; values > 1.0 brighten
            the image.
        */
    struct {
        ilShortBool     enabled;        /* dflt: FALSE */
        double          factor;         /* dflt: 1.0 */
        } gamma;

        /*  Mirror: mirror using "direction": IL_MIRRORX (mirror around X
            axis, i.e. up and down) or IL_MIRRORY (mirror around Y axis).
        */
    struct {
        ilShortBool     enabled;        /* dflt: FALSE */
        int             direction;      /* dflt: IL_MIRRORX */
        } mirror;

        /*  Rotate 90 degree increments: rotate "factor" * 90 degrees
            clockwise; if factor is negative rotate counter-clockwise.
        */
    struct {
        ilShortBool     enabled;        /* dflt: FALSE */
        int             factor;         /* dflt: 0 */
        } rotate90;

        /*  Invert: invert if a bitonal or gray image after "convert".
        */
    struct {
        ilShortBool     enabled;        /* dflt: FALSE */
        } invert;

        /*  Compress: call ilCompress() using the given args.  Done after
            all other filters but before "decompress".
        */
    struct {
        ilShortBool     enabled;        /* dflt: FALSE */
        int             compression;    /* dflt: IL_UNCOMPRESSED */
        ilPtr           pCompData;      /* dflt: NULL */
        long            stripHeight;    /* dflt: 0 => use IL's default */
        } compress;

        /*  Decompress: force the image to be decompressed.
        */
    struct {
        ilShortBool     enabled;        /* dflt: FALSE */
        } decompress;

        /*  Checksum: calculate a checksum, returned to area pointed to 
            "pChecksum" which must point to a static data value.
        */
    struct {
        ilShortBool     enabled;        /* dflt: FALSE */
        iluChecksum    *pChecksum;      /* no dflt */
        } checksum;

        /*  Write (consumer) information: see notes for iluReadWriteRec.

            Fields for writing X drawables (rw.type == ILU_RW_X):
                If "clearWindow" is true, a clear window is done on the
            output window before executing the pipe (rw.type == ILU_RW_X).
                If "useDefaultVisual" is true, the image is always output
            to the default visual (dflt: use a True/DirectColor visual to
            display RGB images if such a visual exists).
                If "usePrivateColormap" is true and the visual is PseudoColor
            or GrayScale a private colormap is used (dflt: use DefaultColormap)
                If "useAllColors" is true then all available colors in
            the colormap are used for display.  Ignored if a bitonal image;
            if a gray image as many grays as are available are used; if
            a palette image allocate colors specific to palette image; 
            if a color image, convert to palette using IL_CHOOSE_COLORS with
            with the number of colors = # available.
                "dstX, dstY" is the offset within the output window where
            the image is displayed (dflt: 0,0).
                "noWM" is used only if windowUsage is ILU_WINDOW_CREATE.
            If true the OverrideRedirect option is applied to the created
            window, causing the Window Manager not to see it - no borders.
            (dflt: false)
                "windowX, windowY" is the window position for a created
            window.  Generally this will be ignored (and the window
            "auto-placed" unless "noWM" is true.

            Fields for writing a TIFF/EFS file:
                "xRes", "yRes" is the x/y resolution of the image, in 
            dots per inch (dpi) times 2, e.g. 200 is 100 dpi.

            Fields for writing a TIFF file (rw.type == ILU_WRITE_TIFF):
                If "doAppend" is true, the image(s) written are appended
            to the output file.  If "doAppend" is false, a new file is created,
            and all subsequent images are appended to the file 
                "compression" is the compression to be used to write the 
            image(s); "pCompData" points to optional compression data.
            The default is use the current compression: same as input file
            if no operations performed, else uncompressed.
                "page" and "nPages" are as passed to ilWriteFileImage():
            if both are -1, no page numbers are written (recommended for
            single page files); otherwise, "nPages" is the number of pages
            in the sequence, and "page" is the page number of the image
            (first image is 0).  If multiple images are written, the page
            number written is incremented after each write.
                "stripHeight" is as passed to ilWriteFileImage(): the height
            of strips to be written to the file, or zero (0) which causes
            the IL default strip height to be used (recommended).
        */
    struct {
        iluReadWriteRec rw;
        ilShortBool     clearWindow;        /* dflt: FALSE */
        ilShortBool     useDefaultVisual;   /* dflt: FALSE */
        ilShortBool     usePrivateColormap; /* dflt: FALSE */
        ilShortBool     useAllColors;       /* dflt: FALSE */
        int             dstX, dstY;     /* dflt: (0,0) */
        ilShortBool     noWM;           /* dflt: FALSE */
        int             windowX, windowY; /* dflt: (0,0) */
        int             ditherMethod;   /* dflt: -1 => use IL's default */
        long            xRes, yRes;     /* dflt: 200 x 200 */
        ilShortBool     doAppend;       /* dflt: FALSE */
        int             compression;    /* dflt: IL_WRITE_CURRENT_COMPRESSION */
        ilPtr           pCompData;      /* dflt: NULL */
        short           page;           /* dflt: -1 */
        short           nPages;         /* dflt: -1 */
        long            stripHeight;    /* dflt: 0 => use IL's default */
        } write;

        /*  Rest of fields internal - dont set them */
    ilContext       context;    /* inited by iluCreateRequest() */
    } iluRequestRec, *iluRequestPtr;


    /*  An iluControl is returned by iluRequest() and is passed to 
        iluExecute().  Its contents are not exposed (internal to ILU).
    */
typedef ilPtr iluControl;


    /*  FUNCTIONS in ilu.c :
    */

    /*  Allocate and initialize an iluRequestRec and return a pointer to it
        or null if failure.
    */
extern iluRequestPtr iluCreateRequest (
    ilContext       context
    );

    /*  Create a control rec from the given request information (*pRequest)
        and return a ptr to it, to *pControl, or return error.  The request
        is then executed by calling iluExecuteControl () with *pControl.
        "context" is an IL context from IL_CREATE_CONTEXT().
            Note that to some extent the request is executed by this function:
        a window may be created, the TIFF file header may be written, etc.
        iluExecuteControl() must be called to complete the request however.
    */
extern iluError iluCreateControl (
    iluRequestPtr   pR,
    iluControl     *control            /* RETURNED */
    );

extern iluError iluExecuteControl (
    iluControl      control
    );

    /*  Destroy the request rec created by iluCreateRequest().
    */
extern iluError iluDestroyRequest (
    iluRequestPtr    pR
    );

extern iluError iluDestroyControl (
    iluControl      control
    );

#endif

