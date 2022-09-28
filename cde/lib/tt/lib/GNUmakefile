# %% 	$Revision: 1.6 $							
# %% 									
# %%  	RESTRICTED CONFIDENTIAL INFORMATION:                            
# %% 									
# %% 	The information in this document is subject to special		
# %% 	restrictions in a confidential disclosure agreement between	
# %% 	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this	
# %% 	document outside HP, IBM, Sun, USL, SCO, or Univel without	
# %% 	Sun's specific written approval.  This document and all copies	
# %% 	and derivative works thereof must be returned or destroyed at	
# %% 	Sun's request.							
# %% 									
# %% 	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.	
# %% 									
# @(#)GNUmakefile	1.6 93/09/08
# The ToolTalk library makefile
#
# Copyright (c) 1992 by Sun Microsystems, Inc.
#

TT_DIR = ..
THIS_DIR = lib
SUBDIRS=

# Don't set SUBDIRS since we don't do normal subdirectory below here.
# But we do need a macro containing the subdirectories for some purposes
# (like sccs clean)

LIB_SUBDIRS = api/c api/dnd db dm mp util tttk

# get a list of all header files in the SCCS directories under here.
# but not directly under, since that's done in the global GNUmakefile-head.mk

LIB_SCCS_HEADERS := $(shell ls */SCCS/s.*.h api/*/SCCS/s.*.h 2>/dev/null | sed 's:SCCS/s\.::')

# make most real targets (not clean or sccs clean)  also depend on all
# those header files so make will get them.

HEADER_DEPENDENCIES	= $(LIB_SCCS_HEADERS)

include $(TT_DIR)/GNUmakefile-head.mk

# always build the includes.

INSTALL_INCLUDE_SUBDIR := Tt

INSTALL_CONTENTS = $(INSTALL_DIR)/include/$(INSTALL_INCLUDE_SUBDIR)/tt_c.h \
		$(INSTALL_DIR)/include/$(INSTALL_INCLUDE_SUBDIR)/tttk.h

ifeq (SunOS,$(OS))
  INSTALL_CONTENTS +=	$(INSTALL_DIR)/lib/locale/templates/LC_MESSAGES/SUNW_TOOLTALK.po
endif

# which flavor of lib to build and install is controlled by
# BUILDSHAREDLIBS and BUILDARCHIVELIBS

ifeq ($(BUILDSHAREDLIBS),Y)
  # Don't build libtt.so in -tcov or -pg as it doesn't work.
  ifeq (,$(findstring $(VARIANT),-tcov-pg))
    PROTO_CONTENTS += $(PICOBJDIR)/libtt.$(SHLIBSUFFIX)
    INSTALL_CONTENTS += $(INSTALL_DIR)/lib/libtt.$(SHLIBSUFFIX)
  endif
endif

ifeq ($(BUILDARCHIVELIBS),Y)
  PROTO_CONTENTS += $(PDCOBJDIR)/libtt.a
  INSTALL_CONTENTS += $(INSTALL_DIR)/lib/libtt.a
endif

all::		proto install		

proto_here::	$(PROTO_CONTENTS)
    
install_here::	$(INSTALL_CONTENTS)

$(INSTALL_DIR)/lib/libtt.$(SHLIBSUFFIX): $(PICOBJDIR)/libtt.$(SHLIBSUFFIX)
		$(TTINSTALL) $(INSTALL_DIR)/lib $<

$(INSTALL_DIR)/lib/libtt.a:	$(PDCOBJDIR)/libtt.a
		$(TTINSTALL) $(INSTALL_DIR)/lib $<

$(INSTALL_DIR)/include/$(INSTALL_INCLUDE_SUBDIR)/tt_c.h:	api/c/tt_c.h
		$(TTINSTALL) $(INSTALL_DIR)/include/$(INSTALL_INCLUDE_SUBDIR) api/c/tt_c.h

$(INSTALL_DIR)/include/$(INSTALL_INCLUDE_SUBDIR)/tttk.h:	tttk/tttk.h
		$(TTINSTALL) $(INSTALL_DIR)/include/$(INSTALL_INCLUDE_SUBDIR) tttk/tttk.h

$(INSTALL_DIR)/lib/locale/templates/LC_MESSAGES/SUNW_TOOLTALK.po: \
		SUNW_TOOLTALK.po
		$(TTINSTALL) $(INSTALL_DIR)/lib/locale/templates/LC_MESSAGES \
		SUNW_TOOLTALK.po

clean_here::

sccs-clean::	$(LIB_SUBDIRS:%=sccs-clean-%)

sccs-clean-%:	FRC
	@cd $(@:sccs-clean-%=%) ; \
	sccs clean; \
	cd ..;



OLD_DB_SERVER_COMPATIBILITY_OBJECTS = \
old_db_server_functions.o \
tt_client_isam_file.o \
tt_client_isam_file_utils.o \
tt_client_isam_key_descriptor.o \
tt_client_isam_key_descriptor_utils.o \
tt_client_isam_record.o \
tt_client_isam_record_utils.o \
tt_old_db.o \
tt_old_db_partition_map_ref.o \
tt_old_db_utils.o \
tt_old_db_message_info.o \
tt_old_db_message_info_utils.o

TTTK_OBJECTS = tttk.o \
ttdesktop.o \
ttmedia.o \
tttkmessage.o \
tttkpattern.o \
ttdtfile.o \
tttkutils.o \
tttk2free.o \
ttdtprocid.o

LIBTT_CONTENTS= \
api_api.o api_storage.o api_mp_otype.o api_handle.o \
api_xdr.o api_file.o \
api_filemap.o \
api_mp.o \
api_mp_io.o api_message.o api_utils.o api_spec.o api_default.o \
tt_status_stub.o api_pattern.o api_session_prop.o api_spec_map_ref.o \
api_error.o api_objid_spec.o api_typecb.o api_typecb_utils.o ttdnd.o \
mp_session.o mp_procid.o mp_pattern.o mp_c_pattern.o \
mp_mp.o mp_file.o mp_file_utils.o \
mp_arg.o mp_arg_utils.o \
mp_context.o mp_context_utils.o \
mp_msg_context.o mp_msg_context_utils.o \
mp_c_msg_context.o mp_c_msg_context_utils.o \
mp_pat_context.o mp_pat_context_utils.o \
mp_message.o mp_message_utils.o mp_mp_utils.o \
mp_rpc_client_utils.o \
mp_pattern_utils.o mp_procid_utils.o mp_rpc_client.o \
mp_session_utils.o \
mp_stream_socket.o \
mp_stream_socket_utils.o mp_xdr_functions.o mp_rpc_fns.o \
mp_desktop.o mp_session_prop.o mp_session_prop_utils.o \
mp_desktop_utils.o mp_c_message.o mp_trace.o \
mp_c_procid.o mp_c_procid_utils.o mp_c_file.o \
mp_c_file_utils.o mp_c_mp.o mp_c_message_utils.o \
mp_c_session.o mp_c_session_prop.o mp_c_session_utils.o dm_path.o dm_oid.o \
dm_dbref.o dm_db.o dm.o dm_transaction.o dm_mfs.o \
dm_oid_prop_record.o dm_magic.o dm_key.o \
dm_docoid_path_record.o dm_dbrefs_table.o dm_dbtables.o dm_db_desc.o \
dm_dbdescs_table.o dm_redirect.o dm_table_desc.o dm_record.o \
dm_oid_container_record.o dm_oid_access_record.o \
dm_msg_queue_record.o dm_access_utils.o dm_dbtables_utils.o \
dm_container_utils.o dm_db_desc_utils.o dm_db_utils.o dm_dbdescs_utils.o \
dm_dbref_utils.o dm_dbrefs_utils.o dm_enums.o dm_docoid_utils.o \
dm_exports.o dm_exports_utils.o dm_key_desc.o \
dm_key_desc_utils.o dm_key_utils.o dm_oid_prop_utils.o dm_mfs_utils.o \
dm_msg_utils.o dm_oid_utils.o \
dm_table_desc_utils.o dm_record_utils.o dm_rpc_mount.o dm_utils.o \
tt_file_system.o tt_file_system_entry_utils.o \
tt_gettext.o tt_string_match.o copyright.o tt_string.o tt_xdr_utils.o \
tt_path.o tt_log.o tt_int_rec.o tt_new.o tt_object.o tt_base64.o \
tt_object_list.o tt_enumname.o tt_ldpath.o  tt_new_ptr.o \
tt_host.o tt_host_equiv.o tt_host_equiv_utils.o tt_object_table.o \
tt_host_utils.o tt_global_env.o \
tt_port.o tt_string_map.o tt_string_map_utils.o \
tt_map_entry.o tt_map_entry_utils.o \
tt_Xlib.o \
tt_trace_gram.o tt_tracefile_parse.o tt_trace_parser.o \
tt_trace_stream.o tt_trace.o tt_audit.o tt_iostream.o \
tt_entry_pt.o \
db_server_clnt.o \
db_server_xdr.o \
tt_db_access_utils.o \
tt_db_client.o \
tt_db_client_utils.o \
tt_db_create_objid.o \
tt_db_file.o \
tt_db_file_utils.o \
tt_db_hostname_global_map_ref.o \
tt_db_hostname_redirection_map.o \
tt_db_key.o \
tt_db_key_utils.o \
tt_db_network_path.o \
tt_db_object.o \
tt_db_object_utils.o \
tt_db_objid_to_key.o \
tt_db_property.o \
tt_db_property_utils.o \
tt_db_rpc_message_routines.o \
tt_db_rpc_routines.o \
$(TTTK_OBJECTS) \
$(OLD_DB_SERVER_COMPATIBILITY_OBJECTS)

ifeq ($(OS),SunOS)
  ifneq ($(REL),4)
    ifeq (,$(findstring $(VARIANT),-tcov-pg))
# turn off reordering for now
#      LDFLAGS += -M Mapfile
#      LIBTTDEPENDENCY += Mapfile
    endif
  endif
endif

ifeq ($(OS),AIX)
  LIBTT_CONTENTS += realpath.o
endif
  

# Let's try this cool VPATH feature so we don't need to
# put in bunches of compile rules like in SunPro Make

vpath %.c api/c:api/dnd:db:dm:mp:util:tttk
vpath %.C api/c:api/dnd:db:dm:mp:util:tttk


# A few modules need additional include libraries or other flags
# which we handle with special case static pattern rules.  This
# keeps us from developing the "everything includes everything"
# disease so often seen, and not incidentally keeps the size of
# the make logs down.

# mp_desktop, tt_Xlib,  and most of tttk need X11 stuff

PDC_TTTK_OBJECTS := $(TTTK_OBJECTS:%=$(PDCOBJDIR)/%)
PIC_TTTK_OBJECTS := $(TTTK_OBJECTS:%=$(PICOBJDIR)/%)

X11_NEEDERS = mp_desktop.o tt_Xlib.o

PDC_X11_NEEDERS := $(X11_NEEDERS:%=$(PDCOBJDIR)/%) $(PDC_TTTK_OBJECTS)
PIC_X11_NEEDERS := $(X11_NEEDERS:%=$(PICOBJDIR)/%) $(PIC_TTTK_OBJECTS)

$(PDC_X11_NEEDERS) : $(PDCOBJDIR)/%.o : %.C
	rm -f $@
	$(COMPILE_PDC.cc) $(X11INCLUDES) -o $@ $<
	@$(MCS_COMMAND)

$(PIC_X11_NEEDERS) : $(PICOBJDIR)/%.o : %.C
	rm -f $@
	$(COMPILE_PIC.cc) $(X11INCLUDES) -o $@ $<
	@$(MCS_COMMAND)

# copyright.o needs Version define

$(PDCOBJDIR)/copyright.o : $(PDCOBJDIR)/%.o : %.C
	rm -f $@
	$(COMPILE_PDC.cc) $(CPPFLAGS-version) -o $@ $<
	@$(MCS_COMMAND)

$(PICOBJDIR)/copyright.o : $(PICOBJDIR)/%.o : %.C
	rm -f $@
	$(COMPILE_PIC.cc) $(CPPFLAGS-version) -o $@ $<
	@$(MCS_COMMAND)

# enough modules need isam we just put it on everywhere.
CPPFLAGS += -I$(NETISAM)

PICOBJECTS	:=	$(LIBTT_CONTENTS:%=$(PICOBJDIR)/%)
PDCOBJECTS	:=	$(LIBTT_CONTENTS:%=$(PDCOBJDIR)/%)

ifeq (SunOS,$(OS))
$(PICOBJDIR)/libtt.$(SHLIBSUFFIX): %:	$(LIBTTDEPENDENCY) $(PICOBJDIR) $(PICOBJECTS) 
	LD_RUN_PATH=$(LD_RUN_PATH) \
	$(LD) $(LDFLAGS) $(LDFLAGS-lib) -o $@ $(PICOBJECTS) $(LDLIBS)
	-ln -s libtt.$(SHLIBSUFFIX) $(PICOBJDIR)/libtt.so	
else
$(PICOBJDIR)/libtt.$(SHLIBSUFFIX): %:	$(LIBTTDEPENDENCY) $(PICOBJDIR) $(PICOBJECTS) 
	$(LD) $(LDFLAGS) $(LDFLAGS-lib) -o $@ $(PICOBJECTS)
endif

$(PDCOBJDIR)/libtt.a:		$(PDCOBJDIR) $(PDCOBJECTS)
	rm -fr $@;$(AR) qc $@ $(PDCOBJECTS); $(RANLIB) $@

# Make the Mapfile, for reordering, by executing a shell script.  

Mapfile:	Mapfile-measured mk_libtt_mapfile $(PICOBJDIR) $(PICOBJECTS)
	./mk_libtt_mapfile $(PICOBJDIR)

SUNW_TOOLTALK.po:        api/c/api_api.C
	xgettext -d SUNW_TOOLTALK -n -c STRING_EXTRACTION api/c/api_api.C

# the tttk routines include tttk.h, which assumes it's part of
# the final product and includes <tt/tt_c.h> which may
# not exist yet.


$(PDC_TTTK_OBJECTS) $(PIC_TTTK_OBJECTS): $(INSTALL_INCLUDE_SUBDIR)/tt_c.h

$(INSTALL_INCLUDE_SUBDIR)/tt_c.h:
	test -d $(INSTALL_INCLUDE_SUBDIR) || mkdir $(INSTALL_INCLUDE_SUBDIR) ;\
	rm -f $(INSTALL_INCLUDE_SUBDIR)/tt_c.h ;\
	ln -s ../api/c/tt_c.h $(INSTALL_INCLUDE_SUBDIR)/tt_c.h

# Because we never descend into lib's subdirectories, we must be
# sure to sccs get subdirectory headers here.

include $(TT_DIR)/GNUmakefile-tail.mk

