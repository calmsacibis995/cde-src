/* 
 * (c) Copyright 1989, 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC. 
 * ALL RIGHTS RESERVED 
*/ 
/* 
 * Motif Release 1.2.3
*/ 

/*
 *++
 *  FACILITY:
 *
 *      UIL Resource Manager (URM): IDB Facility
 *	Byte Swap routines for IDB records
 *
 *  ABSTRACT:
 *
 *
 *--
 */


/*
 *
 *  INCLUDE FILES
 *
 */


#include <Mrm/MrmAppl.h>
#include <Mrm/Mrm.h>
#include <Mrm/IDB.h>
#include <stdio.h>		/* for sprintf() */



Cardinal Idb__BM_SwapRecordBytes (buffer)
     IDBRecordBufferPtr		buffer ;
     
     
     /*
      *++
      *
      *  PROCEDURE DESCRIPTION:
      *
      *	Idb__BM_SwapBytes performs byte swapping on the (currently 6)
      * 	record types in an IDB file
      *
      *  FORMAL PARAMETERS:
      *
      *	buffer		Record buffer to swap in place
      *
      *  IMPLICIT INPUTS:
      *
      *  IMPLICIT OUTPUTS:
      *
      *  FUNCTION VALUE:
      *
      *	MrmSUCCESS	operation succeeded
      *	MrmFAILURE	some other failure
      *
      *  SIDE EFFECTS:
      *
      *--
      */
     
{
    
    /*
     *  External Functions
     */
    
    /*
     *  Local variables
     */
    Cardinal		  ndx;	     /* loop index */
    IDBDummyRecordPtr	  idb_record;/* pointer to the generic IDB record */
    IDBRecordHeaderPtr	  idb_header;/* pointer to hdr w/type and record # */
    IDBHeaderRecordPtr	  header_rec;/* pointer to record type IDBrtHeader */
    IDBHeaderHdrPtr	  header_hdr;/* pointer to the header in the header */
    IDBIndexLeafRecordPtr leaf_rec;  /* pointer to record type IDBrtIndexLeaf */
    IDBIndexNodeRecordPtr node_rec;  /* pointer to record type IDBrtIndexNode */
    IDBridMapRecordPtr	  ridmap_rec;/* pointer to record type IDBrtRIDMap */
    IDBDataRecordPtr	  data_rec;  /* pointer to record type IDBrtData */
    char		  err_msg[300] ;
    
    if ( ! Idb__BM_Valid(buffer) )
      return Urm__UT_Error("Idb__BM_MarkActivity", _MrmMsg_0002,
			   NULL, NULL, MrmNOT_VALID) ;
    
    /* load pointers to the record and record header */
    
    idb_record = (IDBDummyRecordPtr) buffer->IDB_record ;
    idb_header = (IDBRecordHeaderPtr)&idb_record->header ;
    
    
    /* swap the remaining record entries in IDBRecordHeader */
    swapbytes( idb_header->record_type ) ;
    swapbytes( idb_header->record_num ) ;
    
    /*
     * Swap IDB record items based on record type
     */
    
    switch ( idb_header->record_type )
    {	
      case IDBrtHeader:
	header_rec = (IDBHeaderRecordPtr)buffer->IDB_record ;
	header_hdr = (IDBHeaderHdrPtr)&header_rec->header_hdr ;
	
	/* swap the HeaderHdr first */
	swapbytes( header_hdr->index_root );
	swapbytes( header_hdr->num_indexed );
	swapbytes( header_hdr->num_RID );
	/* VAR check */
	swapbytes( header_hdr->next_RID.internal_id.map_rec ); 
	swapbytes( header_hdr->next_RID.internal_id.res_index ); 
	swapbytes( header_hdr->last_record );
	swapbytes( header_hdr->last_data_record );
	for( ndx=0 ; ndx < URMgVecSize ; ndx++)
	  swapbytes(header_hdr->group_counts[ndx]);
	for( ndx=0 ; ndx < IDBrtVecSize ; ndx++)
	  swapbytes(header_hdr->rt_counts[ndx]);
	
	/* now swap the rest of the header */
	/* VAR check */
	for( ndx=0 ; ndx < IDBHeaderRIDMax ; ndx++)
	{
	    swapbytes(header_rec->RID_pointers[ndx].internal_id.rec_no);
	    swapbytes(header_rec->RID_pointers[ndx].internal_id.item_offs);
	}
	swapbytes( header_rec->num_entry );
	swapbytes( header_rec->last_entry );
	swapbytes( header_rec->free_ptr );
	swapbytes( header_rec->free_count );
	break;
	
      case IDBrtIndexLeaf:
	leaf_rec = (IDBIndexLeafRecordPtr)buffer->IDB_record ;
	swapbytes( leaf_rec->leaf_header.parent );
	swapbytes( leaf_rec->leaf_header.index_count );
	swapbytes( leaf_rec->leaf_header.heap_start );
	swapbytes( leaf_rec->leaf_header.free_bytes );
	for( ndx=0 ; ndx < leaf_rec->leaf_header.index_count ; ndx++ ) 
	{
	    swapbytes( leaf_rec->index[ndx].index_stg );	
	    swapbytes( leaf_rec->index[ndx].data.internal_id.rec_no );	
	    swapbytes( leaf_rec->index[ndx].data.internal_id.item_offs );	
	}
	break;
	
      case IDBrtIndexNode:
	node_rec = (IDBIndexNodeRecordPtr)buffer->IDB_record ;
	swapbytes( node_rec->node_header.parent );
	swapbytes( node_rec->node_header.index_count );
	swapbytes( node_rec->node_header.heap_start );
	swapbytes( node_rec->node_header.free_bytes );
	for( ndx=0 ; ndx < node_rec->node_header.index_count ; ndx++ ) 
	{
	    swapbytes( node_rec->index[ndx].index_stg );	
	    swapbytes( node_rec->index[ndx].data.internal_id.rec_no );	
	    swapbytes( node_rec->index[ndx].data.internal_id.item_offs );	
	    swapbytes( node_rec->index[ndx].LT_record );	
	    swapbytes( node_rec->index[ndx].GT_record );	
	}
	break;
	
      case IDBrtRIDMap:
	ridmap_rec = (IDBridMapRecordPtr)buffer->IDB_record ;
	ndx = 0;
	while ( (ndx < IDBridPtrVecMax) && 
	       (ridmap_rec->pointers[ndx].internal_id.rec_no != 0) )
	{
	    swapbytes( ridmap_rec->pointers[ndx].internal_id.rec_no );	
	    swapbytes( ridmap_rec->pointers[ndx].internal_id.item_offs );
	    ndx++;
	}
	break;
	
      case IDBrtData:
	data_rec = (IDBDataRecordPtr)buffer->IDB_record ;
	swapbytes( data_rec->data_header.num_entry );
	swapbytes( data_rec->data_header.last_entry );
	swapbytes( data_rec->data_header.free_ptr );
	swapbytes( data_rec->data_header.free_count );
	break;
	
      default:
	sprintf(err_msg, _MrmMsg_0020, idb_header->record_num,
	       idb_header->record_type);
	return Urm__UT_Error ("Idb__BM_SwapRecordBytes",
			      err_msg, NULL, NULL, MrmFAILURE) ;
    }
        return MrmSUCCESS ;
} 



unsigned Urm__SwapValidation (validation)
     unsigned 		validation ;
     
     /*
      *++
      *
      *  PROCEDURE DESCRIPTION:
      *
      *  FORMAL PARAMETERS:
      *
      *  IMPLICIT INPUTS:
      *
      *  IMPLICIT OUTPUTS:
      *
      *  FUNCTION VALUE:
      *
      *  SIDE EFFECTS:
      *
      *--
      */
     
     
{
    swapbytes(validation);
    return validation;
}


Cardinal Urm__SwapRGMResourceDesc( res_desc )
     RGMResourceDescPtr	res_desc;
{
    IDBridDesc  *idb_rid_ptr;

    swapbytes( res_desc->size );
    swapbytes( res_desc->annex1 );
    if ( res_desc->type == URMrRID ) 
    {
	idb_rid_ptr = (IDBridDesc *)&(res_desc->key.id);
	swapbytes( idb_rid_ptr->internal_id.map_rec );
	swapbytes( idb_rid_ptr->internal_id.res_index );
    }


    return MrmSUCCESS;
}


Cardinal Urm__SwapRGMCallbackDesc(callb_desc, widget_rec)
     RGMCallbackDescPtr	callb_desc;
     RGMWidgetRecordPtr	widget_rec;
{
    Cardinal		ndx;	    /* inner loop index */
    RGMResourceDescPtr	res_desc;   /* resource description literal */
    char		err_msg[300];
    
    swapbytes( callb_desc->validation );
    swapbytes( callb_desc->count );
    swapbytes( callb_desc->annex1 );
    swapbytes( callb_desc->unres_ref_count );
    for (ndx=0 ; ndx < callb_desc->count ; ndx++)
    {
	swapbytes( callb_desc->item[ndx].cb_item.routine );
	swapbytes( callb_desc->item[ndx].cb_item.rep_type );
	switch (callb_desc->item[ndx].cb_item.rep_type)
	{
	  case MrmRtypeInteger:
	  case MrmRtypeBoolean:
	    swapbytes( callb_desc->item[ndx].cb_item.datum.ival );
	    break;	
	  case MrmRtypeNull:
	    break;
	  case MrmRtypeResource:
	    swapbytes( callb_desc->item[ndx].cb_item.datum.offset );
	    res_desc = (RGMResourceDesc *)((char *)widget_rec + 
		        callb_desc->item[ndx].cb_item.datum.offset);
	    Urm__SwapRGMResourceDesc( res_desc );
	    /* flag this resource as needing further byte swapping */
	    res_desc->cvt_type |= MrmResourceUnswapped;
	    break;
	  default:
	    swapbytes( callb_desc->item[ndx].cb_item.datum.offset );
	    sprintf(err_msg, _MrmMsg_0021,
		    callb_desc->item[ndx].cb_item.rep_type, ndx);
	    return Urm__UT_Error ("Urm__SwapRGMCallbackDesc",
				  err_msg, NULL, NULL, MrmFAILURE) ;
	    break;
	}
    }
    return MrmSUCCESS;
}


Cardinal Urm__SwapRGMWidgetRecord(widget_rec)
     RGMWidgetRecordPtr	widget_rec;
     
     /*
      *++
      *
      *  PROCEDURE DESCRIPTION:
      *
      *
      *  FORMAL PARAMETERS:
      *
      *	buffer		Record buffer to swap in place
      *
      *  IMPLICIT INPUTS:
      *
      *  IMPLICIT OUTPUTS:
      *
      *  FUNCTION VALUE:
      *
      *	MrmSUCCESS	operation succeeded
      *	MrmFAILURE	some other failure
      *
      *  SIDE EFFECTS:
      *
      *--
      */
     
{
    
    /*
     *  External Functions
     */
    
    /*
     *  Local variables
     */
    RGMArgListDescPtr	arg_list;   	/* pointer to widget arglist */
    RGMChildrenDescPtr	child_list; 	/* pointer to the widgets children */
    RGMResourceDescPtr	res_desc;   	/* resource description literal */
    RGMCallbackDescPtr	callb_desc; 	/* pointer to a callback decriptor */
    Cardinal		ndx;	   	/* loop index */
    IDBridDesc  	*idb_rid_ptr;
    void		*offset;     	/* generic offset pointer */
    char		err_msg[300];
    
    /* Swap the main part of the widget record */
    
    swapbytes( widget_rec->size );
    swapbytes( widget_rec->access );
    swapbytes( widget_rec->lock );
    swapbytes( widget_rec->type );
    swapbytes( widget_rec->name_offs );
    swapbytes( widget_rec->class_offs );
    swapbytes( widget_rec->arglist_offs );
    swapbytes( widget_rec->children_offs );
    swapbytes( widget_rec->comment_offs );
    swapbytes( widget_rec->creation_offs );
    swapbytes( widget_rec->variety );
    swapbytes( widget_rec->annex );
    
    /* handle the argument list */
    
    if (widget_rec->arglist_offs > 0)
    {	
	arg_list = (RGMArgListDesc *)
	  ((char *)widget_rec + widget_rec->arglist_offs);
	swapbytes( arg_list->count );
	swapbytes( arg_list->extra );
	swapbytes( arg_list->annex1 );
	for ( ndx=0 ; ndx<arg_list->count ; ndx++ )
	{
	    swapbytes( arg_list->args[ndx].tag_code );
	    swapbytes( arg_list->args[ndx].stg_or_relcode.tag_offs );
	    swapbytes( arg_list->args[ndx].arg_val.rep_type );
	    
	    switch( arg_list->args[ndx].arg_val.rep_type )
	    {
	      case MrmRtypeInteger:
	      case MrmRtypeBoolean:
		swapbytes( arg_list->args[ndx].arg_val.datum.ival );
		break;
	      default:
		swapbytes( arg_list->args[ndx].arg_val.datum.offset );
		break;
	    }
	    
	    offset = ((char *)widget_rec+
		      arg_list->args[ndx].arg_val.datum.offset);
	    
	    switch( arg_list->args[ndx].arg_val.rep_type )
	    {	
		/* these are immediate, do nothing special */
	      case MrmRtypeInteger:
	      case MrmRtypeBoolean:
		break;
		/* these are offsets into the file, handle them specially */
	      case MrmRtypeCallback:
		callb_desc = (RGMCallbackDesc * )offset;
		Urm__SwapRGMCallbackDesc( callb_desc, widget_rec );
		break;
	      case MrmRtypeResource:
		res_desc = (RGMResourceDesc *)offset;
		Urm__SwapRGMResourceDesc( res_desc );
		/* flag this resource as needing further byte swapping */
		res_desc->cvt_type |= MrmResourceUnswapped;
		break;
	      case MrmRtypeChar8:
	      case MrmRtypeChar8Vector:
	      case MrmRtypeCString:
	      case MrmRtypeCStringVector:
	      case MrmRtypeFloat:
	      case MrmRtypePixmapImage:
	      case MrmRtypePixmapDDIF:
	      case MrmRtypeNull:
	      case MrmRtypeAddrName:
	      case MrmRtypeIconImage:
	      case MrmRtypeFont:
	      case MrmRtypeFontList:
	      case MrmRtypeColor:
	      case MrmRtypeColorTable:
	      case MrmRtypeAny:
	      case MrmRtypeTransTable:
	      case MrmRtypeClassRecName:
	      case MrmRtypeIntegerVector:
	      case MrmRtypeXBitmapFile:
	      case MrmRtypeCountedVector:
	      case MrmRtypeKeysym:
	      case MrmRtypeSingleFloat:
	      case MrmRtypeWideCharacter:
	      case MrmRtypeFontSet:
		sprintf(err_msg,_MrmMsg_0022,
			arg_list->args[ndx].arg_val.rep_type);
		return Urm__UT_Error ("Urm__SwapRGMWidgetRecord",
				      err_msg, NULL, NULL, MrmFAILURE) ;
		break;
	    }
	}
    }
    
    /* handle the child list */
    
    if (widget_rec->children_offs > 0)
    {
	child_list = (RGMChildrenDesc *)
	  ((char *)widget_rec + widget_rec->children_offs);
	swapbytes( child_list->count );
	swapbytes( child_list->unused1 );
	swapbytes( child_list->annex1 );
	for ( ndx=0 ; ndx<child_list->count ; ndx++ )
	{	
	    swapbytes( child_list->child[ndx].annex1 );
	    if (child_list->child[ndx].type ==  URMrRID ) 
	    {
		idb_rid_ptr = (IDBridDesc *)&(child_list->child[ndx].key.id);
		swapbytes( idb_rid_ptr->internal_id.map_rec );
		swapbytes( idb_rid_ptr->internal_id.res_index );
	    }
	    else
	      swapbytes( child_list->child[ndx].key.index_offs );
	}
    }
    
    /* handle the creation callback, if any */
    
    if (widget_rec->creation_offs > 0)
    {
	callb_desc = (RGMCallbackDesc * )
	  ((char *)widget_rec + widget_rec->creation_offs);
	Urm__SwapRGMCallbackDesc( callb_desc, widget_rec );
    }
    
    return MrmSUCCESS ;
}
