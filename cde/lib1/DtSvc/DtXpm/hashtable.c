/* $XConsortium: hashtable.c /main/cde1_maint/2 1995/09/06 02:19:44 lehors $ */
/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

/*
 * Duplicate code in Motif library, this is a compatibility layer
 */

#include "_xpmI.h"

/* #include "xpmP.h" */
FUNC(_DtxpmHashTableInit, int, (xpmHashTable *table));
FUNC(_DtxpmHashTableFree, void, (xpmHashTable *table));
FUNC(_DtxpmHashSlot, xpmHashAtom *, (xpmHashTable *table, char *s));
FUNC(_DtxpmHashIntern, int, (xpmHashTable *table, char *tag, void *data));


xpmHashAtom *
_DtxpmHashSlot(table, s)
    xpmHashTable *table;
    char *s;
{
    return _XmxpmHashSlot(table, s);
}

int
_DtxpmHashIntern(table, tag, data)
    xpmHashTable *table;
    char *tag;
    void *data;
{
    return _XmxpmHashIntern(table, tag, data);
}

int
_DtxpmHashTableInit(table)
    xpmHashTable *table;
{
    return _XmxpmHashTableInit(table);
}

void
_DtxpmHashTableFree(table)
    xpmHashTable *table;
{
    _XmxpmHashTableFree(table);
}
