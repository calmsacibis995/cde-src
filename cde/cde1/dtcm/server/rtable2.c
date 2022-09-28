/* $XConsortium: rtable2.c /main/cde1_maint/2 1995/10/10 13:35:40 pascale $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)rtable2.c	1.5 96/11/19 Sun Microsystems, Inc."

/*
  veneer layered on top of the real data structures for abstraction.
	implements Version 2 in terms of Version 4 types	
 */



#include <stdio.h>
#include "rtable4.h"
#include "rtable2.h"
#include <sys/param.h>
#include <sys/time.h>
#include <rpc/rpc.h>
#include "rpcextras.h"
#include "convert2-4.h"
#include "convert4-2.h"
#include "rtable2_tbl.i"


/*************** V2 PROTOCOL IMPLEMENTATION PROCS *****************/
extern bool_t
_DtCm_rtable_ping_2_svc(args, res, svcrq)
void *args;
void *res;
struct svc_req *svcrq;
{
	char dummy;	
	res = (void *)dummy;
	return(1); /* for RPC reply */
}

/*	PROC #1		*/
extern bool_t
_DtCm_rtable_lookup_2_svc (args, res, svcrq)
Table_Args_2 *args;
Table_Res_2 *res;
struct svc_req *svcrq;
{
	Table_Args_4 *newargs;
	Table_Res_4 *newres;

	newargs = _DtCm_tableargs2_to_tableargs4(args);
	_DtCm_rtable_lookup_4_svc(newargs, newres, svcrq); 
	
	res = _DtCm_tableres4_to_tableres2(newres);

	if (newargs!=NULL) xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);

	return(1);
}

/*	PROC #2		*/
extern bool_t
_DtCm_rtable_lookup_next_larger_2_svc(args, res, svcrq)
Table_Args_2 *args;
Table_Res_2 *res;
struct svc_req *svcrq;
{
        Table_Args_4 *newargs;   
        Table_Res_4 *newres; 
 
        newargs = _DtCm_tableargs2_to_tableargs4(args);
        _DtCm_rtable_lookup_next_larger_4_svc(newargs, newres, svcrq);
        
        res = _DtCm_tableres4_to_tableres2(newres);
 
        if (newargs!=NULL) xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);
 
        return(1);

}

/*	PROC #3		*/
extern bool_t
_DtCm_rtable_lookup_next_smaller_2_svc(args, res, svcrq)
Table_Args_2 *args;
Table_Res_2 *res;
struct svc_req *svcrq;
{
        Table_Args_4 *newargs;    
        Table_Res_4 *newres; 
  
        newargs = _DtCm_tableargs2_to_tableargs4(args);
        _DtCm_rtable_lookup_next_smaller_4_svc(newargs, newres, svcrq); 
         
        res = _DtCm_tableres4_to_tableres2(newres);
  
        if (newargs!=NULL) xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);
 
        return(1);

}

/*	PROC #4		*/
extern bool_t
_DtCm_rtable_lookup_range_2_svc(args, res, svcrq)
Table_Args_2 *args;
Table_Res_2 *res;
struct svc_req *svcrq;
{
        Table_Args_4 *newargs;     
        Table_Res_4 *newres; 
   
        newargs = _DtCm_tableargs2_to_tableargs4(args); 
        _DtCm_rtable_lookup_range_4_svc(newargs, newres, svcrq);  
          
        res = _DtCm_tableres4_to_tableres2(newres); 
   
        if (newargs!=NULL) xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs); 
   
        return(1);
}

/*	PROC #5		*/
extern bool_t
_DtCm_rtable_abbreviated_lookup_range_2_svc(args, res, svcrq)
Table_Args_2 *args;
Table_Res_2 *res;
struct svc_req *svcrq;
{
        Table_Args_4 *newargs;      
        Table_Res_4 *newres; 
       
        newargs = _DtCm_tableargs2_to_tableargs4(args);  
        _DtCm_rtable_abbreviated_lookup_range_4_svc(newargs, newres, svcrq);  
           
        res = _DtCm_tableres4_to_tableres2(newres);  
   
        if (newargs!=NULL) xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);  
    
        return(1);
}

/*	PROC #6		*/
extern bool_t
_DtCm_rtable_insert_2_svc(args, res, svcrq)
Table_Args_2 *args;
Table_Res_2 *res;
struct svc_req *svcrq;
{
        Table_Args_4 *newargs;
        Table_Res_4 *newres;
      
        newargs = _DtCm_tableargs2_to_tableargs4(args);
        _DtCm_rtable_insert_4_svc(newargs, newres, svcrq);
         
        res = _DtCm_tableres4_to_tableres2(newres);
  
        if (newargs!=NULL) xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);
    
        return(1);
}

/*	PROC #7	*/
extern bool_t
_DtCm_rtable_delete_2_svc(args, res, svcrq)
Table_Args_2 *args;
Table_Res_2 *res;
struct svc_req *svcrq;
{
        Table_Args_4 *newargs;
        Table_Res_4 *newres; 
   
	newargs = _DtCm_tabledelargs2_to_tabledelargs4(args, do_all_4);
        _DtCm_rtable_delete_4_svc(newargs, newres, svcrq); 
        res = _DtCm_tableres4_to_tableres2(newres); 
  
        if (newargs!=NULL) xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs); 
     
        return(1); 
}

/*	PROC #8		*/
extern bool_t
_DtCm_rtable_delete_instance_2_svc(args, res, svcrq)
Table_Args_2 *args;
Table_Res_2 *res;
struct svc_req *svcrq;
{
        Table_Args_4 *newargs;  
        Table_Res_4 *newres;  

	newargs = _DtCm_tabledelargs2_to_tabledelargs4(args, do_one_4);
	_DtCm_rtable_delete_4_svc(newargs, newres, svcrq);  
        res = _DtCm_tableres4_to_tableres2(newres);   
   
        if (newargs!=NULL) xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);   
       
        return(1);
}

/*	PROC #9	*/
extern bool_t
_DtCm_rtable_change_2_svc(args, res, svcrq)
Table_Args_2 *args;
Table_Res_2 *res;
struct svc_req *svcrq;
{
        Table_Args_4 *newargs; 
        Table_Res_4 *newres;  
          
        newargs = _DtCm_tableargs2_to_tableargs4(args); 
        _DtCm_rtable_change_4_svc(newargs, newres, svcrq); 
        res = _DtCm_tableres4_to_tableres2(newres);  
  
        if (newargs!=NULL) xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);  
      
        return(1); 
}


/*	PROC #10	*/
extern bool_t
_DtCm_rtable_change_instance_2_svc(args, res, svcrq)
Table_Args_2 *args;
Table_Res_2 *res;
struct svc_req *svcrq;
{
        Table_Args_4 *newargs;   
        Table_Res_4 *newres;  
            
        newargs = _DtCm_tableargs2_to_tableargs4(args);  
	newargs->args.Args_4_u.apptid.option = do_one_4;
        _DtCm_rtable_change_4_svc(newargs, newres, svcrq);  
        res = _DtCm_tableres4_to_tableres2(newres);   
   
        if (newargs!=NULL) xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);   
       
        return(1);
}

/*	PROC #11	*/
extern bool_t
_DtCm_rtable_lookup_next_reminder_2_svc(args, res, svcrq)
Table_Args_2 *args;
Table_Res_2 *res;
struct svc_req *svcrq;
{
        Table_Args_4 *newargs;    
        Table_Res_4 *newres;   
              
        newargs = _DtCm_tableargs2_to_tableargs4(args);   
        _DtCm_rtable_lookup_next_reminder_4_svc(newargs, newres, svcrq);   
             
        res = _DtCm_tableres4_to_tableres2(newres);   
    
        if (newargs!=NULL) xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);    
        
        return(1); 
}

/*	PROC #12	*/
extern bool_t
_DtCm_rtable_check_2_svc(args, res, svcrq)
Table_Args_2 *args;
Table_Status_2 *res;
struct svc_req *svcrq;
{
        Table_Args_4 *newargs;     
        Table_Status_4 *newres;    
       
        newargs = _DtCm_tableargs2_to_tableargs4(args);    
        _DtCm_rtable_check_4_svc(newargs, newres, svcrq);    
              
        *res = _DtCm_tablestat4_to_tablestat2(*newres);   
     
        if (newargs!=NULL) xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);     
         
        return(1); 
}

/*	PROC #13	*/
extern bool_t
_DtCm_rtable_flush_table_2_svc(args, res, svcrq)
Table_Args_2 *args;
Table_Status_2 *res;
struct svc_req *svcrq;
{
        Table_Args_4 *newargs;     
        Table_Status_4 *newres;     
        
        newargs = _DtCm_tableargs2_to_tableargs4(args);     
        _DtCm_rtable_flush_table_4_svc(newargs, newres, svcrq);     
               
        *res = _DtCm_tablestat4_to_tablestat2(*newres);    
     
        if (newargs!=NULL) xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);      
          
        return(1);
}

/*	PROC #14	*/
extern bool_t
_DtCm_rtable_size_2_svc(args, res, svcrq)
Table_Args_2 *args;
int *res;
struct svc_req *svcrq;
{
        Table_Args_4 *newargs;      
         
        newargs = _DtCm_tableargs2_to_tableargs4(args);     
        _DtCm_rtable_size_4_svc(newargs, res, svcrq);    
      
        if (newargs!=NULL) xdr_free((xdrproc_t)_DtCm_xdr_Table_Args_4, (char*)newargs);       
           
        return(1);
}

/*	PROC #15	*/
bool_t
_DtCm_register_callback_2_svc(r, res, svcrq)
Registration_2 *r;
Registration_Status_2 *res;
struct svc_req *svcrq;
{
        Registration_4 *newreg;       
	Registration_Status_4 *newstat;
          
        newreg = _DtCm_reg2_to_reg4(r);      
        _DtCm_register_callback_4_svc(newreg, newstat, svcrq);      
                 
        *res = _DtCm_regstat4_to_regstat2(*newstat);      
      
        if (newreg!=NULL) xdr_free((xdrproc_t)_DtCm_xdr_Registration_4, (char*)newreg);        
	return(1);
}

/*	PROC #16	*/
bool_t
_DtCm_deregister_callback_2_svc(r, res, svcrq)
Registration_2 *r;
Registration_Status_2 *res;
struct svc_req *svcrq;
{
        Registration_4 *newreg;          
        Registration_Status_4 *newstat; 
           
        newreg = _DtCm_reg2_to_reg4(r);       
        _DtCm_deregister_callback_4_svc(newreg, newstat, svcrq);       
                  
        *res = _DtCm_regstat4_to_regstat2(*newstat);       
       
        if (newreg!=NULL) xdr_free((xdrproc_t)_DtCm_xdr_Registration_4, (char*)newreg);        
        return(1);
}

/*	PROC #17	*/
extern bool_t
_DtCm_rtable_set_access_2_svc(args, res, svcrq)
Access_Args_2 *args;
Access_Status_2 *res;
struct svc_req *svcrq;
{
	Access_Args_4 *newargs;
	Access_Status_4 *newstat;
        
        newargs = _DtCm_accargs2_to_accargs4(args);     
        _DtCm_rtable_set_access_4_svc(newargs, newstat, svcrq);     
               
        *res = _DtCm_accstat4_to_accstat2(*newstat);    
     
        if (newargs!=NULL) xdr_free((xdrproc_t)_DtCm_xdr_Access_Args_4, (char*)newargs);      
	return(1);
}

/*	PROC #18	*/
extern bool_t
_DtCm_rtable_get_access_2_svc(args, res, svcrq)
Access_Args_2 *args;
Access_Args_2 *res;
struct svc_req *svcrq;
{
        Access_Args_4 *newargs;
        Access_Args_4 *newres;
        
        newargs = _DtCm_accargs2_to_accargs4(args);
        _DtCm_rtable_get_access_4_svc(newargs, newres, svcrq);
               
        res = _DtCm_accargs4_to_accargs2(newres);
        
        if (newargs!=NULL) xdr_free((xdrproc_t)_DtCm_xdr_Access_Args_4, (char*)newargs); 

        return(1); 
}

void initrtable2(ph)
        program_handle ph;
{
	int ver = TABLEVERS_2;
        ph->program_num = TABLEPROG;
        ph->prog[TABLEVERS_2].vers = &tableprog_2_table[0];
        ph->prog[TABLEVERS_2].nproc = sizeof(tableprog_2_table)/sizeof(tableprog_2_table[0]);
}

