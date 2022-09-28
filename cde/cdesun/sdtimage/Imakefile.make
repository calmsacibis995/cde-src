XCOMM	@(#)Imakefile.make	1.1 96/03/07

#if defined(SunArchitecture)

OPENWINHOME:sh=		echo ${OPENWINHOME:-/usr/openwin}
DESTDIR:sh=		echo ${OPENWINHOME:-""}

#ifndef DEBUGTREE
STRIPME="stripit"
#else
STRIPME=echo
#endif

LOCAL_MFLAGS	= STRIPME=$(STRIPME) OPENWINHOME=$(OPENWINHOME) DESTDIR=$(DESTDIR)

IMAGE_TARGET	= sdtimage
IMAGE_MAKEFILE 	= Makefile.imagetool
IMAGE_MAKE	= $(MAKE) -f $(IMAGE_MAKEFILE) $(MFLAGS) $(LOCAL_MFLAGS)

COUNTER_TARGET	= sdtpagecounter
COUNTER_MAKEFILE = Makefile.pagecounter
COUNTER_MAKE	= $(MAKE) -f $(COUNTER_MAKEFILE) $(MFLAGS) $(LOCAL_MFLAGS)

all:	$(IMAGE_TARGET) $(COUNTER_TARGET)

$(IMAGE_TARGET):
	$(IMAGE_MAKE) $@

$(COUNTER_TARGET):
	$(COUNTER_MAKE) $@

includes::

clean::
	$(IMAGE_MAKE) $@
	$(COUNTER_MAKE) $@

#endif /* SunArchitecture */

DependTarget()
