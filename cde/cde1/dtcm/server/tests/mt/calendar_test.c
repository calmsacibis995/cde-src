/*
 * CDE Test Program
 * 
 * Calendar Test - calendar_test
 *
 * Test1 - Verify that cms_create_calendar() is thread safe when multiple
 * 	   threads add the same or different calendars at the same time.
 *	 - For each set of three calendars, check that only 1 thread returns
 *         CSA_SUCCESS and all other threads return CSA_E_CALENDAR_EXISTS.
 * Test2 - Verify that cms_open_calendar() is thread safe when multiple 
 *	   threads open the same or different calendars at the same time.
 *	 - Check to make sure all threads succeed.
 * Test3 - Verify that cms_remove_calendar() is thread safe when multiple 
 *         threads try to delete the same calendar and multiple threads delete 
 *         different calendars.
 *	 - For each set of three calendars, check that only 1 thread returns
 *         CSA_SUCCESS and all other threads return CSA_E_CALENDAR_NOT_EXIST.
 * Test4 - Verify that _DtCm_rtable_create() is thread safe when multiple 
 *         threads add the same or different calendars at the same time. 
 *	 - For each set of three calendars, check that only 1 thread returns
 *         CSA_SUCCESS and all other threads return CSA_E_CALENDAR_EXISTS.
 * Test5 - Verify that cms_create_calendar(), cms_open_calendar(), 
 *         cms_remove_calendar(), and _DtCm_rtable_create() are thread safe 
 *         when all four are called on the same calendars.
 *       - Check that create returns CSA_SUCCESS or CSA_E_CALENDAR_EXISTS,
 *         open returns CSA_SUCCESS or CSA_E_CALENDAR_NOT_EXIST, and remove 
 *         returns CSA_SUCCESS or CSA_E_CALENDAR_NOT_EXIST.
 *
 */

#include "cm.h"
#include "rtable4.h"
#include "mtcommon.h"
#include "common.h"

#include <sys/utsname.h>

#define NUM_OF_THREADS	        24             /* Must be under 300 */

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
end_test()
{
  clean_up_calendars();
  printf("Calendar Test: TEST FAILED\n\n");
  exit(1);
}

static void *
API_test1(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	cms_create_args          argument;
	CSA_return_code          result;

	argument.cal = td->cal_addr;
	argument.char_set = NULL;
	argument.pid = td->pid;
	argument.num_attrs = 0;
	argument.attrs = NULL;

	cms_create_calendar_5_svc(&argument, &result, &rqstp);

	td->stat = result;

	/* clean up */ 
	mttest_mutex_lock(&data_lock);
	thr_count++;
	if (thr_count == NUM_OF_THREADS) {
		mttest_mutex_lock(&threads_lock);
		mttest_cond_signal(&cond_done);
		mttest_mutex_unlock(&threads_lock);
	}
	mttest_mutex_unlock(&data_lock);
}

static void *
API_test2(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	cms_open_args            argument;
	cms_open_res             result;

	argument.cal = td->cal_addr;
	argument.pid = td->pid;

	cms_open_calendar_5_svc(&argument, &result, &rqstp);

	td->stat = result.stat;

	/* clean up */ 
	mttest_mutex_lock(&data_lock);
	thr_count++;
	if (thr_count == NUM_OF_THREADS) {
		mttest_mutex_lock(&threads_lock);
		mttest_cond_signal(&cond_done);
		mttest_mutex_unlock(&threads_lock);
	}
	mttest_mutex_unlock(&data_lock);
}

static void *
API_test3(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	cms_remove_args          argument;
	CSA_return_code          result;

	argument.cal = td->cal_addr;
	argument.pid = td->pid;

	cms_remove_calendar_5_svc(&argument, &result, &rqstp);

	td->stat = result;

	/* clean up */ 
	mttest_mutex_lock(&data_lock);
	thr_count++;
	if (thr_count == NUM_OF_THREADS) {
		mttest_mutex_lock(&threads_lock);
		mttest_cond_signal(&cond_done);
		mttest_mutex_unlock(&threads_lock);
	}
	mttest_mutex_unlock(&data_lock);
}

static void *
API_test4(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	Table_Op_Args_4          argument;
	Table_Status_4           result;

	argument.target = td->cal_addr;

	_DtCm_rtable_create_4_svc(&argument, &result, &rqstp);

	td->stat = result;

	/* clean up */ 
	mttest_mutex_lock(&data_lock);
	thr_count++;
	if (thr_count == NUM_OF_THREADS) {
		mttest_mutex_lock(&threads_lock);
		mttest_cond_signal(&cond_done);
		mttest_mutex_unlock(&threads_lock);
	}
	mttest_mutex_unlock(&data_lock);
}

static void *
API_test5(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
        int                      type;

	type = td->index % 4;

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
	}
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

	printf("Calendar Test: TEST BEGUN\n");

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

	/* Assumes under 300 threads. */
	/* Set up calendar names. */
	for(i = 0; i < (NUM_OF_THREADS/3+1); i++) {
	  if (i < 10)
	    addr[i] = (CSA_string)calloc(1, strlen(Csa_host)+12);
	  else
	    addr[i] = (CSA_string)calloc(1, strlen(Csa_host)+13);
	  sprintf(addr[i], "MtTestCal%d@%s", i, Csa_host);
	}

	clean_up_calendars();

	/* set up per thread data */
	for (i = 0; i < NUM_OF_THREADS; i++) {
	  thrdata[i].stat = -1;
	  thrdata[i].cal_addr = addr[i/3];
	  thrdata[i].pid = pid;
	  thrdata[i].index = i;
	}

	printf("Calendar Test: TEST #1\n");

	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test1, thrdata);

	flag = 0;
	for (i = 0; i < NUM_OF_THREADS; i++) {
	  if (thrdata[i].stat == CSA_SUCCESS) {
	    if (flag) {
	      printf("ERROR: two threads created the same calendar\n");
	      end_test();
	    } else {
	      flag = 1;
	    }
	  } else if (thrdata[i].stat != CSA_E_CALENDAR_EXISTS) {
	    printf("ERROR: thread %d returned status of %d\n", i, 
		   thrdata[i].stat);
	    end_test();
	  }
	  if ((i % 3) == 2) {
	    if (!flag) {
	      printf("ERROR: no threads created calendar %d\n", i/3);
	      end_test();
	    }
	    flag = 0;
	  }
	}

	printf("Calendar Test: TEST #2\n");

	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test2, thrdata);

	for (i = 0; i < NUM_OF_THREADS; i++) {
	  if (thrdata[i].stat != CSA_SUCCESS) {
	    printf("ERROR: thread %d returned status of %d\n", i, 
		   thrdata[i].stat);
	    end_test();
	  }
	}

	printf("Calendar Test: TEST #3\n");

	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test3, thrdata);

	flag = 0;
	for (i = 0; i < NUM_OF_THREADS; i++) {
	  if (thrdata[i].stat == CSA_SUCCESS) {
	    if (flag) {
	      printf("ERROR: two threads created the same calendar\n");
	      end_test();
	    } else {
	      flag = 1;
	    }
	  } else if (thrdata[i].stat != CSA_E_CALENDAR_NOT_EXIST) {
	    printf("ERROR: thread %d returned status of %d\n", i, 
		   thrdata[i].stat);
	    end_test();
	  }
	  if ((i % 3) == 2) {
	    if (!flag) {
	      printf("ERROR: no threads removed calendar %d\n", i/3);
	      end_test();
	    }
	    flag = 0;
	  }
	}

	printf("Calendar Test: TEST #4\n");

	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test4, thrdata);

	flag = 0;
	for (i = 0; i < NUM_OF_THREADS; i++) {
	  if (thrdata[i].stat == ok_4) {
	    if (flag) {
	      printf("ERROR: two threads created the same calendar\n");
	      end_test();
	    } else {
	      flag = 1;
	    }
	  } else if (thrdata[i].stat != tbl_exist_4) {
	    printf("ERROR: thread %d returned status of %d\n", i, 
		   thrdata[i].stat);
	    end_test();
	  }
	  if ((i % 3) == 2) {
	    if (!flag) {
	      printf("ERROR: no threads created calendar %d\n", i/3);
	      end_test();
	    }
	    flag = 0;
	  }
	}

	clean_up_calendars();

	printf("Calendar Test: TEST #5\n");

	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test5, thrdata);

	for (i = 0; i < NUM_OF_THREADS; i++) {
	  if ((i % 4) == 0) {
	    if (thrdata[i].stat != CSA_SUCCESS && 
		thrdata[i].stat != CSA_E_CALENDAR_EXISTS) {
	      printf("ERROR: create thread %d returned status of %d\n", i, 
		     thrdata[i].stat);
	      end_test();
	    }
	  }
	  if ((i % 4) == 1) {
	    if (thrdata[i].stat != CSA_SUCCESS && 
		thrdata[i].stat != CSA_E_CALENDAR_NOT_EXIST) {
	      printf("ERROR: open thread %d returned status of %d\n", i, 
		     thrdata[i].stat);
	      end_test();
	    }
	  }
	  if ((i % 4) == 2) {
	    if (thrdata[i].stat != CSA_SUCCESS && 
		thrdata[i].stat != CSA_E_CALENDAR_NOT_EXIST) {
	      printf("ERROR: remove thread %d returned status of %d\n", i, 
		     thrdata[i].stat);
	      end_test();
	    }
	  }
	  if ((i % 4) == 3) {
	    if (thrdata[i].stat != ok_4 && 
		thrdata[i].stat != tbl_exist_4) {
	      printf("ERROR: create thread %d returned status of %d\n", i, 
		     thrdata[i].stat);
	      end_test();
	    }
	  }
	}

	clean_up_calendars();

	printf("Calendar Test: TEST PASSED\n\n");

}
