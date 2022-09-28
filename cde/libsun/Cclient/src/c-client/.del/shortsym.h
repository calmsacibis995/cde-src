/*
 * Program:	Definitions for compilers with 6-letter symbol limits
 *
 * Author:	Mark Crispin
 *		6158 Lariat Loop NE
 *		Bainbridge Island, WA  98110-2098
 *		Internet: MRC@Panda.COM
 *
 * Date:	24 May 1995
 * Last Edited:	9 June 1995
 *
 * Copyright 1995 by Mark Crispin
 *
 *  Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notices appear in all copies and that both the
 * above copyright notices and this permission notice appear in supporting
 * documentation, and that the name of Mark Crispin not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  This software is made available "as is", and
 * MARK CRISPIN DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD TO
 * THIS SOFTWARE, INCLUDING WITHOUT LIMITATION ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE, AND IN NO EVENT SHALL
 * MARK CRISPIN BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES
 * OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
 * WHETHER IN AN ACTION OF CONTRACT, TORT (INCLUDING NEGLIGENCE) OR STRICT
 * LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 *
 */

#include "UnixDefs.h"

#define authenticate auth
#define default_proto d_prot
#define env_parameters e_parm
#define fatal fatal
#define fs_get f_get
#define fs_give f_give
#define fs_resize f_resize
#define imap_OK i_OK
#define imap_append i_appn
#define imap_check i_chck
#define imap_clearflag i_clfg
#define imap_close i_clos
#define imap_copy i_copy
#define imap_create i_crea
#define imap_delete i_del
#define imap_expunge i_expn
#define imap_expunged i_expd
#define imap_fake i_fake
#define imap_fetchbody if_bdy
#define imap_fetchfast if_fst
#define imap_fetchflags if_flg
#define imap_fetchheader if_hed
#define imap_fetchstructure if_str
#define imap_fetchtext if_txt
#define imap_gc i_gc
#define imap_gc_body ig_bdy
#define imap_host i_host
#define imap_list i_list
#define imap_lsub i_lsub
#define imap_manage i_man
#define imap_open i_open
#define imap_parameters i_parm
#define imap_parse_address ip_addr
#define imap_parse_adrlist ip_adrl
#define imap_parse_body ip_body
#define imap_parse_body_parameter ipb_pa
#define imap_parse_body_structure ipb_st
#define imap_parse_data ip_data
#define imap_parse_envelope ip_env
#define imap_parse_extension ip_ext
#define imap_parse_flags ip_flag
#define imap_parse_number ip_numb
#define imap_parse_prop ip_prop
#define imap_parse_reply ip_rep
#define imap_parse_string ip_str
#define imap_parse_unsolicited ip_uns
#define imap_parse_user_flag ipu_fl
#define imap_ping i_ping
#define imap_rename i_ren
#define imap_reply i_rep
#define imap_search i_srch
#define imap_send i_send
#define imap_send_astring is_ast
#define imap_send_literal is_lit
#define imap_send_list is_lst
#define imap_send_sdate iss_da
#define imap_send_slist iss_sl
#define imap_send_spgm iss_pg
#define imap_send_sset iss_st
#define imap_setflag i_sflg
#define imap_sout i_sout
#define imap_subscribe i_sub
#define imap_uid i_uid
#define imap_unsubscribe i_uns
#define imap_valid i_val
#define internal_date in_dat
#define mail_append_full m_appn
#define mail_cdate m_cdat
#define mail_check m_chck
#define mail_clearflag_full m_cflg
#define mail_close_full m_clos
#define mail_copy_full m_copy
#define mail_create m_crea
#define mail_criteria m_crit
#define mail_criteria_date mc_dat
#define mail_criteria_string mc_str
#define mail_date m_date
#define mail_debug m_dbug
#define mail_delete m_del
#define mail_elt m_elt
#define mail_exists m_exst
#define mail_expunge m_expn
#define mail_expunged m_expd
#define mail_fetchbody_full mf_bdy
#define mail_fetchfast_full mf_fst
#define mail_fetchflags_full mf_flg
#define mail_fetchfrom mf_frm
#define mail_fetchheader_full mf_hed
#define mail_fetchstructure_full mf_str
#define mail_fetchsubject mf_sub
#define mail_fetchtext_full mf_txt
#define mail_filter m_filt
#define mail_find m_find
#define mail_find_all m_fall
#define mail_free_address mr_add
#define mail_free_body mr_bdy
#define mail_free_body_data mrb_da
#define mail_free_body_parameter mrb_pr
#define mail_free_body_part mrb_pt
#define mail_free_cache mr_cac
#define mail_free_elt mr_elt
#define mail_free_envelope mr_env
#define mail_free_handle mr_han
#define mail_free_lelt mr_lel
#define mail_free_searchheader mrs_hd
#define mail_free_searchor mrs_or
#define mail_free_searchpgm mrs_pg
#define mail_free_searchpgmlist mrs_pl
#define mail_free_searchset mrs_st
#define mail_free_stringlist mr_sl
#define mail_gc m_gc
#define mail_initbody m_ibdy
#define mail_lelt m_lelt
#define mail_link m_link
#define mail_list m_list
#define mail_lock m_lock
#define mail_lsub m_lsub
#define mail_makehandle m_mhdl
#define mail_newaddr mn_add
#define mail_newbody mn_bdy
#define mail_newbody_parameter mnb_pr
#define mail_newbody_part mnb_pt
#define mail_newenvelope mn_env
#define mail_newsearchheader mns_hd
#define mail_newsearchor mns_or
#define mail_newsearchpgm mns_pg
#define mail_newsearchpgmlist mns_pl
#define mail_newsearchset mns_st
#define mail_newstringlist mn_sl
#define mail_nodebug m_ndbg
#define mail_open m_open
#define mail_parameters m_parm
#define mail_parse_date mp_dat
#define mail_parse_flags mp_flg
#define mail_ping m_ping
#define mail_recent m_rcent
#define mail_rename m_ren
#define mail_search_addr ms_addr
#define mail_search_full m_srch
#define mail_search_keyword ms_key
#define mail_search_msg ms_msg
#define mail_search_string ms_str
#define mail_search_text ms_txt
#define mail_sequence m_seq
#define mail_setflag_full m_sflg
#define mail_stream m_strm
#define mail_string m_strg
#define mail_string_init mt_ini
#define mail_string_next mt_nxt
#define mail_string_setpos mt_sps
#define mail_subscribe m_sub
#define mail_uid m_uid
#define mail_uid_sequence mu_seq
#define mail_unlock m_unl
#define mail_unsubscribe m_uns
#define mail_valid m_val
#define mail_valid_net mv_net
#define mail_valid_net_parse mvn_pr
#define mailboxfile mbxfil
#define mm_cache mm_cac
#define mm_critical mm_crt
#define mm_diskerror mm_dse
#define mm_dlog mm_dlg
#define mm_exists mm_exs
#define mm_expunged mm_exp
#define mm_fatal mm_ftl
#define mm_flags mm_flg
#define mm_gets mm_gts
#define mm_list mm_lst
#define mm_log mm_log
#define mm_login mm_lgi
#define mm_lsub mm_lsb
#define mm_mailbox mm_mbx
#define mm_nocritical mm_ncr
#define mm_notify mm_not
#define mm_searched mm_src
#define myhomedir myhome
#define mylocalhost myhost
#define netmsg_read nm_rea
#define netmsg_slurp nm_slr
#define netmsg_slurp_text nm_slt
#define netmsg_string nm_str
#define netmsg_string_init nm_ini
#define netmsg_string_next nm_nxt
#define netmsg_string_setpos nm_sps
#define newsrc_create ns_crea
#define newsrc_error ns_err
#define newsrc_lsub ns_lsub
#define newsrc_newmessages ns_nms
#define newsrc_newstate ns_nst
#define newsrc_read ns_rea
#define newsrc_update ns_upd
#define newsrc_write ns_wri
#define newsrc_write_error ns_wer
#define nntp_append n_appn
#define nntp_canonicalize n_cano
#define nntp_check n_chck
#define nntp_clearflag n_cflg
#define nntp_close n_clos
#define nntp_copy n_copy
#define nntp_create n_crea
#define nntp_delete n_del
#define nntp_expunge n_expn
#define nntp_fake n_fake
#define nntp_fetchbody nf_bdy
#define nntp_fetchfast nf_fst
#define nntp_fetchflags nf_flg
#define nntp_fetchheader nf_hdr
#define nntp_fetchstructure nf_str
#define nntp_fetchtext nf_txt
#define nntp_fetchtext_work nft_wk
#define nntp_gc n_gc
#define nntp_list n_list
#define nntp_lsub n_lsub
#define nntp_mail n_mail
#define nntp_mclose n_mcls
#define nntp_mopen n_mopn
#define nntp_open n_open
#define nntp_parameters n_parm
#define nntp_ping n_ping
#define nntp_rename n_ren
#define nntp_reply n_repl
#define nntp_send n_send
#define nntp_setflag n_sflg
#define nntp_soutr n_sout
#define nntp_subscribe n_sub
#define nntp_unsubscribe n_uns
#define nntp_valid n_val
#define rfc822_8bit r
#define rfc822_address r_addr
#define rfc822_address_line ra_lin
#define rfc822_base64 r_b64
#define rfc822_binary r_bin
#define rfc822_cat r_cat
#define rfc822_contents r_cont
#define rfc822_cpy r_cpy
#define rfc822_cpy_adr rc_adr
#define rfc822_date r_date
#define rfc822_default_subtype rd_sub
#define rfc822_encode_body_7bit reb_7b
#define rfc822_encode_body_8bit reb_8b
#define rfc822_header r_head
#define rfc822_header_line rh_lin
#define rfc822_output r_out
#define rfc822_output_body ro_bdy
#define rfc822_parse_address rp_adr
#define rfc822_parse_addrspec rp_ads
#define rfc822_parse_adrlist rp_adl
#define rfc822_parse_content rp_cnt
#define rfc822_parse_content_header rpc_hd
#define rfc822_parse_msg rp_msg
#define rfc822_parse_phrase rp_phr
#define rfc822_parse_routeaddr rp_rte
#define rfc822_parse_word rp_wrd
#define rfc822_qprint r_qpnt
#define rfc822_quote r_quot
#define rfc822_skip_comment rs_cmt
#define rfc822_skipws rs_ws
#define rfc822_timezone r_tz
#define rfc822_write_address rw_adr
#define rfc822_write_body_header rwbh_8
#define server_login s_log
#define sm_read sm_rd
#define sm_subscribe sm_sub
#define sm_unsubscribe sm_uns
#define smtp_close s_clos
#define smtp_debug s_dbug
#define smtp_ehlo s_ehlo
#define smtp_fake s_fake
#define smtp_mail s_mail
#define smtp_nodebug s_nodb
#define smtp_open s_open
#define smtp_rcpt s_rcpt
#define smtp_reply s_repl
#define smtp_send s_send
#define smtp_soutr s_str
#define strcrlfcpy sc_cpy
#define strcrlflen sc_len
#define tcp_aopen t_aopn
#define tcp_clienthost t_chst
#define tcp_close t_clos
#define tcp_getbuffer tg_buf
#define tcp_getdata tg_dat
#define tcp_getline tg_lin
#define tcp_host t_host
#define tcp_localhost t_lhst
#define tcp_open t_open
#define tcp_parameters t_parameters
#define tcp_sout t_sout
#define tcp_soutr t_str
