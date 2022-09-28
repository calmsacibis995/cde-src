.KEEP_STATE:
.PARALLEL:

#
# @(#)Makefile	1.31 97/06/03
#
OS:sh			=	uname -s
OSVER:sh		=	uname -r | sed 's/\.//g'
ARCH:sh			=	/usr/ucb/arch
MACH:sh			=	uname -p

PLIBS:sh		=	if [ `uname -r | sed 's/\.//g'` -gt 54 ] ; then echo "-lpthread -lposix4" ; fi

ARRC			=	ar rc
EXTRAAUTHENTICATORS	=
DEFAULTAUTHENTICATORS	=	log

SOURCES.c		= 	\
				auth_log.c \
				dummy.c \
				env_unix.c \
				fs_unix.c \
				ftl_unix.c \
				gr_waitp.c \
				imap4.c \
				log_sv5.c \
				mail.c \
				misc.c \
				misc_sv5.c \
				newsrc.c \
				nl_unix.c \
				netmsg.c \
				rfc822.c \
				scandir.c \
				smanager.c \
				smtp.c \
				solaris.c \
				solaris_checksum.c \
				sun_mime.c \
				sun_att.c \
				tcp_sv5.c \
				tz_sv5.c \
				VALID.c \
				write.c \
				writev.c


SOURCES.h		=	\
				dummy.h \
				env.h \
				env_unix.h \
				fs.h \
				ftl.h \
				imap4.h \
				linkage.c \
				linkage.h \
				mail.h \
				misc.h \
				netmsg.h \
				newsrc.h \
				nl.h \
				os_sv5.h \
				pop3.h \
				rfc822.h \
				smtp.h \
				solaris.h \
				sunatt.h \
				sun_mime.h \
				tcp.h \
				tcp_sv5.h \
				VALID.h \
				UnixDefs.h

SOURCES.all		=	$(SOURCES.c)

OBJECTS			=	$(SOURCES.c:.c=.o)

CPPFLAGS		=	\
				-DSTDPROTO=solarisproto \
				-D$(OS)=$(OSVER) \
				-D_POSIX_PTHREAD_SEMANTICS \
				-D__EXTENSIONS__

CFLAGS			=	-mt -Xa -v $(BLD_FLAGS)

TEXTFLAGS		=
LDFLAGS			=	-mt $(BLD_FLAGS)
LIBS			=	-lsocket -lnsl -lintl

EXTRADRIVERS		=	record
EXTRADRIVERS		=

DEFAULTDRIVERS		=	\
				imap \
				solaris \
				dummy

SHAREDLIB		=

tcov:=	CFLAGS		+=	-g -DDEBUG -xprofile=tcov
tcov:=	LDFLAGS		+=	-g -xprofile=tcov

pic :=  CFLAGS		+=	-KPIC -g -DDEBUG

opt:=	CFLAGS		+=	-xO3 -xspace

debug:=	BLD_FLAGS	+=	-g -DDEBUG


all: $(SOURCES.all) libc-client.a

# Only doing solaris here. 
opt : $(SOURCES.all) libc-client.a mtest

debug : $(SOURCES.all) libc-client.a mtest

tcov : $(SOURCES.all) libc-client.a mtest

pic : $(SOURCES.all) libc-client.a mtest

mtest: mtest.o
	$(CC) $(LDFLAGS) -o mtest mtest.o -L. -lc-client $(LIBS) -R../c-client

# NOTE: the .so file is generated for debug usage only - it is NOT shipped
libc-client.a : auths.c linkage.c linkage.h .WAIT $(OBJECTS)
	$(RM) $(ARCHIVE)
	$(ARRC) $@ $(ARCHIVE) $(OBJECTS)

# Driver linkage
os_sv5.o : auths.c
checkpoint.o : linkage.h linkage.c

auths.c linkage.h linkage.c: stuff.made

stuff.made : drivers mkauths
	chmod +x mkauths
	chmod +x drivers
	./drivers $(EXTRADRIVERS) $(DEFAULTDRIVERS)
	touch linkage.made
	./mkauths $(EXTRAAUTHENTICATORS) $(DEFAULTAUTHENTICATORS)
	touch auths.made linkage.made stuff.made

includes::
	if [ -d  ../../../binstall/include/c-client ]; then set +x; \
	else (set -x; mkdir -p ../../../binstall/include/c-client ); fi
	case '${MFLAGS}' in *[i]*) set +e;; esac; \
	echo "  cd"  ../../../binstall/include/c-client; cd ../../../binstall/include/c-client; \
	for i in $(SOURCES.h); do (set -x; $(RM) $$i; ln -s ../../../libsun/Cclient/c-client/$$i .); done
	if [ -d  ../../../binstall/lib ]; then set +x; \
	else (set -x; mkdir -p  ../../../binstall/lib ); fi
	case '${MFLAGS}' in *[i]*) set +e;; esac; \
echo "  cd"  ../../../binstall/lib; cd  ../../../binstall/lib; \
	for i in libc-client.a; do (set -x; $(RM) $$i; ln -s ../../libsun/Cclient/c-client/$$i .); done

#libc-client.so : libc-client.so.1
#	$(RM) $@
#	ln -s libc-client.so.1 $@
#
#libc-client.so.1 : libc-client.a
#	$(CC) -G $(LDFLAGS) -z defs -z text -h $@ $(OBJECTS) mtest.o -o $@ -lposix4 -lpthread -lsocket -lnsl -lc
#

clean:
	$(RM) *.o linkage.[ch] mtest  $(ARCHIVE) osdep.*
	$(RM) $(SOURCES.c.gen) linkage.made auths.made stuff.made
	$(RM) -r mtest.profile *.tcov
