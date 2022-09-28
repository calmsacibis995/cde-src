#
# @(#)Makefile	1.17 97/06/03
#
.KEEP_STATE:
.NO_PARALLEL:

.SUFFIXES: .csh

#       CShell section.
.csh:
	$(RM) $@
	cat $< > $@
	chmod +x $@

OSVER:sh	=	/bin/uname -r | /bin/sed 's/\.//g'

CFLAGS		=	-mt -Xa -v -g -DDEBUG -DSunOS=$(OSVER) \
			-D_POSIX_PTHREAD_SEMANTICS -D__EXTENSIONS__

# C Source code.
SOURCES.c	=	\
			isNFS.c \
			openAndMmap.c \
			testIndexFile.c

SOURCES.h	=	\
			test.h \
			indexVersion.h

OBJECTS		=	$(SOURCES.c:.c=.o)


# Test scripts
SOURCES.csh	= 	\
			getResults.csh \
			setUpTmp.csh \
			testExisting.csh \
			testExisting.imapWSU41.csh \
			testEverything.csh \
			testIndexFile.script.csh \
			testMisc.csh \
			testMisc.imapWSU41.csh \
			testNewMail.csh \
			testSmtp.csh \
			testVariables.csh \
			verifyAcct.csh

SCRIPTS		=	$(SOURCES.csh:.csh=)

# Answer files
SOURCES.A	=	\
			A.testExisting.1 \
			A.testExisting.2 \
			A.testExisting.imapWSU41 \
			A.testMisc \
			A.testMisc.imapWSU41 \
			A.testNewMail \
			A.testSmtp.1 \
			A.testSmtp.2

# Gold output files, these are diff'd to the run time output.
SOURCES.GOLD	=	\
			Gold.testExisting.1 \
			Gold.testExisting.imapWSU41.1 \
			Gold.testExisting.2 \
			Gold.testIndexFile \
			Gold.testMisc.1 \
			Gold.testMisc.2 \
			Gold.testMisc.imapWSU41.1 \
			Gold.testMisc.imapWSU41.2 \
			Gold.testNewMail.1 \
			Gold.testNewMail.2 \
			Gold.testSmtp.1 \
			Gold.testSmtp.2

TEST_SCRIPTS	=	$(SOURCES.csh:.csh=)

SOURCES.all	=	$(TEST_SCRIPTS) $(SOURCES.A) $(SOURCES.GOLD)

all : $(SOURCES.all) mtest.tcov scripts .WAIT Everything .WAIT tcov

scripts: $(TEST_SCRIPTS)

testIndexFile : $(SOURCES.h) openAndMmap.o testIndexFile.o
	$(CC) $(LDFLAGS) -o $@ openAndMmap.o testIndexFile.o $(LIBS)

isNFS : $(SOURCES.h) isNFS.o
	$(CC) $(LDFLAGS) -o $@ isNFS.o $(LIBS)

indexVersion.h : ../c-client/solaris.c
	/bin/grep INDEXVERSION ../c-client/solaris.c | /bin/grep static | /bin/sed 's/static//' > $@

Everything : testEverything  $(SOURCES.A) $(SOURCES.GOLD) testIndexFile isNFS $(SCRIPTS) mtest.tcov
	$(RM) mtest.profile/tcovd*
	./testEverything
	$(MAKE) tcov

mtest.tcov: 
	(cd ../c-client; $(MAKE) tcov)

tcov : $(SOURCES.all) .WAIT getResults
	$(RM) mtest.profile/tcovd.*
	$(RM) -r results
	/bin/mkdir results
	-for file in ../c-client/*.c ;\
	do \
		/usr/xpg4/bin/grep -sq $$file mtest.profile/tcovd ; \
		if [ $$? = 0 ] ; then \
			/bin/echo "makeing results/\c" ; \
			/bin/echo `/bin/basename $$file`.tcov ; \
			tcov -o results/`/bin/basename $$file`.tcov -x mtest.profile $$file 2>>results/none ; \
		else \
			echo `/bin/basename $$file` >> results/none ; \
		fi ; \
	done
	@/bin/cat results/*.tcov > results/tcov.out
	@/bin/echo "TCOV results in tcov.out"
	@/bin/echo "\n->>These files are either not linked, or never called:"
	@/bin/echo `cat results/none` ;
	@/bin/echo ""
	@./getResults results/tcov.out

clean:
	$(RM) $(TEST_SCRIPTS) *.out.* *.out
	$(RM) $(OBJECTS)
	$(RM) -r mtest.profile
	$(RM) testIndexFile
	$(RM) indexVersion.h
	$(RM) tcov.out *.tcov
	$(RM) -r results
	$(RM) *.input
	$(RM) *.tmp
