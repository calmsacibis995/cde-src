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

    /*  ilulist.c - Functions that print info as part of imageutil.
    */

#include "ilu.h"
#include "ilerrors.h"
#include <stdio.h>

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif

        /*  -------------------- iluPrintEFSFileTypeInfo ----------------- */
        /*  List all EFS file types associated with the given context
            and print out relevant info for each.
        */
void iluPrintEFSFileTypeInfo (
    ilContext       context
    )
{
    int             nFileTypes, i;
    ilEFSFileType  *pFileTypes, *pFileType;
    ilEFSFileTypeInfo info;

        /*  Get a pointer to a list of all file type handles */
    if (!ilEFSListFileTypes (context, &nFileTypes, &pFileTypes)) {
        fprintf (stderr, "? Error '%d' from ilEFSListFileTypes().\n", 
                 context->error);
        return;
        }

    pFileType = pFileTypes;
    while (nFileTypes-- > 0) {
        if (!ilEFSGetFileTypeInfo (*pFileType, &info)) {
            fprintf (stderr, "? Error '%d' from ilEFSGetFileTypeInfo().\n", 
                     context->error);
            continue;
            }

            /*  Print out data from "info" */
        printf ("\n *** File type ***\n");
        printf ("  Name:         %s\n", info.name);
        printf ("  Display Name: '%s'\n", info.displayName);
        printf ("  Check Order:  %d\n", info.checkOrder);
        printf ("  Extensions:  ");
        for (i = 0; i < info.nExtensions; i++)
            printf (" %s", info.extensions[i]);
        printf ("\n");
        printf ("  Open Modes:  ");
        if (info.openModes & (1<<IL_EFS_READ)) 
            printf (" READ");
        if (info.openModes & (1<<IL_EFS_READ_SEQUENTIAL)) 
            printf (" READ_SEQUENTIAL");
        if (info.openModes & (1<<IL_EFS_WRITE)) 
            printf (" WRITE");
        printf("\n");
        printf ("  Attributes:\n");
        if (info.attributes & IL_EFS_MULTI_PAGE_READS)
            printf("    Multi-page reads\n");
        if (info.attributes & IL_EFS_MULTI_PAGE_WRITES)
            printf("    Multi-page writes\n");
        if (info.attributes & IL_EFS_MASK_READS)
            printf("    Mask reads\n");
        if (info.attributes & IL_EFS_MASK_WRITES)
            printf("    Mask writes\n");
        if (info.attributes & IL_EFS_SCALEABLE_READS)
            printf("    Scaleable reads\n");
        if (!info.attributes)
            printf("    None\n");

        pFileType++;                /* next file type in list */
        }
    free (pFileTypes);              /* free the list returned by IL */
}

        /*  -------------------- iluPrintEFSFileInfo ----------------- */
        /*  Query and print the info from the given EFS file.
        */
void iluPrintEFSFileInfo (
    ilEFSFile           file
    )
{
    ilEFSFileInfo       info;
    ilEFSFileTypeInfo   typeInfo;

    if (!ilEFSGetFileInfo (file, &info)) {
        fprintf (stderr, "? Error '%d' from ilGetFileInfo().\n", 
                 file->context->error);
        return;
        }

        /*  Query file type to print name (e.g. TIFF) */
    if (!ilEFSGetFileTypeInfo (info.fileType, &typeInfo)) {
        fprintf (stderr, "? Error '%d' from ilGetFileTypeInfo().\n", 
                 file->context->error);
        return;
        }
    printf ("    File type: %s ('%s')\n", typeInfo.name, typeInfo.displayName);
    printf ("    # pages: %d\n", info.nPages);
    printf ("\n");
}


        /*  -------------------- iluDesEqual ----------------- */
        /*  Return true if the two given ilImageDes structures
            *excluding compression* are equal; else return false.
        */
ilBool iluDesEqual (
    ilImageDes     *pDes1,
    ilImageDes     *pDes2
    )
{ 
    int i;

    if (pDes1->type              != pDes2->type         ||
        pDes1->blackIsZero       != pDes2->blackIsZero  ||
        pDes1->nSamplesPerPixel  != pDes2->nSamplesPerPixel ||
        pDes1->flags             != pDes2->flags ) 
        return FALSE;

    if (pDes1->compression ==  IL_JPEG) {
        if (pDes1->compInfo.JPEG.reserved != pDes2->compInfo.JPEG.reserved ||
            pDes1->compInfo.JPEG.process  != pDes2->compInfo.JPEG.process) 
            return FALSE;
        }
    else if (pDes1->compression ==  IL_G3) {
        if (pDes1->compInfo.g3.flags      != pDes2->compInfo.g3.flags ) 
            return FALSE;
        }
    
    if (pDes1->type == IL_YCBCR) {
        if (pDes1->typeInfo.YCbCr.sample[0].refBlack != pDes2->typeInfo.YCbCr.sample[0].refBlack  ||
            pDes1->typeInfo.YCbCr.sample[0].refWhite != pDes2->typeInfo.YCbCr.sample[0].refWhite ||
            pDes1->typeInfo.YCbCr.sample[0].subsampleHoriz != pDes2->typeInfo.YCbCr.sample[0].subsampleHoriz ||
            pDes1->typeInfo.YCbCr.sample[0].subsampleVert != pDes2->typeInfo.YCbCr.sample[0].subsampleVert ||
            pDes1->typeInfo.YCbCr.sample[1].refBlack != pDes2->typeInfo.YCbCr.sample[1].refBlack  ||
            pDes1->typeInfo.YCbCr.sample[1].refWhite != pDes2->typeInfo.YCbCr.sample[1].refWhite ||
            pDes1->typeInfo.YCbCr.sample[1].subsampleHoriz != pDes2->typeInfo.YCbCr.sample[1].subsampleHoriz ||
            pDes1->typeInfo.YCbCr.sample[1].subsampleVert != pDes2->typeInfo.YCbCr.sample[1].subsampleVert ||
            pDes1->typeInfo.YCbCr.sample[2].refBlack != pDes2->typeInfo.YCbCr.sample[2].refBlack  ||
            pDes1->typeInfo.YCbCr.sample[2].refWhite != pDes2->typeInfo.YCbCr.sample[2].refWhite ||
            pDes1->typeInfo.YCbCr.sample[2].subsampleHoriz != pDes2->typeInfo.YCbCr.sample[2].subsampleHoriz ||
            pDes1->typeInfo.YCbCr.sample[2].subsampleVert != pDes2->typeInfo.YCbCr.sample[2].subsampleVert ||
            pDes1->typeInfo.YCbCr.lumaRed   != pDes2->typeInfo.YCbCr.lumaRed   ||
            pDes1->typeInfo.YCbCr.lumaGreen != pDes2->typeInfo.YCbCr.lumaGreen ||
            pDes1->typeInfo.YCbCr.lumaBlue  != pDes2->typeInfo.YCbCr.lumaBlue  ||
            pDes1->typeInfo.YCbCr.positioning != pDes2->typeInfo.YCbCr.positioning) 
            return FALSE;
        }
    else if (pDes1->type == IL_PALETTE) {
        if (pDes1->typeInfo.palette.levels[0] != pDes2->typeInfo.palette.levels[0] ||
            pDes1->typeInfo.palette.levels[1] != pDes2->typeInfo.palette.levels[1] ||
            pDes1->typeInfo.palette.levels[2] != pDes2->typeInfo.palette.levels[2]) 
            return FALSE;
         }

    for (i = 0; i < pDes1->nSamplesPerPixel; i++) {
        if (pDes1->nLevelsPerSample [i] != pDes2->nLevelsPerSample [i]) 
            return FALSE;
    }

    return TRUE;
}


        /*  -------------------- iluPrintImageDes ----------------- */
        /*  Print info on the given ilImageDes
        */
void iluPrintImageDes (
    ilImageDes         *pDes
    )
{
    int i;

    printf("    Image Descriptor: ");
    if (iluDesEqual (pDes, IL_DES_BITONAL))      printf ("IL_DES_BITONAL\n");
    else if (iluDesEqual (pDes, IL_DES_GRAY))    printf ("IL_DES_GRAY\n");
    else if (iluDesEqual (pDes, IL_DES_RGB))     printf ("IL_DES_RGB\n");
    else if (iluDesEqual (pDes, IL_DES_PALETTE)) printf ("IL_DES_PALETTE\n");
    else if (iluDesEqual (pDes, IL_DES_YCBCR))   printf ("IL_DES_YCBCR\n");
    else if (iluDesEqual (pDes, IL_DES_YCBCR_2)) printf ("IL_DES_YCBCR_2\n");
    else {
        printf ("\n    type: %s\n", ((pDes->type == IL_BITONAL) ? "IL_BITONAL" :
                                 ((pDes->type == IL_GRAY) ? "IL_GRAY" : 
                                 ((pDes->type == IL_PALETTE) ? "IL_PALETTE" : 
                                 ((pDes->type == IL_RGB) ? "IL_RGB" : 
                                 ((pDes->type == IL_YCBCR) ? "IL_YCBCR" : "unknown"))))) );
        if ((pDes->type == IL_GRAY) || (pDes->type == IL_BITONAL))
            printf ("    blackIsZero: %s\n", pDes->blackIsZero ? "True" : "False");
        printf ("    nSamplesPerPixel: %d\n", pDes->nSamplesPerPixel);

        if (pDes->type == IL_YCBCR) {
            printf ("    YCbCr type Data:\n");
            printf ("        Sample 0: %d %d %d %d\n", 
                    pDes->typeInfo.YCbCr.sample[0].refBlack,
                    pDes->typeInfo.YCbCr.sample[0].refWhite,
                    pDes->typeInfo.YCbCr.sample[0].subsampleHoriz,
                    pDes->typeInfo.YCbCr.sample[0].subsampleVert);
            printf ("        Sample 1: %d %d %d %d\n", 
                    pDes->typeInfo.YCbCr.sample[1].refBlack,
                    pDes->typeInfo.YCbCr.sample[1].refWhite,
                    pDes->typeInfo.YCbCr.sample[1].subsampleHoriz,
                    pDes->typeInfo.YCbCr.sample[1].subsampleVert);
            printf ("        Sample 2: %d %d %d %d\n", 
                    pDes->typeInfo.YCbCr.sample[2].refBlack,
                    pDes->typeInfo.YCbCr.sample[2].refWhite,
                    pDes->typeInfo.YCbCr.sample[2].subsampleHoriz,
                    pDes->typeInfo.YCbCr.sample[2].subsampleVert);
            printf ("        Lumas: %d %d %d\n", pDes->typeInfo.YCbCr.lumaRed,
                pDes->typeInfo.YCbCr.lumaGreen, pDes->typeInfo.YCbCr.lumaBlue);
            printf ("        Positioning: %d\n",
                 pDes->typeInfo.YCbCr.positioning);
            }
        else if ((pDes->type == IL_PALETTE) &&
            (pDes->flags & IL_DITHERED_PALETTE)) {
                printf ("    Dithered palette. Levels: %d %d %d\n", 
                    pDes->typeInfo.palette.levels[0],
                    pDes->typeInfo.palette.levels[1],  
                    pDes->typeInfo.palette.levels[2]);
                }
    
        printf ("    Levels per Sample: ");
        for (i = 0; i < pDes->nSamplesPerPixel; i++)
            printf (" %d", pDes->nLevelsPerSample[i]);
        printf ("\n");
        }   /* END not standard */

        /*  Print out compression and related info */
    printf ("    Compression: %s\n", 
        ((pDes->compression == IL_UNCOMPRESSED) ? "uncompressed" :
        ((pDes->compression == IL_G3) ? "IL_G3" :
        ((pDes->compression == IL_JPEG) ? "IL_JPEG" :
        ((pDes->compression == IL_G4) ? "IL_G4" :
        ((pDes->compression == IL_LZW) ? "IL_LZW" :
        ((pDes->compression == IL_PACKBITS) ? "IL_PACKBITS" :
        "unknown")))))) );
    if (pDes->compression ==  IL_JPEG) {
        printf ("    JPEG process: %d\n", pDes->compInfo.JPEG.process);
        }
    else if (pDes->compression ==  IL_G3) {
        printf ("    G3 flags:\n");
        if (pDes->compInfo.g3.flags & IL_G3M_LSB_FIRST) 
            printf ("        IL_G3M_LSB_FIRST\n");
        if (pDes->compInfo.g3.flags & IL_G3M_2D) 
            printf ("        IL_G3M_2D\n");
        if (pDes->compInfo.g3.flags & IL_G3M_UNCOMPRESSED) 
            printf ("        IL_G3M_UNCOMPRESSED\n");
        if (pDes->compInfo.g3.flags & IL_G3M_EOL_MARKERS) 
            printf ("        IL_G3M_EOL_MARKERS\n");
        if (pDes->compInfo.g3.flags & IL_G3M_EOL_UNALIGNED) 
            printf ("        IL_G3M_EOL_UNALIGNED\n");
        if (pDes->compInfo.g3.flags == IL_G3M_TIFF) 
            printf ("        TIFF compatible\n");
        if (pDes->compInfo.g3.flags == IL_G3M_CLASS_F) 
            printf ("        Class F\n");
        }
    
}


        /*  -------------------- iluPrintEFSPageInfo ----------------- */
        /*  Query and print the info on the current page in the given EFS file.
        */
void iluPrintEFSPageInfo (
    ilEFSFile       file
    )
{
    ilEFSPageInfo       info;
    int                 i;

    if (!ilEFSGetPageInfo (file, &info)) {
        fprintf (stderr, "? Error '%d' from ilGetPageInfo().\n", 
                 file->context->error);
        return;
        }
    printf ("  Page %d info:\n", info.page);

    if (info.attributes & IL_EFS_MASKED) 
        printf ("    Mask image present\n");
    if (info.attributes & IL_EFS_SCALEABLE ) 
        printf ("    Page is scaleable\n");
    printf ("    Size: %d x %d\n", info.width, info.height);
    printf ("    Resolution (dpi): %d x %d\n", info.xRes/2, info.yRes/2);

    iluPrintImageDes (&info.des);
    if (info.des.nSamplesPerPixel > 1)
        printf ("    Sample order: %s\n", 
            ((info.format.sampleOrder == IL_SAMPLE_PLANES) ? 
            "IL_SAMPLE_PLANES" : "IL_SAMPLE_PIXELS"));

    printf ("    Bits/sample: ");
    for (i = 0; i < info.des.nSamplesPerPixel; i++)
        printf ("%d ", info.format.nBitsPerSample[i]);
    printf ("\n\n");
}

