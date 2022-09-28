/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the list of Sdm Class Ids.

#ifndef _SDM_CLASSIDS_H
#define _SDM_CLASSIDS_H

#pragma ident "@(#)ClassIds.hh	1.38 96/08/05 SMI"

static const int Sdm_CL_INVALID = -1;

// Data Types
static const int Sdm_CL_StrStrTUP = 101;
static const int Sdm_CL_IntStrTUP = 102;

static const int Sdm_CL_Queue = 201;
static const int Sdm_CL_RequestQueue = 202;
static const int Sdm_CL_ReplyQueue = 203;

// Data Structs
static const int Sdm_CL_Defs =  301;
static const int Sdm_CL_Error = 302;
static const int Sdm_CL_Prim = 302;

//-SdmPrim Derived
static const int Sdm_CL_ServiceFunction = 401;
static const int Sdm_CL_Search = 402;
static const int Sdm_CL_Sort = 403;
static const int Sdm_CL_DeliveryResponse = 404;
static const int Sdm_CL_Token = 405;
static const int Sdm_CL_LockObject = 406;

// Manager Classes
static const int Sdm_CL_Connect = 501;
static const int Sdm_CL_Session = 502;
static const int Sdm_CL_InSession = 503;
static const int Sdm_CL_OutSession = 504;


// Data Objects Classes
static const int Sdm_CL_Data = 601;
//--Message Components
static const int Sdm_CL_Message = 602;
static const int Sdm_CL_MessageEnvelope = 603;
static const int Sdm_CL_MessageBody = 604;

// Data Store Classes
static const int Sdm_CL_Server = 700;
//--Data Servers
static const int Sdm_CL_DataStore = 701;
static const int Sdm_CL_MessageStore = 702;
static const int Sdm_CL_OutgoingStore = 703;

// Transaction Classes
static const int Sdm_CL_Event = 800;
static const int Sdm_CL_Request = 801;
static const int Sdm_CL_Reply = 802;
static const int Sdm_CL_ArgumentList = 803;

// Control Objects Classes
static const int Sdm_CL_Controller = 901;
static const int Sdm_CL_SystemWideRepository = 902;
static const int Sdm_CL_EntityRepository = 903;
static const int Sdm_CL_MemoryRepository = 904;
static const int Sdm_CL_Dispatcher = 905;
static const int Sdm_CL_TickleDispatcher = 906;
static const int Sdm_CL_TimedDispatcher = 907;

// Port-n-Channel Classes
static const int Sdm_CL_DataPort = 1001;
static const int Sdm_CL_DataChannel = 1002;

#endif




