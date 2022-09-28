/* dtlpstat.x: Remote printer status protocol */

const MAXNAMELEN = 255;		/* maximum length of a status line entry */

typedef string nametype<MAXNAMELEN>;	/* a status line entry */

typedef struct namenode *namelist;	/* a link in the listing */

/* A node in the status line listing */
struct namenode {
	nametype name;		/* line status line entry */
	namelist next;		/* next entry */
};

/* The result of a DTLPLISTJOBS operation.  */
union dtlpstat_res switch (int errno) {
case 0:
	namelist list;	/* no error: return status line listing */
default:
	void;		/* error occurred: nothing else to return */
};

/* The status line program definition */
program DTLPSTATPROG {
	version DTLPSTATVERS {
		dtlpstat_res
		DTLPLISTJOBS(nametype) = 1;
	} = 1;
} = 0x20000001;
