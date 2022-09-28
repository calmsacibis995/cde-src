/*
 * CDE Test Program
 * 
 * Entry Test - entry_test
 * 
 * Test1 - Verify that cms_insert_entry() is thread safe when multiple threads 
 *         call it with the same and different entries and the same and 
 *         different calendars.
 *       - Check that each insert returns a valid result.
 * Test2 - Verify that cms_get_entry_attr() is thread safe when multiple 
 *         threads get the same entry in the same and different calendars.
 *       - Check that all threads return the same entry.
 * Test3 - Verify that cms_update_entry() is thread safe when multiple threads 
 *         call to update an entry in the same and different calendars.
 *       - Check that all threads get CSA_SUCCESS and that the entry is
 *         updated correctly.
 * Test4 - Verify that cms_delete_entry() is thread safe when multiple threads 
 *         call it with the same entry handles referring to the same and
 *         different calendars.
 *       - Check that only 1 thread gets CSA_SUCCESS for each calendar and 
 *         that all others get CSA_X_DT_E_ENTRY_NOT_FOUND.
 * Test5 - Verify that _DtCm_rtable_insert() is thread safe when multiple 
 *         threads call it with the same entry and the same and different 
 *         calendars.
 *       - Check that each insert returns a valid result.
 * Test6 - Verify that _DtCm_rtable_change() is thread safe when multiple 
 *         threads call to update an entry in the same and different calendars.
 *       - Check that all threads get access_ok_4 and that the entry is
 *         updated correctly.
 * Test7 - Verify that _DtCm_rtable_delete() is thread safe when multiple 
 *         threads call it with the same entry handles referring to the same 
 *         and different calendars.
 *       - Check that only 1 thread gets access_ok_4 for each calendar and 
 *         that all others get access_other_4.
 * Test8 - Verify that cms_get_entry_attr(), cms_insert_entry(), 
 *         cms_update_entry(), cms_delete_entry(), _DtCm_rtable_insert(),
 *         _DtCm_rtable_change(), and _DtCm_rtable_delete() are thread safe 
 *         when called at the same time with the same entry and the same and
 *         different calendars.
 *       - Check that all threads get CSA_SUCCESS, CSA_X_DT_E_ENTRY_NOT_FOUND,
 *         access_other_4, or access_ok_4.
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
char    user[200];
char    message[200];

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
  printf("Entry Test: TEST FAILED\n");
  passed = 0;
}

static void *
API_test1(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	cms_insert_args          argument;
	cms_entry_res *          result = (cms_entry_res *)calloc(1, sizeof(cms_entry_res));

	argument.cal = td->cal_addr;
	argument.pid = td->pid;
	argument.num_attrs = 6;
	argument.attrs = td->cms_attrs;

	cms_insert_entry_5_svc(&argument, result, &rqstp);

	td->stat = result->stat;
	td->cms_entry = result->entry;

	clean_up_threads();
}

static void *
API_test2(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	cms_get_entry_attr_args  argument;
	cms_get_entry_attr_res * result = (cms_get_entry_attr_res *)calloc(1, sizeof(cms_get_entry_attr_res));
	cms_key                  keys;
	cms_attr_name            names;

	keys.time = START_TIME1;
	keys.id = 1;
	names.num = 0;
	names.name = strdup(CSA_ENTRY_ATTR_SUMMARY);
	argument.cal = td->cal_addr;
	argument.num_keys = 1;
	argument.keys = &keys;
	argument.num_names = 1;
        argument.names = &names;

	cms_get_entry_attr_5_svc(&argument, result, &rqstp);

	td->stat = result->stat;
	td->cms_item = result->entries;

	clean_up_threads();
}

static void *
API_test3(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	cms_update_args          argument;
	cms_entry_res *          result = (cms_entry_res *)calloc(1, sizeof(cms_entry_res));
	cms_key                  keys;

	keys.time = START_TIME1;
	keys.id = 1;
	argument.cal = td->cal_addr;
	argument.pid = td->pid;
	argument.entry = keys;
	argument.scope = 1;
	argument.num_attrs = 1;
	argument.attrs = td->cms_new_attrs;

	cms_update_entry_5_svc(&argument, result, &rqstp);

	td->stat = result->stat;
	td->cms_entry = result->entry;

	clean_up_threads();
}

static void *
API_test4(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	cms_delete_args          argument;
	CSA_return_code          result;
	cms_key                  keys;

	keys.time = START_TIME1;
	keys.id = 1;
	argument.cal = td->cal_addr;
	argument.pid = td->pid;
	argument.entry = keys;
	argument.scope = 1;

	cms_delete_entry_5_svc(&argument, &result, &rqstp);

	td->stat = result;

	clean_up_threads();
}

static void *
API_test5(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	Table_Args_4             argument;
	Table_Res_4 *            result = (Table_Res_4 *)calloc(1, sizeof(Table_Res_4));
	Appt_4                   appt;
	Tag_4                    tag;
	Attribute_4              attr, attr2;

	attr2.clientdata = strdup("");
	attr2.value = strdup("300");
	attr2.attr = strdup("bp");
	attr2.next = NULL;
	attr.clientdata = strdup("");
	attr.value = strdup("300");
	attr.attr = strdup("op");
	attr.next = &attr2;
	tag.tag = appointment_4;
	tag.showtime = 1;
	tag.next = NULL;
	appt.appt_id.tick = START_TIME1;
	appt.appt_id.key = 1;
	appt.tag = &tag;
        appt.duration = 3600;
	appt.ntimes = 0;
	appt.what = strdup("test appointment");
	appt.period.period = single_4;
	appt.period.nth = 0;
	appt.period.enddate = 0;
	appt.author = strdup(user);
	appt.client_data = strdup("");
	appt.exception = NULL;
	appt.attr = &attr;
	appt.appt_status = active_4;
	appt.privacy = public_4;
	appt.next = NULL;
	argument.target = td->cal_addr;
	argument.pid = td->pid;
	argument.args.tag = APPT_4;
	argument.args.Args_4_u.appt = &appt;

	_DtCm_rtable_insert_4_svc(&argument, result, &rqstp);

	td->stat = result->status;
	td->res_list = &(result->res);

	clean_up_threads();
}

static void *
API_test6(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	Table_Args_4             argument;
	Table_Res_4 *            result = (Table_Res_4 *)calloc(1, sizeof(Table_Res_4));
	Apptid_4                 apptid;
	Id_4                     id;
	Appt_4                   appt;
	Tag_4                    tag;
	Attribute_4              attr, attr2;

	attr2.clientdata = strdup("");
	attr2.value = strdup("300");
	attr2.attr = strdup("bp");
	attr2.next = NULL;
	attr.clientdata = strdup("");
	attr.value = strdup("300");
	attr.attr = strdup("op");
	attr.next = &attr2;
	tag.tag = appointment_4;
	tag.showtime = 1;
	tag.next = NULL;
	appt.appt_id.tick = START_TIME1;
	appt.appt_id.key = 1;
	appt.tag = &tag;
        appt.duration = 3600;
	appt.ntimes = 0;
	appt.what = strdup("changed appointment");
	appt.period.period = single_4;
	appt.period.nth = 0;
	appt.period.enddate = 0;
	appt.author = strdup(user);
	appt.client_data = strdup("");
	appt.exception = NULL;
	appt.attr = &attr;
	appt.appt_status = active_4;
	appt.privacy = public_4;
	appt.next = NULL;

	id.tick = START_TIME1;
	id.key = 1;
	argument.target = td->cal_addr;
	argument.pid = td->pid;
	argument.args.tag = APPTID_4;
	argument.args.Args_4_u.apptid.oid = &id;
	argument.args.Args_4_u.apptid.new_appt = &appt;
	argument.args.Args_4_u.apptid.option = do_all_4;

	_DtCm_rtable_change_4_svc(&argument, result, &rqstp);

	td->stat = result->status;
	td->res_list = &(result->res);

	clean_up_threads();
}

static void *
API_test7(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	Table_Args_4             argument;
	Table_Res_4 *            result = (Table_Res_4 *)calloc(1, sizeof(Table_Res_4));
	Uidopt_4                 uidopt;

	uidopt.appt_id.tick = START_TIME1;
	uidopt.appt_id.key = 1;
	uidopt.option = do_all_4;
	uidopt.next = NULL;
	argument.target = td->cal_addr;
	argument.pid = td->pid;
	argument.args.tag = UIDOPT_4;
	argument.args.Args_4_u.uidopt = &uidopt;

	/* Only one thread/calendar because V4 doesn't allow deletion of non-
	   existant entries. */
	if (td->index % 3 == 1) {
	  _DtCm_rtable_delete_4_svc(&argument, result, &rqstp);
	}

	td->stat = result->status;
	td->res_list = &(result->res);

	clean_up_threads();
}

static void *
API_test8(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
        int                      type;

	type = td->index % 7;

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
	case 5:
	  API_test6(client_data);
	  break;
	case 6:
	  API_test7(client_data);
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
	    (ent->attrs[5].value->type != 6) ||
	    (strcmp(ent->attrs[5].value->item.string_value, user)))
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
	cms_entry *         ent;
	char *              start_time;
	char *              end_time;
	int                 start;

	if ((thread % 3) != 1) {
	  start_time = strdup(APPOINTMENT_START_TIME1);
	  start = START_TIME1;
	  end_time = strdup(APPOINTMENT_END_TIME1);
	}
	else {
	  start_time = strdup(APPOINTMENT_START_TIME2);
	  start = START_TIME2;
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
	if (ent->key.time != start) {
	  sprintf(message,"ERROR: thread %d returned wrong key: %d instead of %d\n", thread, ent->key.time, start);
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

	if (ent->next != NULL) {
	  sprintf(message,"ERROR: thread %d returned too many entries\n", thread);
	  end_test(message); 
	  return(1);
	}
	return(0);
}

static int
verify2(int thread)
{
        cms_get_entry_attr_res_item * item;

	if (thrdata[thread].stat != CSA_SUCCESS) {
	  sprintf(message, "ERROR: thread %d returned status of %d\n", thread, 
		 thrdata[thread].stat);
	  end_test(message);
	  return(1);
	}
	item = thrdata[thread].cms_item;

	if (item->key.time != START_TIME1) {
	  sprintf(message, "ERROR: thread %d returned different key: time %d instead of %d\n", thread, item->key.time, START_TIME1);
	  end_test(message);  
	  return(1);
	}
	if (item->num_attrs != 1) {
	  sprintf(message, "ERROR: thread %d returned wrong number of attributes\n", thread);
	  end_test(message);  
	  return(1);
	}
	if ((item->attrs[11].name.num != 26) ||
	    (strcmp(item->attrs[11].name.name, "-//XAPIA/CSA/ENTRYATTR//NONSGML Summary//EN")) ||
	    (item->attrs[11].value->type != 5))
 	  return(1);

	if (item->next != NULL) {
	  sprintf(message,"ERROR: thread %d returned too many items\n", thread);
	  end_test(message); 
	  return(1);
	}
	return(0);
}

static int
verify3(int thread)
{    
        cms_entry *         ent;
	char *              start_time = strdup(APPOINTMENT_START_TIME1);
	char *              end_time = strdup(APPOINTMENT_END_TIME1);
	int                 start = START_TIME1;

        if (thrdata[thread].stat != 0) {
	  sprintf(message, "ERROR: thread %d returned status of %d\n", thread, 
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
	if (ent->key.time != start) {
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

	if (ent->next != NULL) {
	  sprintf(message,"ERROR: thread %d returned too many entries\n", thread);
	  end_test(message); 
	  return(1);
	}

	if ((ent->attrs[11].name.num != 26) ||
	    (strcmp(ent->attrs[11].name.name, "-//XAPIA/CSA/ENTRYATTR//NONSGML Summary//EN")) ||
	    (ent->attrs[11].value->type != 5) ||
	    (strcmp(ent->attrs[11].value->item.string_value, "changed appointment")))
 	  return(1);
	return(0);
}

static int
verify4(int thread)
{    
        static int          flag;

	if ((thread % 3) == 0) {
	  if ((flag == 0) && (thread != 0)) {
	    sprintf(message, "ERROR: entry not deleted in %s\n", thrdata[thread-1].cal_addr);
	    end_test(message); 
	    return(1);
	  }
	  flag = 0;
	}
        if (thrdata[thread].stat == 0) {
	  if (flag) {
	    sprintf(message, "ERROR: two threads deleted the entry\n");
	    end_test(message); 
	    return(1);
	  }
	  flag = 1;
	}
	return(0);
}

static int
verify5(int thread)
{
	if (thrdata[thread].stat != ok_4) {
	  sprintf(message,"ERROR: thread %d returned status of %d\n", thread, 
		 thrdata[thread].stat);
	  end_test(message); 
	  return(1);
	}
	if (thrdata[thread].res_list->tag != AP_4) {
	  sprintf(message,"ERROR: thread %d returned wrong type\n", thread);
	  end_test(message); 
	  return(1);
	}
	return(0);
}


static int
verify6(int thread)
{
	if (thrdata[thread].stat != ok_4) {
	  sprintf(message,"ERROR: thread %d returned status of %d\n", thread, 
		 thrdata[thread].stat);
	  end_test(message); 
	  return(1);
	}
	if (thrdata[thread].res_list->tag != AP_4) {
	  sprintf(message,"ERROR: thread %d returned wrong type\n", thread);
	  end_test(message); 
	  return(1);
	}
	return(0);
}

static int
verify7(int thread)
{
        /* Only one thread/calendar.  Server will crash if a non-existant entry
           is deleted. */
	if ((thread % 3) == 1) {
	  if (thrdata[thread].stat != access_ok_4) {
	    sprintf(message, "ERROR: entry not deleted in %s\n", thrdata[thread].cal_addr);
	    end_test(message); 
	    return(1);
	  }
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
	cms_attribute *     attrs = (cms_attribute *)calloc(1, sizeof(cms_attribute));
	cms_attribute_value * val = (cms_attribute_value *)calloc(1, sizeof(cms_attribute_value));

	printf("Entry Test: TEST BEGUN\n");

	pgname = argv[0];

	MtTest_Init_Server();

	/* Find name of system we are on. */
	uname(&sname);
	Csa_host = sname.nodename;

	/* Find name of nobody user. */
	sprintf(user,"nobody@%s", Csa_host);

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

	/* Set up attribute. */
	attrs[0].name.num = 0;
	attrs[0].name.name = CSA_ENTRY_ATTR_SUMMARY;
	attrs[0].value = &val[0];
	val[0].type = CSA_VALUE_STRING;
	val[0].item.string_value = "changed appointment";

	/* Set up calendar names. */
	for(i = 0; i < (NUM_OF_THREADS/3+1); i++) {
	  if (i < 10)
	    addr[i] = (CSA_string)calloc(1, strlen(Csa_host)+12);
	  else
	    addr[i] = (CSA_string)calloc(1, strlen(Csa_host)+13);
	  sprintf(addr[i], "MtTestCal%d@%s", i, Csa_host);
	}

	clean_up_calendars();

	/* Create calendars. */
	for(i = 0; i < (NUM_OF_THREADS/3+1); i++) {
	  Create_Calendar(addr[i], &rqstp);
	}

	/* set up per thread data */
	for (i = 0; i < NUM_OF_THREADS; i++) {
		thrdata[i].stat = -1;
		if ((i % 3) == 0) {
		  thrdata[i].cal_addr = addr[i/3];
		  thrdata[i].cms_attrs = Make_Attrs(1);
		  thrdata[i].cms_new_attrs = attrs;
		}
		if ((i % 3) == 1) {
		  thrdata[i].cal_addr = addr[i/3];
		  thrdata[i].cms_attrs = Make_Attrs(2);
		  thrdata[i].cms_new_attrs = attrs;
		}
		if ((i % 3) == 2) {
		  thrdata[i].cal_addr = addr[i/3];
		  thrdata[i].cms_attrs = Make_Attrs(1);
		  thrdata[i].cms_new_attrs = attrs;
		}
		thrdata[i].pid = pid;
		thrdata[i].index = i;
	}

	printf("Entry Test: TEST #1\n");
	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test1, thrdata);
	for(i = 0; i < NUM_OF_THREADS; i++) {
	  if (verify1(i))
	    break;
	}

	clean_up_calendars();

	/* Create calendars and add two entries to them. */
	for(i = 0; i < (NUM_OF_THREADS/3+1); i++) {
	  Create_Calendar(addr[i], &rqstp);
	  Add_Entry(addr[i], &rqstp, 1);
	  Add_Entry(addr[i], &rqstp, 2);
	}

	printf("Entry Test: TEST #2\n");
	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test2, thrdata);
	for(i = 0; i < NUM_OF_THREADS; i++) {
	  if (verify2(i))
	    break;
	}

	clean_up_calendars();

	/* Create calendars and add one entry to them. */
	for(i = 0; i < (NUM_OF_THREADS/3+1); i++) {
	  Create_Calendar(addr[i], &rqstp);
	  Add_Entry(addr[i], &rqstp, 1);
	}

	printf("Entry Test: TEST #3\n");
	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test3, thrdata);
	for(i = 0; i < NUM_OF_THREADS; i++) {
	  if (verify3(i))
	    break;
	}

	clean_up_calendars();

	/* Create calendars and add one entry to them. */
	for(i = 0; i < (NUM_OF_THREADS/3+1); i++) {
	  Create_Calendar(addr[i], &rqstp);
	  Add_Entry(addr[i], &rqstp, 1);
	}

	printf("Entry Test: TEST #4\n");
	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test4, thrdata);
	for(i = 0; i < NUM_OF_THREADS; i++) {
	  if (verify4(i))
	    break;
	}
	for(i = 0; i < (NUM_OF_THREADS/3); i++) {
	  API_test2(&thrdata[i]);
	  if (thrdata[i].cms_item->stat == 0) {
	    printf("ERROR: entry not deleted in %s\n", thrdata[i].cal_addr);
	    printf("Entry Test: TEST FAILED\n");
	    passed = 0;
	    break;
	  }
	}

	clean_up_calendars();

	/* Create calendars and add one entry to them. */
	for(i = 0; i < (NUM_OF_THREADS/3+1); i++) {
	  Create_Calendar(addr[i], &rqstp);
	}

	printf("Entry Test: TEST #5\n");
	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test5, thrdata);
	for(i = 0; i < NUM_OF_THREADS; i++) {
	  if (verify5(i))
	    break;
	}

	clean_up_calendars();

	/* Create calendars and add one entry to them. */
	for(i = 0; i < (NUM_OF_THREADS/3+1); i++) {
	  Create_Calendar(addr[i], &rqstp);
	  Add_Entry(addr[i], &rqstp, 1);
	}

	printf("Entry Test: TEST #6\n");
	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test6, thrdata);
	for(i = 0; i < NUM_OF_THREADS; i++) {
	  if (verify6(i))
	    break;
	}

	clean_up_calendars();

	/* Create calendars and add one entry to them. */
	for(i = 0; i < (NUM_OF_THREADS/3+1); i++) {
	  Create_Calendar(addr[i], &rqstp);
	  Add_Entry(addr[i], &rqstp, 1);
	}

	printf("Entry Test: TEST #7\n");
	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test7, thrdata);
	for(i = 0; i < NUM_OF_THREADS; i++) {
	  if (verify7(i))
	    break;
	}

	clean_up_calendars();

	/* Create calendars and add one entry to them. */
	for(i = 0; i < (NUM_OF_THREADS/3+1); i++) {
	  Create_Calendar(addr[i], &rqstp);
	  Add_Entry(addr[i], &rqstp, 1);
	}

	printf("Entry Test: TEST #8\n");
	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test8, thrdata);
	for(i = 0; i < NUM_OF_THREADS; i++) {
	  if ((i % 7) == 0) {
	    if (thrdata[i].stat != CSA_SUCCESS) {
	      sprintf(message,"ERROR: thread %d returned status of %d\n", i, thrdata[i].stat);
	      end_test(message); 
	      break;
	    }
	  }
	  if ((i % 7) == 1) {
	    if (thrdata[i].cms_item->stat != CSA_SUCCESS) {
	      if (thrdata[i].cms_item->stat != CSA_X_DT_E_ENTRY_NOT_FOUND) {
		sprintf(message, "ERROR: thread %d returned status of %d\n", i, thrdata[i].stat);
		end_test(message);
		break;
	      }
	    }
	    else {
	      if (verify2(i))
		break;
	    }
	  }
	  if ((i % 7) == 2) {
	    if (thrdata[i].stat != CSA_SUCCESS) {
	      if (thrdata[i].stat != CSA_X_DT_E_ENTRY_NOT_FOUND) {
		sprintf(message, "ERROR: thread %d returned status of %d\n", i, thrdata[i].stat);
		end_test(message);
		break;
	      }
	    }
	    else {
	      if (verify3(i))
		break;
	    }
	  }
	  if ((i % 7) == 3) {
	    if (thrdata[i].stat != CSA_SUCCESS) {
	      if (thrdata[i].stat != CSA_X_DT_E_ENTRY_NOT_FOUND) {
		sprintf(message, "ERROR: thread %d returned status of %d\n", i, thrdata[i].stat);
		end_test(message);
		break;
	      }
	    }
	    else {
	      if (verify4(i))
		break;
	    }
	  }
	  if ((i % 7) == 4) {
	    if (verify5(i))
	      break;
	  }
	  if ((i % 7) == 5) {
	    if (thrdata[i].stat != access_ok_4) {
	      if (thrdata[i].stat != access_other_4) {
		sprintf(message, "ERROR: thread %d returned status of %d\n", i, thrdata[i].stat);
		end_test(message);
		break;
	      }
	    }
	    else {
	      if (verify6(i))
		break;
	    }
	  }
	  if ((i % 7) == 6) {
	    if (thrdata[i].stat != access_ok_4) {
	      if (thrdata[i].stat != access_other_4) {
		sprintf(message, "ERROR: thread %d returned status of %d\n", i, thrdata[i].stat);
		end_test(message);
		break;
	      }
	    }
	    else {
	      if (verify7(i))
		break;
	    }
	  }
	}

	clean_up_calendars();

	if (passed)
	  printf("Entry Test: TEST PASSED\n\n");
}
