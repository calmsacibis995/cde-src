/* $XConsortium: Dts.h /main/cde1_maint/1 1995/07/17 18:05:30 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 */

#ifndef _Dt_Dts_h
#define _Dt_Dts_h

#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Constants
 */

#define	DtDTS_DC_NAME			"DATA_CRITERIA"
#define	DtDTS_NAME_PATTERN		"NAME_PATTERN"
#define	DtDTS_PATH_PATTERN		"PATH_PATTERN"
#define	DtDTS_CONTENT			"CONTENT"
#define	DtDTS_MODE			"MODE"
#define	DtDTS_LINK_PATH			"LINK_PATH"
#define	DtDTS_LINK_NAME			"LINK_NAME"
#define	DtDTS_DATA_ATTRIBUTES_NAME	"DATA_ATTRIBUTES_NAME"

#define	DtDTS_DT_RECURSIVE_LINK		"RECURSIVE_LINK"
#define	DtDTS_DT_BROKEN_LINK		"BROKEN_LINK"
#define	DtDTS_DT_UNKNOWN		"UNKNOWN"

#define	DtDTS_DA_IS_SYNTHETIC		"IS_SYNTHETIC"
#define	DtDTS_DA_LABEL			"LABEL"
#define	DtDTS_DA_NAME			"DATA_ATTRIBUTES"
#define	DtDTS_DA_DESCRIPTION		"DESCRIPTION"
#define	DtDTS_DA_DATA_HOST		"DATA_HOST"
#define	DtDTS_DA_ICON			"ICON"
#define	DtDTS_DA_INSTANCE_ICON		"INSTANCE_ICON"
#define	DtDTS_DA_PROPERTIES		"PROPERTIES"
#define	DtDTS_DA_ACTION_LIST		"ACTIONS"
#define DtDTS_DA_NAME_TEMPLATE		"NAME_TEMPLATE"
#define DtDTS_DA_MODE_TEMPLATE		"MODE_TEMPLATE"
#define	DtDTS_DA_MOVE_TO_ACTION		"MOVE_TO_ACTION"
#define	DtDTS_DA_COPY_TO_ACTION		"COPY_TO_ACTION"
#define	DtDTS_DA_LINK_TO_ACTION		"LINK_TO_ACTION"
#define	DtDTS_DA_IS_TEXT		"IS_TEXT"
#define	DtDTS_DA_MEDIA			"MEDIA"
#define	DtDTS_DA_MIME_TYPE		"MIME_TYPE"
#define	DtDTS_DA_MIME_TO_MEDIA_FILTER	"MIME_TO_MEDIA_FILTER"
#define	DtDTS_DA_MEDIA_TO_MIME_FILTER	"MEDIA_TO_MIME_FILTER"
#define	DtDTS_DA_X400_TYPE		"X400_TYPE"
#define	DtDTS_DA_X400_TO_MEDIA_FILTER	"X400_TO_MEDIA_FILTER"
#define	DtDTS_DA_MEDIA_TO_X400_FILTER	"MEDIA_TO_X400_FILTER"
#define	DtDTS_DA_IS_ACTION		"IS_ACTION"
#define	DtDTS_DA_IS_EXECUTABLE		"IS_EXECUTABLE"

#define	DtDTS_DT_DIR	".DtDirDataType"


/*
 * Types
 */

typedef	struct	_DtDtsAttribute
{
	char	*name;
	char	*value;
} DtDtsAttribute;


/*
 * Functions
 */

extern void DtDtsLoadDataTypes(void);
extern void DtDtsRelease(void);

#if _FILE_OFFSET_BITS == 64
#	ifdef	__PRAGMA_REDEFINE_EXTNAME
#		pragma redefine_extname	DtDtsDataToDataType	DtDtsDataToDataType64
#	else
#		define	DtDtsDataToDataType	DtDtsDataToDataType64
#	endif
#endif
extern char *DtDtsDataToDataType(
		const char		*filepath,
		const void		*buffer,
		const int		size,
		const struct stat	*stat_buff,
		const char		*link_name,
		const struct stat	*link_stat_buff,
		const char		*opt_name);

extern char *DtDtsFileToDataType(
		const char		*filepath);

extern char *DtDtsFileToAttributeValue(
		const char		*filepath,
		const char		*attr);

extern DtDtsAttribute **DtDtsFileToAttributeList(
		const char		*filepath);

extern char *DtDtsBufferToDataType(
		const void		*buffer,
		const int		size,
		const char		*opt_name);

extern char *DtDtsBufferToAttributeValue(
		const void		*buffer,
		const int		size,
		const char		*attr,
		const char		*opt_name);

extern	DtDtsAttribute	**DtDtsBufferToAttributeList(
		const void		*buffer,
		const int		size,
		const char		*opt_name);

extern char *DtDtsDataTypeToAttributeValue(
		const char		*datatype,
		const char		*attr,
		const char		*opt_name);

extern	DtDtsAttribute	**DtDtsDataTypeToAttributeList(
		const char		*datatype,
		const char		*opt_name);

extern void DtDtsFreeDataTypeNames(
		char			**namelist);

extern void DtDtsFreeAttributeList(
		DtDtsAttribute		**attr_list);

extern void DtDtsFreeAttributeValue(
		char			*attr_value);

extern void DtDtsFreeDataType(
		char			*datatype);

extern char **DtDtsDataTypeNames(void);

extern char **DtDtsFindAttribute(
		const char		*name,
		const char		*value);

extern char *DtDtsSetDataType(
		const char		*filepath,
		const char		*datatype,
		const int		override);

extern int DtDtsDataTypeIsAction(
		const char		*datatype);

#ifdef __cplusplus
}
#endif

#endif  /* _Dt_Dts_h */
