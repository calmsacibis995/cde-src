.ds SI @(#) 1.2 94/12/19
.Ho Contents 3
.tr#
.TS
tab(@);
l1w(2c) l1w(1.5c) l1w(10.5c) rw(0.6c).
.sp 1v
\s+2\f3Chapter@1@Introduction\s0\a@1\f1
##@1.1@###Overview\a@1
##@1.2@###Conformance\a@4
##@1.3@###Terminology\a@5
##@1.4@###Format of Entries\a@7
##@1.4.1@######C-Language Functions and Headers\a@7
##@1.4.2@######Messages\a@8
##@1.4.3@######Service Interfaces\a@8
##@1.5@###Rationale\a@16
##@1.5.1@######Components Not Standardised\a@16
.sp 1v
\s+2\f3Chapter@2@Glossary\s0\a@19\f1
##@2.1@###Terms Defined by \s-1XCDE\s+1\a@19
##@2.2@###Terms From Other Standards\a@35
.sp 1v
\s+2\f3Chapter@3@General Definitions and Requirements\s0\a@39\f1
##@3.1@###\s-1XCDE\s+1 Data Format Naming\a@39
.sp 1v
\s+2\f3Chapter@4@X Windows and Motif\s0\a@43\f1
##@4.1@###X Protocol\a@43
##@4.2@###Xlib Library\a@43
##@4.3@###Xt Intrinsics\a@44
##@4.4@###ICCCM, CT, XLFD, BDF\a@44
##@4.5@###Motif Libraries\a@44
##@4.6@###X Windows and Motif Data Types\a@44
##@4.7@###\s-1XCDE\s+1 Widgets\a@45
##@##@\f2DtComboBox\f1\^(\|)\a@46
##@##@\f2DtMenuButton\f1\^(\|)\a@51
##@##@\f2DtSpinBox\f1\^(\|)\a@56
##@4.8@###\s-1XCDE\s+1 Widget Convenience Functions\a@64
##@##@\f2DtComboBoxAddItem\f1\^(\|)\a@65
##@##@\f2DtComboBoxDeletePos\f1\^(\|)\a@66
##@##@\f2DtComboBoxSelectItem\f1\^(\|)\a@67
##@##@\f2DtComboBoxSetItem\f1\^(\|)\a@68
##@##@\f2DtCreateComboBox\f1\^(\|)\a@69
##@##@\f2DtCreateMenuButton\f1\^(\|)\a@70
##@##@\f2DtCreateSpinBox\f1\^(\|)\a@71
##@##@\f2DtSpinBoxAddItem\f1\^(\|)\a@72
##@##@\f2DtSpinBoxDeletePos\f1\^(\|)\a@73
##@##@\f2DtSpinBoxSetItem\f1\^(\|)\a@74
##@4.9@###\s-1XCDE\s+1 Widget Headers\a@75
##@##@\f3<Dt/ComboBox.h>\f1\a@76
##@##@\f3<Dt/MenuButton.h>\f1\a@77
##@##@\f3<Dt/SpinBox.h>\f1\a@78
##@4.10@###Rationale and Open Issues\a@80
.sp 1v
\s+2\f3Chapter@5@Miscellaneous Desktop Services\s0\a@81\f1
##@5.1@###Introduction\a@81
##@5.2@###Functions\a@81
##@##@\f2DtInitialize\f1\^(\|)\a@82
##@5.3@###Headers\a@83
##@##@\f3<Dt/Dt.h>\f1\a@84
.sp 1v
\s+2\f3Chapter@6@Message Services\s0\a@87\f1
##@6.1@###Introduction\a@87
##@6.2@###Functions\a@88
##@##@\f2tt_X_session\f1\^(\|)\a@89
##@##@\f2tt_bcontext_join\f1\^(\|)\a@90
##@##@\f2tt_bcontext_quit\f1\^(\|)\a@91
##@##@\f2tt_close\f1\^(\|)\a@93
##@##@\f2tt_context_join\f1\^(\|)\a@94
##@##@\f2tt_context_quit\f1\^(\|)\a@95
##@##@\f2tt_default_file\f1\^(\|)\a@97
##@##@\f2tt_default_file_set\f1\^(\|)\a@98
##@##@\f2tt_default_procid\f1\^(\|)\a@99
##@##@\f2tt_default_procid_set\f1\^(\|)\a@100
##@##@\f2tt_default_ptype\f1\^(\|)\a@101
##@##@\f2tt_default_ptype_set\f1\^(\|)\a@102
##@##@\f2tt_default_session\f1\^(\|)\a@103
##@##@\f2tt_default_session_set\f1\^(\|)\a@104
##@##@\f2tt_error_int\f1\^(\|)\a@106
##@##@\f2tt_error_pointer\f1\^(\|)\a@107
##@##@\f2tt_fd\f1\^(\|)\a@108
##@##@\f2tt_file_copy\f1\^(\|)\a@110
##@##@\f2tt_file_destroy\f1\^(\|)\a@112
##@##@\f2tt_file_join\f1\^(\|)\a@114
##@##@\f2tt_file_move\f1\^(\|)\a@116
##@##@\f2tt_file_netfile\f1\^(\|)\a@118
##@##@\f2tt_file_objects_query\f1\^(\|)\a@120
##@##@\f2tt_file_quit\f1\^(\|)\a@122
##@##@\f2tt_free\f1\^(\|)\a@123
##@##@\f2tt_host_file_netfile\f1\^(\|)\a@124
##@##@\f2tt_host_netfile_file\f1\^(\|)\a@126
##@##@\f2tt_icontext_join\f1\^(\|)\a@128
##@##@\f2tt_icontext_quit\f1\^(\|)\a@129
##@##@\f2tt_initial_session\f1\^(\|)\a@131
##@##@\f2tt_int_error\f1\^(\|)\a@132
##@##@\f2tt_is_err\f1\^(\|)\a@133
##@##@\f2tt_malloc\f1\^(\|)\a@134
##@##@\f2tt_mark\f1\^(\|)\a@135
##@##@\f2tt_message_accept\f1\^(\|)\a@136
##@##@\f2tt_message_address\f1\^(\|)\a@137
##@##@\f2tt_message_address_set\f1\^(\|)\a@139
##@##@\f2tt_message_arg_add\f1\^(\|)\a@141
##@##@\f2tt_message_arg_bval\f1\^(\|)\a@143
##@##@\f2tt_message_arg_bval_set\f1\^(\|)\a@145
##@##@\f2tt_message_arg_ival\f1\^(\|)\a@147
##@##@\f2tt_message_arg_ival_set\f1\^(\|)\a@148
##@##@\f2tt_message_arg_mode\f1\^(\|)\a@150
##@##@\f2tt_message_arg_type\f1\^(\|)\a@152
##@##@\f2tt_message_arg_val\f1\^(\|)\a@154
##@##@\f2tt_message_arg_val_set\f1\^(\|)\a@156
##@##@\f2tt_message_arg_xval\f1\^(\|)\a@157
##@##@\f2tt_message_arg_xval_set\f1\^(\|)\a@159
##@##@\f2tt_message_args_count\f1\^(\|)\a@161
##@##@\f2tt_message_barg_add\f1\^(\|)\a@162
##@##@\f2tt_message_bcontext_set\f1\^(\|)\a@164
##@##@\f2tt_message_callback_add\f1\^(\|)\a@166
##@##@\f2tt_message_class\f1\^(\|)\a@168
##@##@\f2tt_message_class_set\f1\^(\|)\a@170
##@##@\f2tt_message_context_bval\f1\^(\|)\a@172
##@##@\f2tt_message_context_ival\f1\^(\|)\a@174
##@##@\f2tt_message_context_set\f1\^(\|)\a@176
##@##@\f2tt_message_context_slotname\f1\^(\|)\a@177
##@##@\f2tt_message_context_val\f1\^(\|)\a@179
##@##@\f2tt_message_context_xval\f1\^(\|)\a@181
##@##@\f2tt_message_contexts_count\f1\^(\|)\a@183
##@##@\f2tt_message_create\f1\^(\|)\a@184
##@##@\f2tt_message_create_super\f1\^(\|)\a@185
##@##@\f2tt_message_destroy\f1\^(\|)\a@187
##@##@\f2tt_message_disposition\f1\^(\|)\a@188
##@##@\f2tt_message_disposition_set\f1\^(\|)\a@190
##@##@\f2tt_message_fail\f1\^(\|)\a@192
##@##@\f2tt_message_file\f1\^(\|)\a@194
##@##@\f2tt_message_file_set\f1\^(\|)\a@195
##@##@\f2tt_message_gid\f1\^(\|)\a@196
##@##@\f2tt_message_handler\f1\^(\|)\a@197
##@##@\f2tt_message_handler_ptype\f1\^(\|)\a@198
##@##@\f2tt_message_handler_ptype_set\f1\^(\|)\a@199
##@##@\f2tt_message_handler_set\f1\^(\|)\a@200
##@##@\f2tt_message_iarg_add\f1\^(\|)\a@201
##@##@\f2tt_message_icontext_set\f1\^(\|)\a@203
##@##@\f2tt_message_id\f1\^(\|)\a@204
##@##@\f2tt_message_object\f1\^(\|)\a@205
##@##@\f2tt_message_object_set\f1\^(\|)\a@206
##@##@\f2tt_message_op\f1\^(\|)\a@207
##@##@\f2tt_message_op_set\f1\^(\|)\a@208
##@##@\f2tt_message_opnum\f1\^(\|)\a@209
##@##@\f2tt_message_otype\f1\^(\|)\a@210
##@##@\f2tt_message_otype_set\f1\^(\|)\a@211
##@##@\f2tt_message_pattern\f1\^(\|)\a@212
##@##@\f2tt_message_print\f1\^(\|)\a@213
##@##@\f2tt_message_receive\f1\^(\|)\a@214
##@##@\f2tt_message_reject\f1\^(\|)\a@216
##@##@\f2tt_message_reply\f1\^(\|)\a@217
##@##@\f2tt_message_scope\f1\^(\|)\a@218
##@##@\f2tt_message_scope_set\f1\^(\|)\a@220
##@##@\f2tt_message_send\f1\^(\|)\a@222
##@##@\f2tt_message_send_on_exit\f1\^(\|)\a@224
##@##@\f2tt_message_sender\f1\^(\|)\a@226
##@##@\f2tt_message_sender_ptype\f1\^(\|)\a@227
##@##@\f2tt_message_sender_ptype_set\f1\^(\|)\a@228
##@##@\f2tt_message_session\f1\^(\|)\a@229
##@##@\f2tt_message_session_set\f1\^(\|)\a@230
##@##@\f2tt_message_state\f1\^(\|)\a@231
##@##@\f2tt_message_status\f1\^(\|)\a@233
##@##@\f2tt_message_status_set\f1\^(\|)\a@234
##@##@\f2tt_message_status_string\f1\^(\|)\a@235
##@##@\f2tt_message_status_string_set\f1\^(\|)\a@236
##@##@\f2tt_message_uid\f1\^(\|)\a@237
##@##@\f2tt_message_user\f1\^(\|)\a@238
##@##@\f2tt_message_user_set\f1\^(\|)\a@240
##@##@\f2tt_message_xarg_add\f1\^(\|)\a@242
##@##@\f2tt_message_xcontext_join\f1\^(\|)\a@244
##@##@\f2tt_message_xcontext_set\f1\^(\|)\a@245
##@##@\f2tt_netfile_file\f1\^(\|)\a@247
##@##@\f2tt_objid_equal\f1\^(\|)\a@249
##@##@\f2tt_objid_objkey\f1\^(\|)\a@251
##@##@\f2tt_onotice_create\f1\^(\|)\a@252
##@##@\f2tt_open\f1\^(\|)\a@254
##@##@\f2tt_orequest_create\f1\^(\|)\a@256
##@##@\f2tt_otype_base\f1\^(\|)\a@258
##@##@\f2tt_otype_derived\f1\^(\|)\a@259
##@##@\f2tt_otype_deriveds_count\f1\^(\|)\a@260
##@##@\f2tt_otype_hsig_arg_mode\f1\^(\|)\a@261
##@##@\f2tt_otype_hsig_arg_type\f1\^(\|)\a@263
##@##@\f2tt_otype_hsig_args_count\f1\^(\|)\a@265
##@##@\f2tt_otype_hsig_count\f1\^(\|)\a@266
##@##@\f2tt_otype_hsig_op\f1\^(\|)\a@267
##@##@\f2tt_otype_is_derived\f1\^(\|)\a@269
##@##@\f2tt_otype_opnum_callback_add\f1\^(\|)\a@270
##@##@\f2tt_otype_osig_arg_mode\f1\^(\|)\a@272
##@##@\f2tt_otype_osig_arg_type\f1\^(\|)\a@274
##@##@\f2tt_otype_osig_args_count\f1\^(\|)\a@276
##@##@\f2tt_otype_osig_count\f1\^(\|)\a@277
##@##@\f2tt_otype_osig_op\f1\^(\|)\a@278
##@##@\f2tt_pattern_address_add\f1\^(\|)\a@280
##@##@\f2tt_pattern_arg_add\f1\^(\|)\a@282
##@##@\f2tt_pattern_barg_add\f1\^(\|)\a@284
##@##@\f2tt_pattern_bcontext_add\f1\^(\|)\a@286
##@##@\f2tt_pattern_callback_add\f1\^(\|)\a@288
##@##@\f2tt_pattern_category\f1\^(\|)\a@290
##@##@\f2tt_pattern_category_set\f1\^(\|)\a@292
##@##@\f2tt_pattern_class_add\f1\^(\|)\a@294
##@##@\f2tt_pattern_context_add\f1\^(\|)\a@296
##@##@\f2tt_pattern_create\f1\^(\|)\a@298
##@##@\f2tt_pattern_destroy\f1\^(\|)\a@300
##@##@\f2tt_pattern_disposition_add\f1\^(\|)\a@301
##@##@\f2tt_pattern_file_add\f1\^(\|)\a@303
##@##@\f2tt_pattern_iarg_add\f1\^(\|)\a@304
##@##@\f2tt_pattern_icontext_add\f1\^(\|)\a@306
##@##@\f2tt_pattern_object_add\f1\^(\|)\a@308
##@##@\f2tt_pattern_op_add\f1\^(\|)\a@310
##@##@\f2tt_pattern_opnum_add\f1\^(\|)\a@311
##@##@\f2tt_pattern_otype_add\f1\^(\|)\a@312
##@##@\f2tt_pattern_print\f1\^(\|)\a@313
##@##@\f2tt_pattern_register\f1\^(\|)\a@314
##@##@\f2tt_pattern_scope_add\f1\^(\|)\a@316
##@##@\f2tt_pattern_sender_add\f1\^(\|)\a@318
##@##@\f2tt_pattern_sender_ptype_add\f1\^(\|)\a@319
##@##@\f2tt_pattern_session_add\f1\^(\|)\a@320
##@##@\f2tt_pattern_state_add\f1\^(\|)\a@321
##@##@\f2tt_pattern_unregister\f1\^(\|)\a@323
##@##@\f2tt_pattern_user\f1\^(\|)\a@324
##@##@\f2tt_pattern_user_set\f1\^(\|)\a@326
##@##@\f2tt_pattern_xarg_add\f1\^(\|)\a@327
##@##@\f2tt_pattern_xcontext_add\f1\^(\|)\a@329
##@##@\f2tt_pnotice_create\f1\^(\|)\a@331
##@##@\f2tt_pointer_error\f1\^(\|)\a@333
##@##@\f2tt_prequest_create\f1\^(\|)\a@334
##@##@\f2tt_ptr_error\f1\^(\|)\a@336
##@##@\f2tt_ptype_declare\f1\^(\|)\a@337
##@##@\f2tt_ptype_exists\f1\^(\|)\a@338
##@##@\f2tt_ptype_opnum_callback_add\f1\^(\|)\a@339
##@##@\f2tt_ptype_undeclare\f1\^(\|)\a@341
##@##@\f2tt_release\f1\^(\|)\a@342
##@##@\f2tt_session_bprop\f1\^(\|)\a@343
##@##@\f2tt_session_bprop_add\f1\^(\|)\a@345
##@##@\f2tt_session_bprop_set\f1\^(\|)\a@347
##@##@\f2tt_session_join\f1\^(\|)\a@349
##@##@\f2tt_session_prop\f1\^(\|)\a@350
##@##@\f2tt_session_prop_add\f1\^(\|)\a@352
##@##@\f2tt_session_prop_count\f1\^(\|)\a@354
##@##@\f2tt_session_prop_set\f1\^(\|)\a@355
##@##@\f2tt_session_propname\f1\^(\|)\a@357
##@##@\f2tt_session_propnames_count\f1\^(\|)\a@359
##@##@\f2tt_session_quit\f1\^(\|)\a@360
##@##@\f2tt_session_types_load\f1\^(\|)\a@361
##@##@\f2tt_spec_bprop\f1\^(\|)\a@362
##@##@\f2tt_spec_bprop_add\f1\^(\|)\a@364
##@##@\f2tt_spec_bprop_set\f1\^(\|)\a@366
##@##@\f2tt_spec_create\f1\^(\|)\a@368
##@##@\f2tt_spec_destroy\f1\^(\|)\a@370
##@##@\f2tt_spec_file\f1\^(\|)\a@371
##@##@\f2tt_spec_move\f1\^(\|)\a@373
##@##@\f2tt_spec_prop\f1\^(\|)\a@375
##@##@\f2tt_spec_prop_add\f1\^(\|)\a@377
##@##@\f2tt_spec_prop_count\f1\^(\|)\a@379
##@##@\f2tt_spec_prop_set\f1\^(\|)\a@381
##@##@\f2tt_spec_propname\f1\^(\|)\a@383
##@##@\f2tt_spec_propnames_count\f1\^(\|)\a@385
##@##@\f2tt_spec_type\f1\^(\|)\a@387
##@##@\f2tt_spec_type_set\f1\^(\|)\a@389
##@##@\f2tt_spec_write\f1\^(\|)\a@391
##@##@\f2tt_status_message\f1\^(\|)\a@393
##@##@\f2tt_trace_control\f1\^(\|)\a@394
##@##@\f2tt_xcontext_quit\f1\^(\|)\a@395
##@##@\f2ttdt_Get_Modified\f1\^(\|)\a@396
##@##@\f2ttdt_Revert\f1\^(\|)\a@398
##@##@\f2ttdt_Save\f1\^(\|)\a@400
##@##@\f2ttdt_close\f1\^(\|)\a@402
##@##@\f2ttdt_file_event\f1\^(\|)\a@403
##@##@\f2ttdt_file_join\f1\^(\|)\a@405
##@##@\f2ttdt_file_notice\f1\^(\|)\a@409
##@##@\f2ttdt_file_quit\f1\^(\|)\a@411
##@##@\f2ttdt_file_request\f1\^(\|)\a@413
##@##@\f2ttdt_message_accept\f1\^(\|)\a@415
##@##@\f2ttdt_open\f1\^(\|)\a@418
##@##@\f2ttdt_sender_imprint_on\f1\^(\|)\a@419
##@##@\f2ttdt_session_join\f1\^(\|)\a@422
##@##@\f2ttdt_session_quit\f1\^(\|)\a@427
##@##@\f2ttdt_subcontract_manage\f1\^(\|)\a@428
##@##@\f2ttmedia_Deposit\f1\^(\|)\a@430
##@##@\f2ttmedia_load\f1\^(\|)\a@432
##@##@\f2ttmedia_load_reply\f1\^(\|)\a@436
##@##@\f2ttmedia_ptype_declare\f1\^(\|)\a@438
##@##@\f2tttk_Xt_input_handler\f1\^(\|)\a@444
##@##@\f2tttk_block_while\f1\^(\|)\a@446
##@##@\f2tttk_message_abandon\f1\^(\|)\a@448
##@##@\f2tttk_message_create\f1\^(\|)\a@450
##@##@\f2tttk_message_destroy\f1\^(\|)\a@452
##@##@\f2tttk_message_fail\f1\^(\|)\a@453
##@##@\f2tttk_message_reject\f1\^(\|)\a@454
##@##@\f2tttk_op_string\f1\^(\|)\a@455
##@##@\f2tttk_string_op\f1\^(\|)\a@456
##@6.3@###Headers\a@457
##@##@\f3<Tt/tt_c.h>\f1\a@458
##@##@\f3<Tt/tttk.h>\f1\a@474
##@6.4@###Command-Line Interfaces\a@479
##@##@\f2tt_type_comp\f1\^\a@480
##@##@\f2ttcp\f1\^\a@485
##@##@\f2ttmv\f1\^\a@489
##@##@\f2ttrm\f1\^\a@492
##@##@\f2ttrmdir\f1\^\a@495
##@##@\f2ttsession\f1\^\a@498
##@##@\f2tttar\f1\^\a@505
##@6.5@###Data Formats\a@514
##@6.5.1@######Defining Process Types\a@514
##@6.5.2@######Defining Object Types\a@518
##@6.6@###Protocol Message Sets\a@522
##@6.6.1@######Desktop Message Set\a@523
##@##@\f2Get_Environment\f1\^\a@525
##@##@\f2Get_Geometry\f1\^\a@526
##@##@\f2Get_Iconified\f1\^\a@527
##@##@\f2Get_Locale\f1\^\a@528
##@##@\f2Get_Mapped\f1\^\a@530
##@##@\f2Get_Modified\f1\^\a@531
##@##@\f2Get_Situation\f1\^\a@533
##@##@\f2Get_Status\f1\^\a@534
##@##@\f2Get_Sysinfo\f1\^\a@536
##@##@\f2Get_XInfo\f1\^\a@538
##@##@\f2Lower\f1\^\a@540
##@##@\f2Modified\f1\^\a@541
##@##@\f2Pause\f1\^\a@542
##@##@\f2Quit\f1\^\a@544
##@##@\f2Raise\f1\^\a@546
##@##@\f2Resume\f1\^\a@547
##@##@\f2Revert\f1\^\a@548
##@##@\f2Reverted\f1\^\a@549
##@##@\f2Save\f1\^\a@550
##@##@\f2Saved\f1\^\a@551
##@##@\f2Set_Environment\f1\^\a@552
##@##@\f2Set_Geometry\f1\^\a@553
##@##@\f2Set_Iconified\f1\^\a@555
##@##@\f2Set_Locale\f1\^\a@557
##@##@\f2Set_Mapped\f1\^\a@559
##@##@\f2Set_Situation\f1\^\a@560
##@##@\f2Signal\f1\^\a@561
##@##@\f2Started\f1\^\a@562
##@##@\f2Status\f1\^\a@563
##@##@\f2Stopped\f1\^\a@565
##@6.6.2@######Media Exchange Message Set\a@566
##@##@\f2Deposit\f1\^\a@568
##@##@\f2Display\f1\^\a@570
##@##@\f2Edit\f1\^\a@572
##@##@\f2Mail\f1\^\a@575
##@##@\f2Print\f1\^\a@577
##@##@\f2Translate\f1\^\a@580
##@6.7@###Rationale and Open Issues\a@583
.sp 1v
\s+2\f3Chapter@7@Drag and Drop\s0\a@587\f1
##@7.1@###Introduction\a@587
##@7.2@###Functions\a@587
##@##@\f2DtDndCreateSourceIcon\f1\^(\|)\a@588
##@##@\f2DtDndDragStart\f1\^(\|)\a@590
##@##@\f2DtDndDropRegister\f1\^(\|)\a@598
##@7.3@###Headers\a@606
##@##@\f3<Dt/Dnd.h>\f1\a@607
##@7.4@###Protocols\a@610
##@7.4.1@######Protocol Overview\a@610
##@7.4.1.1@#########Drag and Drop API Protocol\a@610
##@7.4.1.2@#########Export/Import Targets\a@610
##@7.4.1.3@#########Data Transfer Protocol\a@611
##@7.4.1.4@#########Move Completion\a@611
##@7.4.2@######Text Transfer Protocol\a@611
##@7.4.2.1@#########Drag and Drop API\a@611
##@7.4.2.2@#########Export/Import Targets\a@611
##@7.4.2.3@#########Data Transfer Protocol\a@612
##@7.4.2.4@#########Move Completion\a@612
##@7.4.3@######File Name Transfer Protocol\a@612
##@7.4.3.1@#########Drag and Drop API\a@612
##@7.4.3.2@#########Export/Import Targets\a@612
##@7.4.3.3@#########Data Transfer Protocol\a@612
##@7.4.3.4@#########Move Completion\a@613
##@7.4.4@######Buffer Transfer Protocol\a@613
##@7.4.4.1@#########Drag and Drop API\a@613
##@7.4.4.2@#########Export/Import Targets\a@613
##@7.4.4.3@#########Data Transfer Protocol\a@613
##@7.4.4.4@#########Move Completion\a@614
##@7.4.5@######Selection Targets\a@614
##@7.5@###Rationale and Open Issues\a@615
.sp 1v
\s+2\f3Chapter@8@Data Typing\s0\a@617\f1
##@8.1@###Introduction\a@617
##@8.2@###Functions\a@617
##@##@\f2DtDtsBufferToAttributeList\f1\^(\|)\a@618
##@##@\f2DtDtsBufferToAttributeValue\f1\^(\|)\a@620
##@##@\f2DtDtsBufferToDataType\f1\^(\|)\a@622
##@##@\f2DtDtsDataToDataType\f1\^(\|)\a@624
##@##@\f2DtDtsDataTypeIsAction\f1\^(\|)\a@626
##@##@\f2DtDtsDataTypeNames\f1\^(\|)\a@627
##@##@\f2DtDtsDataTypeToAttributeList\f1\^(\|)\a@628
##@##@\f2DtDtsDataTypeToAttributeValue\f1\^(\|)\a@629
##@##@\f2DtDtsFileToAttributeList\f1\^(\|)\a@631
##@##@\f2DtDtsFileToAttributeValue\f1\^(\|)\a@632
##@##@\f2DtDtsFileToDataType\f1\^(\|)\a@634
##@##@\f2DtDtsFindAttribute\f1\^(\|)\a@635
##@##@\f2DtDtsFreeAttributeList\f1\^(\|)\a@636
##@##@\f2DtDtsFreeAttributeValue\f1\^(\|)\a@637
##@##@\f2DtDtsFreeDataType\f1\^(\|)\a@638
##@##@\f2DtDtsFreeDataTypeNames\f1\^(\|)\a@639
##@##@\f2DtDtsIsTrue\f1\^(\|)\a@640
##@##@\f2DtDtsLoadDataTypes\f1\^(\|)\a@641
##@##@\f2DtDtsRelease\f1\^(\|)\a@642
##@##@\f2DtDtsSetDataType\f1\^(\|)\a@643
##@8.3@###Headers\a@644
##@##@\f3<Dt/Dts.h>\f1\a@645
##@8.4@###Data Formats\a@647
##@8.4.1@######Location of Actions and Data Types Database\a@647
##@8.4.2@######Data Types and Actions Database Syntax\a@648
##@8.4.2.1@#########Comments\a@648
##@8.4.2.2@#########Database Version\a@648
##@8.4.2.3@#########String Variables\a@649
##@8.4.2.4@#########Environment Variables\a@649
##@8.4.2.5@#########Line Continuation\a@649
##@8.4.2.6@#########Record Name\a@649
##@8.4.2.7@#########Record Delimiters\a@650
##@8.4.2.8@#########Fields\a@650
##@8.4.2.9@#########Record Types\a@650
##@8.4.3@######Data Criteria Records\a@650
##@8.4.3.1@#########NAME_PATTERN Field\a@651
##@8.4.3.2@#########PATH_PATTERN Field\a@651
##@8.4.3.3@#########CONTENT Field\a@651
##@8.4.3.4@#########MODE Field\a@652
##@8.4.3.5@#########LINK_NAME Field\a@653
##@8.4.3.6@#########LINK_PATH Field\a@654
##@8.4.3.7@#########DATA_ATTRIBUTES_NAME Field\a@654
##@8.4.3.8@#########Logical Expressions\a@654
##@8.4.3.9@#########White Space\a@654
##@8.4.3.10@#########Escape Character\a@654
##@8.4.3.11@#########Data Criteria Format\a@655
##@8.4.3.12@#########Data Criteria Sorting\a@656
##@8.4.4@######Data Attribute Records\a@658
##@8.4.4.1@#########DESCRIPTION Field\a@658
##@8.4.4.2@#########ICON Field\a@658
##@8.4.4.3@#########INSTANCE_ICON Field\a@659
##@8.4.4.4@#########PROPERTIES Field\a@659
##@8.4.4.5@#########ACTIONS Field\a@659
##@8.4.4.6@#########NAME_TEMPLATE Field\a@660
##@8.4.4.7@#########IS_EXECUTABLE Field\a@660
##@8.4.4.8@#########MOVE_TO_ACTION Field\a@660
##@8.4.4.9@#########COPY_TO_ACTION Field\a@661
##@8.4.4.10@#########LINK_TO_ACTION Field\a@661
##@8.4.4.11@#########IS_TEXT Field\a@661
##@8.4.4.12@#########MEDIA Field\a@663
##@8.4.4.13@#########MIME_TYPE Field\a@664
##@8.4.4.14@#########X400_TYPE Field\a@664
##@8.4.4.15@#########DATA_HOST Attribute\a@664
##@8.4.4.16@#########Modifiers\a@664
##@8.4.4.17@#########Data Attributes Format\a@665
##@8.4.4.18@#########Examples\a@665
.sp 1v
\s+2\f3Chapter@9@Execution Management\s0\a@667\f1
##@9.1@###Introduction\a@667
##@9.1.1@######Scope\a@667
##@9.1.2@######Components\a@667
##@9.1.3@######Action Database Entries\a@668
##@9.1.4@######Command-Line Actions\a@668
##@9.2@###Functions\a@669
##@##@\f2DtActionCallbackProc\f1\^(\|)\a@670
##@##@\f2DtActionDescription\f1\^(\|)\a@675
##@##@\f2DtActionExists\f1\^(\|)\a@676
##@##@\f2DtActionIcon\f1\^(\|)\a@677
##@##@\f2DtActionInvoke\f1\^(\|)\a@678
##@##@\f2DtActionLabel\f1\^(\|)\a@686
##@##@\f2DtDbLoad\f1\^(\|)\a@687
##@##@\f2DtDbReloadNotify\f1\^(\|)\a@688
##@9.3@###Headers\a@689
##@##@\f3<Dt/Action.h>\f1\a@690
##@9.4@###Command-Line Interfaces\a@692
##@##@\f2dtaction\f1\^\a@693
##@9.5@###Data Formats\a@697
##@9.5.1@######Action File Syntax\a@697
##@9.5.2@######Classes of Actions\a@697
##@9.5.2.1@#########Command Actions\a@697
##@9.5.2.2@#########ToolTalk Message Actions\a@698
##@9.5.2.3@#########Map Actions\a@698
##@9.5.2.4@#########Common Fields\a@698
##@9.5.2.5@#########Keywords\a@698
##@9.5.2.6@#########Argument References\a@699
##@9.5.2.7@#########Action Selection\a@700
##@9.5.2.8@#########ARG_CLASS Field\a@701
##@9.5.2.9@#########ARG_COUNT Field \a@702
##@9.5.2.10@#########ARG_MODE Field\a@702
##@9.5.2.11@#########ARG_TYPE Field\a@703
##@9.5.2.12@#########CWD Field\a@703
##@9.5.2.13@#########DESCRIPTION Field\a@704
##@9.5.2.14@#########EXEC_HOST Field\a@704
##@9.5.2.15@#########EXEC_STRING Field\a@704
##@9.5.2.16@#########ICON Field\a@705
##@9.5.2.17@#########LABEL Field\a@705
##@9.5.2.18@#########MAP_ACTION Field\a@705
##@9.5.2.19@#########TERM_OPTS Field\a@706
##@9.5.2.20@#########TT_ARGn_MODE Field\a@706
##@9.5.2.21@#########TT_ARGn_REP_TYPE Field\a@706
##@9.5.2.22@#########TT_ARGn_VALUE Field\a@707
##@9.5.2.23@#########TT_ARGn_VTYPE Field\a@707
##@9.5.2.24@#########TT_CLASS Field\a@708
##@9.5.2.25@#########TT_FILE Field\a@708
##@9.5.2.26@#########TT_OPERATION Field\a@708
##@9.5.2.27@#########TT_SCOPE Field\a@709
##@9.5.2.28@#########TYPE Field\a@709
##@9.5.2.29@#########WINDOW_TYPE Field\a@709
##@9.5.3@######Resources\a@710
##@9.5.4@######Examples\a@712
##@9.5.5@######Application Usage\a@713
##@9.6@###Rationale and Open Issues\a@714
.sp 1v
##@##@\s+2\f3Index\s0\a@715\f1
.TE
.tr ##
.eF e
