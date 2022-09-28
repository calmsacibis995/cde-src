/* $XConsortium: SmHftRing.c /main/cde1_maint/2 1995/08/30 16:27:42 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*************************************<+>*************************************
 *****************************************************************************
 **
 **  File:        SmHftRing.c
 **
 **  Project:     DT Session Manager (dtsession)
 **
 **  Description:
 **  -----------
 **  This file contains the AIX 3.2.x specific code required to enable and
 **  disable the HFT ring. This is used for session locking.
 **
 *****************************************************************************
 *************************************<+>*************************************/

#include <stdio.h>
#include <sys/hft.h>
#include <fcntl.h>
#include "Sm.h"

/*
 * Local functions.
 */

#ifdef _NO_PROTO
static int open_hft_file();
static void close_hft_file();
static int get_hft_ring();
static int process_hft();
static int scan_hft_ring();
static int my_strpos();
static int is_local_display();
#else
static int open_hft_file(char *);
static void close_hft_file(int);
static int get_hft_ring(void);
static int process_hft(int, unsigned int);
static int scan_hft_ring(int);
static int my_strpos(char *, char);
static int is_local_display(void);
#endif

#define HFTRFILE      "/dev/hft/0"
#define HFQSTAT       hftqstat
#define HFSMGRCMD     hftsmgrcmd
#define HFQEIO        HFQERROR
#define HFCSMGR       HFTCSMGR
#define HFQSMGR       HFTQSMGR
#define HFVTSTATUS    (HFVTACTIVE | HFVTNOHOTKEY)
#define HF_VTIODN     hf_vtid

/*
 * Global variables.
 */
int                   hft_file;
struct HFQSTAT        hft_ring;
extern int            errno;

static int
#ifdef _NO_PROTO
open_hft_file(hft_file_name)
char *hft_file_name;
#else
open_hft_file(char *hft_file_name)
#endif
{
  return(open(hft_file_name, O_RDONLY));
}

static void
#ifdef _NO_PROTO
close_hft_file(hft_file)
int hft_file;
#else
close_hft_file(int hft_file)
#endif
{
  close(hft_file); 
}

static int
#ifdef _NO_PROTO
get_hft_ring()
#else
get_hft_ring(void)
#endif
{
  int rc = -1;
  struct hfbuf hft_buffer;

  hft_buffer.hf_bufp = (char *)&hft_ring;
  hft_buffer.hf_buflen = sizeof(hft_ring);


 /*
  * Open the hft queue manager.
  */ 
  if ((hft_file = open_hft_file(HFTRFILE)) >= 0)
  {
    rc = ioctl(hft_file, HFQSMGR, &hft_buffer);
    close_hft_file(hft_file);
  }

  return(rc);
}

static int
#ifdef _NO_PROTO
process_hft(hft_action, hft_vtid)
int hft_action;
unsigned int hft_vtid;
#else
process_hft(int hft_action, unsigned int hft_vtid)
#endif
{
  struct HFSMGRCMD hft_manager;
  char hft_file_name[20];
  int rc = -1;

  hft_manager.hf_cmd = hft_action;
  hft_manager.hf_vtid = hft_vtid;

  sprintf(hft_file_name, "/dev/hft/%u", hft_vtid);
  if ((hft_file = open_hft_file(hft_file_name)) >= 0)
  {
    rc = ioctl(hft_file, HFCSMGR, &hft_manager);
    close_hft_file(hft_file);  
  }

  return(rc);
}

static int
#ifdef _NO_PROTO
scan_hft_ring(hft_action)
int hft_action;
#else
scan_hft_ring(int hft_action)
#endif
{
  int hft;
  int rc = 0;

  if (get_hft_ring() != 0)
  {
    return(-1);
  }

  for (hft = 0; hft < hft_ring.hf_numvts; hft++)
  {
    if (!(hft_ring.hf_vtinfo[hft].hf_vtstate & HFVTSTATUS))
    {
      if (process_hft(hft_action, hft_ring.hf_vtinfo[hft].HF_VTIODN) != 0)
      {
        return(-1);
      }
    }
  }  
  return(0);
}

static int
#ifdef _NO_PROTO
my_strpos(s, c)
char *s;
char c;
#else
my_strpos(char *s, char c)
#endif
{
  char *t;

  return((t=strchr(s,c)) == NULL ? -1 : t - s);
}

static int 
#ifdef _NO_PROTO
is_local_display()
#else
is_local_display(void)
#endif
{
  static int is_local = -1; /* display status */
  char *dpyname; /* display name */
  int n; /* position of colon char */
  char hostname[MAXHOSTNAMELEN]; /* host name */

  if (is_local == -1)
  {
    is_local =
      (((dpyname = getenv("DISPLAY")) != NULL) &&  /* name not null */
       ((n = my_strpos(dpyname, ':')) != -1) && /* name has a colon */
       ((n == 0) || /* name is :n */
        (strncmp(dpyname, "unix", n) == 0) || /* name is unix:n */
        (strncmp(dpyname, "localhost", n) == 0) || /* name is localhost:n */
        ((gethostname(hostname, MAXHOSTNAMELEN) == 0) && /* hostname ok */
         (strncmp(dpyname, hostname, n) == 0)))); /* name == hostname */
  }
   
  return(is_local);
}

void
#ifdef _NO_PROTO
AixEnableHftRing(enable)
int enable;
#else
AixEnableHftRing(int enable)
#endif
{
  if (is_local_display())
  {
    scan_hft_ring(enable ? SMUNHIDE : SMHIDE);
  }
}
