/*
 * CDE Test Program
 * 
 * register_test
 * 
 * Test1 - Verify that cms_register() is thread safe when multiple threads
 *         register the same and different callbacks.
 *       - Check that each thread has the callback registered.  Use 
 *         cms_unregister to verify.
 * Test2 - Verify that _DtCm_register_callback() is thread safe when multiple 
 *         threads register the same and different callbacks.
 *       - Check that each thread has the callback registered.  Use 
 *         _DtCm_deregister_callback to verify.
 * Test3 - Verify that cms_unregister() is thread safe when multiple threads 
 *         unregister the same and different callbacks.
 *	 - Check that only 1 thread for each client gets CSA_SUCCESS  All 
 *         others should get CSA_E_CALLBACK_NOT_REGISTERED.
 * Test4 - Verify that _DtCm_deregister_callback() is thread safe  when 
 *         multiple threads unregister the same and different callbacks.
 *	 - Check that only 1 thread for each client gets CSA_SUCCESS. All 
 *         others should get CSA_E_CALLBACK_NOT_REGISTERED.
 * Test5 - Verify that cms_register(), _DtCm_register_callback(), 
 *         cms_unregister(), and _DtCm_deregister_callback() are thread safe
 *         when called concurrently.
 *       - Check that all return valid results.
 *
 */

#include "cm.h"
#include "rtable4.h"
#include "mtcommon.h"
#include "common.h"

#include <sys/utsname.h>

#define NUM_OF_THREADS	        24         /* Must be under 300 */

static	ThrData	thrdata[NUM_OF_THREADS];
static	int	thr_count = 0;
static  pthread_mutex_t           data_lock;
/* Transport */
static  char *  Csa_host;
static  struct __svcxprt          rq_xprt;
static  struct svc_req            rqstp0;
static  struct authunix_parms     ucred0;
static  struct svc_req            rqstp1;
static  struct authunix_parms     ucred1;
static  struct svc_req            rqstp2;
static  struct authunix_parms     ucred2;
static  struct svc_req            rqstp3;
static  struct authunix_parms     ucred3;
static  char    netid[4];    
static  char    buf[16];            

extern  char *  pgname;

/* Foward declaration of API tests. */
static void * API_test1(void *client_data);
static void * API_test2(void *client_data);
static void * API_test3(void *client_data);
static void * API_test4(void *client_data);
static void * API_test5(void *client_data);
static void * verify1(int thread);
static void * verify2(int thread);
static void * verify3(int thread);
static void * verify4(int thread);
static void * verify5(int thread);

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
    Clean_Up_Calendar(name, thrdata[i].rqstp);
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
	cms_register_args        argument;
	CSA_return_code          result;
	struct svc_req *         rqstp;

	argument.cal = td->cal_addr;
	argument.update_type = 32; /* Any number less than 40 is valid. */
	argument.prognum = 4;
	argument.versnum = 5;
	argument.procnum = 6;
	argument.pid = td->pid;
	
	rqstp = (struct svc_req *)td->rqstp;

	cms_register_5_svc(&argument, &result, rqstp);

	td->stat = result;

	clean_up_threads();
}

static void *
API_test2(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	Registration_4           argument;
	Registration_Status_4    result;
	struct svc_req *         rqstp;

	argument.target = td->cal_addr;
	argument.prognum = 4;
	argument.versnum = 5;
	argument.procnum = 6;
	argument.next = NULL;
	argument.pid = td->pid;
	
	rqstp = (struct svc_req *)td->rqstp;

	_DtCm_register_callback_4_svc(&argument, &result, rqstp);

	td->stat = result;

	clean_up_threads();
}

static void *
API_test3(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	cms_register_args        argument;
	CSA_return_code          result;
	struct svc_req *         rqstp;

	argument.cal = td->cal_addr;
	argument.update_type = 32; /* Any number less than 40 is valid. */
	argument.prognum = 4;
	argument.versnum = 5;
	argument.procnum = 6;
	argument.pid = td->pid;
	
	rqstp = (struct svc_req *)td->rqstp;

	cms_unregister_5_svc(&argument, &result, rqstp);

	td->stat = result;

	clean_up_threads();
}

static void *
API_test4(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	Registration_4           argument;
	Registration_Status_4    result;
	struct svc_req *         rqstp;

	argument.target = td->cal_addr;
	argument.prognum = 4;
	argument.versnum = 5;
	argument.procnum = 6;
	argument.next = NULL;
	argument.pid = td->pid;
	
	rqstp = (struct svc_req *)td->rqstp;

	_DtCm_deregister_callback_4_svc(&argument, &result, rqstp);

	td->stat = result;

	clean_up_threads();
}

static void *
API_test5(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	int                      type = td->index % 4;

	switch(type) {
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
	}
}

static void *
verify1(int thread)
{
    if (thrdata[thread].stat != CSA_SUCCESS) {
      printf("ERROR: thread %d returned status of %d\n", thread, 
	     thrdata[thread].stat);
      printf("Register Test: TEST FAILED\n\n");
      end_test();
    }
    API_test3(&thrdata[thread]);
    verify3(thread);
}

static void *
verify2(int thread)
{
    if (thrdata[thread].stat != registered_4) {
      printf("ERROR: thread %d returned status of %d\n", thread, 
	     thrdata[thread].stat);
      printf("Register Test: TEST FAILED\n\n");
      end_test();
    }
    API_test4(&thrdata[thread]);
    verify4(thread);
}

static void *
verify3(int thread)
{
    if (thrdata[thread].stat != CSA_SUCCESS) {
      printf("ERROR: thread %d returned status of %d\n", thread, 
	     thrdata[thread].stat);
      printf("Register Test: TEST FAILED\n\n");
      end_test();
    }
}

static void *
verify4(int thread)
{
    if (thrdata[thread].stat != deregistered_4) {
      printf("ERROR: thread %d returned status of %d\n", thread, 
	     thrdata[thread].stat);
      printf("Register Test: TEST FAILED\n\n");
      end_test();
    }
}

static void *
verify5(int thread)
{
	int       type = thread % 4;

	switch(type) {
	case 0:
	  verify1(thread);
	  break;
	case 1:
	  verify2(thread);
	  break;
	case 2:
	  if ((thrdata[thread].stat != CSA_SUCCESS) &&
	      (thrdata[thread].stat != CSA_E_CALLBACK_NOT_REGISTERED)) {
	    printf("ERROR: thread %d returned status of %d\n", thread, 
		   thrdata[thread].stat);
	    printf("Register Test: TEST FAILED\n\n");
	    end_test();
	  }
	  break;
	case 3:
	  if ((thrdata[thread].stat != deregistered_4) &&
	      (thrdata[thread].stat != failed_4)) {
	    printf("ERROR: thread %d returned status of %d\n", thread, 
		   thrdata[thread].stat);
	    printf("Register Test: TEST FAILED\n\n");
	    end_test();
	  }
	  break;
	}
}

void
main(int argc, char **argv)
{
        CSA_string          addr[NUM_OF_THREADS/3+1];
	int                 i;
	CSA_calendar_user * ptr;
	struct utsname      sname;
	char *              dir = _DtCMS_DEFAULT_DIR;
	static  pid_t       pid;

	printf("Register Test: TEST BEGUN\n");

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
	ucred0.aup_uid = 1000;
	ucred0.aup_machname = Csa_host;
	ucred1.aup_uid = 1001;
	ucred1.aup_machname = Csa_host;	
	ucred2.aup_uid = 1002;
	ucred2.aup_machname = Csa_host;
	ucred3.aup_uid = 1003;
	ucred3.aup_machname = Csa_host;
	rq_xprt.xp_rtaddr.maxlen = 16;
	rq_xprt.xp_rtaddr.len = 16;
	rq_xprt.xp_rtaddr.buf = buf;
	rq_xprt.xp_netid = netid;
	rqstp0.rq_cred.oa_flavor = AUTH_UNIX;
	rqstp0.rq_clntcred = (caddr_t)&ucred0;
	rqstp0.rq_xprt = &rq_xprt;
	rqstp1.rq_cred.oa_flavor = AUTH_UNIX;
	rqstp1.rq_clntcred = (caddr_t)&ucred1;
	rqstp1.rq_xprt = &rq_xprt;
	rqstp2.rq_cred.oa_flavor = AUTH_UNIX;
	rqstp2.rq_clntcred = (caddr_t)&ucred2;
	rqstp2.rq_xprt = &rq_xprt;
	rqstp3.rq_cred.oa_flavor = AUTH_UNIX;
	rqstp3.rq_clntcred = (caddr_t)&ucred3;
	rqstp3.rq_xprt = &rq_xprt;

	/* Set up calendar names. */
	for(i = 0; i < (NUM_OF_THREADS/3+1); i++) {
	  if (i < 10)
	    addr[i] = (CSA_string)calloc(1, strlen(Csa_host)+12);
	  else
	    addr[i] = (CSA_string)calloc(1, strlen(Csa_host)+13);
	  sprintf(addr[i], "MtTestCal%d@%s", i, Csa_host);
	}

	/* Set up per thread data. */
	for (i = 0; i < NUM_OF_THREADS; i++) {
		thrdata[i].stat = -1;
		thrdata[i].cal_addr = addr[i/3];
		thrdata[i].pid = pid;
		thrdata[i].index = i;
		if ((i % 4) == 0)
		  thrdata[i].rqstp = &rqstp0;
		if ((i % 4) == 1)
		  thrdata[i].rqstp = &rqstp1;
		if ((i % 4) == 2)
		  thrdata[i].rqstp = &rqstp2;
		if ((i % 4) == 3)
		  thrdata[i].rqstp = &rqstp3;
	}

	clean_up_calendars();

	/* Create calendars. */
	for(i = 0; i < (NUM_OF_THREADS/3+1); i++) {
	  Create_Calendar(addr[i], thrdata[i].rqstp);
	}

	printf("Register Test: TEST #1\n");

	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test1, thrdata);
	for (i=0;i<NUM_OF_THREADS;i++) {
	  verify1(i);
	}

	printf("Register Test: TEST #2\n");

	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test2, thrdata);
	for (i=0;i<NUM_OF_THREADS;i++) {
	  verify2(i);
	  API_test1(&thrdata[i]); /* Register clients for next test. */
	}

	printf("Register Test: TEST #3\n");

	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test3, thrdata);
	for (i=0;i<NUM_OF_THREADS;i++) {
	  verify3(i);
	  API_test1(&thrdata[i]); /* Register clients for next test. */
	}

	printf("Register Test: TEST #4\n");

	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test4, thrdata);
	for (i=0;i<NUM_OF_THREADS;i++) {
	  verify4(i);
	}

	printf("Register Test: TEST #5\n");

	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test5, thrdata);
	for (i=0;i<NUM_OF_THREADS;i++) {
	  verify5(i);
	}

	clean_up_calendars();

	printf("Register Test: TEST PASSED\n\n");

}
