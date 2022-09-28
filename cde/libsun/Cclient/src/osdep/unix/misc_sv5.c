/*
 * Edit Log:
 *
 * [10-04-96] localtime_r implemented for MT-safe.
 * [10-21-96] Passing stream on both local_host_name
 * 	      and mylocalhost, as type void for now.
 */
#include "os_sv5.h"

#include <sys/types.h>
#include <utime.h>
#include <pthread.h>

#define HWIDLEN 40
static char		miscHwSerial[HWIDLEN];

/* If we are using PTHREADS, then this is the once lock for misc_once() */
#if (_POSIX_C_SOURCE >= 199506L)
static pthread_once_t   miscOnceVar = PTHREAD_ONCE_INIT;
#endif

static unsigned long loc_index();

/*
 * Initalize any misc global variables - once.
 * Callable from pthread_once().
 */
void
misc_once()
{
  unsigned long mimeSequenceNumber = loc_index();

  if (sysinfo(SI_HW_SERIAL, miscHwSerial, sizeof(miscHwSerial)-1) < 0) {
    sprintf(miscHwSerial, "%x", &mimeSequenceNumber);
  }
  return;
}

char *
local_host_name(void *stream )
{
  return(mylocalhost(stream));
}

/*
 * Tue Oct 26 hh:mm ZZZ YYYY
 */
void
rfc822_from_date (char *date)
{
  int 			zone;
  struct tm *		t;
  struct tm		res_tm;  	/* MT-safe */
  struct timeval 	tv;
  struct timezone 	tz;

  gettimeofday(&tv, &tz);		/* get time and timezone poop */
  t = localtime_r(&tv.tv_sec, &res_tm);	/* convert to individual items */
  tzset();			/* get timezone from TZ environment stuff */
  zone = -timezone/60;
				/* and output it */
  sprintf (date,"%s %s %2d %02d:%02d %s %4d",
	   days[t->tm_wday],
	   months[t->tm_mon],
	   t->tm_mday,
	   t->tm_hour,t->tm_min,
	   tzname[daylight ? t->tm_isdst : 0],
	   t->tm_year+1900);

  return;
}

/*
 * Return elaspsed time in milliseconds
 */
long
elapsed_ms(void)
{
  struct timeval 	tv;
  struct timezone 	tz;
  long 			ms;

  gettimeofday(&tv, &tz);

  ms = (tv.tv_sec * 1000) + (tv.tv_usec/1000);

  return(ms);
}

/*
 * Return a unique hw id
 */
char *
unique_hwid()
{
#if (_POSIX_C_SOURCE >= 199506L)
  pthread_once(&miscOnceVar, misc_once);
#else
  if (miscHwSerial[0] == '\0') { /* if Not yet initalized */
    misc_once();
  }
#endif
  return(miscHwSerial);
}

/*
 * We use our own local index for indexing
 * MIME boundaries. Just as good as a random
 * number. Just a fixed montone sequence that
 * wraps at the largest ulong.
 */
static unsigned long 
loc_index()
{
  static unsigned long 	indice = 0;
  indice += 1;

  return(indice);
}

/*
 *  Sets the mime boundary given a buffer:
 *  make cookie not in BASE64 or QUOTEPRINT
 *  MT-safe. Generates a string of at most 51 chars.
 */
void
set_mime_boundary(char *buf)
{
  unsigned long mimeSequenceNumber = loc_index();
  time_t 	t0 = time(0);

  unique_hwid();

  sprintf (buf, "-LA_F%sR-%luA-%lu=:%dNCE.cde_",
	   miscHwSerial,
	   mimeSequenceNumber,
	   t0,
	   getpid());

  return;
}
