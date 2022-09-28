# %%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
# %%  (c) Copyright 1993, 1994 International Business Machines Corp.	
# %%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
# %%  (c) Copyright 1993, 1994 Novell, Inc. 				
# %%  $Revision: $ 			 				
# @(#)simple.mk	1.6 93/09/07

TT_DIR	= ..

all:	libtt.a
#all:	libtt.a libtt.so.1

include $(TT_DIR)/simple-macros.mk

API_C_OBJECTS=\
api_api.o \
api_default.o \
api_error.o \
api_file.o \
api_handle.o \
api_message.o \
api_mp.o \
api_mp_io.o \
api_mp_otype.o \
api_objid_spec.o \
api_pattern.o \
api_session_prop.o \
api_spec.o \
api_spec_map_ref.o \
api_storage.o \
api_typecb.o \
api_typecb_utils.o \
api_utils.o \
api_xdr.o \
tt_c.o \
tt_status_stub.o

MP_OBJECTS=\
mp_arg.o \
mp_arg_utils.o \
mp_c_file.o \
mp_c_file_utils.o \
mp_c_message.o \
mp_c_message_utils.o \
mp_c_mp.o \
mp_c_msg_context.o \
mp_c_msg_context_utils.o \
mp_c_pattern.o \
mp_c_procid.o \
mp_c_procid_utils.o \
mp_c_session.o \
mp_c_session_prop.o \
mp_c_session_utils.o \
mp_context.o \
mp_context_utils.o \
mp_desktop.o \
mp_desktop_utils.o \
mp_file.o \
mp_file_utils.o \
mp_lock_utils.o \
mp_message.o \
mp_message_utils.o \
mp_mp.o \
mp_mp_utils.o \
mp_msg_context.o \
mp_msg_context_utils.o \
mp_node.o \
mp_node_utils.o \
mp_pat_context.o \
mp_pat_context_utils.o \
mp_pattern.o \
mp_pattern_utils.o \
mp_procid.o \
mp_procid_utils.o \
mp_rpc_client.o \
mp_rpc_client_utils.o \
mp_rpc_fns.o \
mp_session.o \
mp_session_prop.o \
mp_session_prop_utils.o \
mp_session_utils.o \
mp_stream_socket.o \
mp_stream_socket_utils.o \
mp_xdr_functions.o

DB_OBJECTS=\
db_server_clnt.o \
db_server_xdr.o \
old_db_server_functions.o \
tt_client_isam_file.o \
tt_client_isam_file_utils.o \
tt_client_isam_key_descriptor.o \
tt_client_isam_key_descriptor_utils.o \
tt_client_isam_record.o \
tt_client_isam_record_utils.o \
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
tt_old_db.o \
tt_old_db_message_info.o \
tt_old_db_message_info_utils.o \
tt_old_db_partition_map_ref.o \
tt_old_db_utils.o

DM_OBJECTS=\
dm.o \
dm_access_utils.o \
dm_container_utils.o \
dm_db.o \
dm_db_desc.o \
dm_db_desc_utils.o \
dm_db_utils.o \
dm_dbdescs_table.o \
dm_dbdescs_utils.o \
dm_dbref.o \
dm_dbref_utils.o \
dm_dbrefs_table.o \
dm_dbrefs_utils.o \
dm_dbtables.o \
dm_dbtables_utils.o \
dm_docoid_path_record.o \
dm_docoid_utils.o \
dm_enums.o \
dm_exports.o \
dm_exports_utils.o \
dm_key.o \
dm_key_desc.o \
dm_key_desc_utils.o \
dm_key_utils.o \
dm_magic.o \
dm_mfs.o \
dm_mfs_utils.o \
dm_msg_queue_record.o \
dm_msg_utils.o \
dm_oid.o \
dm_oid_access_record.o \
dm_oid_container_record.o \
dm_oid_prop_record.o \
dm_oid_prop_utils.o \
dm_oid_utils.o \
dm_path.o \
dm_record.o \
dm_record_utils.o \
dm_redirect.o \
dm_rpc_mount.o \
dm_table_desc.o \
dm_table_desc_utils.o \
dm_transaction.o \
dm_utils.o

UTIL_OBJECTS=\
copyright.o \
tt_base64.o \
tt_enumname.o \
tt_file_system.o \
tt_file_system_entry_utils.o \
tt_gettext.o \
tt_global_env.o \
tt_host.o \
tt_host_utils.o \
tt_int_rec.o \
tt_ldpath.o \
tt_log.o \
tt_map_entry.o \
tt_map_entry_utils.o \
tt_new.o \
tt_new_ptr.o \
tt_object.o \
tt_object_list.o \
tt_object_table.o \
tt_path.o \
tt_port.o \
tt_string.o \
tt_string_map.o \
tt_string_map_utils.o \
tt_string_match.o \
tt_trace.o \
tt_xdr_utils.o

LIBTT_OBJECTS = $(API_C_OBJECTS) $(MP_OBJECTS) $(DB_OBJECTS) \
		$(DM_OBJECTS) $(UTIL_OBJECTS) \
		realpath.o

libtt.a:	$(LIBTT_OBJECTS)
	$(RM) libtt.a; $(AR) libtt.a $(LIBTT_OBJECTS)
	$(RANLIB) libtt.a

api_api.o:	api/c/api_api.C
	$(CCC) $(LIBCCCFLAGS) -o api_api.o -c api/c/api_api.C

api_default.o:	api/c/api_default.C
	$(CCC) $(LIBCCCFLAGS) -o api_default.o -c api/c/api_default.C

api_error.o:	api/c/api_error.C
	$(CCC) $(LIBCCCFLAGS) -o api_error.o -c api/c/api_error.C

api_file.o:	api/c/api_file.C
	$(CCC) $(LIBCCCFLAGS) -o api_file.o -c api/c/api_file.C

api_handle.o:	api/c/api_handle.C
	$(CCC) $(LIBCCCFLAGS) -o api_handle.o -c api/c/api_handle.C

api_message.o:	api/c/api_message.C
	$(CCC) $(LIBCCCFLAGS) -o api_message.o -c api/c/api_message.C

api_mp.o:	api/c/api_mp.C
	$(CCC) $(LIBCCCFLAGS) -o api_mp.o -c api/c/api_mp.C

api_mp_io.o:	api/c/api_mp_io.C
	$(CCC) $(LIBCCCFLAGS) -o api_mp_io.o -c api/c/api_mp_io.C

api_mp_otype.o:	api/c/api_mp_otype.C
	$(CCC) $(LIBCCCFLAGS) -o api_mp_otype.o -c api/c/api_mp_otype.C

api_objid_spec.o:	api/c/api_objid_spec.C
	$(CCC) $(LIBCCCFLAGS) -o api_objid_spec.o -c api/c/api_objid_spec.C

api_pattern.o:	api/c/api_pattern.C
	$(CCC) $(LIBCCCFLAGS) -o api_pattern.o -c api/c/api_pattern.C

api_session_prop.o:	api/c/api_session_prop.C
	$(CCC) $(LIBCCCFLAGS) -o api_session_prop.o -c api/c/api_session_prop.C

api_spec.o:	api/c/api_spec.C
	$(CCC) $(LIBCCCFLAGS) -o api_spec.o -c api/c/api_spec.C

api_spec_map_ref.o:	api/c/api_spec_map_ref.C
	$(CCC) $(LIBCCCFLAGS) -o api_spec_map_ref.o -c api/c/api_spec_map_ref.C

api_storage.o:	api/c/api_storage.C
	$(CCC) $(LIBCCCFLAGS) -o api_storage.o -c api/c/api_storage.C

api_typecb.o:	api/c/api_typecb.C
	$(CCC) $(LIBCCCFLAGS) -o api_typecb.o -c api/c/api_typecb.C

api_typecb_utils.o:	api/c/api_typecb_utils.C
	$(CCC) $(LIBCCCFLAGS) -o api_typecb_utils.o -c api/c/api_typecb_utils.C

api_utils.o:	api/c/api_utils.C
	$(CCC) $(LIBCCCFLAGS) -o api_utils.o -c api/c/api_utils.C

api_xdr.o:	api/c/api_xdr.C
	$(CCC) $(LIBCCCFLAGS) -o api_xdr.o -c api/c/api_xdr.C

tt_c.o:	api/c/tt_c.C
	$(CCC) $(LIBCCCFLAGS) -o tt_c.o -c api/c/tt_c.C

tt_status_stub.o:	api/c/tt_status_stub.C
	$(CCC) $(LIBCCCFLAGS) -o tt_status_stub.o -c api/c/tt_status_stub.C

mp_arg.o: 	 mp/mp_arg.C
	$(CCC) $(LIBCCCFLAGS) -o mp_arg.o -c mp/mp_arg.C

mp_arg_utils.o: 	mp/mp_arg_utils.C
	$(CCC) $(LIBCCCFLAGS) -o mp_arg_utils.o -c mp/mp_arg_utils.C

mp_c_file.o: 	mp/mp_c_file.C
	$(CCC) $(LIBCCCFLAGS) -o mp_c_file.o -c mp/mp_c_file.C

mp_c_file_utils.o: 	mp/mp_c_file_utils.C
	$(CCC) $(LIBCCCFLAGS) -o mp_c_file_utils.o -c mp/mp_c_file_utils.C

mp_c_message.o: 	mp/mp_c_message.C
	$(CCC) $(LIBCCCFLAGS) -o mp_c_message.o -c mp/mp_c_message.C

mp_c_message_utils.o: 	mp/mp_c_message_utils.C
	$(CCC) $(LIBCCCFLAGS) -o mp_c_message_utils.o -c mp/mp_c_message_utils.C

mp_c_mp.o: 	mp/mp_c_mp.C
	$(CCC) $(LIBCCCFLAGS) -o mp_c_mp.o -c mp/mp_c_mp.C

mp_c_msg_context.o: 	mp/mp_c_msg_context.C
	$(CCC) $(LIBCCCFLAGS) -o mp_c_msg_context.o -c mp/mp_c_msg_context.C

mp_c_msg_context_utils.o: 	mp/mp_c_msg_context_utils.C
	$(CCC) $(LIBCCCFLAGS) -o mp_c_msg_context_utils.o -c mp/mp_c_msg_context_utils.C

mp_c_pattern.o: 	mp/mp_c_pattern.C
	$(CCC) $(LIBCCCFLAGS) -o mp_c_pattern.o -c mp/mp_c_pattern.C

mp_c_procid.o: 	mp/mp_c_procid.C
	$(CCC) $(LIBCCCFLAGS) -o mp_c_procid.o -c mp/mp_c_procid.C

mp_c_procid_utils.o: 	mp/mp_c_procid_utils.C
	$(CCC) $(LIBCCCFLAGS) -o mp_c_procid_utils.o -c mp/mp_c_procid_utils.C

mp_c_session.o: 	mp/mp_c_session.C
	$(CCC) $(LIBCCCFLAGS) -o mp_c_session.o -c mp/mp_c_session.C

mp_c_session_prop.o: 	mp/mp_c_session_prop.C
	$(CCC) $(LIBCCCFLAGS) -o mp_c_session_prop.o -c mp/mp_c_session_prop.C

mp_c_session_utils.o: 	mp/mp_c_session_utils.C
	$(CCC) $(LIBCCCFLAGS) -o mp_c_session_utils.o -c mp/mp_c_session_utils.C

mp_context.o: 	mp/mp_context.C
	$(CCC) $(LIBCCCFLAGS) -o mp_context.o -c mp/mp_context.C

mp_context_utils.o: 	mp/mp_context_utils.C
	$(CCC) $(LIBCCCFLAGS) -o mp_context_utils.o -c mp/mp_context_utils.C

mp_desktop.o: 	mp/mp_desktop.C
	$(CCC) $(LIBCCCFLAGS) -o mp_desktop.o -c mp/mp_desktop.C

mp_desktop_utils.o: 	mp/mp_desktop_utils.C
	$(CCC) $(LIBCCCFLAGS) -o mp_desktop_utils.o -c mp/mp_desktop_utils.C

mp_file.o: 	mp/mp_file.C
	$(CCC) $(LIBCCCFLAGS) -o mp_file.o -c mp/mp_file.C

mp_file_utils.o: 	mp/mp_file_utils.C
	$(CCC) $(LIBCCCFLAGS) -o mp_file_utils.o -c mp/mp_file_utils.C

mp_lock_utils.o: 	mp/mp_lock_utils.C
	$(CCC) $(LIBCCCFLAGS) -o mp_lock_utils.o -c mp/mp_lock_utils.C

mp_message.o: 	mp/mp_message.C
	$(CCC) $(LIBCCCFLAGS) -o mp_message.o -c mp/mp_message.C

mp_message_utils.o: 	mp/mp_message_utils.C
	$(CCC) $(LIBCCCFLAGS) -o mp_message_utils.o -c mp/mp_message_utils.C

mp_mp.o: 	mp/mp_mp.C
	$(CCC) $(LIBCCCFLAGS) -o mp_mp.o -c mp/mp_mp.C

mp_mp_utils.o: 	mp/mp_mp_utils.C
	$(CCC) $(LIBCCCFLAGS) -o mp_mp_utils.o -c mp/mp_mp_utils.C

mp_msg_context.o: 	mp/mp_msg_context.C
	$(CCC) $(LIBCCCFLAGS) -o mp_msg_context.o -c mp/mp_msg_context.C

mp_msg_context_utils.o: 	mp/mp_msg_context_utils.C
	$(CCC) $(LIBCCCFLAGS) -o mp_msg_context_utils.o -c mp/mp_msg_context_utils.C

mp_node.o: 	mp/mp_node.C
	$(CCC) $(LIBCCCFLAGS) -o mp_node.o -c mp/mp_node.C

mp_node_utils.o: 	mp/mp_node_utils.C
	$(CCC) $(LIBCCCFLAGS) -o mp_node_utils.o -c mp/mp_node_utils.C

mp_pat_context.o: 	mp/mp_pat_context.C
	$(CCC) $(LIBCCCFLAGS) -o mp_pat_context.o -c mp/mp_pat_context.C

mp_pat_context_utils.o: 	mp/mp_pat_context_utils.C
	$(CCC) $(LIBCCCFLAGS) -o mp_pat_context_utils.o -c mp/mp_pat_context_utils.C

mp_pattern.o: 	mp/mp_pattern.C
	$(CCC) $(LIBCCCFLAGS) -o mp_pattern.o -c mp/mp_pattern.C

mp_pattern_utils.o: 	mp/mp_pattern_utils.C
	$(CCC) $(LIBCCCFLAGS) -o mp_pattern_utils.o -c mp/mp_pattern_utils.C

mp_procid.o: 	mp/mp_procid.C
	$(CCC) $(LIBCCCFLAGS) -o mp_procid.o -c mp/mp_procid.C

mp_procid_utils.o: 	mp/mp_procid_utils.C
	$(CCC) $(LIBCCCFLAGS) -o mp_procid_utils.o -c mp/mp_procid_utils.C

mp_rpc_client.o: 	mp/mp_rpc_client.C
	$(CCC) $(LIBCCCFLAGS) -o mp_rpc_client.o -c mp/mp_rpc_client.C

mp_rpc_client_utils.o: 	mp/mp_rpc_client_utils.C
	$(CCC) $(LIBCCCFLAGS) -o mp_rpc_client_utils.o -c mp/mp_rpc_client_utils.C

mp_rpc_fns.o: 	mp/mp_rpc_fns.c
	$(CC) $(LIBCCFLAGS) -o mp_rpc_fns.o -c mp/mp_rpc_fns.c

mp_session.o: 	mp/mp_session.C
	$(CCC) $(LIBCCCFLAGS) -o mp_session.o -c mp/mp_session.C

mp_session_prop.o: 	mp/mp_session_prop.C
	$(CCC) $(LIBCCCFLAGS) -o mp_session_prop.o -c mp/mp_session_prop.C

mp_session_prop_utils.o: 	mp/mp_session_prop_utils.C
	$(CCC) $(LIBCCCFLAGS) -o mp_session_prop_utils.o -c mp/mp_session_prop_utils.C

mp_session_utils.o: 	mp/mp_session_utils.C
	$(CCC) $(LIBCCCFLAGS) -o mp_session_utils.o -c mp/mp_session_utils.C

mp_stream_socket.o: 	mp/mp_stream_socket.C
	$(CCC) $(LIBCCCFLAGS) -o mp_stream_socket.o -c mp/mp_stream_socket.C

mp_stream_socket_utils.o: 	mp/mp_stream_socket_utils.C
	$(CCC) $(LIBCCCFLAGS) -o mp_stream_socket_utils.o -c mp/mp_stream_socket_utils.C

mp_xdr_functions.o: 	mp/mp_xdr_functions.C
	$(CCC) $(LIBCCCFLAGS) -o mp_xdr_functions.o -c mp/mp_xdr_functions.C

db_server_clnt.o:	db/db_server_clnt.C
	$(CCC) $(LIBCCCFLAGS) -o db_server_clnt.o -c db/db_server_clnt.C

db_server_xdr.o:	db/db_server_xdr.C
	$(CCC) $(LIBCCCFLAGS) -o db_server_xdr.o -c db/db_server_xdr.C

old_db_server_functions.o:	db/old_db_server_functions.C
	$(CCC) $(LIBCCCFLAGS) -o old_db_server_functions.o -c db/old_db_server_functions.C

tt_client_isam_file.o:	db/tt_client_isam_file.C
	$(CCC) $(LIBCCCFLAGS) -o tt_client_isam_file.o -c db/tt_client_isam_file.C

tt_client_isam_file_utils.o:	db/tt_client_isam_file_utils.C
	$(CCC) $(LIBCCCFLAGS) -o tt_client_isam_file_utils.o -c db/tt_client_isam_file_utils.C

tt_client_isam_key_descriptor.o:	db/tt_client_isam_key_descriptor.C
	$(CCC) $(LIBCCCFLAGS) -o tt_client_isam_key_descriptor.o -c db/tt_client_isam_key_descriptor.C

tt_client_isam_key_descriptor_utils.o:	db/tt_client_isam_key_descriptor_utils.C
	$(CCC) $(LIBCCCFLAGS) -o tt_client_isam_key_descriptor_utils.o -c db/tt_client_isam_key_descriptor_utils.C

tt_client_isam_record.o:	db/tt_client_isam_record.C
	$(CCC) $(LIBCCCFLAGS) -o tt_client_isam_record.o -c db/tt_client_isam_record.C

tt_client_isam_record_utils.o:	db/tt_client_isam_record_utils.C
	$(CCC) $(LIBCCCFLAGS) -o tt_client_isam_record_utils.o -c db/tt_client_isam_record_utils.C

tt_db_access_utils.o:	db/tt_db_access_utils.C
	$(CCC) $(LIBCCCFLAGS) -o tt_db_access_utils.o -c db/tt_db_access_utils.C

tt_db_client.o:	db/tt_db_client.C
	$(CCC) $(LIBCCCFLAGS) -o tt_db_client.o -c db/tt_db_client.C

tt_db_client_utils.o:	db/tt_db_client_utils.C
	$(CCC) $(LIBCCCFLAGS) -o tt_db_client_utils.o -c db/tt_db_client_utils.C

tt_db_create_objid.o:	db/tt_db_create_objid.C
	$(CCC) $(LIBCCCFLAGS) -o tt_db_create_objid.o -c db/tt_db_create_objid.C

tt_db_file.o:	db/tt_db_file.C
	$(CCC) $(LIBCCCFLAGS) -o tt_db_file.o -c db/tt_db_file.C

tt_db_file_utils.o:	db/tt_db_file_utils.C
	$(CCC) $(LIBCCCFLAGS) -o tt_db_file_utils.o -c db/tt_db_file_utils.C

tt_db_hostname_global_map_ref.o:	db/tt_db_hostname_global_map_ref.C
	$(CCC) $(LIBCCCFLAGS) -o tt_db_hostname_global_map_ref.o -c db/tt_db_hostname_global_map_ref.C

tt_db_hostname_redirection_map.o:	db/tt_db_hostname_redirection_map.C
	$(CCC) $(LIBCCCFLAGS) -o tt_db_hostname_redirection_map.o -c db/tt_db_hostname_redirection_map.C

tt_db_key.o:	db/tt_db_key.C
	$(CCC) $(LIBCCCFLAGS) -o tt_db_key.o -c db/tt_db_key.C

tt_db_key_utils.o:	db/tt_db_key_utils.C
	$(CCC) $(LIBCCCFLAGS) -o tt_db_key_utils.o -c db/tt_db_key_utils.C

tt_db_network_path.o:	db/tt_db_network_path.C
	$(CCC) $(LIBCCCFLAGS) -o tt_db_network_path.o -c db/tt_db_network_path.C

tt_db_object.o:	db/tt_db_object.C
	$(CCC) $(LIBCCCFLAGS) -o tt_db_object.o -c db/tt_db_object.C

tt_db_object_utils.o:	db/tt_db_object_utils.C
	$(CCC) $(LIBCCCFLAGS) -o tt_db_object_utils.o -c db/tt_db_object_utils.C

tt_db_objid_to_key.o:	db/tt_db_objid_to_key.C
	$(CCC) $(LIBCCCFLAGS) -o tt_db_objid_to_key.o -c db/tt_db_objid_to_key.C

tt_db_property.o:	db/tt_db_property.C
	$(CCC) $(LIBCCCFLAGS) -o tt_db_property.o -c db/tt_db_property.C

tt_db_property_utils.o:	db/tt_db_property_utils.C
	$(CCC) $(LIBCCCFLAGS) -o tt_db_property_utils.o -c db/tt_db_property_utils.C

tt_db_rpc_message_routines.o:	db/tt_db_rpc_message_routines.C
	$(CCC) $(LIBCCCFLAGS) -o tt_db_rpc_message_routines.o -c db/tt_db_rpc_message_routines.C

tt_db_rpc_routines.o:	db/tt_db_rpc_routines.C
	$(CCC) $(LIBCCCFLAGS) -o tt_db_rpc_routines.o -c db/tt_db_rpc_routines.C

tt_old_db.o:	db/tt_old_db.C
	$(CCC) $(LIBCCCFLAGS) -o tt_old_db.o -c db/tt_old_db.C

tt_old_db_message_info.o:	db/tt_old_db_message_info.C
	$(CCC) $(LIBCCCFLAGS) -o tt_old_db_message_info.o -c db/tt_old_db_message_info.C

tt_old_db_message_info_utils.o:	db/tt_old_db_message_info_utils.C
	$(CCC) $(LIBCCCFLAGS) -o tt_old_db_message_info_utils.o -c db/tt_old_db_message_info_utils.C

tt_old_db_partition_map_ref.o:	db/tt_old_db_partition_map_ref.C
	$(CCC) $(LIBCCCFLAGS) -o tt_old_db_partition_map_ref.o -c db/tt_old_db_partition_map_ref.C

tt_old_db_utils.o:	db/tt_old_db_utils.C
	$(CCC) $(LIBCCCFLAGS) -o tt_old_db_utils.o -c db/tt_old_db_utils.C

dm.o:	dm/dm.C
	$(CCC) $(LIBCCCFLAGS) -o dm.o -c dm/dm.C

dm_access_utils.o:	dm/dm_access_utils.C
	$(CCC) $(LIBCCCFLAGS) -o dm_access_utils.o -c dm/dm_access_utils.C

dm_container_utils.o:	dm/dm_container_utils.C
	$(CCC) $(LIBCCCFLAGS) -o dm_container_utils.o -c dm/dm_container_utils.C

dm_db.o:	dm/dm_db.C
	$(CCC) $(LIBCCCFLAGS) -o dm_db.o -c dm/dm_db.C

dm_db_desc.o:	dm/dm_db_desc.C
	$(CCC) $(LIBCCCFLAGS) -o dm_db_desc.o -c dm/dm_db_desc.C

dm_db_desc_utils.o:	dm/dm_db_desc_utils.C
	$(CCC) $(LIBCCCFLAGS) -o dm_db_desc_utils.o -c dm/dm_db_desc_utils.C

dm_db_utils.o:	dm/dm_db_utils.C
	$(CCC) $(LIBCCCFLAGS) -o dm_db_utils.o -c dm/dm_db_utils.C

dm_dbdescs_table.o:	dm/dm_dbdescs_table.C
	$(CCC) $(LIBCCCFLAGS) -o dm_dbdescs_table.o -c dm/dm_dbdescs_table.C

dm_dbdescs_utils.o:	dm/dm_dbdescs_utils.C
	$(CCC) $(LIBCCCFLAGS) -o dm_dbdescs_utils.o -c dm/dm_dbdescs_utils.C

dm_dbref.o:	dm/dm_dbref.C
	$(CCC) $(LIBCCCFLAGS) -o dm_dbref.o -c dm/dm_dbref.C

dm_dbref_utils.o:	dm/dm_dbref_utils.C
	$(CCC) $(LIBCCCFLAGS) -o dm_dbref_utils.o -c dm/dm_dbref_utils.C

dm_dbrefs_table.o:	dm/dm_dbrefs_table.C
	$(CCC) $(LIBCCCFLAGS) -o dm_dbrefs_table.o -c dm/dm_dbrefs_table.C

dm_dbrefs_utils.o:	dm/dm_dbrefs_utils.C
	$(CCC) $(LIBCCCFLAGS) -o dm_dbrefs_utils.o -c dm/dm_dbrefs_utils.C

dm_dbtables.o:	dm/dm_dbtables.C
	$(CCC) $(LIBCCCFLAGS) -o dm_dbtables.o -c dm/dm_dbtables.C

dm_dbtables_utils.o:	dm/dm_dbtables_utils.C
	$(CCC) $(LIBCCCFLAGS) -o dm_dbtables_utils.o -c dm/dm_dbtables_utils.C

dm_docoid_path_record.o:	dm/dm_docoid_path_record.C
	$(CCC) $(LIBCCCFLAGS) -o dm_docoid_path_record.o -c dm/dm_docoid_path_record.C

dm_docoid_utils.o:	dm/dm_docoid_utils.C
	$(CCC) $(LIBCCCFLAGS) -o dm_docoid_utils.o -c dm/dm_docoid_utils.C

dm_enums.o:	dm/dm_enums.C
	$(CCC) $(LIBCCCFLAGS) -o dm_enums.o -c dm/dm_enums.C

dm_exports.o:	dm/dm_exports.C
	$(CCC) $(LIBCCCFLAGS) -o dm_exports.o -c dm/dm_exports.C

dm_exports_utils.o:	dm/dm_exports_utils.C
	$(CCC) $(LIBCCCFLAGS) -o dm_exports_utils.o -c dm/dm_exports_utils.C

dm_key.o:	dm/dm_key.C
	$(CCC) $(LIBCCCFLAGS) -o dm_key.o -c dm/dm_key.C

dm_key_desc.o:	dm/dm_key_desc.C
	$(CCC) $(LIBCCCFLAGS) -o dm_key_desc.o -c dm/dm_key_desc.C

dm_key_desc_utils.o:	dm/dm_key_desc_utils.C
	$(CCC) $(LIBCCCFLAGS) -o dm_key_desc_utils.o -c dm/dm_key_desc_utils.C

dm_key_utils.o:	dm/dm_key_utils.C
	$(CCC) $(LIBCCCFLAGS) -o dm_key_utils.o -c dm/dm_key_utils.C

dm_magic.o:	dm/dm_magic.C
	$(CCC) $(LIBCCCFLAGS) -o dm_magic.o -c dm/dm_magic.C

dm_mfs.o:	dm/dm_mfs.C
	$(CCC) $(LIBCCCFLAGS) -o dm_mfs.o -c dm/dm_mfs.C

dm_mfs_utils.o:	dm/dm_mfs_utils.C
	$(CCC) $(LIBCCCFLAGS) -o dm_mfs_utils.o -c dm/dm_mfs_utils.C

dm_msg_queue_record.o:	dm/dm_msg_queue_record.C
	$(CCC) $(LIBCCCFLAGS) -o dm_msg_queue_record.o -c dm/dm_msg_queue_record.C

dm_msg_utils.o:	dm/dm_msg_utils.C
	$(CCC) $(LIBCCCFLAGS) -o dm_msg_utils.o -c dm/dm_msg_utils.C

dm_oid.o:	dm/dm_oid.C
	$(CCC) $(LIBCCCFLAGS) -o dm_oid.o -c dm/dm_oid.C

dm_oid_access_record.o:	dm/dm_oid_access_record.C
	$(CCC) $(LIBCCCFLAGS) -o dm_oid_access_record.o -c dm/dm_oid_access_record.C

dm_oid_container_record.o:	dm/dm_oid_container_record.C
	$(CCC) $(LIBCCCFLAGS) -o dm_oid_container_record.o -c dm/dm_oid_container_record.C

dm_oid_prop_record.o:	dm/dm_oid_prop_record.C
	$(CCC) $(LIBCCCFLAGS) -o dm_oid_prop_record.o -c dm/dm_oid_prop_record.C

dm_oid_prop_utils.o:	dm/dm_oid_prop_utils.C
	$(CCC) $(LIBCCCFLAGS) -o dm_oid_prop_utils.o -c dm/dm_oid_prop_utils.C

dm_oid_utils.o:	dm/dm_oid_utils.C
	$(CCC) $(LIBCCCFLAGS) -o dm_oid_utils.o -c dm/dm_oid_utils.C

dm_path.o:	dm/dm_path.C
	$(CCC) $(LIBCCCFLAGS) -o dm_path.o -c dm/dm_path.C

dm_record.o:	dm/dm_record.C
	$(CCC) $(LIBCCCFLAGS) -o dm_record.o -c dm/dm_record.C

dm_record_utils.o:	dm/dm_record_utils.C
	$(CCC) $(LIBCCCFLAGS) -o dm_record_utils.o -c dm/dm_record_utils.C

dm_redirect.o:	dm/dm_redirect.C
	$(CCC) $(LIBCCCFLAGS) -o dm_redirect.o -c dm/dm_redirect.C

dm_rpc_mount.o:	dm/dm_rpc_mount.C
	$(CCC) $(LIBCCCFLAGS) -o dm_rpc_mount.o -c dm/dm_rpc_mount.C

dm_table_desc.o:	dm/dm_table_desc.C
	$(CCC) $(LIBCCCFLAGS) -o dm_table_desc.o -c dm/dm_table_desc.C

dm_table_desc_utils.o:	dm/dm_table_desc_utils.C
	$(CCC) $(LIBCCCFLAGS) -o dm_table_desc_utils.o -c dm/dm_table_desc_utils.C

dm_transaction.o:	dm/dm_transaction.C
	$(CCC) $(LIBCCCFLAGS) -o dm_transaction.o -c dm/dm_transaction.C

dm_utils.o:	dm/dm_utils.C
	$(CCC) $(LIBCCCFLAGS) -o dm_utils.o -c dm/dm_utils.C

copyright.o:	util/copyright.C
	$(CCC) $(LIBCCCFLAGS) $(VEROPT) -o copyright.o -c util/copyright.C

tt_base64.o:	util/tt_base64.C
	$(CCC) $(LIBCCCFLAGS) -o tt_base64.o -c util/tt_base64.C

tt_enumname.o:	util/tt_enumname.C
	$(CCC) $(LIBCCCFLAGS) -o tt_enumname.o -c util/tt_enumname.C

tt_file_system.o:	util/tt_file_system.C
	$(CCC) $(LIBCCCFLAGS) -o tt_file_system.o -c util/tt_file_system.C

tt_file_system_entry_utils.o:	util/tt_file_system_entry_utils.C
	$(CCC) $(LIBCCCFLAGS) -o tt_file_system_entry_utils.o -c util/tt_file_system_entry_utils.C

tt_gettext.o:	util/tt_gettext.C
	$(CCC) $(LIBCCCFLAGS) -o tt_gettext.o -c util/tt_gettext.C

tt_global_env.o:	util/tt_global_env.C
	$(CCC) $(LIBCCCFLAGS) -o tt_global_env.o -c util/tt_global_env.C

tt_host.o:	util/tt_host.C
	$(CCC) $(LIBCCCFLAGS) -o tt_host.o -c util/tt_host.C

tt_host_utils.o:	util/tt_host_utils.C
	$(CCC) $(LIBCCCFLAGS) -o tt_host_utils.o -c util/tt_host_utils.C

tt_int_rec.o:	util/tt_int_rec.C
	$(CCC) $(LIBCCCFLAGS) -o tt_int_rec.o -c util/tt_int_rec.C

tt_ldpath.o:	util/tt_ldpath.C
	$(CCC) $(LIBCCCFLAGS) -o tt_ldpath.o -c util/tt_ldpath.C

tt_log.o:	util/tt_log.C
	$(CCC) $(LIBCCCFLAGS) -o tt_log.o -c util/tt_log.C

tt_map_entry.o:	util/tt_map_entry.C
	$(CCC) $(LIBCCCFLAGS) -o tt_map_entry.o -c util/tt_map_entry.C

tt_map_entry_utils.o:	util/tt_map_entry_utils.C
	$(CCC) $(LIBCCCFLAGS) -o tt_map_entry_utils.o -c util/tt_map_entry_utils.C

tt_new.o:	util/tt_new.C
	$(CCC) $(LIBCCCFLAGS) -o tt_new.o -c util/tt_new.C

tt_new_ptr.o:	util/tt_new_ptr.C
	$(CCC) $(LIBCCCFLAGS) -o tt_new_ptr.o -c util/tt_new_ptr.C

tt_object.o:	util/tt_object.C
	$(CCC) $(LIBCCCFLAGS) -o tt_object.o -c util/tt_object.C

tt_object_list.o:	util/tt_object_list.C
	$(CCC) $(LIBCCCFLAGS) -o tt_object_list.o -c util/tt_object_list.C

tt_object_table.o:	util/tt_object_table.C
	$(CCC) $(LIBCCCFLAGS) -o tt_object_table.o -c util/tt_object_table.C

tt_path.o:	util/tt_path.C
	$(CCC) $(LIBCCCFLAGS) -o tt_path.o -c util/tt_path.C

tt_port.o:	util/tt_port.C
	$(CCC) $(LIBCCCFLAGS) -o tt_port.o -c util/tt_port.C

tt_string.o:	util/tt_string.C
	$(CCC) $(LIBCCCFLAGS) -o tt_string.o -c util/tt_string.C

tt_string_map.o:	util/tt_string_map.C
	$(CCC) $(LIBCCCFLAGS) -o tt_string_map.o -c util/tt_string_map.C

tt_string_map_utils.o:	util/tt_string_map_utils.C
	$(CCC) $(LIBCCCFLAGS) -o tt_string_map_utils.o -c util/tt_string_map_utils.C

tt_string_match.o:	util/tt_string_match.C
	$(CCC) $(LIBCCCFLAGS) -o tt_string_match.o -c util/tt_string_match.C

tt_trace.o:	util/tt_trace.C
	$(CCC) $(LIBCCCFLAGS) -o tt_trace.o -c util/tt_trace.C

tt_xdr_utils.o:	util/tt_xdr_utils.C
	$(CCC) $(LIBCCCFLAGS) -o tt_xdr_utils.o -c util/tt_xdr_utils.C

# Edit this line to compile the correct realpath.xx.c. You may not
# need it at all if libc has realpath, as it does on Solaris.
realpath.o: realpath.hp.c
	$(CC) $(LIBCCFLAGS) -o realpath.o -c realpath.hp.c

clean:
	$(RM) $(LIBTT_OBJECTS) libtt.a

install: libtt.a api/c/tt_c.h
	@test -d $(INSTALLLIB) || mkdir -p $(INSTALLLIB)
	@test -d $(INSTALLINCL) || mkdir -p $(INSTALLINCL)
	$(RM) $(INSTALLLIB)/libtt.a; cp libtt.a $(INSTALLLIB); $(RANLIB) -t $(INSTALLLIB)/libtt.a; chmod 444 $(INSTALLLIB)/libtt.a
	$(RM) $(INSTALLINCL)/tt_c.h; cp api/c/tt_c.h $(INSTALLINCL); chmod 444 $(INSTALLINCL)/tt_c.h

