/* dtrlpstat.c: Remote printer status listing client */

#include <stdio.h>
#include <string.h>
#include "dtrlpstat.h"

#include <sys/socket.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <netdb.h>

#ifdef NO_SPERROR
char RemoteLpstatErrorString[256];
#else
char *RemoteLpstatErrorString;
#endif

extern int errno;
static CLIENT *GetClient();
static dtlpstat_res *DtLplistJobs();

#ifdef NO_XDR_POINTER
bool_t
xdr_pointer(xdrs,objpp,obj_size,xdr_obj)
   register XDR *xdrs;
   char **objpp;
   u_int obj_size;
   xdrproc_t xdr_obj;
{
   bool_t more_data;

   more_data = (*objpp != NULL);
   if (! xdr_bool(xdrs,&more_data))
      return (FALSE);

   if (! more_data)
    {
      *objpp = NULL;
      return (TRUE);
    }
   return (xdr_reference(xdrs,objpp,obj_size,xdr_obj));
}
#endif

#ifdef NO_CLNT_CONTROL

#define clnt_control(cl,rq,in)
extern CLIENT *clnt_create();
void
xdr_free(proc, objp)
   xdrproc_t proc;
   char *objp;
{
   XDR x;

   x.x_op = XDR_FREE;
   (*proc)(&x, objp);
}

#endif

int
GetRemotePrintJobs(server, printer, return_job_list, return_n_jobs)
   char *server;
   char *printer;
   StatusLineList *return_job_list;
   int *return_n_jobs;
{
   static StatusLineList job_list = NULL;
   static int prev_n_jobs = 0;
   static dtlpstat_res *result = NULL;
   CLIENT *cl;
   int n_jobs;
   namelist nl;

#ifdef NO_SPERROR
   *RemoteLpstatErrorString = '\0';
#else
   RemoteLpstatErrorString = NULL;
#endif
   /* Get Client Handle */
   if (!(cl = GetClient(server)))
    {
      *return_job_list = NULL;
      *return_n_jobs = 0;
      return 0;
    }
	
   /* Free previous result */
   if (result)
      xdr_free(xdr_dtlpstat_res, result);

   /* Call the remote procedure "dtlpstat" on the server */
   result = DtLplistJobs(&printer, cl);
   if (result == NULL)
    {
     /*
      * An error occurred while calling the server. 
      * Print error message and die.
      */
#ifdef NO_SPERROR
      sprintf(RemoteLpstatErrorString, "Could not connect to %s", server);
#else
      RemoteLpstatErrorString = clnt_sperror(cl, server);
#endif
      *return_job_list = NULL;
      *return_n_jobs = 0;
      return 0;
    }

   /* Okay, we successfully called the remote procedure.  */
   if (result->errno != 0)
    {
     /* A remote system error occurred. Print error message and die.  */
      errno = result->errno;
      perror(printer);
      *return_job_list = NULL;
      *return_n_jobs = 0;
      return 0;
    }

   /* Successfuly got printer status.  Fill Structure.  */
   n_jobs = 0;
   for (nl = result->dtlpstat_res_u.list; nl != NULL; nl = nl->next)
    {
      if (n_jobs >= prev_n_jobs)
       {
         if (n_jobs == 0)
            job_list = (StatusLineList) malloc(sizeof(StatusLine));
	 else
            job_list = (StatusLineList) realloc(job_list, (n_jobs + 1) *
						sizeof(StatusLine));
	 job_list[n_jobs] = (StatusLine) malloc(sizeof(StatusLineStruct));
       }
      job_list[n_jobs]->line = nl->name;
      n_jobs++;
    }
   prev_n_jobs = prev_n_jobs > n_jobs ? prev_n_jobs : n_jobs; 
   *return_job_list = job_list;
   *return_n_jobs = n_jobs;
   return 1;
}

static dtlpstat_res *
DtLplistJobs(argp, clnt)
   nametype *argp;
   CLIENT *clnt;
{
   /* Default timeout can be changed using clnt_control() */
   static struct timeval TIMEOUT = { 25, 0 };
   static dtlpstat_res res;

   (void) memset((char *)&res, 0, sizeof(res));
   if (clnt_call(clnt, DTLPLISTJOBS, xdr_nametype, argp, xdr_dtlpstat_res,
		 &res, TIMEOUT) != RPC_SUCCESS)
      return (NULL);
   else
      return (&res);
}

typedef struct {
   char server[128];
   CLIENT *client;
   long last_access;
   int sock;
} ClientHandleCacheStruct, *ClientHandleCache, **ClientHandleCacheList;

#define MAX_CLIENT_CACHE 10

static int SortByAccessTime();
static CLIENT *ClntCreate();
static ClientHandleCacheList G_client_cache = NULL;
static int G_n_clients = 0;

int FreeRemotePrintJobs()
{
   int i;

   for (i = 0; i < G_n_clients; i++)
    {
      clnt_destroy(G_client_cache[i]->client);
      close(G_client_cache[i]->sock);
    }
   return 1;
}

static CLIENT *
GetClient(server)
   char *server;
{
   CLIENT *cl;
   int sock;
   int i;

   if (!G_client_cache)
    {
      G_client_cache = (ClientHandleCacheList)malloc(sizeof(ClientHandleCache) *
						     MAX_CLIENT_CACHE);
      for (i = 0; i < MAX_CLIENT_CACHE; i++)
         G_client_cache[i] = (ClientHandleCache)
                               malloc(sizeof(ClientHandleCacheStruct));
    }
   for (i = 0; i < G_n_clients; i++)
      if (!strcmp(server, G_client_cache[i]->server))
	 return G_client_cache[i]->client;

   if (G_n_clients < MAX_CLIENT_CACHE)
    {
     /*
      * Create client "handle" used for calling DTLPSTATPROG on the server.
      * Use the "tcp" protocol when contacting the server.
      */
      if (!(cl = ClntCreate(server, DTLPSTATPROG, DTLPSTATVERS, &sock)))
       {
        /*
         * Couldn't establish connection with server.
         * Print error message and die.
         */
#ifdef NO_SPERROR
         sprintf(RemoteLpstatErrorString, "Could not connect to %s", server);
#else
         RemoteLpstatErrorString = clnt_spcreateerror(server);
#endif
         return NULL;
       }
      strcpy(G_client_cache[G_n_clients]->server, server);
      G_client_cache[G_n_clients]->client = cl;
      G_client_cache[G_n_clients]->sock = sock;
      time(&(G_client_cache[G_n_clients]->last_access));
      G_n_clients++;
      return cl;
    }
   qsort(G_client_cache, MAX_CLIENT_CACHE, sizeof(ClientHandleCacheList),
	 SortByAccessTime);
   G_n_clients--;
   clnt_destroy(G_client_cache[G_n_clients]->client);
   close(G_client_cache[G_n_clients]->sock);
   if (!(cl = ClntCreate(server, DTLPSTATPROG, DTLPSTATVERS, &sock)))
    {
#ifdef NO_SPERROR
         sprintf(RemoteLpstatErrorString, "Could not connect to %s", server);
#else
         RemoteLpstatErrorString = clnt_spcreateerror(server);
#endif
      return NULL;
    }
   strcpy(G_client_cache[G_n_clients]->server, server);
   G_client_cache[G_n_clients]->client = cl;
   G_client_cache[G_n_clients]->sock = sock;
   time(&(G_client_cache[G_n_clients]->last_access));
   G_n_clients++;
   return cl;
}

static int SortByAccessTime(first, second)
   ClientHandleCacheList first;
   ClientHandleCacheList second;
{
   return (int)((**first).last_access - (**second).last_access);
}

static CLIENT *
ClntCreate(hostname, prog, vers, sock)
   char *hostname;
   unsigned prog;
   unsigned vers;
   int *sock;
{
   struct hostent *h;
   struct sockaddr_in sin;
   struct timeval tv;
   CLIENT *client;

   if (!(h = gethostbyname(hostname)))
    {
      rpc_createerr.cf_stat = RPC_UNKNOWNHOST;
      return NULL;
    }
   if (h->h_addrtype != AF_INET)
    {
      /* Only support INET for now */
      rpc_createerr.cf_stat = RPC_SYSTEMERROR;
      rpc_createerr.cf_error.re_errno = EAFNOSUPPORT; 
      return NULL;
    }
   (void) memset((char *)&sin, 0, sizeof(sin));
   sin.sin_family = h->h_addrtype;
   sin.sin_port = 0;
   memcpy((char*)&sin.sin_addr, h->h_addr, h->h_length);
   *sock = RPC_ANYSOCK;
   if (!(client = clnttcp_create(&sin, prog, vers, sock, 0, 0)))
      return NULL;
   tv.tv_sec = 20;
   tv.tv_usec = 0;
   clnt_control(client, RPC_TIMEDOUT, &tv);
   return client;
}
