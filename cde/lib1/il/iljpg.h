/* $XConsortium: iljpg.h /main/cde1_maint/1 1995/07/17 18:55:27 drk $ */
/**---------------------------------------------------------------------
***	
***    (c)Copyright 1992 Hewlett-Packard Co.
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



#ifndef ILJPG_H
#define ILJPG_H

#define ILJPG_MAX_COMPS    4   /* max # of components (IL "samples") */

    /*  JPEG marker codes */
#define ILJPGM_FIRST_BYTE    0xFF        /* first byte of all JPEG markers */
#define ILJPGM_TEM           0x01        /* TEMporary */
#define ILJPGM_SOF0          0xC0        /* Start Of Image, 0 = baseline JPEG */
#define ILJPGM_DHT           0xC4        /* Define Huffman Table */
#define ILJPGM_SOI           0xD8        /* Start Of Image */
#define ILJPGM_RST0          0xD0        /* ReSeT markers 0..7 */
#define ILJPGM_RST1          0xD1
#define ILJPGM_RST2          0xD2
#define ILJPGM_RST3          0xD3
#define ILJPGM_RST4          0xD4
#define ILJPGM_RST5          0xD5
#define ILJPGM_RST6          0xD6
#define ILJPGM_RST7          0xD7
#define ILJPGM_EOI           0xD9        /* End Of Image */
#define ILJPGM_SOS           0xDA        /* Start Of Scan */
#define ILJPGM_DQT           0xDB        /* Define Quantization Table */
#define ILJPGM_DRI           0xDD        /* Define Restart Interval */
#define ILJPGM_APP0          0xE0        /* APPlication marker "0" */


typedef int iljpgError;

typedef unsigned char iljpgByte;
typedef iljpgByte *iljpgPtr;

typedef struct {
    int                 horiFactor;
    int                 vertFactor;
    int                 QTableIndex;
    int                 DCTableIndex;
    int                 ACTableIndex;
    } iljpgCompDataRec, *iljpgCompDataPtr;

typedef struct {
    iljpgPtr            QTables[4];
    iljpgPtr            DCTables[4];
    iljpgPtr            ACTables[4];
    int                 restartInterval;
    int                 width;
    int                 height;
    int                 maxHoriFactor;
    int                 maxVertFactor;
    int                 nComps;
    iljpgCompDataRec    comp[ILJPG_MAX_COMPS];
    } iljpgDataRec, *iljpgDataPtr;

    /*  Declarations to enable symbol hiding if single compiled */
#ifndef ILJPG_PUBLIC_EXTERN
#define ILJPG_PUBLIC_EXTERN extern
#endif

#ifndef ILJPG_PUBLIC
#define ILJPG_PUBLIC
#endif

#ifndef ILJPG_PRIVATE_EXTERN
#define ILJPG_PRIVATE_EXTERN extern
#endif

#ifndef ILJPG_PRIVATE
#define ILJPG_PRIVATE
#endif

#endif
