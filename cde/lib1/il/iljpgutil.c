/* $XConsortium: iljpgutil.c /main/cde1_maint/1 1995/07/17 19:00:04 drk $ */
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

#include "iljpgint.h"


    /* JPEG zigzag scanning order */
    ILJPG_PRIVATE
int _iljpgZigzagTable[64] = {
 0, 1, 5, 6,14,15,27,28,
 2, 4, 7,13,16,26,29,42,
 3, 8,12,17,25,30,41,43,
 9,11,18,24,31,40,44,53,
10,19,23,32,39,45,52,54,
20,22,33,38,46,51,55,60,
21,34,37,47,50,56,59,61,
35,36,48,49,57,58,62,63
};


    /*  -------------------- _iljpgValidPars -------------------------- */
    /*  Validate the given parameter block and return true iff valid.
    */
    ILJPG_PRIVATE
iljpgBool _iljpgValidPars (
    register iljpgDataPtr pData
    )
{
    int                 comp;
    iljpgCompDataPtr    pCompData;
    register unsigned int   index;
#define VALID_FACTOR(_f) ( ((_f) == 1) || ((_f) == 2) || ((_f) == 4) )

        /*  Validate *pData: valid hori/vertFactor, tables present, etc. */
    if ((pData->nComps <= 0) 
     || (pData->nComps > ILJPG_MAX_COMPS)
     || (pData->nComps > ILJPG_MAX_COMPS)
     || !VALID_FACTOR (pData->maxHoriFactor)
     || !VALID_FACTOR (pData->maxVertFactor)
     || (pData->width <= 0)
     || (pData->height <= 0))
        return FALSE;

    for (comp = 0, pCompData = pData->comp; comp < pData->nComps; comp++, pCompData++) {
        if (!VALID_FACTOR (pCompData->horiFactor)
         || !VALID_FACTOR (pCompData->vertFactor))
            return FALSE;
        index = (unsigned int)pCompData->QTableIndex;
        if ((index > 3) || !pData->QTables[index])
            return FALSE;
        index = (unsigned int)pCompData->DCTableIndex;
        if ((index > 3) || !pData->DCTables[index])
            return FALSE;
        index = (unsigned int)pCompData->ACTableIndex;
        if ((index > 3) || !pData->ACTables[index])
            return FALSE;
        }

    return TRUE;
}


