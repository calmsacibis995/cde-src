/* $XConsortium: Pstack.c /main/cde1_maint/1 1995/07/17 23:00:48 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 * File:         PStack.c
 *
 * Project:      Cache Creek (Rivers) Project
 * Description:  Utility functions for volumegen program
 * Author:       Brian Cripe
 *               Modifications by (Mike Wilson)
 * Language:     C
 *
 * (C) Copyright 1992, Hewlett-Packard, all rights reserved.
 *
 ****************************************************************************
 ************************************<+>*************************************/


#include <stdio.h>
#include "PstackI.h"

#define REALLOC_INCR  10





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
PStack PStackCreate()
#else
PStack PStackCreate()
#endif
{
    PStack stack;
    stack = (PStack) malloc (sizeof (struct PStackRec));
    stack->count = 0;
    stack->size = REALLOC_INCR;
    stack->items = (void **) malloc (stack->size * sizeof (void **));

    return (stack);
}



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
void PStackDestroy(stack)
     PStack stack;
#else
void PStackDestroy(
     PStack stack)
#endif
{
    free (stack->items);
    free (stack);
}





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
void PStackPush(stack, ptr)
     PStack stack;
     void *ptr;
#else
void PStackPush(
     PStack stack,
     void *ptr)
#endif
{
    if (stack->count == stack->size) {
	stack->size += REALLOC_INCR;
	stack->items = (void **) realloc (stack->items, 
					  stack->size * sizeof (void **));
    }

    *(stack->items + stack->count) = ptr;
    stack->count++;
}



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
void *PStackPeek(stack)
     PStack stack;
#else
void *PStackPeek(
     PStack stack)
#endif

{
    if (stack->count == 0)
	return (NULL);

    else
	return (*(stack->items + stack->count - 1));
}


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
void *PStackPop(stack)
     PStack stack;
#else
void *PStackPop(
     PStack stack)
#endif
{
    if (stack->count == 0)
	return (NULL);

    else {
	stack->count--;
	return (*(stack->items + stack->count));
    }
}

	    
















