/*
 * CDE Test Program
 * 
 * Lookup Test - lookup_test
 * 
 * Test1 - Verify that cms_lookup_reminder() is thread safe when multiple 
 *         threads call it with the same and different calendars and 
 *         different matching criteria.
 *       - Check that each thread gets back the same reminders for a givin
 *         calendar and criteria.
 * Test2 - Verify that _DtCm_rtable_lookup_range() is thread safe when 
 *         multiple threads call it with the same and different calendars and 
 *         different matching criteria.
 *       - Check that each thread gets back the appointments in the time range.
 * Test3 - Verify that cms_lookup_entries() is thread safe when multiple 
 *         threads call it with the same and different calendars and 
 *         different matching criteria.
 *       - Check that each thread gets back the right entries with the correct
 *         data.
 * Test4 - Verify that _DtCm_rtable_lookup() is thread safe when multiple 
 *         threads call it with the same and different calendars and different 
 *         matching criteria.
 *       - Check that each thread gets back the appointment associated with
 *         the key.
 * Test5 - Verify that _DtCm_rtable_abbreviated_lookup_range() is thread safe 
 *         when multiple threads call it with the same and different calendars
 *         and different matching criteria.
 *       - Check that each thread gets back the same appointment in the time
 *         range.
 * Test6 - Verify that cms_lookup_entries(), cms_lookup_reminder(), and
 *	   _DtCm_rtable_lookup_range() are thread safe when multiple threads 
 *         lookup entries in the same and different calendars with different
 *         matching criteria.
 *	 - Check that each thread gets back appropriate entries from the 
 *         corresponding calendar.
 *
 */

#include "cm.h"
#include "rtable4.h"
#include "mtcommon.h"
#include "common.h"

#include <sys/utsname.h>

#define NUM_OF_THREADS	         24        /* Must be under 300 */

static	ThrData	thrdata[NUM_OF_THREADS];
static	int	thr_count = 0;
static  pthread_mutex_t           data_lock;
/* Transport */
static  char *  Csa_host;
static  struct __svcxprt          rq_xprt;
static  struct svc_req            rqstp;
static  struct authunix_parms     ucred;
static  char    netid[4];    
static  char    buf[16];            
int     passed = 1;

extern  char *  pgname;

void *
clean_up_calendars()
{
  char *                   name;
  int                      i;

  for(i = 0; i < (NUM_OF_THREADS/3+1); i++) {
    if (i < 10)
      name = (CSA_string)calloc(1, strlen(Csa_host)+12);
    else
      name = (CSA_string)calloc(1, strlen(Csa_host)+13);
    sprintf(name, "MtTestCal%d@%s", i, Csa_host);
    Clean_Up_Calendar(name, &rqstp);
  }

}

void *
clean_up_threads()
{
  mttest_mutex_lock(&data_lock);
  thr_count++;
  if (thr_count == NUM_OF_THREADS) {
    mttest_mutex_lock(&threads_lock);
    mttest_cond_signal(&cond_done);
    mttest_mutex_unlock(&threads_lock);
  }
  mttest_mutex_unlock(&data_lock);
}

void *
end_test(char * message)
{
  printf("%s", message);
  printf("Lookup Test: TEST FAILED\n");
  passed = 0;
}

static void *
API_test1(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	cms_reminder_args        argument;
	cms_reminder_res *       result = (cms_reminder_res *)calloc(1, sizeof(cms_reminder_res));

	argument.cal = td->cal_addr;
	argument.tick = td->tick;
	argument.num_names = 0;
	argument.names = NULL;

	cms_lookup_reminder_5_svc(&argument, result, &rqstp);

	td->stat = result->stat;
	td->refs = result->rems;

	clean_up_threads();
}

static void *
API_test2(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	Table_Args_4             argument;
	Table_Res_4 *            result = (Table_Res_4 *)calloc(1, sizeof(Table_Res_4));
	Range_4 *                range = (Range_4 *)calloc(1, sizeof(Range_4));

	range->key1 = (long)td->key1;
	range->key2 = (long)td->key2;
	range->next = NULL;
	argument.target = td->cal_addr;
	argument.args.Args_4_u.range = range;

	_DtCm_rtable_lookup_range_4_svc(&argument, result, &rqstp);

	td->stat = result->status;
	td->res = result;

	clean_up_threads();
}

static void *
API_test3(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	cms_lookup_entries_args  argument;
	cms_entries_res          result;

	argument.cal = td->cal_addr;
	argument.num_attrs = 2;
	argument.attrs = td->cms_attrs;
	argument.ops = td->ops;

	cms_lookup_entries_5_svc(&argument, &result, &rqstp);

	td->stat = result.stat;
	td->cms_entry = result.entries;

	clean_up_threads();
}

static void *
API_test4(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	Table_Args_4             argument;
	Table_Res_4 *            result = (Table_Res_4 *)calloc(1, sizeof(Table_Res_4));
	Uid_4 *                  key = (Uid_4 *)calloc(1, sizeof(Uid_4));

	key->appt_id.tick = START_TIME1;
	key->appt_id.key = 1;
	key->next = NULL;
	argument.target = td->cal_addr;
	argument.args.Args_4_u.key = key;

	_DtCm_rtable_lookup_4_svc(&argument, result, &rqstp);

	td->stat = result->status;
	td->res = result;

	clean_up_threads();
}

static void *
API_test5(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	Table_Args_4             argument;
	Table_Res_4 *            result = (Table_Res_4 *)calloc(1, sizeof(Table_Res_4));
	Range_4 *                range = (Range_4 *)calloc(1, sizeof(Range_4));

	range->key1 = (long)td->key1;
	range->key2 = (long)td->key2;
	range->next = NULL;
	argument.target = td->cal_addr;
	argument.args.Args_4_u.range = range;

	_DtCm_rtable_abbreviated_lookup_range_4_svc(&argument, result, &rqstp);

	td->stat = result->status;
	td->res = result;

	clean_up_threads();
}


static void *
API_test6(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
        int                      type;

	type = td->index % 5;

	switch (type) {
	case 0:
	  API_test1(client_data);
	  break;
	case 1:
	  API_test2(client_data);
	  break;
	case 2:
	  API_test3(client_data);
	  break;
	case 3:
	  API_test4(client_data);
	  break;
	case 4:
	  API_test5(client_data);
	  break;
	}
}

static int
verify_attribute(cms_entry * ent, char * start_time, char * end_time)
{
	if ((ent->attrs[0].name.num != 2) ||
	    (strcmp(ent->attrs[0].name.name, "-//XAPIA/CSA/ENTRYATTR//NONSGML Audio Reminder//EN")) ||
	    (ent->attrs[0].value->type !=  13) ||
	    (strcmp(ent->attrs[0].value->item.reminder_value->lead_time, "+PT300S")) ||
	    (ent->attrs[0].value->item.reminder_value->repeat_count != 0) ||
	    (ent->attrs[0].value->item.reminder_value->reminder_data.size != 0))
	  return(1);
	if ((ent->attrs[1].name.num != 3) ||
	    (strcmp(ent->attrs[1].name.name, "-//XAPIA/CSA/ENTRYATTR//NONSGML Classification//EN")) ||
	    (ent->attrs[1].value->type != 4) ||
	    (ent->attrs[1].value->item.sint32_value != 0))
 	  return(1);  	
	if ((ent->attrs[2].name.num != 5) ||
	    (strcmp(ent->attrs[2].name.name, "-//XAPIA/CSA/ENTRYATTR//NONSGML Date Created//EN")) ||
	    (ent->attrs[2].value->type != 7))
 	  return(1);  	
	if ((ent->attrs[3].name.num != 8) ||
	    (strcmp(ent->attrs[3].name.name, "-//XAPIA/CSA/ENTRYATTR//NONSGML End Date//EN")) ||
	    (ent->attrs[3].value->type != 7) ||
	    (strcmp(ent->attrs[3].value->item.calendar_user_value, end_time)))
 	  return(1);  	
	if ((ent->attrs[4].name.num != 12) ||
	    (strcmp(ent->attrs[4].name.name, "-//XAPIA/CSA/ENTRYATTR//NONSGML Last Update//EN")) ||
	    (ent->attrs[4].value->type != 7))
 	  return(1);  		
	if ((ent->attrs[5].name.num != 15) ||
	    (strcmp(ent->attrs[5].name.name, "-//XAPIA/CSA/ENTRYATTR//NONSGML Organizer//EN")) ||
	    (ent->attrs[5].value->type != 6))
 	  return(1);  
	if ((ent->attrs[6].name.num != 16) ||
	    (strcmp(ent->attrs[6].name.name, "-//XAPIA/CSA/ENTRYATTR//NONSGML Popup Reminder//EN")) ||
	    (ent->attrs[6].value->type !=  13) ||
	    (strcmp(ent->attrs[6].value->item.reminder_value->lead_time, "+PT300S")) ||
	    (ent->attrs[6].value->item.reminder_value->repeat_count != 0) ||
	    (ent->attrs[6].value->item.reminder_value->reminder_data.size != 0))
	  return(1);
	if ((ent->attrs[7].name.num != 20) ||
	    (strcmp(ent->attrs[7].name.name, "-//XAPIA/CSA/ENTRYATTR//NONSGML Reference Identifier//EN")) ||
	    (ent->attrs[7].value->type != 14))
 	  return(1);  
	if ((ent->attrs[8].name.num != 23) ||
	    (strcmp(ent->attrs[8].name.name, "-//XAPIA/CSA/ENTRYATTR//NONSGML Start Date//EN")) ||
	    (ent->attrs[8].value->type != 7) ||
	    (strcmp(ent->attrs[8].value->item.calendar_user_value, start_time)))
 	  return(1);
	if ((ent->attrs[9].name.num != 24) ||
	    (strcmp(ent->attrs[9].name.name, "-//XAPIA/CSA/ENTRYATTR//NONSGML Status//EN")) ||
	    (ent->attrs[9].value->type != 4) ||
	    (ent->attrs[9].value->item.sint32_value != 2304))
 	  return(1);
	if ((ent->attrs[10].name.num != 25) ||
	    (strcmp(ent->attrs[10].name.name, "-//XAPIA/CSA/ENTRYATTR//NONSGML Subtype//EN")) ||
	    (ent->attrs[10].value->type != 5))
 	  return(1);
	if ((ent->attrs[11].name.num != 26) ||
	    (strcmp(ent->attrs[11].name.name, "-//XAPIA/CSA/ENTRYATTR//NONSGML Summary//EN")) ||
	    (ent->attrs[11].value->type != 5))
 	  return(1);
	if ((ent->attrs[12].name.num != 28) ||
	    (strcmp(ent->attrs[12].name.name, "-//XAPIA/CSA/ENTRYATTR//NONSGML Type//EN")) ||
	    (ent->attrs[12].value->type != 4))
 	  return(1);
	if ((ent->attrs[13].name.num != 29) ||
	    (strcmp(ent->attrs[13].name.name, "-//CDE_XAPIA_PRIVATE/CSA/ENTRYATTR//NONSGML Show Time//EN")) ||
	    (ent->attrs[13].value->type != 3) ||
	    (ent->attrs[13].value->item.flags_value != 1))
 	  return(1);
	if ((ent->attrs[14].name.num != 30) ||
	    (strcmp(ent->attrs[14].name.name, "-//CDE_XAPIA_PRIVATE/CSA/ENTRYATTR//NONSGML Repeat Type//EN")) ||
	    (ent->attrs[14].value->type != 3) ||
	    (ent->attrs[14].value->item.flags_value != 0))
 	  return(1);
	return(0);
}

static int
verify1(int thread)
{
        cms_reminder_ref *  ref;
	char                message[200];

	if (thrdata[thread].stat != CSA_SUCCESS) {
	  sprintf(message, "ERROR: thread %d returned status of %d\n", thread, 
		 thrdata[thread].stat);
	  end_test(message);
	  return(1);
	}
	ref = thrdata[thread].refs;

	if (strcmp(ref->reminder_name, "-//XAPIA/CSA/ENTRYATTR//NONSGML Audio Reminder//EN")) {
	  sprintf(message, "ERROR: thread %d returned different name: %s instead of -//XAPIA/CSA/ENTRYATTR//NONSGML Audio Reminder//EN\n", thread, ref->reminder_name);
	  end_test(message);
	  return(1);
	}
	if (ref->key.time != START_TIME1 || ref->key.id != 1) {
	  sprintf(message, "ERROR: thread %d returned different key, time: %d and id: %d instead of time: %d and id: 1\n", thread, ref->key.time, ref->key.id, START_TIME1);
	  end_test(message);  
	  return(1);
	}
	if (ref->runtime != START_TIME1 - 300) {
	  sprintf(message, "ERROR: thread %d returned different run time: %d instead of %d\n", thread, ref->runtime, START_TIME1 - 300);
	  end_test(message); 
	  return(1);
	}
	ref = ref->next;

	if (strcmp(ref->reminder_name, "-//XAPIA/CSA/ENTRYATTR//NONSGML Popup Reminder//EN")) {
	  sprintf(message,"ERROR: thread %d returned different name: %s instead of -//XAPIA/CSA/ENTRYATTR//NONSGML Audio Reminder//EN\n", thread, ref->reminder_name);
	  end_test(message);
	  return(1);
	}
	if (ref->key.time != START_TIME1 || ref->key.id != 1) {
	  sprintf(message, "ERROR: thread %d returned different key, time: %d and id: %d instead of time: %d and id: 1\n", thread, ref->key.time, ref->key.id, START_TIME1);
	  end_test(message);  
	  return(1);
	}
	if (ref->runtime != START_TIME1 - 300) {
	  sprintf(message, "ERROR: thread %d returned different run time: %d instead of %d\n", thread, ref->runtime, START_TIME1 - 300);
	  end_test(message); 
	  return(1);
	}

	if (ref->next != NULL) {
	  sprintf(message,"ERROR: thread %d returned too many reminders\n", thread);
	  end_test(message); 
	  return(1);
	}
	return(0);
}

static int
verify2(int thread)
{     
        Appt_4 *   ref;
	char       message[200];

        if (thrdata[thread].stat != 0) {
	  sprintf(message, "ERROR: thread %d returned status of %d\n", thread, 
		 thrdata[thread].stat);
	  end_test(message); 
	  return(1);
	}
	if (thrdata[thread].res->res.tag != AP_4) {
	  sprintf(message,"ERROR: thread %d returned wrong result type: %d instead of %d\n", thread, thrdata[thread].res->res.tag, AP_4);
	  end_test(message); 
	  return(1);
	}
	ref = thrdata[thread].res->res.Table_Res_List_4_u.a;

	if ((thrdata[thread].index % 3) < 2) {
	  if (ref == NULL) {
	    sprintf(message,"ERROR: thread %d returned too few appointments\n", thread);
	    end_test(message); 
	    return(1);
	  }
	  if ((ref->appt_id.tick != START_TIME1) && (ref->appt_id.key != 1)) {
	    sprintf(message,"ERROR: thread %d returned the wrong appt_id\n", thread);
	    end_test(message); 
	    return(1);
	  }
	  ref = ref->next;
	}

	if ((thrdata[thread].index % 3) > 0) {
	  if (ref == NULL) {
	    sprintf(message,"ERROR: thread %d returned too few appointments\n", thread);
	    end_test(message); 
	    return(1);
	  }
	  if ((ref->appt_id.tick != START_TIME2) && (ref->appt_id.key != 1)) {
	    sprintf(message,"ERROR: thread %d returned the wrong appt_id\n", thread);
	    end_test(message); 
	    return(1);
	  }
	  ref = ref->next;
	}

	if (ref != NULL) {
	  sprintf(message,"ERROR: thread %d returned too many appointments\n", thread);
	  end_test(message); 
	  return(1);
	}
	return(0);
}

static int
verify3(int thread)
{
	cms_entry *         ent;
	char *              start_time;
	char *              end_time;
	int                 start, id;
	char                message[200];

	if ((thread % 3) < 2) {
	  start_time = strdup(APPOINTMENT_START_TIME1);
	  start = START_TIME1;
	  id = 1;
	  end_time = strdup(APPOINTMENT_END_TIME1);
	}
	else {
	  start_time = strdup(APPOINTMENT_START_TIME2);
	  start = START_TIME2;
	  id = 2;
	  end_time = strdup(APPOINTMENT_END_TIME2);
	}

	if (thrdata[thread].stat != CSA_SUCCESS) {
	  sprintf(message,"ERROR: thread %d returned status of %d\n", thread, 
		 thrdata[thread].stat);
	  end_test(message); 
	  return(1);
	}
	ent = thrdata[thread].cms_entry;

	if (ent == NULL) {
	  sprintf(message,"ERROR: thread %d returned too few entries\n", thread);
	  end_test(message); 
	  return(1);
	}
	if ((ent->key.time != start) || (ent->key.id != id)) {
	  sprintf(message,"ERROR: thread %d returned wrong key\n", thread);
	  end_test(message); 
	  return(1);
	}
	if (ent->num_attrs != 15) {
	  sprintf(message,"ERROR: thread %d returned wrong number of attributes: %d instead of 15\n", thread, ent->num_attrs);
	  end_test(message); 
	  return(1);
	}

	if (verify_attribute(ent, start_time, end_time)) {
	  sprintf(message,"ERROR: thread %d returned entry with wrong attribute\n", thread);
	  end_test(message);
	  return(1);
	}

	if ((thread % 3) == 1) {
	  ent = ent->next;

	  if (ent == NULL) {
	    sprintf(message,"ERROR: thread %d returned too few entries\n", thread);
	    end_test(message); 
	    return(1);
	  }
	  start_time = strdup(APPOINTMENT_START_TIME2);
	  start = START_TIME2;
	  id = 2;
	  end_time = strdup(APPOINTMENT_END_TIME2);

	  if (ent->key.time != start || ent->key.id != id) {
	    sprintf(message,"ERROR: thread %d returned wrong key\n", thread);
	    end_test(message); 
	    return(1);
	  }
	  if (ent->num_attrs != 15) {
	    sprintf(message, "ERROR: thread %d returned wrong number of attributes: %d instead of 15\n", thread, ent->num_attrs);
	    end_test(message); 
	    return(1);
	  }
	  if (verify_attribute(ent, start_time, end_time)) {
	    sprintf(message,"ERROR: thread %d returned entry with wrong attribute\n", thread);
	    end_test(message);
	    return(1);
	  }
	}

	if (ent->next != NULL) {
	  sprintf(message,"ERROR: thread %d returned too many entries\n", thread);
	  end_test(message); 
	  return(1);
	}
	return(0);
}

static int
verify4(int thread)
{     
        Appt_4 *            ref;
	char                message[200];

        if (thrdata[thread].stat != 0) {
	  sprintf(message,"ERROR: thread %d returned status of %d\n", thread, 
		 thrdata[thread].stat);
	  end_test(message); 
	  return(1);
	}
	if (thrdata[thread].res->res.tag != AP_4) {
	  sprintf(message,"ERROR: thread %d returned wrong result type: %d instead of %d\n", thread, thrdata[thread].res->res.tag, AP_4);
	  end_test(message); 
	  return(1);
	}
	ref = thrdata[thread].res->res.Table_Res_List_4_u.a;

	if ((ref->appt_id.tick != START_TIME1) && (ref->appt_id.key != 1)) {
	  sprintf(message,"ERROR: thread %d returned the wrong appt_id\n", thread);
	  end_test(message); 
	  return(1);
	}

	return(0);
}

static int
verify5(int thread)
{     
        Abb_Appt_4 *        ref;
	char                message[200]; 

        if (thrdata[thread].stat != 0) {
	  sprintf(message,"ERROR: thread %d returned status of %d\n", thread, 
		 thrdata[thread].stat);
	  end_test(message);
	  return(1);
	}
        if (thrdata[thread].res->status != 0) {
	  sprintf(message,"ERROR: thread %d returned status of %d instead of access_ok_4\n", thread, thrdata[thread].res->status);
	  end_test(message);
	  return(1);
	}
	if (thrdata[thread].res->res.tag != AB_4) {
	  sprintf(message,"ERROR: thread %d returned wrong result type: %d instead of %d\n", thread, thrdata[thread].res->res.tag, AB_4);
	  end_test(message);
	  return(1);
	}
	ref = thrdata[thread].res->res.Table_Res_List_4_u.b;

	if (ref== NULL) {
	  sprintf(message,"ERROR: thread %d returned too few appointments\n", thread);
	  end_test(message); 
	  return(1);
	}
	if ((thrdata[thread].index % 3) < 2) {
	  if ((ref->appt_id.tick != START_TIME1) && (ref->appt_id.key != 1)) {
	    sprintf(message,"ERROR: thread %d returned the wrong appt_id\n", thread);
	    end_test(message); 
	    return(1);
	  }
	  ref = ref->next;
	}

	if ((thrdata[thread].index % 3) > 0) {
	  if (ref== NULL) {
	    sprintf(message,"ERROR: thread %d returned too few appointments\n", thread);
	    end_test(message); 
	    return(1);
	  }
	  if ((ref->appt_id.tick != START_TIME2) && (ref->appt_id.key != 1)) {
	    sprintf(message,"ERROR: thread %d returned the wrong appt_id\n", thread);
	    end_test(message); 
	    return(1);
	  }
	  ref = ref->next;
	}

	if (ref != NULL) {
	  sprintf(message,"ERROR: thread %d returned too many appointments\n", thread);
	  end_test(message); 
	  return(1);
	}
	return(0);
}

void
main(int argc, char **argv)
{
        CSA_string          addr[NUM_OF_THREADS/3+1];
	int                 i, j, flag;
	CSA_calendar_user * ptr;
	struct utsname      sname;
	char *              dir = _DtCMS_DEFAULT_DIR;
	static  pid_t       pid;
	time_t		    tick;
	cms_attribute *	    match_attr1;
	cms_attribute *	    match_attr2;
	cms_attribute *	    match_attr3;
	CSA_enum *	    ops1;
	CSA_enum *	    ops2;
	CSA_enum *	    ops3;

	printf("Lookup Test: TEST BEGUN\n");

	pgname = argv[0];

	MtTest_Init_Server();

	/* Find name of system we are on. */
	uname(&sname);
	Csa_host = sname.nodename;

	/* Change to the calendar directory. */
	chdir(dir);
	
	/* Find pid of this process. */
	pid = getpid();

	/* Set up transport data. */
	sprintf(netid, "tcp");
	ucred.aup_uid = 60001;
	ucred.aup_machname = Csa_host;
	rq_xprt.xp_rtaddr.maxlen = 16;
	rq_xprt.xp_rtaddr.len = 16;
	rq_xprt.xp_rtaddr.buf = buf;
	rq_xprt.xp_netid = netid;
	rqstp.rq_cred.oa_flavor = AUTH_UNIX;
	rqstp.rq_clntcred = (caddr_t)&ucred;
	rqstp.rq_xprt = &rq_xprt;

	/* Set up calendar names. */
	for(i = 0; i < (NUM_OF_THREADS/3+1); i++) {
	  if (i < 10)
	    addr[i] = (CSA_string)calloc(1, strlen(Csa_host)+12);
	  else
	    addr[i] = (CSA_string)calloc(1, strlen(Csa_host)+13);
	  sprintf(addr[i], "MtTestCal%d@%s", i, Csa_host);
	}

	clean_up_calendars();

	/* Create calendars and add two entries to them. */
	for(i = 0; i < (NUM_OF_THREADS/3+1); i++) {
	  Create_Calendar(addr[i], &rqstp);
	  Add_Entry(addr[i], &rqstp, 1);
	  Add_Entry(addr[i], &rqstp, 2);
	}

	/* set up matching criteria */
	Set_Up_Time_Criteria(EARLY_TIME, CSA_MATCH_GREATER_THAN, MIDDLE_TIME,
			     CSA_MATCH_LESS_THAN, &match_attr1, &ops1);
	Set_Up_Time_Criteria(EARLY_TIME, CSA_MATCH_GREATER_THAN, LATE_TIME,
			     CSA_MATCH_LESS_THAN, &match_attr2, &ops2);
	Set_Up_Time_Criteria(MIDDLE_TIME, CSA_MATCH_GREATER_THAN, LATE_TIME,
			     CSA_MATCH_LESS_THAN, &match_attr3, &ops3);

	/* set up per thread data */
	for (i = 0; i < NUM_OF_THREADS; i++) {
		thrdata[i].stat = -1;
		thrdata[i].tick = 0;
		if ((i % 3) == 0) {
		  thrdata[i].cal_addr = addr[i/3];
		  thrdata[i].cms_attrs = match_attr1;
		  thrdata[i].key1 = EARLY_TIME;
		  thrdata[i].key2 = MIDDLE_TIME;
		  thrdata[i].ops = ops1;
		}
		if ((i % 3) == 1) {
		  thrdata[i].cal_addr = addr[i/3];
		  thrdata[i].cms_attrs = match_attr2;
		  thrdata[i].key1 = EARLY_TIME;
		  thrdata[i].key2 = LATE_TIME;
		  thrdata[i].ops = ops2;
		}
		if ((i % 3) == 2) {
		  thrdata[i].cal_addr = addr[i/3];
		  thrdata[i].cms_attrs = match_attr3;
		  thrdata[i].key1 = MIDDLE_TIME;
		  thrdata[i].key2 = LATE_TIME;
		  thrdata[i].ops = ops3;
		}
		thrdata[i].pid = pid;
		thrdata[i].index = i;
	}

	printf("Lookup Test: TEST #1\n");
	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test1, thrdata);
	for(i = 0; i < NUM_OF_THREADS; i++) {
	  if (verify1(i))
	    break;
	}

	printf("Lookup Test: TEST #2\n");
	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test2, thrdata);
	for(i = 0; i < NUM_OF_THREADS; i++) {
	  if (verify2(i))
	    break;
	}

	printf("Lookup Test: TEST #3\n");
	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test3, thrdata);
	for(i = 0; i < NUM_OF_THREADS; i++) {
	  if (verify3(i))
	    break;
	}

	printf("Lookup Test: TEST #4\n");
	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test4, thrdata);
	for(i = 0; i < NUM_OF_THREADS; i++) {
	  if (verify4(i))
	    break;
	}

	printf("Lookup Test: TEST #5\n");
	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test5, thrdata);
	for(i = 0; i < NUM_OF_THREADS; i++) {
	  if (verify5(i))
	    break;
	}

	printf("Lookup Test: TEST #6\n");
	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test6, thrdata);
	for(i = 0; i < NUM_OF_THREADS; i++) {
	  if ((i % 5) == 0) {
	    if (verify1(i))
	      break;
	  }
	  if ((i % 5) == 1) {
	    if (verify2(i))
	      break;
	  }
	  if ((i % 5) == 2) {
	    if (verify3(i))
	      break;
	  }
	  if ((i % 5) == 3) {
	    if (verify4(i))
	      break;
	  }
	  if ((i % 5) == 4) {
	    if (verify5(i))
	      break;
	  }
	}

	clean_up_calendars();

	if (passed)
	  printf("Lookup Test: TEST PASSED\n\n");
}
