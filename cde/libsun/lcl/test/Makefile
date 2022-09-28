#ident	"@(#)Makefile 1.8	96/08/28 SMI"

include Makefile.tmpl

          TOP = ..
		
    PARENTDIR = 
      PROGRAM = test

       OBJDIR = $(OBJTOPDIR)/$(PARENTDIR)/$(PROGRAM)
     PROTODIR = $(PROTOTOPDIR)/test

      TARGETS = lcl01.$(CPU) \
		lcl02.$(CPU) \
		lcl03.$(CPU) \
		lcl04.$(CPU) \
		lcl05.$(CPU) \
		lcl06.$(CPU) \
		lcl07.$(CPU) \
		lcl08.$(CPU) \
		lcl09.$(CPU) \
		lcl10.$(CPU) \
		lcl11.$(CPU) \
		lcl12.$(CPU) \
		lcl13.$(CPU) \
		lcl14.$(CPU) \
		incoming.$(CPU) \
		outgoing.$(CPU) \
		printing.$(CPU) \
		content.$(CPU) \
		mail_in.$(CPU) \
		mail_out.$(CPU) \
		header.$(CPU)

 PROTOTARGETS = $(TARGETS:.$(CPU)=)


    DATAFILES = attach.euc \
		attach_body.euc \
		attach.iso-2022-jp \
		attach_body.iso-2022-jp \
		attach.iso-2022-jp-base64 \
		attach_header.euc \
		attach.iso8859-1-q \
		attach_header.iso-2022-jp-b \
		simple_attach.euc-jp \
		simple_header.iso-2022-jp-b \
		header.euc-jp \
		header.iso-2022-jp-b \
		body.euc-jp \
		body.iso-2022-jp  

         HDRS = 

         SRCS = lcl01.c \
		lcl02.c \
		lcl03.c \
		lcl04.c \
		lcl05.c \
		lcl06.c \
		lcl07.c \
		lcl08.c \
		lcl09.c \
		lcl10.c \
		lcl11.c \
		lcl12.c \
		lcl13.c \
		lcl14.c \
		incoming.c \
		outgoing.c \
		printing.c \
		content.c \
		mail_in.c \
		mail_out.c \
		header.c

         OBJS = $(OBJDIR)/lcl01.o \
		$(OBJDIR)/lcl02.o \
		$(OBJDIR)/lcl03.o \
		$(OBJDIR)/lcl04.o \
		$(OBJDIR)/lcl05.o \
		$(OBJDIR)/lcl06.o \
		$(OBJDIR)/lcl07.o \
		$(OBJDIR)/lcl08.o \
		$(OBJDIR)/lcl09.o \
		$(OBJDIR)/lcl10.o \
		$(OBJDIR)/lcl11.o \
		$(OBJDIR)/lcl12.o \
		$(OBJDIR)/lcl13.o \
		$(OBJDIR)/lcl14.o \
		$(OBJDIR)/incoming.o \
		$(OBJDIR)/outgoing.o \
		$(OBJDIR)/printing.o \
		$(OBJDIR)/content.o \
		$(OBJCIR)/mail_in.o \
		$(OBJCIR)/mail_out.o \
		$(OBJCIR)/header.o

     MAKEFILE = Makefile.dst

      DEFINES = 

     INCLUDES = -I../include

    LIBRARIES = ../lib/liblcl.a


all: setup $(HDRS) $(SRCS) $(TARGETS)

%.$(CPU): $(OBJDIR)/%.o
	$(CC) -o $@ $< $(LDFLAGS)

$(OBJDIR)/%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<


setup:
	-@if [ -d $(OBJDIR) ]; then set +x; \
	else (set -x; $(MKDIRHIER) $(OBJDIR)); fi
 

install: install_bin install_data

install_bin:
	@if [ -d $(PROTODIR) ]; then set +x; \
	else (set -x; $(MKDIRHIER) $(PROTODIR)); fi
	(set -x; for i in $(PROTOTARGETS); do \
	$(INSTALL) $(INSTALLFLAGS) $(INSTBINFLAGS) $$i.$(CPU) $(PROTODIR)/$$i;\
	done)

install_data: $(DATAFILES) $(MAKEFILE)
	@if [ -d $(PROTODIR) ]; then set +x; \
	else (set -x; $(MKDIRHIER) $(PROTODIR)); fi
	$(INSTALL) $(INSTALLFLAGS) $(INSTDATFLAGS) $(DATAFILES) $(PROTODIR)
	$(INSTALL) $(INSTALLFLAGS) $(INSTDATFLAGS) $(SRCS) $(PROTODIR)
	$(INSTALL) $(INSTALLFLAGS) $(INSTDATFLAGS) $(MAKEFILE) $(PROTODIR)/Makefile

clean:
	$(RM) $(OBJS) $(TARGETS) core
