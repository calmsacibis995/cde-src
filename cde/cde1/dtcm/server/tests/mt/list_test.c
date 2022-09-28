/*
 * CDE Test Program
 * 
 * list_test
 * 
 * Test1 - Verify that cms_list_calendars() is thread safe when called by 
 *         multiple threads.
 *	 - Check to make sure the right data is returned.
 *
 */

#include <cm.h>
#include <mtcommon.h>

#define NUM_OF_THREADS	        24

static	ThrData	thrdata[NUM_OF_THREADS];
static	int	thr_count = 0;
static  pthread_mutex_t data_lock;

static void *
API_test(void *client_data)
{
	ThrData *                td = (ThrData *)client_data;
	void *                   argument;
	cms_list_calendars_res   result;
	struct svc_req           rqstp;

	cms_list_calendars_5_svc(&argument, &result, &rqstp);

	td->stat = result.stat;
	td->num_names = result.num_names;
	td->cal_list = result.names;

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

void
main(int argc, char **argv)
{
        CSA_string          addr[1];
	int                 i, j;
	CSA_calendar_user * ptr;
	char *              dir = _DtCMS_DEFAULT_DIR;

	printf("List Calendars Test: TEST BEGUN\n");
	printf("List Calendars Test: TEST #1\n");

	chdir (dir);

	/* set up per thread data */
	for (i = 0; i < NUM_OF_THREADS; i++) {
		thrdata[i].stat = -1;
		thrdata[i].num_names = 0;
		thrdata[i].cal_list = NULL;
	}

	thr_count = 0;
	create_test_threads(NUM_OF_THREADS, API_test, thrdata);

	for (i = 0; i < NUM_OF_THREADS; i++) {
	  if (thrdata[i].stat != CSA_SUCCESS) {
	    printf("thread %d returned status of %d\n", i, thrdata[i].stat);
	    printf("List Calendars Test: TEST FAILED\n\n");
	  }
	  if (thrdata[i].num_names != thrdata[0].num_names) {
	    printf("thread %d returned %d calendars\n", i, 
		   thrdata[i].num_names);
	    printf("List Calendars Test: TEST FAILED\n\n");
	  }
	  for (j = 0; j < thrdata[i].num_names; j++) {
	    if (strcmp(thrdata[i].cal_list[j],thrdata[0].cal_list[j])) {
	      printf("thread %d returned different list\n", i);
	      printf("List Calendars Test: TEST FAILED\n\n");
	    }
	  }
	}

	printf("List Calendars Test: TEST PASSED\n\n");

}
