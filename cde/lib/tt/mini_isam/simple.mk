# %%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
# %%  (c) Copyright 1993, 1994 International Business Machines Corp.	
# %%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
# %%  (c) Copyright 1993, 1994 Novell, Inc. 				
# %%  $Revision: $ 			 				
# @(#)simple.mk	1.4 93/07/30

TT_DIR	= ..

include $(TT_DIR)/simple-macros.mk

all:	libisam.a

LIBISAM_OBJECTS =\
isadd1key.o \
isaddindex.o \
isaddprimary.o \
isalloc.o \
isamaddindex.o \
isamdelcurr.o \
isamdelrec.o \
isamerror.o \
isamhooks.o \
isamopen.o \
isamwrite.o \
isapplmr.o \
isapplmw.o \
isbsearch.o \
isbtree.o \
isbtree2.o \
isbtree3.o \
isbuild.o \
isbytearray.o \
ischange1key.o \
isclose.o \
iscntl.o \
isdatconv.o \
isdel1key.o \
isdelcurr.o \
isdelrec.o \
isdiskbufs2.o \
isdlink.o \
iserase.o \
iserror.o \
isfab.o \
isfcb.o \
isfcbindex.o \
isfcbwatchfd.o \
isfd.o \
isfileio.o \
isfixrec.o \
isfname.o \
isindexconv.o \
isindexinfo.o \
isindfreel.o \
iskeyaux.o \
iskeycalc.o \
iskeycmp.o \
iskeyconv.o \
iskeyvalid.o \
isminmax.o \
ismngfcb.o \
isopen.o \
ispageio.o \
isperm.o \
isread.o \
isrename.o \
isrepair.o \
isrewcurr.o \
isrewrec.o \
issignals.o \
issort.o \
isstart.o \
issync.o \
isvarrec.o \
isvars.o \
isversion.o \
iswrite.o \
iswrrec.o

libisam.a:	$(LIBISAM_OBJECTS)
	$(RM) libisam.a; $(AR) libisam.a $(LIBISAM_OBJECTS)
	$(RANLIB) libisam.a

isadd1key.o:	isadd1key.c
	$(CC) $(LIBCCFLAGS) -o isadd1key.o -c isadd1key.c

isaddindex.o:	isaddindex.c
	$(CC) $(LIBCCFLAGS) -o isaddindex.o -c isaddindex.c

isaddprimary.o:	isaddprimary.c
	$(CC) $(LIBCCFLAGS) -o isaddprimary.o -c isaddprimary.c

isalloc.o:	isalloc.c
	$(CC) $(LIBCCFLAGS) -o isalloc.o -c isalloc.c

isamaddindex.o:	isamaddindex.c
	$(CC) $(LIBCCFLAGS) -o isamaddindex.o -c isamaddindex.c

isamdelcurr.o:	isamdelcurr.c
	$(CC) $(LIBCCFLAGS) -o isamdelcurr.o -c isamdelcurr.c

isamdelrec.o:	isamdelrec.c
	$(CC) $(LIBCCFLAGS) -o isamdelrec.o -c isamdelrec.c

isamerror.o:	isamerror.c
	$(CC) $(LIBCCFLAGS) -o isamerror.o -c isamerror.c

isamhooks.o:	isamhooks.c
	$(CC) $(LIBCCFLAGS) -o isamhooks.o -c isamhooks.c

isamopen.o:	isamopen.c
	$(CC) $(LIBCCFLAGS) -o isamopen.o -c isamopen.c

isamwrite.o:	isamwrite.c
	$(CC) $(LIBCCFLAGS) -o isamwrite.o -c isamwrite.c

isapplmr.o:	isapplmr.c
	$(CC) $(LIBCCFLAGS) -o isapplmr.o -c isapplmr.c

isapplmw.o:	isapplmw.c
	$(CC) $(LIBCCFLAGS) -o isapplmw.o -c isapplmw.c

isbsearch.o:	isbsearch.c
	$(CC) $(LIBCCFLAGS) -o isbsearch.o -c isbsearch.c

isbtree.o:	isbtree.c
	$(CC) $(LIBCCFLAGS) -o isbtree.o -c isbtree.c

isbtree2.o:	isbtree2.c
	$(CC) $(LIBCCFLAGS) -o isbtree2.o -c isbtree2.c

isbtree3.o:	isbtree3.c
	$(CC) $(LIBCCFLAGS) -o isbtree3.o -c isbtree3.c

isbuild.o:	isbuild.c
	$(CC) $(LIBCCFLAGS) -o isbuild.o -c isbuild.c

isbytearray.o:	isbytearray.c
	$(CC) $(LIBCCFLAGS) -o isbytearray.o -c isbytearray.c

ischange1key.o:	ischange1key.c
	$(CC) $(LIBCCFLAGS) -o ischange1key.o -c ischange1key.c

isclose.o:	isclose.c
	$(CC) $(LIBCCFLAGS) -o isclose.o -c isclose.c

iscntl.o:	iscntl.c
	$(CC) $(LIBCCFLAGS) -o iscntl.o -c iscntl.c

isdatconv.o:	isdatconv.c
	$(CC) $(LIBCCFLAGS) -o isdatconv.o -c isdatconv.c

isdel1key.o:	isdel1key.c
	$(CC) $(LIBCCFLAGS) -o isdel1key.o -c isdel1key.c

isdelcurr.o:	isdelcurr.c
	$(CC) $(LIBCCFLAGS) -o isdelcurr.o -c isdelcurr.c

isdelrec.o:	isdelrec.c
	$(CC) $(LIBCCFLAGS) -o isdelrec.o -c isdelrec.c

isdiskbufs2.o:	isdiskbufs2.c
	$(CC) $(LIBCCFLAGS) -o isdiskbufs2.o -c isdiskbufs2.c

isdlink.o:	isdlink.c
	$(CC) $(LIBCCFLAGS) -o isdlink.o -c isdlink.c

iserase.o:	iserase.c
	$(CC) $(LIBCCFLAGS) -o iserase.o -c iserase.c

iserror.o:	iserror.c
	$(CC) $(LIBCCFLAGS) -o iserror.o -c iserror.c

isfab.o:	isfab.c
	$(CC) $(LIBCCFLAGS) -o isfab.o -c isfab.c

isfcb.o:	isfcb.c
	$(CC) $(LIBCCFLAGS) -o isfcb.o -c isfcb.c

isfcbindex.o:	isfcbindex.c
	$(CC) $(LIBCCFLAGS) -o isfcbindex.o -c isfcbindex.c

isfcbwatchfd.o:	isfcbwatchfd.c
	$(CC) $(LIBCCFLAGS) -o isfcbwatchfd.o -c isfcbwatchfd.c

isfd.o:	isfd.c
	$(CC) $(LIBCCFLAGS) -o isfd.o -c isfd.c

isfileio.o:	isfileio.c
	$(CC) $(LIBCCFLAGS) -o isfileio.o -c isfileio.c

isfixrec.o:	isfixrec.c
	$(CC) $(LIBCCFLAGS) -o isfixrec.o -c isfixrec.c

isfname.o:	isfname.c
	$(CC) $(LIBCCFLAGS) -o isfname.o -c isfname.c

isindexconv.o:	isindexconv.c
	$(CC) $(LIBCCFLAGS) -o isindexconv.o -c isindexconv.c

isindexinfo.o:	isindexinfo.c
	$(CC) $(LIBCCFLAGS) -o isindexinfo.o -c isindexinfo.c

isindfreel.o:	isindfreel.c
	$(CC) $(LIBCCFLAGS) -o isindfreel.o -c isindfreel.c

iskeyaux.o:	iskeyaux.c
	$(CC) $(LIBCCFLAGS) -o iskeyaux.o -c iskeyaux.c

iskeycalc.o:	iskeycalc.c
	$(CC) $(LIBCCFLAGS) -o iskeycalc.o -c iskeycalc.c

iskeycmp.o:	iskeycmp.c
	$(CC) $(LIBCCFLAGS) -o iskeycmp.o -c iskeycmp.c

iskeyconv.o:	iskeyconv.c
	$(CC) $(LIBCCFLAGS) -o iskeyconv.o -c iskeyconv.c

iskeyvalid.o:	iskeyvalid.c
	$(CC) $(LIBCCFLAGS) -o iskeyvalid.o -c iskeyvalid.c

isminmax.o:	isminmax.c
	$(CC) $(LIBCCFLAGS) -o isminmax.o -c isminmax.c

ismngfcb.o:	ismngfcb.c
	$(CC) $(LIBCCFLAGS) -o ismngfcb.o -c ismngfcb.c

isopen.o:	isopen.c
	$(CC) $(LIBCCFLAGS) -o isopen.o -c isopen.c

ispageio.o:	ispageio.c
	$(CC) $(LIBCCFLAGS) -o ispageio.o -c ispageio.c

isparsname.o:	isparsname.c
	$(CC) $(LIBCCFLAGS) -o isparsname.o -c isparsname.c

isperm.o:	isperm.c
	$(CC) $(LIBCCFLAGS) -o isperm.o -c isperm.c

isread.o:	isread.c
	$(CC) $(LIBCCFLAGS) -o isread.o -c isread.c

isrename.o:	isrename.c
	$(CC) $(LIBCCFLAGS) -o isrename.o -c isrename.c

isrepair.o:	isrepair.c
	$(CC) $(LIBCCFLAGS) -o isrepair.o -c isrepair.c

isrewcurr.o:	isrewcurr.c
	$(CC) $(LIBCCFLAGS) -o isrewcurr.o -c isrewcurr.c

isrewrec.o:	isrewrec.c
	$(CC) $(LIBCCFLAGS) -o isrewrec.o -c isrewrec.c

issignals.o:	issignals.c
	$(CC) $(LIBCCFLAGS) -o issignals.o -c issignals.c

issort.o:	issort.c
	$(CC) $(LIBCCFLAGS) -o issort.o -c issort.c

isstart.o:	isstart.c
	$(CC) $(LIBCCFLAGS) -o isstart.o -c isstart.c

issync.o:	issync.c
	$(CC) $(LIBCCFLAGS) -o issync.o -c issync.c

isvarrec.o:	isvarrec.c
	$(CC) $(LIBCCFLAGS) -o isvarrec.o -c isvarrec.c

isvars.o:	isvars.c
	$(CC) $(LIBCCFLAGS) -o isvars.o -c isvars.c

isversion.o:	isversion.c
	$(CC) $(LIBCCFLAGS) $(VEROPT) -o isversion.o -c isversion.c

iswrite.o:	iswrite.c
	$(CC) $(LIBCCFLAGS) -o iswrite.o -c iswrite.c

iswrrec.o:	iswrrec.c
	$(CC) $(LIBCCFLAGS) -o iswrrec.o -c iswrrec.c

use_mtab.AIX.o:	use_mtab.AIX.c
	$(CC) $(LIBCCFLAGS) -o use_mtab.AIX.o -c use_mtab.AIX.c

use_mtab.HP-UX.o:	use_mtab.HP-UX.c
	$(CC) $(LIBCCFLAGS) -o use_mtab.HP-UX.o -c use_mtab.HP-UX.c

use_mtab.SunOS.o:	use_mtab.SunOS.c
	$(CC) $(LIBCCFLAGS) -o use_mtab.SunOS.o -c use_mtab.SunOS.c

use_mtab.ULTRIX.o:	use_mtab.ULTRIX.c
	$(CC) $(LIBCCFLAGS) -o use_mtab.ULTRIX.o -c use_mtab.ULTRIX.c



clean:
	$(RM) $(LIBISAM_OBJECTS) libisam.a


install:
