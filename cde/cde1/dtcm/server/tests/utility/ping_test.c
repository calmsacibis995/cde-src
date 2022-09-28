#include <stdio.h>
#include <rpc/rpc.h>
#include <sys/systeminfo.h>
#include <rpcsvc/ypclnt.h>
#include <rpcsvc/yp_prot.h>

struct timeval tv;

void
main(int argc, char **argv)
{
	CLIENT	*cl;
	char *domainname;
	char domain[100];
	char host[BUFSIZ];
	ulong	ver;
	int i;

	printf("Running.\n");

	if (argc < 2) {
		fprintf(stderr, "%s hostname callnumber\n", argv[0]);
		return;
	}

	if ((cl = clnt_create_vers(argv[1], 100068, &ver, 2, 5, "tcp")) == NULL) {
		clnt_pcreateerror("create");
		exit(1);
	}

	tv.tv_sec = 10;
	tv.tv_usec = 0;

	for (i = 0; i < 100000; i++) {
		clnt_call(cl, 0, xdr_void, (char *)NULL, xdr_void, (char *)NULL,
			tv);
	}
	clnt_destroy(cl);
}
