/*
 * @(#)write.c	1.3 97/05/19
 *
 * Contents compleatly rewritten for Solaris.
 */
#include "os_sv5.h"

ssize_t
safe_write(const int fd, const void * buf, const size_t nBytes)
{
  const char	*	ptr = (const char *)buf;
  size_t		toGo = nBytes;
  ssize_t		results = -1;
  size_t		bytesWritten;
  size_t		totalBytesWritten = 0;
  boolean_t		error = _B_FALSE;

  /* do the output */
  do {
    if ((bytesWritten = write(fd, ptr, toGo)) > 0) {
      totalBytesWritten += bytesWritten;
      if (bytesWritten < toGo) {
	toGo -= bytesWritten;
	ptr += bytesWritten;
	continue;
      }
      break;	/* DONE */
    }
	
    /* Maybe an error */
    switch (errno) {

    case EINTR:
      if (bytesWritten > -1) {
	toGo -= bytesWritten;
	ptr += bytesWritten;
	totalBytesWritten += bytesWritten;
      }
      /*FALLTHRU*/
    case 0:
      continue;	/* Try again */
      

    default:		/* Some kind of error */
      error = _B_TRUE;
      results = -1;
      break;
      
    }
  } while(!error && totalBytesWritten < nBytes);

  /* If there was not an error and we wrote all of the bytes -> success */
  if (!error && totalBytesWritten == nBytes) {
    results = nBytes;
  }
  return(results);
}

