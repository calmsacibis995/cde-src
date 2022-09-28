/*SNOTICE*****************************************************************
**************************************************************************
*
*                   Common Desktop Environment
* 
* (c) Copyright 1993, 1994 Hewlett-Packard Company 
* (c) Copyright 1993, 1994 International Business Machines Corp.             
* (c) Copyright 1993, 1994 Sun Microsystems, Inc.
* (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of Novell, Inc.
*                                                                    
* 
*                     RESTRICTED RIGHTS LEGEND                              
* 
* Use, duplication, or disclosure by the U.S. Government is subject to
* restrictions as set forth in subparagraph (c)(1)(ii) of the Rights in
* Technical Data and Computer Software clause in DFARS 252.227-7013.  Rights
* for non-DOD U.S. Government Departments and Agencies are as set forth in
* FAR 52.227-19(c)(1,2).
* 
* Hewlett-Packard Company, 3000 Hanover Street, Palo Alto, CA 94304 U.S.A.
* International Business Machines Corp., Route 100, Somers, NY 10589 U.S.A. 
* Sun Microsystems, Inc., 2550 Garcia Avenue, Mountain View, CA 94043 U.S.A.
* Unix System Labs, Inc., 190 River Road, Summit, NJ 07901 U.S.A.
*
*****************************************************************ENOTICE*/

#ifndef _COMMON_H
#define _COMMON_H


#include <sys/types.h>
#include <time.h>

#include "csa.h"


/* Define TRUE and FALSE values. */
#ifndef TRUE
#define TRUE	1
#endif
 
#ifndef FALSE
#define FALSE	0
#endif

#ifndef UNKNOWN
#define UNKNOWN	-1
#endif


/* These tell whether the attribute passed is a CALENDAR_ATTRIBUTE or
 * an ENTRY_ATTRIBUTE.
 */
#define	CALENDAR_ATTRIBUTE	0
#define	ENTRY_ATTRIBUTE		1


/* These are the versions of the rpc.cmsd daemon. */
#define RPCVER2		2
#define	RPCVER3		3
#define	RPCVER4		4
#define	RPCVER5		5


/* These are the data versions of the callog files. */
#define	DATAVER1	1	/* Versions 1 and 2 rpc.cmsd data. */
#define	DATAVER2	2	/* Version 3 rpc.cmsd data. */
#define	DATAVER3	3	/* Version 4 rpc.cmsd data. */
#define	DATAVER4	4	/* Version 5 rpc.cmsd data. */


/* Define the size of certain CSA character arrays. */
#define	UTC_ARRAY_SIZE	17


#define LARG         50

#define TESTFILES       "./data/%s"
#define CONFIGFILES     "./data/%s.cf"


/*============================= Macro definitions ============================*/

#define APPOINTMENT_START_TIME1 "19990101T160000Z"
#define APPOINTMENT_START_TIME2 "19990105T080000Z"
#define APPOINTMENT_END_TIME1   "19990101T170000Z"
#define APPOINTMENT_END_TIME2   "19990105T100000Z"
#define EARLY_TIME              915050000
#define START_TIME1             915206400
#define MIDDLE_TIME             915350000
#define START_TIME2             915523200
#define LATE_TIME               915650000


/*============================ User defined types ============================*/
/* Are we reading or writing out an appoinment? */
typedef enum {appt_read, appt_write} Allocation_reason;


/* For creating or updating either calendar or entry attributes. */
typedef enum {dtcm_create, dtcm_update} Calendar_op;


/* Different versions to check - csa library, callog data file, and rpc. */
typedef enum {ver_csalib, ver_callog_data, ver_rpc_cmsd} Version_type;


/*============================= Global variables =============================*/

/* Set of valid user login and valid machine host names. */
extern	char	*Csa_user1, *Csa_user2, *Csa_user3;
extern	char	*Csa_calname1, *Csa_calname2, *Csa_calname3;
extern	char	*Csa_host1, *Csa_host2, *Csa_host3, *Csa_host4;


/* Have an invalid user and host name as well. */
extern	char	*Csa_invalid_user, *Csa_invalid_host;


/* Location of the CSA spool directory associated with DT_HOSTNAME, 
 * DT_HOSTNAME2 and DT_HOSTNAME3 respectively.
 */
extern  char	*Csa_spool, *Csa_spool_host2, *Csa_spool_host3;


/*============================= Global functions =============================*/

void MtTest_Init_Server();

void Clean_Up_Calendar(char * name, void * rqstp);

void Create_Calendar(char * name, void * rqstp);

cms_attribute * Make_Attrs(int entry_id);

void Add_Entry(char * name, void * rqstp, int entry_num);

void Init_Entry_Attr(int id, cms_attribute * attrs, int type);

void Set_Up_Time_Criteria(time_t stime, CSA_enum op1, time_t etime, CSA_enum op2, cms_attribute ** attrs, CSA_enum ** new_ops);

#endif /* _COMMON_H */
