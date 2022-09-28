/*
 * CDE Test Program
 * 
 * attribute_test
 * 
 * Test1 - Verify that cms_set_calendar_attr() and are thread safe when 
 *         multiple threads update different attributes with different values 
 *         in both the same and different calendars.
 *       - Check that the updated attributes match one of the sets of values 
 *         used to do the update.  Use cms_get_calendar_attr() to verify.
 * Test2 - Verify that _DtCm_rtable_set_access() and are thread safe when 
 *         multiple threads update different attributes with different values 
 *         in both the same and different calendars.
 *       - Check that the updated attributes match one of the sets of values 
 *         used to do the update.  Use _DtCm_rtable_get_access() to verify.
 * Test3 - Verify that cms_set_calendar_attr() and _DtCm_rtable_set_access() 
 *         are thread safe when multiple threads update different attributes
 *         with different values in both the same and different calendars.
 *       - Check that the updated attributes match one of the sets of values 
 *         used to do the update.  Use cms_get_calendar_attr() and 
 *         _DtCm_rtable_get_access() to verify.
 * Test4 - Verify that cms_get_calendar_attr() and are thread safe when 
 *         multiple threads call them with different attributes in both the 
 *         same and different calendars.
 *       - Check that each thread gets back the correct value for the 
 *         attribute it requested.
 * Test5 - Verify that _DtCm_rtable_get_access() and are thread safe when 
 *         multiple threads call them with different attributes in both the 
 *         same and different calendars.
 *       - Check that each thread gets back the correct value for the 
 *         attribute it requested.
 * Test6 - Verify that cms_get_calendar_attr() and _DtCm_rtable_get_access() 
 *         are thread safe when multiple threads call them with different 
 *         attributes in both the same and different calendars.
 *       - Check that each thread gets back the correct value for the 
 *         attribute it requested.
 * Test7 - Verify that cms_set_calendar_attr(), _DtCm_rtable_set_access(), 
 *         cms_get_calendar_attr(), _DtCm_rtable_get_access() are thread safe
 *         when all are called on the same attribute of the same calendar.
 *       - Check that the "get" results are consistent with a set of values 
 *         passed to "set."
 *
 */

#include "cm.h"
#include "rtable4.h"
#include "mtcommon.h"
#include "common.h"

#include <sys/utsname.h>

#define NUM_OF_THREADS	        12        /* Must be under 300 */

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

extern  char *  pgname;

/* Foward declaration of API tests. */
static void * API_test1(void *client_data);
static void * API_test2(void *client_data);
static void * API_test3(void *client_data);
static void * API_test4(void *client_data);
static void * API_test5(void *client_data);
static void * API_test6(void *client_data);
static void * API_test7(void *client_data);
static void * verify1(int thread);
static void * verify2(int thread);
static void * verify3(int thread);
static void * verify4(int thread);

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
end_test()
{
  clean_up_calendars();
  exit(1);
}

static void *
API_test1(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	cms_set_cal_attr_args    argument;
	CSA_return_code          result;
	cms_attribute            attrs[2];

	argument.cal = td->cal_addr;
	argument.pid = td->pid;
	argument.num_attrs = 2;
	attrs[0].name.num = td->cms_attrs[0].name.num;
	attrs[0].name.name = strdup(td->cms_attrs[0].name.name);
	attrs[0].value = td->cms_attrs[0].value;
	attrs[1].name.num = td->cms_attrs[1].name.num;
	attrs[1].name.name = strdup(td->cms_attrs[1].name.name);
	attrs[1].value = td->cms_attrs[1].value;
	argument.attrs = attrs;

	cms_set_calendar_attr_5_svc(&argument, &result, &rqstp);

	td->stat = result;

	clean_up_threads();
}

static void *
API_test2(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	Access_Args_4            argument;
	Access_Status_4          result;

	argument.target = td->cal_addr;
	argument.access_list = td->access;

	_DtCm_rtable_set_access_4_svc(&argument, &result, &rqstp);

	td->stat = result;

	clean_up_threads();
}

static void *
API_test3(void *client_data)
{

	ThrData *                td = (ThrData *)client_data;

        if ((td->index % 3) == 1)
	  API_test2(client_data);
	else
	  API_test1(client_data);
}

static void *
API_test4(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	cms_get_cal_attr_args    argument;
	cms_get_cal_attr_res     result;

	argument.cal = td->cal_addr;
	argument.num_names = 2;
	argument.names = (cms_attr_name *)calloc(1, sizeof(cms_attr_name) * 2);
	argument.names[0].num = 0;
	argument.names[0].name = strdup("FIRST_NEW_ATTR");
	argument.names[1].num = 0;
	argument.names[1].name = strdup("SECOND_NEW_ATTR");	

	cms_get_calendar_attr_5_svc(&argument, &result, &rqstp);

	td->stat = result.stat;
	td->num_attrs = result.num_attrs;
	td->cms_new_attrs = result.attrs;

	clean_up_threads();
}

static void *
API_test5(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	Access_Args_4            argument;
	Access_Args_4            result;

	argument.target = td->cal_addr;

	_DtCm_rtable_get_access_4_svc(&argument, &result, &rqstp);

	td->access = result.access_list;

	clean_up_threads();
}

static void *
API_test6(void *client_data)
{

	ThrData *                td = (ThrData *)client_data;

        if ((td->index % 3) == 1)
	  API_test3(client_data);
	else
	  API_test4(client_data);
}

static void *
API_test7(void *client_data)
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
	  API_test4(client_data);
	  break;
	case 3:
	  API_test5(client_data);
	  break;
	case 4:
	  API_test4(client_data);
	  break;
	}
}

static void *
verify1(int thread)
{
    if (thrdata[thread].stat != CSA_SUCCESS) {
      printf("ERROR: thread %d returned status of %d\n", thread, 
	     thrdata[thread].stat);
      printf("Attribute Test: TEST FAILED\n\n");
      end_test();
    }
    API_test4(&thrdata[thread]);
    verify3(thread);
}

static void *
verify2(int thread)
{
    if (thrdata[thread].stat != ok_4) {
      printf("ERROR: thread %d returned status of %d\n", thread, 
	     thrdata[thread].stat);
      printf("Attribute Test: TEST FAILED\n\n");
      end_test();
    }
    API_test5(&thrdata[thread]);
    verify4(thread);
}

static void *
verify3(int thread)
{
    cms_attribute *   attrs;
    char *            value1;
    char *            value2;
    char *            value3;
    char *            value4;

    value1 = strdup("first value 1");
    value2 = strdup("second value 1");
    value3 = strdup("first value 2");
    value4 = strdup("second value 2");

    if (thrdata[thread].stat != CSA_SUCCESS) {
      printf("ERROR: thread %d returned status of %d\n", thread, 
	     thrdata[thread].stat);
      printf("Attribute Test: TEST FAILED\n\n");
      end_test();
    }
    if (thrdata[thread].num_attrs != 2) {
      printf("ERROR: thread %d returned %d attributes instead of 2\n", thread, 
	     thrdata[thread].stat);
      printf("Attribute Test: TEST FAILED\n\n");
      end_test();
    }
    attrs = thrdata[thread].cms_new_attrs;
    if ((strcmp(attrs[0].name.name, "FIRST_NEW_ATTR")) ||
	(strcmp(attrs[1].name.name, "SECOND_NEW_ATTR"))) {
      printf("ERROR: thread %d returned wrong attributes: \n", thread);
      printf("Attribute Test: TEST FAILED\n\n");
      end_test();
    }
    if ((attrs[0].value->type != CSA_VALUE_STRING) ||
	((strcmp(attrs[0].value->item.string_value, value1)) &&
	 (strcmp(attrs[0].value->item.string_value, value3))) ||
	(attrs[1].value->type != CSA_VALUE_STRING) ||
	((strcmp(attrs[1].value->item.string_value, value2)) &&
	 (strcmp(attrs[1].value->item.string_value, value4)))) {
      printf("ERROR: thread %d returned wrong attribute value\n", thread);
      printf("Attribute Test: TEST FAILED\n\n");
      end_test();
    }
}

static void *
verify4(int thread)
{
    Access_Entry_4 *  access;

    access = thrdata[thread].access;
    if ((access->access_type != 0) ||
	(access->next->access_type != 0)) {
      printf("ERROR: thread %d returned wrong access type: both should be 0, but one was %d and the other was %d\n", thread, access->access_type, access->next->access_type);
      printf("Attribute Test: TEST FAILED\n\n");
      end_test();
    }
    if (((strcmp(access->who, "TEST_USER1")) &&
	 (strcmp(access->who, "TEST_USER3"))) ||
	((strcmp(access->next->who, "TEST_USER2")) &&
	 (strcmp(access->next->who, "TEST_USER4")))) {
      printf("ERROR: thread %d returned wrong access list\n", thread);
      printf("Attribute Test: TEST FAILED\n\n");
      end_test();
    }
}

void
main(int argc, char **argv)
{
        CSA_string          addr[NUM_OF_THREADS/3+1];
	int                 i, j;
	CSA_calendar_user * ptr;
	struct utsname      sname;
	char *              dir = _DtCMS_DEFAULT_DIR;
	static  pid_t       pid;
	cms_attribute *     attrs1;
	cms_attribute *     attrs2;
	Access_Entry_4 *    access1a;
	Access_Entry_4 *    access2a;
	Access_Entry_4 *    access1b;
	Access_Entry_4 *    access2b;

	printf("Attribute Test: TEST BEGUN\n");

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

	/* Create calendars. */
	for(i = 0; i < (NUM_OF_THREADS/3+1); i++) {
	  Create_Calendar(addr[i], &rqstp);
	}

	/* Set up test attributes. */
	attrs1 = (cms_attribute *)calloc(1, sizeof(cms_attribute) * 2);
	attrs1[0].name.num = 0;
	attrs1[0].name.name = strdup("FIRST_NEW_ATTR");
	attrs1[0].value = (cms_attribute_value *)calloc(1,
	                  sizeof(cms_attribute_value));
	attrs1[0].value->type = CSA_VALUE_STRING;
	attrs1[0].value->item.string_value = strdup("first value 1");
	attrs1[1].name.num = 0;
	attrs1[1].name.name = strdup("SECOND_NEW_ATTR");
	attrs1[1].value = (cms_attribute_value *)calloc(1,
			  sizeof(cms_attribute_value));
	attrs1[1].value->type = CSA_VALUE_STRING;
	attrs1[1].value->item.string_value = strdup("second value 1");
	attrs2 = (cms_attribute *)calloc(1, sizeof(cms_attribute) * 2);
	attrs2[0].name.num = 0;
	attrs2[0].name.name = strdup("FIRST_NEW_ATTR");
	attrs2[0].value = (cms_attribute_value *)calloc(1,
	                  sizeof(cms_attribute_value));
	attrs2[0].value->type = CSA_VALUE_STRING;
	attrs2[0].value->item.string_value = strdup("first value 2");
	attrs2[1].name.num = 0;
	attrs2[1].name.name = strdup("SECOND_NEW_ATTR");
	attrs2[1].value = (cms_attribute_value *)calloc(1,
			  sizeof(cms_attribute_value));
	attrs2[1].value->type = CSA_VALUE_STRING;
	attrs2[1].value->item.string_value = strdup("second value 2");
	access1a = (Access_Entry_4 *)calloc(1, sizeof(Access_Entry_4));
	access1b = (Access_Entry_4 *)calloc(1, sizeof(Access_Entry_4));
	access1a->who = strdup("TEST_USER1");
	access1a->access_type = 0;
	access1a->next = access1b;
	access1b->who = strdup("TEST_USER2");
	access1b->access_type = 0;
	access1b->next = NULL;
	access2a = (Access_Entry_4 *)calloc(1, sizeof(Access_Entry_4));
	access2b = (Access_Entry_4 *)calloc(1, sizeof(Access_Entry_4));
	access2a->who = strdup("TEST_USER3");
	access2a->access_type = 0;
	access2a->next = access2b;
	access2b->who = strdup("TEST_USER4");
	access2b->access_type = 0;
	access2b->next = NULL;

	/* Set up per thread data. */
	for (i = 0; i < NUM_OF_THREADS; i++) {
		thrdata[i].stat = -1;
		thrdata[i].cal_addr = addr[i/3];
		thrdata[i].pid = pid;
		thrdata[i].index = i;
		if ((i % 2) == 0) {
		  thrdata[i].cms_attrs = attrs1;
		  thrdata[i].access = access1a;
		}
		else {
		  thrdata[i].cms_attrs = attrs2;
		  thrdata[i].access = access2a;
		}
	}

	printf("Attribute Test: TEST #1\n");

	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test1, thrdata);
	for (i=0;i<NUM_OF_THREADS;i++) {
	  verify1(i);
	}

	printf("Attribute Test: TEST #2\n");

	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test2, thrdata);
	for (i=0;i<NUM_OF_THREADS;i++) {
	  verify2(i);
	}

	printf("Attribute Test: TEST #3\n");

	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test3, thrdata);
	for (i=0;i<NUM_OF_THREADS;i++) {
	  if ((i % 3) == 1)
	    verify2(i);
	  else
	    verify1(i);
	}

	printf("Attribute Test: TEST #4\n");

	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test4, thrdata);
	for (i=0;i<NUM_OF_THREADS;i++) {
	  verify3(i);
	}

	printf("Attribute Test: TEST #5\n");

	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test5, thrdata);
	for (i=0;i<NUM_OF_THREADS;i++) {
	  verify4(i);
	}

	printf("Attribute Test: TEST #6\n");

	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test6, thrdata);
	for (i=0;i<NUM_OF_THREADS;i++) {
	  if ((i % 3) == 1)
	    verify4(i);
	  else
	    verify3(i);
	}

	printf("Attribute Test: TEST #7\n");

	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test7, thrdata);
	for (i=0;i<NUM_OF_THREADS;i++) {
	  j = i % 5;
	  switch (j) {
	  case 0:
	    verify1(i);
	    break;
	  case 1:
	    verify2(i);
	    break;
	  case 2:
	    verify3(i);
	    break;
	  case 3:
	    verify4(i);
	    break;
	  case 4:
	    verify3(i);
	    break;
	  }
	}

	clean_up_calendars();

	printf("Attribute Test: TEST PASSED\n\n");

}
