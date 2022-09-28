/* $XConsortium: PstackI.h /main/cde1_maint/1 1995/07/17 23:01:01 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 * File:         PstackI.h
 *
 * Project:      Cache Creek (Rivers) Project
 * Description:  Header file for Utility functions for PStack.h
 * Author:       Brian Cripe
 *               Modifications by (Mike Wilson)
 * Language:     C
 *
 * (C) Copyright 1992, Hewlett-Packard, all rights reserved.
 *
 ****************************************************************************
 ************************************<+>*************************************/

struct PStackRec {
    int size;
    int count;
    void **items;
};

typedef struct PStackRec *PStack;





/*****************************************************************************
 * Function:	    PStack PStackCreate();
 *
 * Parameters:      NONE.
 *
 * Return Value:    PStack.
 *
 * Purpose:         Generates a PStack structure and returns it to the 
 *                  caller.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
extern PStack PStackCreate();
#else
extern PStack PStackCreate();
#endif


/*****************************************************************************
 * Function:	    PStack PStackDestroy();
 *
 * Parameters:      stack:
 *
 * Return Value:    void.
 *
 * Purpose:         Destroys a PStack structure.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
extern void PStackDestroy();
#else
extern void PStackDestroy(
     PStack stack);
#endif


/*****************************************************************************
 * Function:	    PStack PStackPush();
 *
 * Parameters:      stack:
 *                  ptr:
 *
 * Return Value:    void
 *
 * Purpose:         Pushes a PStack structure on the current stack.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
extern void PStackPush();
#else
extern void PStackPush(
     PStack stack,
     void *ptr);
#endif


/*****************************************************************************
 * Function:	    PStack *PStackPeek();
 *
 * Parameters:      stack:
 *
 * Return Value:    void.
 *
 * Purpose:         Gets a PStack.
 *
 *****************************************************************************/
#ifdef _NO_PROTO
extern void *PStackPeek();
#else
extern void *PStackPeek(
     PStack stack);
#endif


/*****************************************************************************
 * Function:	    PStack *PStackPop();
 *
 * Parameters:      stack:
 *
 * Return Value:    void.
 *
 * Purpose:         Pops the top element off the stack
 *
 *****************************************************************************/
#ifdef _NO_PROTO
extern void *PStackPop();
#else
extern void *PStackPop(
     PStack stack);
#endif


















