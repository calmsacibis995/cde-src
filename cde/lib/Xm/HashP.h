#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "OSF/Motif: @(#)_HashP.h	4.16 91/09/12";
#endif /* lint */
#endif /* REV_INFO */
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1989, 1990, 1991 OPEN SOFTWARE FOUNDATION, INC.
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
*  (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992, 1993 HEWLETT-PACKARD COMPANY
*  ALL RIGHTS RESERVED
*  
*  	THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED
*  AND COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND
*  WITH THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR
*  ANY OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
*  AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
*  SOFTWARE IS HEREBY TRANSFERRED.
*  
*  	THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
*  NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY OPEN SOFTWARE
*  FOUNDATION, INC. OR ITS THIRD PARTY SUPPLIERS  
*  
*  	OPEN SOFTWARE FOUNDATION, INC. AND ITS THIRD PARTY SUPPLIERS,
*  ASSUME NO RESPONSIBILITY FOR THE USE OR INABILITY TO USE ANY OF ITS
*  SOFTWARE .   OSF SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*  KIND, AND OSF EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES, INCLUDING
*  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
*  FITNESS FOR A PARTICULAR PURPOSE.
*  
*  Notice:  Notwithstanding any other lease or license that may pertain to,
*  or accompany the delivery of, this computer software, the rights of the
*  Government regarding its use, reproduction and disclosure are as set
*  forth in Section 52.227-19 of the FARS Computer Software-Restricted
*  Rights clause.
*  
*  (c) Copyright 1989, 1990, 1991 Open Software Foundation, Inc.  Unpublished - all
*  rights reserved under the Copyright laws of the United States.
*  
*  RESTRICTED RIGHTS NOTICE:  Use, duplication, or disclosure by the
*  Government is subject to the restrictions as set forth in subparagraph
*  (c)(1)(ii) of the Rights in Technical Data and Computer Software clause
*  at DFARS 52.227-7013.
*  
*  Open Software Foundation, Inc.
*  11 Cambridge Center
*  Cambridge, MA   02142
*  (617)621-8700
*  
*  RESTRICTED RIGHTS LEGEND:  This computer software is submitted with
*  "restricted rights."  Use, duplication or disclosure is subject to the
*  restrictions as set forth in NASA FAR SUP 18-52.227-79 (April 1985)
*  "Commercial Computer Software- Restricted Rights (April 1985)."  Open
*  Software Foundation, Inc., 11 Cambridge Center, Cambridge, MA  02142.  If
*  the contract contains the Clause at 18-52.227-74 "Rights in Data General"
*  then the "Alternate III" clause applies.
*  
*  (c) Copyright 1989, 1990, 1991 Open Software Foundation, Inc.
*  ALL RIGHTS RESERVED 
*  
*  
* Open Software Foundation is a trademark of The Open Software Foundation, Inc.
* OSF is a trademark of Open Software Foundation, Inc.
* OSF/Motif is a trademark of Open Software Foundation, Inc.
* Motif is a trademark of Open Software Foundation, Inc.
* DEC is a registered trademark of Digital Equipment Corporation
* DIGITAL is a registered trademark of Digital Equipment Corporation
* X Window System is a trademark of the Massachusetts Institute of Technology
*
*******************************************************************************
******************************************************************************/
/*
 * @OSF_COPYRIGHT@
 */
/*
 * HISTORY
 * Motif Release 1.2.5
 */
#ifndef _XmHashP_h
#define _XmHashP_h
 
#include <Xm/XmP.h>

#ifdef __cplusplus
extern "C" {
#endif
/*
 * the structure is used as a common header part for different
 * users of the hash functions in order to locate the key
 */
typedef XtPointer XmHashKey;

typedef XmHashKey (*XmGetHashKeyFunc)();
typedef void (*XmReleaseKeyProc)();
typedef Boolean (*XmHashEnumerateFunc)();


typedef struct _XmHashEntryTypePartRec {
    unsigned int		entrySize;
    XmGetHashKeyFunc		getKeyFunc;
    XtPointer			getKeyClientData;
    XmReleaseKeyProc		releaseKeyProc;
}XmHashEntryTypePartRec, *XmHashEntryTypePart;

typedef struct _XmHashEntryTypeRec {
    XmHashEntryTypePartRec	hash;
}XmHashEntryTypeRec, *XmHashEntryType;

typedef struct _XmHashEntryPartRec {
    unsigned int	type:16;
    unsigned int	flags:16;
}XmHashEntryPartRec, *XmHashEntryPart;

typedef struct _XmHashEntryRec {
    XmHashEntryPartRec	hash;
}XmHashEntryRec, *XmHashEntry;

typedef struct _XmHashTableRec *XmHashTable;



/********    Private Function Declarations for Hash.c    ********/
#ifdef _NO_PROTO

extern void _XmRegisterHashEntry() ;
extern void _XmUnregisterHashEntry() ;
extern XmHashEntry _XmEnumerateHashTable() ;
extern XmHashEntry _XmKeyToHashEntry() ;
extern XmHashTable _XmAllocHashTable() ;
extern void _XmFreeHashTable() ;
extern XmHashKey _XmGetEmbeddedKey() ;
extern XmHashKey _XmGetIndirectKey() ;

#else

extern void _XmRegisterHashEntry( 
                        register XmHashTable tab,
                        register XmHashKey key,
                        register XmHashEntry entry) ;
extern void _XmUnregisterHashEntry( 
                        register XmHashTable tab,
                        register XmHashEntry entry) ;
extern XmHashEntry _XmEnumerateHashTable( 
                        register XmHashTable tab,
                        register XmHashEnumerateFunc enumFunc,
                        register XtPointer clientData) ;
extern XmHashEntry _XmKeyToHashEntry( 
                        register XmHashTable tab,
                        register XmHashKey key) ;
extern XmHashTable _XmAllocHashTable( 
                        XmHashEntryType *hashEntryTypes,
                        Cardinal numHashEntryTypes,
#if NeedWidePrototypes
                        int keyIsString) ;
#else
                        Boolean keyIsString) ;
#endif /* NeedWidePrototypes */
extern void _XmFreeHashTable( 
                        XmHashTable hashTable) ;
extern XmHashKey _XmGetEmbeddedKey( 
                        XmHashEntry entry,
                        XtPointer clientData) ;
extern XmHashKey _XmGetIndirectKey( 
                        XmHashEntry entry,
                        XtPointer clientData) ;

#endif /* _NO_PROTO */
/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmHashP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
