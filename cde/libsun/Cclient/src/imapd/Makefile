# Program:	IMAPD Makefile
.KEEP_STATE:
.PARALLEL:
#
# Author:	Mark Crispin
#		Networks and Distributed Computing
#		Computing & Communications
#		University of Washington
#		Administration Building, AG-44
#		Seattle, WA  98195
#		Internet: MRC@CAC.Washington.EDU
#
# Date:		5 November 1990
# Last Edited:	07 October 1996
#
# Copyright 1993 by the University of Washington
#
#  Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose and without fee is hereby granted, provided
# that the above copyright notice appears in all copies and that both the
# above copyright notice and this permission notice appear in supporting
# documentation, and that the name of the University of Washington not be
# used in advertising or publicity pertaining to distribution of the software
# without specific, written prior permission.  This software is made
# available "as is", and
# THE UNIVERSITY OF WASHINGTON DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED,
# WITH REGARD TO THIS SOFTWARE, INCLUDING WITHOUT LIMITATION ALL IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, AND IN
# NO EVENT SHALL THE UNIVERSITY OF WASHINGTON BE LIABLE FOR ANY SPECIAL,
# INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
# LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, TORT
# (INCLUDING NEGLIGENCE) OR STRICT LIABILITY, ARISING OUT OF OR IN CONNECTION
# WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

PWD:sh			=	pwd
CCLIENTDIR		=	$(PWD)/../c-client
CCLIENTLIB		=	-L$(CCLIENTDIR) -lc-client -R$(CCLIENTDIR)

# Get local CFLAGS and LFLAGS definitions from c-client directory

CPPFLAGS		=	\
				-I$(CCLIENTDIR) \
                                -D_POSIX_C_SOURCE=199506L \
                                -D_POSIX_PTRHEAD_SEMANITCS \
                                -D__EXTENSIONS__

BLD_FLAG		=	-g
debug			:=	BLD_FLAG	= -g
opt			:=	BLD_FLAG	= -XO2

CFLAGS 			=	-mt -Xc -v $(BLD_FLAG)

LDFLAGS			=	\
				$(CCLIENTLIB) -R/usr/dt/bin \
				-mt -i

LIBS			=	-lsocket -lnsl -lintl

SOURCES.c		=	imapd.c
SOURCES.h		=
OBJECTS			=	$(SOURCES.c:.c=.o)

imapd opt debug: $(OBJECTS)
	$(CC) $(BLD_FLAG) $(LDFLAGS) -o $@ $(OBJECTS) $(LIBS)

clean:
	$(RM) $(OBJECTS) imapd
	$(RM) .make.* *~
