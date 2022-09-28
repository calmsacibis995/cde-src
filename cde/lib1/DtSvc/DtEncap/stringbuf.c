/*
 * File:         stringbuf.c $XConsortium: stringbuf.c /main/cde1_maint/1 1995/07/14 20:20:43 drk $
 * Language:     C
 *
 * (c) Copyright 1988, Hewlett-Packard Company, all rights reserved.
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc.				*
 */

#include <bms/sbport.h>       /* NOTE: sbport.h must be the first include. */

#include <stdarg.h>

#include <bms/bms.h>
#include <bms/stringbuf.h>    /* public declarations */
#include <bms/MemoryMgr.h>    /* Xe_make_struct ... */

/* Forwards */
static int ExpandStringBuffer (XeStringBuffer buffer);

#ifndef DEBUG
#   define DEBUG 0
#endif

#define EXPANSION_FACTOR 4

  /** 
     BUFFER_MIN_SIZE  was chosen to be 16 so 
        BUFSIZE % EXPANSION_FACTOR > 0
	and 
	(BUFSIZE + (BUFSIZE % EXPANSION_FACTOR)) % EXPANSION_FACTOR 
	> (BUFSIZE % EXPANSION_FACTOR)
   **/	
     
#define BUFFER_MIN_SIZE 16

/*------------------------------------------------------------------------+*/
static int
ExpandStringBuffer(XeStringBuffer buffer)
/*------------------------------------------------------------------------+*/
{
   int new_size = buffer->size + buffer->increment;
   XeString new_string = Xe_make_ntype(new_size, XeChar);

#  if DEBUG >= 3
      sprintf(debug_string, "expanding string buffer. current size is: %d", buffer->size);
      print_debug_info(3, debug_string);
#  endif

   memcpy(new_string, buffer->buffer, buffer->last_char + 1);
   XeFree(buffer->buffer);
   buffer->size = new_size;
   buffer->buffer = new_string;
   buffer->increment += (buffer->increment % EXPANSION_FACTOR);
#  if DEBUG >= 3
      sprintf(debug_string, "new size is: %d", buffer->size);
      print_debug_info(4, debug_string);
#  endif
   return(0);
}

/*------------------------------------------------------------------------+*/
int
XeAppendToStringBuffer(XeStringBuffer buffer, XeString string)
/*------------------------------------------------------------------------+*/
{
   int size = buffer->size;
   XeString buffer_position;  /* so operations are pointer based */
   int i;
   
#  if DEBUG >= 5
      sprintf(debug_string, "appending: \"%s\" to: \"%s\"", string, buffer->buffer);
      print_debug_info(5, debug_string);
#  endif

   if (!(string && string[0])) {
      return(0);
   }
   
   buffer_position = &(buffer->buffer[buffer->last_char]);
   for (i = buffer->last_char; *string ; i++) {
      if (i == size) {
	  buffer->last_char = i - 1;
	  if (-1 == ExpandStringBuffer(buffer)) return(-1);
	  size = buffer->size;
	  buffer_position = &(buffer->buffer[i]);
      }
      *buffer_position++ = *string++;
   }
   if (i == size) {
       buffer->last_char = i - 1;
       if (-1 == ExpandStringBuffer(buffer)) return(-1);
   }
   buffer->buffer[i] = (XeChar) NULL;
   buffer->last_char = i;
#  if DEBUG >= 5
      sprintf(debug_string, "new string is: \"%s\"", buffer->buffer);
      print_debug_info(6, debug_string);
#  endif
   return(0);
}

/*------------------------------------------------------------------------+*/
void
XeClearStringBuffer(XeStringBuffer buffer)
/*------------------------------------------------------------------------+*/
{
   XeString string = buffer->buffer;
   string[0] = (XeChar) NULL;
   buffer->last_char = 0;
}


/*------------------------------------------------------------------------+*/
XeStringBuffer
XeMakeStringBuffer(int increment_size)
/*------------------------------------------------------------------------+*/
{
   XeStringBuffer new_buffer;
   increment_size = (increment_size > BUFFER_MIN_SIZE)
      			? increment_size 
      			: BUFFER_MIN_SIZE;

   new_buffer = Xe_make_struct(_XeStringBuffer);
   new_buffer->buffer = Xe_make_ntype(increment_size, XeChar);
   new_buffer->increment = increment_size;
   new_buffer->size = increment_size;
   XeClearStringBuffer(new_buffer);
   return(new_buffer);
}
