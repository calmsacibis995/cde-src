/* $XConsortium: BMSCompat.c /main/cde1_maint/1 1995/07/17 18:08:34 drk $ */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **   File:         BMSCompat.c
 **
 **   Project:	    DT
 **
 **   Description:  This file contains the BMS Compatibility layer of
 **		    the DT library.  This layer maps some of the BMS
 **                 calls into the new ICCCM based messaging.
 **		  
 **
 **   (c) Copyright 1987, 1988, 1989, 1990 by Hewlett-Packard Company
 **
 *
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef  _NO_PROTO
#  include <stdarg.h>
#else
#  include <varargs.h>
#endif

/*
 * Account for the various macros on different systems which indicate that
 * stdarg.h has been included.	Code in this file only checks for 
 * _STDARG_INCLUDED.  If a given system defines another macro that means the
 * same thing -- then define _STDARG_INCLUDED here.
 *
 *	System		Macro Indicating stdarg.h has been included
 *     --------        ---------------------------------------------
 *	HPUX		_STDARG_INCLUDED
 *	AIX		_H_STDARG
 *	SOLARIS		_STDARG_H
 */
#ifdef _H_STDARG
#define	_STDARG_INCLUDED
#endif
#ifdef  _STDARG_H
#define	_STDARG_INCLUDED
#endif

#define TOOLKIT
#include <Dt/DataTypes.h>
#include <Dt/DtP.h>
#include <Dt/Message.h>
#include <Dt/Service.h>
#include <Dt/DtNlUtils.h>

typedef struct {
   char * tool_class;
   DtSvcHandle service;
} ServiceHandleTable;

typedef struct {
   Boolean temporaryEntry;
   int numDataFields;
   DtSvcReceiveProc callbackProc;
   XtPointer clientData;
} ClientInfo;

Widget _Dt_msg_widget = NULL;
static ServiceHandleTable * serviceHandles = NULL;
static int numServiceHandles = 0;
static ClientInfo * clientInfo = NULL;
static int numClientInfoEntries = 0;


/*
 * Register the information which allows us to take the recieved message
 * data, and to parse it up into the number of fields expected by the
 * old BMS style callback.
 */

static int
RegisterClientInfo (callbackProc, clientData, numDataFields, temporaryEntry)

   DtSvcReceiveProc callbackProc;
   XtPointer clientData;
   int numDataFields;
   Boolean temporaryEntry;

{
   clientInfo = (ClientInfo *)XtRealloc((char *)clientInfo,
                  sizeof(ClientInfo) * (numClientInfoEntries + 1));

   clientInfo[numClientInfoEntries].callbackProc = callbackProc;
   clientInfo[numClientInfoEntries].clientData = clientData;
   clientInfo[numClientInfoEntries].numDataFields = numDataFields;
   clientInfo[numClientInfoEntries].temporaryEntry = temporaryEntry;

   numClientInfoEntries++;
   return(numClientInfoEntries - 1);
}

/*
 * This function helps to parse the incoming strings, using the space
 * character as the delimiter, as was done by the BMS.
 */

static int
ParseFields (numToParse, fields, currentNum, str)

   int numToParse;
   char ** fields;
   int * currentNum;
   char * str;

{
   char * start;
   char * ptr;
   char * ptr2;
   char * ptr3;

   if (numToParse <= 0)
      return(numToParse);

   start = str;

   while (numToParse > 0)
   {
      if (numToParse == 1)
      {
         /* Simply use the remainder of the string */
         fields[*currentNum] = XtNewString(start);
         (*currentNum)++;
         numToParse--;
         break;
      }

      ptr = DtStrchr(start, ' ');
      ptr2 = DtStrchr(start, '\\');
      /* Check for an escaped space character */
tryAgain:
      if (ptr && ptr2 && (ptr2 + 1 == ptr))
      {
         ptr3 = ptr + 1;
         ptr = DtStrchr(ptr3, ' ');
         ptr2 = DtStrchr(ptr3, '\\');
         goto tryAgain;
      }

      if (ptr)
         *ptr = '\0';
      fields[*currentNum] = XtNewString(start);
      (*currentNum)++;
      numToParse--;
      if (ptr)
      {
         *ptr = ' ';
         start = ptr + 1;
      }
      else
         break;
   }

   return(numToParse);
}


/*
 * This function removes a temporary entry.
 */

static void
RemoveClientInfo (entryId)
   int entryId;

{
   int i;

   numClientInfoEntries--;
   for (i = entryId; i < numClientInfoEntries; i++)
      clientInfo[i] = clientInfo[i+1];

   clientInfo = (ClientInfo *)XtRealloc((char *)clientInfo,
                       sizeof(ClientInfo) * numClientInfoEntries);
}


/*
 * This callback intercepts callbacks intended for the client, and
 * parses the incoming string parameters, using space delimiters;
 * it then passes the new array onto the real callback.
 */

static void
CompatCBProc (service, replyContext, clientData, messageFields, numFields)

   DtSvcHandle service;
   DtSvcMsgContext replyContext;
   XtPointer clientData;
   String * messageFields;
   int numFields;

{
   char ** fields;
   int entryId;
   int i, j, left, current;

   entryId = (int)clientData;

   if (clientInfo[entryId].callbackProc)
   {
      /* Allocate the string array; make room for terminating NULL */
      fields = (char **)XtMalloc(sizeof(char *) *
                        (clientInfo[entryId].numDataFields + 2));
      for (i = 0; i < (clientInfo[entryId].numDataFields + 2); i++)
         fields[i] = NULL;

      left = clientInfo[entryId].numDataFields;
      j = 1;
      current = 1;
      fields[0] = XtNewString(messageFields[DT_MSG_COMMAND]);

      while((left > 0) && (j < numFields))
      {
         left = ParseFields(left, fields, &current, messageFields[j]);
         j++;
      }

      /* Call the real callback */
      (*(clientInfo[entryId].callbackProc))(service, replyContext,
                 clientInfo[entryId].clientData, fields, current);
      
      /* clean up */
      for (i = 0; i < (clientInfo[entryId].numDataFields + 2); i++)
         XtFree(fields[i]);
      XtFree((char *)fields);
   }

   if (clientInfo[entryId].temporaryEntry)
      RemoveClientInfo(entryId);
}

static void
CompatNotifyCBProc (service, clientData, messageFields, numFields)

   DtSvcHandle service;
   XtPointer clientData;
   String * messageFields;
   int numFields;

{
   char ** fields;
   int entryId;
   int i, j, left, current;

   entryId = (int)clientData;

   if (clientInfo[entryId].callbackProc)
   {
      /* Allocate the string array; make room for terminating NULL */
      fields = (char **)XtMalloc(sizeof(char *) *
                        (clientInfo[entryId].numDataFields + 2));
      for (i = 0; i < clientInfo[entryId].numDataFields + 2; i++)
         fields[i] = NULL;

      left = clientInfo[entryId].numDataFields;
      j = 1;
      current = 1;
      fields[0] = XtNewString(messageFields[DT_MSG_COMMAND]);

      while((left > 0) && (j < numFields))
      {
         left = ParseFields(left, fields, &current, messageFields[j]);
         j++;
      }

      /* Call the real callback */
      (*(clientInfo[entryId].callbackProc))(service, 
                 clientInfo[entryId].clientData, fields, current);
   
      /* clean up */
      for (i = 0; i < clientInfo[entryId].numDataFields + 2; i++)
         XtFree(fields[i]);
      XtFree((char *)fields);
   }

   if (clientInfo[entryId].temporaryEntry)
      RemoveClientInfo(entryId);
}


static DtSvcHandle GetServiceHandle(
      String		tool_class,
      Widget		widget);


#ifdef _NO_PROTO
void _DtSendReady ( tool_class)
     String tool_class;
#else
void _DtSendReady (
     String tool_class)
#endif
{
    DtSvcHandle service;

    service = GetServiceHandle(tool_class, _Dt_msg_widget);

    _DtSvcRegister (service, False, (DtSvcReceiveProc) NULL, 
		    (Pointer) NULL, (DtSvcStatusProc) NULL, 
		    (Pointer) NULL);
}

int
#ifdef _NO_PROTO
_DtAddRequestCallback (tool_class, command, callback_proc,
			   client_data, num_data_fields, va_alist)
     String tool_class;
     String command;
     DtSvcReceiveProc callback_proc;
     void *client_data;
     int num_data_fields;
     va_dcl
#else
_DtAddRequestCallback (
     String tool_class,
     String command,
     DtSvcReceiveProc callback_proc,
     void *client_data,
     int num_data_fields,
     ...)
#endif
{
    DtSvcHandle service;
    int entryId;

    service = GetServiceHandle(tool_class, _Dt_msg_widget);

    /* This function makes the following assumptions about how the BMS
       function is used:

	   - 'num_data_fields' must be greater than or equal to the 
	     number of arguments provided by the sender of the request

	   - 'va_dcl' is a NULL list
     */

    entryId = RegisterClientInfo(callback_proc, client_data, num_data_fields,
                                 False);
    _DtSvcRequestRegister (service, command, CompatCBProc, (XtPointer)entryId);
}

String
#ifdef _NO_PROTO
_DtSendRequest (tool_class, command, va_alist)
     String tool_class;
     String command;
     va_dcl
#else
_DtSendRequest (
     String tool_class,
     String command,
     ...)
#endif
{
    String		*args = NULL;
    va_list		walk_args;
    String		next_arg;
    int			num_args = 0;
    DtSvcHandle service;

    service = GetServiceHandle(tool_class, _Dt_msg_widget);

    /* This function assumes that the returned request-id is not
       needed and it simply returns NULL.
     */

    /* Turn the varargs into a string array. */
#ifdef _STDARG_INCLUDED
    va_start (walk_args,command);
#else
    va_start (walk_args);
#endif
    while ((next_arg = va_arg (walk_args, String)) != NULL) {
	num_args++;
	args = realloc (args, sizeof (String) * num_args);
	*(args + num_args - 1) = next_arg;
    }
    va_end (walk_args);

    _DtSvcRequestSendStart (service, command, args, num_args, NULL, NULL,
			    NULL, NULL);
    free (args);
    return (NULL);
}


int
#ifdef _NO_PROTO
_DtSendResponse (is_success, echo_data, message, response_info,
		     va_alist)
    Boolean		is_success;
    Boolean		echo_data;
    DtSvcMsgContext	message;
    String		response_info;
    va_dcl
#else
_DtSendResponse (
    Boolean             is_success,
    Boolean             echo_data,
    DtSvcMsgContext     message,
    String              response_info,
    ... )
#endif
{
    DtSvcHandle service;
    String		*args = NULL;
    va_list		walk_args;
    String		next_arg;
    int			num_args = 0;

    /* WHAT SHOULD BE USED FOR TOOL CLASS IN THIS CALL? */
    service = GetServiceHandle(_dtToolClass, _Dt_msg_widget);

    /* Turn the varargs into a string array. */
   if (response_info)
   {
      args = malloc (sizeof (String));
      args[num_args] = response_info;
      num_args++;
   }
#ifdef _STDARG_INCLUDED
    va_start (walk_args,response_info);
#else
    va_start (walk_args);
#endif
    while ((next_arg = va_arg (walk_args, String)) != NULL) {
	num_args++;
	args = realloc (args, sizeof (String) * num_args);
	*(args + num_args - 1) = next_arg;
    }
    va_end (walk_args);

    _DtSvcRequestReply (service, message, args, num_args, is_success);

   free(args);
}

int
#ifdef _NO_PROTO
_DtSendRequestCatchResponse (tool_class, command, 
				 success_callback_proc, success_data,
				 num_success_fields, fail_callback_proc,
				 fail_data, num_fail_fields, va_alist)
    String		tool_class;
    String		command;
    DtSvcMessageProc	success_callback_proc;
    caddr_t		success_data;
    int 		num_success_fields;
    DtSvcMessageProc	fail_callback_proc;
    caddr_t		fail_data;
    int 		num_fail_fields;
    va_dcl
#else
_DtSendRequestCatchResponse (
    String              tool_class,
    String              command,
    DtSvcMessageProc   success_callback_proc,
    caddr_t             success_data,
    int                 num_success_fields,
    DtSvcMessageProc   fail_callback_proc,
    caddr_t             fail_data,
    int                 num_fail_fields,
    ...)
#endif
{
    String		*args = NULL;
    va_list		walk_args;
    String		next_arg;
    int			num_args = 0;
    DtSvcHandle service;
    int successEntryId;
    int failureEntryId;

    service = GetServiceHandle(tool_class, _Dt_msg_widget);

    /* This function assumes that the returned request-id is not
       needed and it simply returns NULL.
     */

    /* Turn the varargs into a string array. */
#ifdef _STDARG_INCLUDED
    va_start (walk_args,num_fail_fields);
#else
    va_start (walk_args);
#endif
    while ((next_arg = va_arg (walk_args, String)) != NULL) {
	num_args++;
	args = realloc (args, sizeof (String) * num_args);
	*(args + num_args - 1) = next_arg;
    }
    va_end (walk_args);

    successEntryId = RegisterClientInfo(
                       (DtMsgReceiveProc)success_callback_proc, 
                       success_data, num_success_fields, True);
    failureEntryId = RegisterClientInfo(
                       (DtMsgReceiveProc)fail_callback_proc, 
                       fail_data, num_fail_fields, True);

    _DtSvcRequestSendStart (service, command, args, num_args, 
			    CompatNotifyCBProc, (XtPointer)successEntryId,
			    CompatNotifyCBProc, (XtPointer)failureEntryId);
    free (args);
    return (NULL);
}

void 
#ifdef _NO_PROTO
_DtAddNotificationCallback (tool_class, command,
				 callback_proc, client_data,
				 num_data_fields, va_alist)
    String		tool_class;
    String		command;
    DtSvcMessageProc	callback_proc;
    caddr_t		client_data;
    int			num_data_fields;
    va_dcl
#else
_DtAddNotificationCallback (
    String              tool_class,
    String              command,
    DtSvcMessageProc   callback_proc,
    caddr_t             client_data,
    int                 num_data_fields,
    ...)
#endif
{
    DtSvcHandle service;
    int entryId;

    service = GetServiceHandle(tool_class, _Dt_msg_widget);

    entryId = RegisterClientInfo((DtMsgReceiveProc)callback_proc, client_data,
                                  num_data_fields, False);
    _DtSvcNotifyMsgRegister(service, command, CompatNotifyCBProc,(
                            XtPointer)entryId);
}

int
#ifdef _NO_PROTO
_DtSendSuccessNotification (tool_class, request_id, command,
				va_alist)
    String		tool_class;
    String		request_id;
    String		command;
    va_dcl
#else
_DtSendSuccessNotification (
    String              tool_class,
    String              request_id,
    String              command,
    ...)
#endif
{
    String		*args = NULL;
    va_list		walk_args;
    String		next_arg;
    int			num_args = 0;
    int			ret_val;
    DtSvcHandle service;

    service = GetServiceHandle(tool_class, _Dt_msg_widget);

    /* This function assumes that the returned request-id is not
       needed and it simply returns NULL.
     */

    /* Turn the varargs into a string array. */
#ifdef _STDARG_INCLUDED
    va_start (walk_args,command);
#else
    va_start (walk_args);
#endif

    while ((next_arg = va_arg (walk_args, String)) != NULL) {
	num_args++;
	args = realloc (args, sizeof (String) * num_args);
	*(args + num_args - 1) = next_arg;
    }
    va_end (walk_args);

    ret_val = _DtSvcNotifySend (service, command, args, num_args);
  
    free (args);
    return (ret_val);
}

static DtSvcHandle GetServiceHandle(tool_class, widget)
    String		tool_class;
    Widget		widget;
{
   int i;

   for (i = 0; i < numServiceHandles; i++)
   {
      if (((serviceHandles[i].tool_class == NULL) && (tool_class == NULL)) ||
          (serviceHandles[i].tool_class && 
           (strcmp(serviceHandles[i].tool_class, tool_class) == 0)))
      {
         return(serviceHandles[i].service);
      }
   }

   /* Allocate a new service handle */
   serviceHandles = (ServiceHandleTable *)XtRealloc((char *)serviceHandles,
                       sizeof(ServiceHandleTable) * (numServiceHandles + 1));
   serviceHandles[numServiceHandles].tool_class = XtNewString(tool_class);
   serviceHandles[numServiceHandles].service = 
                             _DtSvcNewHandle (tool_class, widget);
   numServiceHandles++;
   return (serviceHandles[numServiceHandles-1].service);
}
