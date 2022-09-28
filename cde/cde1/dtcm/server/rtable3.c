/* $XConsortium: rtable3.c /main/cde1_maint/2 1995/10/10 13:35:59 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)rtable3.c	1.6 97/02/10 Sun Microsystems, Inc."

/*
  veneer layered on top of the real data structures for abstraction.
	implements Version 3 in terms of Version 4 types	
 */



#include <stdio.h>
#include "rtable4.h"
#include "rtable3.h"
#include <sys/param.h>
#include <sys/time.h>
#include <rpc/rpc.h>
#include "rpcextras.h"
#include "convert3-4.h"
#include "convert4-3.h"
#include "rtable3_tbl.i"


/*************** V3 PROTOCOL IMPLEMENTATION PROCS *****************/
bool_t
_DtCm_rtable_ping_3_svc(void * args, void * res, struct svc_req * svcrq)
{
	res = args;
	return(1); /* for RPC reply */
}

/*	PROC #1		*/
bool_t
_DtCm_rtable_lookup_3_svc (Table_Args_3 * args,
			   Table_Res_3 *  res,
			   struct svc_req *svcrq)
{
	Table_Args_4 *newargs;
	Table_Res_4 *newres;

	newargs = _DtCm_tableargs3_to_tableargs4(args);
	_DtCm_rtable_lookup_4_svc(newargs, newres, svcrq); 
	res = _DtCm_tableres4_to_tableres3(newres);

	if (newargs != NULL) {
	  xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);
	}

	return(1);
}

/*	PROC #2		*/
bool_t
_DtCm_rtable_lookup_next_larger_3_svc(Table_Args_3 *args, Table_Res_3 *res, struct svc_req *svcrq)
{
	Table_Args_4 *newargs;   
	Table_Res_4 *newres; 
  
	newargs = _DtCm_tableargs3_to_tableargs4(args);
	_DtCm_rtable_lookup_next_larger_4_svc(newargs, newres, svcrq);
	res = _DtCm_tableres4_to_tableres3(newres);
 
	if (newargs != NULL) {
	  xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);
	}
 
	return(1);
}

/*	PROC #3		*/
bool_t
_DtCm_rtable_lookup_next_smaller_3_svc(Table_Args_3 *args,
				       Table_Res_3 *res,
				       struct svc_req *svcrq)
{
	Table_Args_4 *newargs;    
	Table_Res_4 *newres; 
  
	newargs = _DtCm_tableargs3_to_tableargs4(args);
	_DtCm_rtable_lookup_next_smaller_4_svc(newargs, newres, svcrq); 
	res = _DtCm_tableres4_to_tableres3(newres);

	if (newargs != NULL) {
	  xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);
	}

	return(1);
}

/*	PROC #4		*/
bool_t
_DtCm_rtable_lookup_range_3_svc(Table_Args_3 *args,
				Table_Res_3 *res,
				struct svc_req *svcrq)
{
        Table_Args_4 *newargs;     
        Table_Res_4 *newres; 
   
        newargs = _DtCm_tableargs3_to_tableargs4(args); 
        _DtCm_rtable_lookup_range_4_svc(newargs, newres, svcrq);  
        res = _DtCm_tableres4_to_tableres3(newres); 

        if (newargs != NULL) {
	  xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs); 
	}

        return(1);
}

/*	PROC #5		*/
bool_t
_DtCm_rtable_abbreviated_lookup_range_3_svc(Table_Args_3 *args,
					    Table_Res_3 *res,
					    struct svc_req *svcrq)
{
        Table_Args_4 *newargs;      
        Table_Res_4 *newres; 

        newargs = _DtCm_tableargs3_to_tableargs4(args);  
        _DtCm_rtable_abbreviated_lookup_range_4_svc(newargs, newres, svcrq);  
        res = _DtCm_tableres4_to_tableres3(newres);  

        if (newargs != NULL) {
	  xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);  
	}

        return(1);
}

/*	PROC #6		*/
bool_t
_DtCm_rtable_insert_3_svc(Table_Args_3 *args,
			  Table_Res_3 *res,
			  struct svc_req *svcrq)
{
        Table_Args_4 *newargs;
        Table_Res_4 *newres;

        newargs = _DtCm_tableargs3_to_tableargs4(args);
        _DtCm_rtable_insert_4_svc(newargs, newres, svcrq);
        res = _DtCm_tableres4_to_tableres3(newres);

        if (newargs != NULL) {
	  xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);
	}

        return(1);
}

/*	PROC #7	*/
bool_t
_DtCm_rtable_delete_3_svc(Table_Args_3 *args,
			  Table_Res_3 *res,
			  struct svc_req *svcrq)
{
        Table_Args_4 *newargs;
        Table_Res_4 *newres; 

	newargs = _DtCm_tabledelargs3_to_tabledelargs4(args, do_all_4);
        _DtCm_rtable_delete_4_svc(newargs, newres, svcrq); 
        res = _DtCm_tableres4_to_tableres3(newres); 

        if (newargs != NULL) {
	  xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs); 
	}

        return(1); 
}

/*	PROC #8		*/
bool_t
_DtCm_rtable_delete_instance_3_svc(Table_Args_3 *args,
				   Table_Res_3 *res,
				   struct svc_req *svcrq)
{
        Table_Args_4 *newargs;  
        Table_Res_4 *newres;  

	newargs = _DtCm_tabledelargs3_to_tabledelargs4(args, do_one_4);
        _DtCm_rtable_delete_4_svc(newargs, newres, svcrq);  
        res = _DtCm_tableres4_to_tableres3(newres);   

        if (newargs != NULL) {
	  xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);   
	}
       
        return(1);
}

/*	PROC #9	*/
bool_t
_DtCm_rtable_change_3_svc(Table_Args_3 *args,
			  Table_Res_3 *res,
			  struct svc_req *svcrq)
{
        Table_Args_4 *newargs; 
        Table_Res_4 *newres;  

        newargs = _DtCm_tableargs3_to_tableargs4(args); 
        _DtCm_rtable_change_4_svc(newargs, newres, svcrq); 
        res = _DtCm_tableres4_to_tableres3(newres);  

        if (newargs != NULL) {
	  xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);  
	}

        return(1); 
}


/*	PROC #10	*/
bool_t
_DtCm_rtable_change_instance_3_svc(Table_Args_3 *args,
				   Table_Res_3 *res,
				   struct svc_req *svcrq)
{
        Table_Args_4 *newargs;   
        Table_Res_4 *newres;  

        newargs = _DtCm_tableargs3_to_tableargs4(args);  
	newargs->args.Args_4_u.apptid.option = do_one_4;
        _DtCm_rtable_change_4_svc(newargs, newres, svcrq);  
        res = _DtCm_tableres4_to_tableres3(newres);   

        if (newargs != NULL) {
	  xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);   
	}

        return(1);
}

/*	PROC #11	*/
bool_t
_DtCm_rtable_lookup_next_reminder_3_svc(Table_Args_3 *args,
					Table_Res_3 *res,
					struct svc_req *svcrq)
{
        Table_Args_4 *newargs;    
        Table_Res_4 *newres;   

        newargs = _DtCm_tableargs3_to_tableargs4(args);   
        _DtCm_rtable_lookup_next_reminder_4_svc(newargs, newres, svcrq);   
        res = _DtCm_tableres4_to_tableres3(newres);   

        if (newargs != NULL) {
	  xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);    
	}

        return(1); 
}

/*	PROC #12	*/
bool_t
_DtCm_rtable_check_3_svc(Table_Args_3 *args,
			 Table_Status_3 *res,
			 struct svc_req *svcrq)
{
        Table_Args_4 *newargs;     
        Table_Status_4 *newres;    

        newargs = _DtCm_tableargs3_to_tableargs4(args);    
        _DtCm_rtable_check_4_svc(newargs, newres, svcrq);    
        *res = _DtCm_tablestat4_to_tablestat3(*newres);   

        if (newargs != NULL) {
	  xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);     
	}

        return(1); 
}

/*	PROC #13	*/
bool_t
_DtCm_rtable_flush_table_3_svc(Table_Args_3 *args,
			       Table_Status_3 *res,
			       struct svc_req *svcrq)
{
        Table_Args_4 *newargs;     
        Table_Status_4 *newres;     

        newargs = _DtCm_tableargs3_to_tableargs4(args);     
        _DtCm_rtable_flush_table_4_svc(newargs, newres, svcrq);     
        *res = _DtCm_tablestat4_to_tablestat3(*newres);    

        if (newargs != NULL) {
	  xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);      
	}

        return(1);
}

/*	PROC #14	*/
bool_t
_DtCm_rtable_size_3_svc(Table_Args_3 *args,
			int *res,
			struct svc_req *svcrq)
{
        Table_Args_4 *newargs;      

        newargs = _DtCm_tableargs3_to_tableargs4(args);     
        _DtCm_rtable_size_4_svc(newargs, res, svcrq);    

        if (newargs != NULL) {
	  xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);       
	}

        return(1);
}

/*	PROC #15	*/
bool_t
_DtCm_register_callback_3_svc(Registration_3 *r,
			      Registration_Status_3 *res,
			      struct svc_req *svcrq)
{
        Registration_4 *newreg;       
	Registration_Status_4 *newstat;

        newreg = _DtCm_reg3_to_reg4(r);      
        _DtCm_register_callback_4_svc(newreg, newstat, svcrq);      
        *res = _DtCm_regstat4_to_regstat3(*newstat);      

        if (newreg != NULL) {
	  xdr_free((xdrproc_t)_DtCm_xdr_Registration_4, (char*)newreg);        
	}
	return(1);
}

/*	PROC #16	*/
bool_t
_DtCm_deregister_callback_3_svc(Registration_3 *r,
				Registration_Status_3 *res,
				struct svc_req *svcrq)
{
        Registration_4 *newreg;          
        Registration_Status_4 *newstat; 

        newreg = _DtCm_reg3_to_reg4(r);       
        _DtCm_deregister_callback_4_svc(newreg, newstat, svcrq);       
        *res = _DtCm_regstat4_to_regstat3(*newstat);       

        if (newreg != NULL) {
	  xdr_free((xdrproc_t)_DtCm_xdr_Registration_4, (char*)newreg);        
	}
        return(1);
}

/*	PROC #17	*/
bool_t
_DtCm_rtable_set_access_3_svc(Access_Args_3 *args,
			      Access_Status_3 *res,
			      struct svc_req *svcrq)
{
	Access_Args_4 *newargs;
	Access_Status_4 *newstat;

        newargs = _DtCm_accargs3_to_accargs4(args);     
        _DtCm_rtable_set_access_4_svc(newargs, newstat, svcrq);     
        *res = _DtCm_accstat4_to_accstat3(*newstat);    

        if (newargs != NULL) {
	  xdr_free((xdrproc_t)_DtCm_xdr_Access_Args_4, (char*)newargs);      
	}
	return(1);
}

/*	PROC #18	*/
bool_t
_DtCm_rtable_get_access_3_svc(Access_Args_3 *args,
			      Access_Args_3 *res,
			      struct svc_req *svcrq)
{
        Access_Args_4 *newargs;
        Access_Args_4 *newres;

        newargs = _DtCm_accargs3_to_accargs4(args);
        _DtCm_rtable_get_access_4_svc(newargs, newres, svcrq);
        res = _DtCm_accargs4_to_accargs3(newres);

        if (newargs != NULL) {
	  xdr_free((xdrproc_t)_DtCm_xdr_Access_Args_4, (char*)newargs); 
	}
        return(res == NULL ? 0 : 1);
}

/*	PROC #19	*/
bool_t
_DtCm_rtable_abbreviated_lookup_key_range_3_svc(Table_Args_3 *args,
						Table_Res_3 *res,
						struct svc_req *svcrq)
{
        Table_Args_4 *newargs;   
        Table_Res_4 *newres;  

        newargs = _DtCm_tableargs3_to_tableargs4(args);
        _DtCm_rtable_abbreviated_lookup_key_range_4_svc(newargs, newres, svcrq);
        res = _DtCm_tableres4_to_tableres3(newres);

        if (newargs != NULL) {
	  xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);
	}
        return(1); 
}

/*	PROC #20	*/
bool_t
_DtCm_rtable_gmtoff_3_svc(void *args,
			  long *res,
			  struct svc_req *svcrq)
{
        _DtCm_rtable_gmtoff_4_svc(NULL, res, svcrq);

	return(1);
}

void initrtable3(program_handle ph)
{
        ph->program_num = TABLEPROG;
        ph->prog[TABLEVERS_3].vers = &tableprog_3_table[0];
        ph->prog[TABLEVERS_3].nproc = sizeof(tableprog_3_table)/sizeof(tableprog_3_table[0]);
}

