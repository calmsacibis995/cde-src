/*
 * File:         spc-xt.c $XConsortium: spc-xt.c /main/cde1_maint/1 1995/07/14 20:20:14 drk $
 * Language:     C
 *
 * (c) Copyright 1989, Hewlett-Packard Company, all rights reserved.
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc.				*
 */

#define __need_fd_set

#include <bms/sbport.h> /* NOTE: sbport.h must be the first include. */
#include "spcP.h"
/* #include <bms/SbEvent.h>  */ /* This file now included by spcP.h */

/* Externals */

extern int break_on_termination;

/* Utility functions */

/* First, declarations */

typedef struct {
  SbInputId 			read_id;
  SbInputId			except_id;
} SPC_Callback_Struct;

static SPC_Callback_Struct **SPC_Fd_Mapping = NULL;

#define SPC_LOOKUP_FD_MAPPING(fd)   SPC_Fd_Mapping[(fd)]

static Boolean spc_xe_termination_flag;

/*-----------------------------------------------------------------------+*/
static int SPC_AddInput(int 			source,
		 SPC_Callback_Condition		condition,
		 SbInputId	 		id)
/*-----------------------------------------------------------------------+*/
{
  SPC_Callback_Struct *structptr = NULL;
  
  if (SPC_Fd_Mapping == NULL) {
    SPC_Fd_Mapping = (SPC_Callback_Struct **) 
	             XeMalloc (FD_SETSIZE * sizeof (SPC_Callback_Struct *));
    memset(SPC_Fd_Mapping, NULL, FD_SETSIZE * sizeof(SPC_Callback_Struct *));
  }
  structptr=SPC_LOOKUP_FD_MAPPING(source);

  if(!structptr) {
    structptr=(SPC_Callback_Struct *) XeMalloc(sizeof(SPC_Callback_Struct));
    SPC_LOOKUP_FD_MAPPING(source)=structptr;
  }

  switch (condition) {

  case SPC_Input:
  case SPC_Terminator:
  case SPC_Client:
    structptr->read_id   = id;
    break;

  case SPC_Exception:
    structptr->except_id = id;
    break;

  default:
    break;
  }

  return(source);
}


/*-----------------------------------------------------------------------+*/
static SbInputId SPC_RemoveInput(int 			 source,
			  SPC_Callback_Condition condition)
/*-----------------------------------------------------------------------+*/
{
  SPC_Callback_Struct *structptr = NULL;
  
  if (SPC_Fd_Mapping == NULL) {
    SPC_Fd_Mapping = (SPC_Callback_Struct **) 
		     XeMalloc (FD_SETSIZE * sizeof (SPC_Callback_Struct *));
    memset(SPC_Fd_Mapping, NULL, FD_SETSIZE * sizeof(SPC_Callback_Struct *));
  }
  structptr=SPC_LOOKUP_FD_MAPPING(source);

  switch(condition) {
    
  case SPC_Input:
  case SPC_Terminator:
  case SPC_Client:
     return structptr->read_id;

  case SPC_Exception:
     return structptr->except_id;
    
  }

  return NULL;
}
  
/*-----------------------------------------------------------------------+*/
static SPC_Select(void )
/*-----------------------------------------------------------------------+*/
{
    break_on_termination=TRUE;
    spc_xe_termination_flag= FALSE;
    
    /* Use a function pointer so we don't have explict dependancy */
    /* on libXe.a						    */
    /* ---------------------------------------------------------- */
    if (SbMainLoopUntil_hookfn == NULL)
      (void) fprintf (stderr, "Error: SbMainLoopUntil = NULL\n");
    else
      (*SbMainLoopUntil_hookfn)(&spc_xe_termination_flag);
    
    break_on_termination=FALSE;
  return(TRUE);
}


/*-----------------------------------------------------------------------+*/
int SPC_Wait_For_Termination(SPC_Channel_Ptr channel)
/*-----------------------------------------------------------------------+*/
{
  int result;
  
  call_parent_method(channel, wait_for_termination, (channel), result);
  
  if(result==SPC_ERROR) return(SPC_ERROR);

  do {

    if(SPC_Select() == SPC_ERROR)
      return(SPC_ERROR);
    
  } while(IS_ACTIVE(channel));
  
  return(TRUE);
  
}


/*-----------------------------------------------------------------------+*/
void SPC_XtBreak(void)
/*-----------------------------------------------------------------------+*/
{if(!spc_xe_termination_flag) {
     spc_xe_termination_flag = TRUE;
     if (SbBreakMainLoop_hookfn == NULL)
       (void) fprintf (stderr, "Error: SbBreakMainLoop = NULL\n");
     else
       (*SbBreakMainLoop_hookfn)();      
 } 
}


  
/*-----------------------------------------------------------------------+*/
void SPC_XtAddInput(SPC_Channel_Ptr channel,
		    int *id_addr,
		    int fd,
		    spc_handler_func_type handler,
		    SPC_Callback_Condition condition)
/*-----------------------------------------------------------------------+*/
{
  SbInputId id;
  
  switch(condition) {
    
  case SPC_Input:
  case SPC_Terminator:
  case SPC_Client:
    /* fprintf(stderr, "SPC add input/terminator for %d\n", fd); */
    if (SbAddInput_hookfn == NULL)
      (void) fprintf (stderr, "Error: SbAddInput = NULL\n");
    else
      id = (*SbAddInput_hookfn)(fd, handler, channel);
    break;
    
  case SPC_Exception:
    /* fprintf(stderr, "SPC add exception for %d\n", fd); */
    if (SbAddException_hookfn == NULL)
      (void) fprintf (stderr, "Error: SbAddException = NULL\n");
    else
      id = (*SbAddException_hookfn)(fd, handler, channel);
    break;
    
  }
  
  *id_addr=SPC_AddInput(fd, condition, id);
}


/*-----------------------------------------------------------------------+*/
void SPC_XtRemoveInput(int *id_addr, 
		       SPC_Callback_Condition condition)
/*-----------------------------------------------------------------------+*/
{
  if((*id_addr) != -1) {
    
    switch(condition) {
      
    case SPC_Input:
    case SPC_Terminator:
    case SPC_Client:
      /* fprintf(stderr, "SPC remove input/terminator\n"); */
      if (SbRemoveInput_hookfn == NULL)
        (void) fprintf (stderr, "Error: SbRemoveInput = NULL\n");
      else
        (*SbRemoveInput_hookfn)(SPC_RemoveInput(*id_addr, condition));
      break;
      
    case SPC_Exception:
      /* fprintf(stderr, "SPC remove exception"); */
      if (SbRemoveException_hookfn == NULL)
        (void) fprintf (stderr, "Error: SbRemoveException = NULL\n");
      else
        (*SbRemoveException_hookfn)(SPC_RemoveInput(*id_addr, condition));
      break;
      
    }
    
    *id_addr=(-1);
  }
}
