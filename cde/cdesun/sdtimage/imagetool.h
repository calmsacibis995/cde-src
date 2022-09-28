/*
 * @(#)imagetool.h 1.22 97/03/19
 *
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

#ifndef _IMAGETOOL_H
#define _IMAGETOOL_H


#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include <nl_types.h>
#include <X11/Intrinsic.h>
#include <xil/xil.h>

#define IMAGETOOL_VERSION  "1.2"
#define IMAGETOOL_ICON     "SDtimage"
#define IMAGETOOL_CAT      "sdtimage"

#define OFF		0
#define ON		1

#define Min(a,b)        ((a) > (b) ? (b) : (a))
#define Max(a,b)        ((a) > (b) ? (a) : (b))

#define MAXCOLORS       256

typedef struct mmap_handle {
  caddr_t       addr;
  int           len;
} mmap_handle_t;
 
typedef struct ENCODED_BM {
  unsigned int    sieve[256];
  unsigned char **strings;
  unsigned int   *lengths;
  unsigned int    len;
  unsigned int    nstrings;
} encoded_bm;

typedef struct {
	char	        *name;		/* Name of this application  	  */
	char	        *rel;		/* Release (from ds_relname  	  */
	char	        *directory;	/* Current working directory 	  */
	char	        *hostname;	/* Hostname where running    	  */
	char		*newsserver;	/* String used for ps_open_server */
	int		 dps;		/* True if using DPS server	  */
	int		 remote;	/* True if display is remote      */
	int		 news_opened;   /* True if opened news server     */
        int              xil_opened;    /* True if opened XIL	          */
	int		 frame_mapped;  /* Frame has been mapped	  */
	int              ce_okay;       /* CE initialized successfully    */
	int              sb_right;      /* True if scrolbars on on right  */
	int              def_ps_zoom;   /* Set default PS zoom factor     */
	char            *wm_cmdstr;     /* WM_COMMAND format version      */
	int              wm_cmdstrlen;  /* Size of wm_cmdstr buffer       */
	int		 tt_started;	/* True if started with -tooltalk */
        int              snapshot_on_start; /* true is started with -snapshot */
	char		*tt_sender;	/* Tooltalk sender id		  */
	int		 tt_load;	/* True if data from tt		  */
        int		 tt_timer_set;  /* True if imagetool sleeping     */
	char		*file_template; /* Template to use for tmp files  */
	char		*datafile;	/* File we wrote data into	  */
	char		*compfile;	/* Compressed file written here   */
	char		*uncompfile;	/* Uncompressed file written here */
	char		*printfile;	/* Print tmpfile name		  */
	char		*rashfile;	/* Rash output tmpfile name	  */
	char		*def_printer;	/* Default printer		  */
	int		 timeout;	/* timeout in sec		  */
	int		 verbose;	/* Print out additional info 	  */
	uid_t		 uid;
	gid_t 		 gid;
	nl_catd          catd;		/* message catalog handle         */
	int              standardin;    /* flag to use data from stdin    */
	char            *standardinfile;/* filename of stdin data         */
	int              check_for_color; /* If True, then check if PS is color */
	int		 ps_mono;         /* True, if PS in monochrome    */
	int              footer_priority; /* priority of footer message   */
} ProgInfo;

extern	ProgInfo        *prog;

typedef void (*DoneDataProc)(void *);
typedef XilLookup (*XilLookupCreateProc)(XilSystemState, XilDataType, 
		      XilDataType, unsigned int, unsigned int, short, void *);
typedef XilLookup (*XilColorcubeCreateProc)(XilSystemState, XilDataType, 
		      XilDataType, unsigned int, short, int *, unsigned int *);


typedef struct {
        void                *handle;    
        XilSystemState       (*open)();
#ifdef DEBUG
        void                 (*state_set_synchronize)(XilSystemState, Xil_boolean);
#endif
	int                  (*install_error_handler)(XilSystemState, XilErrorFunc);
	void                 (*roi_add_rect)(XilRoi, long, long, long, long);
	void       	     (*set_roi)(XilImage, XilRoi);
	char * 	             (*error_get_string)(XilError);
	XilErrorCategory     (*error_get_category)(XilError);
	Xil_boolean          (*error_get_primary)(XilError);
	unsigned int         (*get_width)(XilImage);
	unsigned int         (*get_height)(XilImage);
        XilDataType          (*get_datatype)(XilImage);
        unsigned int         (*get_nbands)(XilImage);
	XilRoi               (*roi_create)(XilSystemState);
	void                 (*roi_destroy)(XilRoi);
    	void                 (*set_value)(XilImage, float *);
    	void                 (*set_origin)(XilImage, float, float);
    	void                 (*copy)(XilImage, XilImage);
    	void                 (*destroy)(XilImage);
        void     	     (*rescale)(XilImage, XilImage, float *, float *);
    	XilImage             (*create)(XilSystemState, unsigned int, 
				       unsigned int, unsigned int, XilDataType);
    	XilImage             (*create_child)(XilImage, unsigned int, unsigned int,
				unsigned int, unsigned int, unsigned int,
				unsigned int);
	XilImage      	     (*create_from_device)(XilSystemState, char *, XilDevice);
	int      	     (*set_device_attribute)(XilImage, char *, void *);
	int      	     (*get_device_attribute)(XilImage, char *, void *);
    	int                  (*export_ptr)(XilImage);
        void     	     (*import_ptr)(XilImage, Xil_boolean);
    	void     	     (*lookup)(XilImage, XilImage, XilLookup);
    	void                 (*lookup_get_values)(XilLookup, short, unsigned int,
				void *);
   	XilLookup     	     (*lookup_get_by_name)(XilSystemState, char *);
        void     	     (*lookup_set_values)(XilLookup, short, unsigned int,
				void *);
        short     	     (*lookup_get_offset)(XilLookup);
        unsigned int         (*lookup_get_num_entries)(XilLookup);
    	XilLookupCreateProc  lookup_create;
        XilLookup     	     (*lookup_convert)(XilLookup, XilLookup);
     	void     	     (*lookup_destroy)(XilLookup);
        XilLookup            (*choose_colormap)(XilImage, unsigned int);
        Xil_boolean          (*get_memory_storage)(XilImage, XilMemoryStorage *);
        void                 (*set_memory_storage)(XilImage, XilMemoryStorage *);
        void                 (*get_info)(XilImage, unsigned int *, 
				unsigned int *, unsigned int *, XilDataType *);
        XilDitherMask        (*dithermask_get_by_name)(XilSystemState, char *);
        XilDitherMask        (*dithermask_create)(XilSystemState, unsigned int,
				unsigned int, unsigned int, float *);
        XilColorcubeCreateProc colorcube_create;
     	void     	     (*ordered_dither)(XilImage, XilImage, XilLookup,
				XilDitherMask);
    	XilColorspace        (*colorspace_get_by_name)(XilSystemState, char *);
     	void    	     (*set_colorspace)(XilImage, XilColorspace);
    	void     	     (*color_convert)(XilImage, XilImage);
     	void     	     (*threshold)(XilImage, XilImage, float *, float *,
				float *);
 	void     	     (*cast)(XilImage, XilImage);
        void     	     (*multiply_const)(XilImage, float *, XilImage);
        void     	     (*xor_const)(XilImage, unsigned int *, XilImage);
        void        	     (*subtract_const)(XilImage, float *, XilImage);
	void     	     (*add_const)(XilImage, float *, XilImage);
     	XilKernel     	     (*kernel_get_by_name)(XilSystemState, char *);
        XilCis    	     (*cis_create)(XilSystemState, char *);
        void     	     (*cis_destroy)(XilCis);
        void     	     (*cis_put_bits_ptr)(XilCis, int, int, void *,
				DoneDataProc);
        void *               (*cis_get_bits_ptr)(XilCis, int *, int *);
        int                  (*cis_has_data)(XilCis);
 	XilImageType         (*cis_get_output_type)(XilCis);
        void      	     (*affine)(XilImage, XilImage, char *, float *);
        XilImage     	     (*create_from_window)(XilSystemState, Display *, Window);
        XilImage     	     (*create_copy)(XilImage, unsigned int, unsigned int,
				unsigned int, unsigned int, unsigned int,
				unsigned int);
    	void     	     (*error_diffusion)(XilImage, XilImage, XilLookup,
				XilKernel);
	void     	     (*imagetype_get_info)(XilImageType, unsigned int *,
				unsigned int *, unsigned int *, XilDataType *);
	void     	     (*decompress)(XilCis, XilImage);
	void     	     (*compress)(XilImage, XilCis);
	void     	     (*rotate)(XilImage, XilImage, char *, float);
	void     	     (*scale)(XilImage, XilImage, char *, float, float);
} XilProcs;

extern XilProcs        *xil;

typedef struct {
    String     name;
    String     title;
    String     geometry;
    String     icon_name;
    String     visual;
    String     session_file;
} AppResourceInfo;

extern AppResourceInfo  *appresources;

/* Function prototypes */
extern	char		*make_pathname (char *, char *);
extern  char		*basename (char *);
extern	char		*strip_filename (char *);
extern  int		 check_float_value (char *, double *);
extern  mmap_handle_t	*fast_read (char *);
extern  encoded_bm	*strbmencode (char **);
extern  char		*strbmexec (unsigned char *, int, encoded_bm *);
extern  void            OutOfMemory();

#endif /* _IMAGETOOL_H */
