/*
 * File:         spcd_event.c $XConsortium: spcd_event.c /main/cde1_maint/1 1995/07/15 01:08:04 drk $
 * Language:     C
 *
 * (c) Copyright 1988, Hewlett-Packard Company, all rights reserved.
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc.				*
 */

#define __need_fd_set

#include <bms/sbport.h>	 /* NOTE: sbport.h must be the first include. */
#include <SPC/spcP.h>
#include <bms/SbEvent.h> 
#include <sys/types.h>   /* for fd_set, FD_SET macros, et. al. */
#include <errno.h>

struct {SbInputCallbackProc handler; void* data; }
  SPCD_input_handlers [FD_SETSIZE],
  SPCD_except_handlers[FD_SETSIZE];

int SPCD_max_fd = 1;

fd_set Sb_Input_Mask, Sb_Except_Mask;

SbInputId SPCD_AddInput(int fd, SbInputCallbackProc proc, void* data)
 {SPCD_input_handlers[fd].handler = proc;
  SPCD_input_handlers[fd].data    = data;
  FD_SET(fd, &Sb_Input_Mask);
  if (SPCD_max_fd < fd) SPCD_max_fd = fd;
  return fd; }

SbInputId SPCD_AddException(int fd, SbInputCallbackProc proc, void* data)
 {SPCD_except_handlers[fd].handler = proc;
  SPCD_except_handlers[fd].data    = data;
  FD_SET(fd, &Sb_Except_Mask);
  if (SPCD_max_fd < fd) SPCD_max_fd = fd;
  return fd; }

void SPCD_RemoveInput(SbInputId id)
 {FD_CLR(id, &Sb_Input_Mask);
 }

void SPCD_RemoveException(SbInputId id)
 {FD_CLR(id, &Sb_Except_Mask);
 }


void SPCD_MainLoopUntil(Boolean *flag)
{
  
  int fd_vec_size = howmany(SPCD_max_fd, NFDBITS);
  fd_set input_mask, except_mask;
  int n, fd;
  int result;
  do {
    for (n=0; n<fd_vec_size; n++) {
      input_mask.fds_bits[n] = Sb_Input_Mask.fds_bits[n];
      except_mask.fds_bits[n] = Sb_Except_Mask.fds_bits[n];
    }
    
#ifdef sun
    do result=select(SPCD_max_fd + 1, (fd_set *) &input_mask, (fd_set *) NULL, (fd_set *) &except_mask, NULL);
#else
    do result=select(SPCD_max_fd + 1, (int *) &input_mask, (int *) NULL, (int *) &except_mask, NULL);
#endif /* sun */
    while(result == -1 && errno==EINTR);
    
    if(result < 0) {
      SPC_Error(SPC_Bad_Select);
      SPC_Format_Log((XeString)"Exiting server ...");
      SPC_Close_Log();
      exit (3);
    }

    /* Modified loop to break after a single hit on the select.  This */
    /* is necessary because there is nothing which stops the lower */
    /* level input handlers from reading the data from a random file */
    /* descriptor.  If this happens, and the random file descriptor */
    /* happens to be one on which this loop detected input, the */
    /* process might hang.  The solution is therefore to go back to */
    /* the select after every input handler call. */
    
    for (fd=0; fd < SPCD_max_fd+1; fd++)
      {
	SbInputId id = fd;
	if(FD_ISSET(fd, &except_mask)) {
	  (*SPCD_except_handlers[fd].handler)(SPCD_except_handlers[fd].data, &fd, &id);
	  break;
	}
	if(FD_ISSET(fd, &input_mask)) {
	  (*SPCD_input_handlers[fd].handler)(SPCD_input_handlers[fd].data, &fd, &id);
	  break;
	}
      }
  } while (!*flag);
}

void SPCD_BreakMainLoop(void) 
{ 
  /* no need to do anything */ 
}


