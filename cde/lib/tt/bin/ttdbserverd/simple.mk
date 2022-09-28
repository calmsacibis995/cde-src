# %%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
# %%  (c) Copyright 1993, 1994 International Business Machines Corp.	
# %%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
# %%  (c) Copyright 1993, 1994 Novell, Inc. 				
# %%  $Revision: $ 			 				
# @(#)simple.mk	1.7 93/09/07

TT_DIR	= ../..

all:	rpc.ttdbserverd

include $(TT_DIR)/simple-macros.mk

RPC.TTDBSERVERD_OBJECTS =\
db_server_functions.o \
db_server_svc.o \
dm_access_cache.o \
dm_server.o \
tt_db_message_info_utils.o \
tt_db_msg_q_lock.o \
tt_db_msg_q_lock_entry_utils.o \
tt_db_partition_global_map_ref.o \
tt_db_partition_redirection_map.o \
tt_db_server_db.o \
tt_db_server_db_utils.o \
tt_isam_file.o \
tt_isam_file_utils.o \
tt_isam_key_descriptor.o \
tt_isam_key_descriptor_utils.o \
tt_isam_record.o \
tt_isam_record_utils.o

rpc.ttdbserverd:	$(RPC.TTDBSERVERD_OBJECTS) $(LDDEP) $(ISAMLIBDIR)/lib$(ISAMLIB).a
	$(CCC) $(CCCFLAGS) $(LDSCCCFLAGS) -L$(ISAMLIBDIR) -o rpc.ttdbserverd \
	$(RPC.TTDBSERVERD_OBJECTS) $(LDSCCCLIBS) -l$(ISAMLIB)

db_server_functions.o:	db_server_functions.C
	$(CCC) $(LIBCCCFLAGS) -I$(TT_DIR)/slib -o db_server_functions.o -c db_server_functions.C

db_server_svc.o:	db_server_svc.C
	$(CCC) $(LIBCCCFLAGS) -I$(TT_DIR)/slib $(VEROPT) -o db_server_svc.o -c db_server_svc.C

dm_access_cache.o:	dm_access_cache.C
	$(CCC) $(LIBCCCFLAGS) -I$(TT_DIR)/slib -o dm_access_cache.o -c dm_access_cache.C

dm_server.o:	dm_server.C
	$(CCC) $(LIBCCCFLAGS) -I$(TT_DIR)/slib -o dm_server.o -c dm_server.C

tt_db_message_info_utils.o:	tt_db_message_info_utils.C
	$(CCC) $(LIBCCCFLAGS) -I$(TT_DIR)/slib -o tt_db_message_info_utils.o -c tt_db_message_info_utils.C

tt_db_msg_q_lock.o:	tt_db_msg_q_lock.C
	$(CCC) $(LIBCCCFLAGS) -I$(TT_DIR)/slib -o tt_db_msg_q_lock.o -c tt_db_msg_q_lock.C

tt_db_msg_q_lock_entry_utils.o:	tt_db_msg_q_lock_entry_utils.C
	$(CCC) $(LIBCCCFLAGS) -I$(TT_DIR)/slib -o tt_db_msg_q_lock_entry_utils.o -c tt_db_msg_q_lock_entry_utils.C

tt_db_partition_global_map_ref.o:	tt_db_partition_global_map_ref.C
	$(CCC) $(LIBCCCFLAGS) -I$(TT_DIR)/slib -o tt_db_partition_global_map_ref.o -c tt_db_partition_global_map_ref.C

tt_db_partition_redirection_map.o:	tt_db_partition_redirection_map.C
	$(CCC) $(LIBCCCFLAGS) -I$(TT_DIR)/slib -o tt_db_partition_redirection_map.o -c tt_db_partition_redirection_map.C

tt_db_server_db.o:	tt_db_server_db.C
	$(CCC) $(LIBCCCFLAGS) -I$(TT_DIR)/slib -o tt_db_server_db.o -c tt_db_server_db.C

tt_db_server_db_utils.o:	tt_db_server_db_utils.C
	$(CCC) $(LIBCCCFLAGS) -I$(TT_DIR)/slib -o tt_db_server_db_utils.o -c tt_db_server_db_utils.C

tt_isam_file.o:	tt_isam_file.C
	$(CCC) $(LIBCCCFLAGS) -I$(TT_DIR)/slib -o tt_isam_file.o -c tt_isam_file.C

tt_isam_file_utils.o:	tt_isam_file_utils.C
	$(CCC) $(LIBCCCFLAGS) -I$(TT_DIR)/slib -o tt_isam_file_utils.o -c tt_isam_file_utils.C

tt_isam_key_descriptor.o:	tt_isam_key_descriptor.C
	$(CCC) $(LIBCCCFLAGS) -I$(TT_DIR)/slib -o tt_isam_key_descriptor.o -c tt_isam_key_descriptor.C

tt_isam_key_descriptor_utils.o:	tt_isam_key_descriptor_utils.C
	$(CCC) $(LIBCCCFLAGS) -I$(TT_DIR)/slib -o tt_isam_key_descriptor_utils.o -c tt_isam_key_descriptor_utils.C

tt_isam_record.o:	tt_isam_record.C
	$(CCC) $(LIBCCCFLAGS) -I$(TT_DIR)/slib -o tt_isam_record.o -c tt_isam_record.C

tt_isam_record_utils.o:	tt_isam_record_utils.C
	$(CCC) $(LIBCCCFLAGS) -I$(TT_DIR)/slib -o tt_isam_record_utils.o -c tt_isam_record_utils.C

clean:
	$(RM) $(RPC.TTDBSERVERD_OBJECTS) rpc.ttdbserverd

install: rpc.ttdbserverd
	@test -d $(INSTALLBIN) || mkdir -p $(INSTALLBIN)
	$(RM) $(INSTALLBIN)/rpc.ttdbserverd; cp rpc.ttdbserverd $(INSTALLBIN); chmod 555 $(INSTALLBIN)/rpc.ttdbserverd
