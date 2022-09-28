/*
 *  (c) Copyright 1996 Sun Microsystems, Inc.
 */

#pragma ident "@(#)sdtcm_admin.c	1.7 96/11/12 Sun Microsystems, Inc."

#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <nl_types.h>
#include <locale.h>
#include <LocaleXlate.h>
#include <libcsa/csa.h>
#include <libDtCmP/cm_tty.h>
#include "props.h"
#include "util.h"

extern int _csa_iso8601_to_tick(char *, time_t*);

typedef struct _Cal_Info {
	CSA_access_rights	*acls;
	char			*cal_name;
	char			*cal_owner;
	CSA_uint32		 size;
	char			*char_set;
	char			*date_created;
	CSA_uint32		 num_entries;
	char			*product_id;
	char			*version;
	CSA_uint32 		 server_version;
	CSA_uint32		 data_version;
} Cal_Info;

typedef enum _Acl_Level {
	PUBLIC_RIGHTS,
	CONFIDENTIAL_RIGHTS,
	PRIVATE_RIGHTS,
	CAL_RIGHTS,
	OWNER_RIGHTS,
	OLD_RIGHTS
} Acl_Level;

typedef enum _Perm_Token {
	T_EQUAL,
	T_PLUS,
	T_MINUS,
	T_COMMA,
	T_VIEW,
	T_INSERT,
	T_CHANGE,
	T_BROWSE,
	T_DELETE,
	T_PUBLIC,
	T_SEMIPRIV,
	T_PRIVATE,
	T_ATTRS,
	T_ROLE,
	T_OWNER,
	T_AUTHOR,
	T_SPONSOR,
	T_ORGANIZER,
	T_END,
	T_UNKNOWN
} Perm_Token;

/* routine to get common locale/charset name */
extern void _DtI18NXlateOpToStdLocale(char *operation, char *opLocale,
				      char **ret_stdLocale, char **ret_stdLang,
				      char **ret_stdSet);
static boolean_t add_cal(char *);
static boolean_t delete_cal(Props *, CSA_session_handle , char *, int);
static void list_cal(char *);
static void cal_info(CSA_session_handle, char *, u_long);
static char *get_user_calendar(Props *);
static CSA_session_handle cal_logon(char *, CSA_session_handle *);
static char *handle_error(CSA_return_code);
static char *print_acl(Acl_Level, CSA_flags);
static boolean_t mod_access(CSA_session_handle, char *, char *, u_long);
static void reset_mode(CSA_flags *, Perm_Token, u_int);
static void add_mode(CSA_flags *, Perm_Token, u_int);
static void remove_mode(CSA_flags *, Perm_Token, u_int);
static boolean_t update_access(CSA_flags *, char *);
static void update_acl(CSA_session_handle, CSA_access_rights *, 
		       CSA_attribute *, char *, char *, CSA_uint32);
static void set_old_mode(CSA_flags *, u_int);
static boolean_t update_old_access(CSA_flags *, char *);
static void remove_from_acl(CSA_session_handle, CSA_access_rights *, 
			    CSA_attribute *, char *, CSA_uint32);
static void list_access(CSA_session_handle);
static void print_short_acl(char *, CSA_flags, CSA_uint32);
static char *get_cal_name(char *, Props *);


#define ADD_FLG (1L<<0)
#define DEL_FLG (1L<<1)
#define INF_FLG (1L<<2)
#define LST_FLG (1L<<3)
#define PRM_FLG (1L<<4)
#define RM_FLG  (1L<<5)
#define PRT_FLG (1L<<6)

#define VIEW_PERM   (1L<<0)
#define CHANGE_PERM (1L<<1)
#define INSERT_PERM (1L<<2)
#define OWNER_PERM  (1L<<3)
#define AUTHOR_PERM  (1L<<4)
#define BROWSE_OPERM (1L<<5)
#define INSERT_OPERM (1L<<6)
#define DELETE_OPERM (1L<<7)

static nl_catd   catd;
char		*prog_name;

main(
	int	  argc,
	char	**argv)
{
	int	  	 c;
	extern char	*optarg;
	extern int	 optind;
	int		 errflg = 0;
	u_long		 flags = 0;
	boolean_t	 dont_ask = _B_FALSE;
	char		*cal_name = NULL;
	char		*name = NULL;
	char		*host = NULL;
	char		*new_rights_string = NULL;
	Props		*props = NULL;
	CSA_session_handle
			 cal;
	CSA_return_code	 status;

	prog_name = argv[0];

	while ((c = getopt(argc, argv, "?ac:dfh:iln:p:xz")) != EOF)
		switch (c) {
		case 'a':
			if (flags)
				errflg++;
			else
				flags |= ADD_FLG;
			break;
		case 'd':
			if (flags)
				errflg++;
			else
				flags |= DEL_FLG;
			break;
		case 'c':
			cal_name = optarg;
			break;
		case 'f':
			dont_ask = _B_TRUE;
			break;
		case 'h':
			host = optarg;
			break;
		case 'i':
			if (flags)
				errflg++;
			else
				flags |= INF_FLG;
			break;
		case 'l':
			if (flags)
				errflg++;
			else
				flags |= LST_FLG;
			break;
		case 'n':
			name = optarg;
			break;
		case 'p':
			if (flags)
				errflg++;
			else {
				flags |= PRM_FLG;
				new_rights_string = optarg;
			}
			break;
		case 'x':
			if (flags)
				errflg++;
			else
				flags |= PRT_FLG;
			break;
		case 'z':
			if (flags)
				errflg++;
			else
				flags |= RM_FLG;
			break;
		case '?':
			errflg++;
		}

	setlocale(LC_ALL, "");
	catd = catopen("SUNW_SDTCM_ADMIN", NL_CAT_LOCALE);
	
	if (!flags || errflg) {
		/* NL_COMMENT

		   Messages 3-24:

		   These messages are used to create the usage message.
		   The individual options (e.g. -a, -z, etc) should
		   not be translated.

		   usage: ./sdtcm_admin
		     -a [ -c <calendar@host> ]          Add a new calendar.
	       	     -d [ -c <calendar@host> ] [ -f ]   Remove a calendar. [-f don't prompt ]
		     -i [ -c <calendar@host> ]          Print information about a calendar.
		     -l [ -h <hostname> ]               List calendars.
		     -p <symbolic-mode-list>            Modify calendar's access list.
			[ -c <calendar@host> ]
			[ -n <name> ]
		     -x [ -c <calendar@host> ]          List calendar's access and rights.
		     -z [ -c <calendar@host> ]          Remove name from access list.
			[ -n <name> ]

		*/

		fprintf (stderr,
			catgets(catd, 1, 3, "usage: %s\n"), argv[0]);
		fprintf (stderr, catgets(catd, 1, 4, 
					"  %-35sAdd a new calendar.\n"), 
				 catgets(catd, 1, 5, 
					"-a [ -c <calendar@host> ]"));
		fprintf (stderr, catgets(catd, 1, 6, 
			"  %-35sRemove a calendar. [-f don't prompt ]\n"),
				 catgets(catd, 1, 7, 
				 	 "-d [ -c <calendar@host> ] [ -f ]"));
		fprintf (stderr, catgets(catd, 1, 8, 
				"  %-35sPrint information about a calendar.\n"),
				 catgets(catd, 1, 9, 
					"-i [ -c <calendar@host> ]"));
		fprintf (stderr, catgets(catd, 1, 10, 
					"  %-35sList calendars.\n"),
				 catgets(catd, 1, 11, "-l [ -h <hostname> ]"));
		fprintf (stderr, catgets(catd, 1, 12, 
				"  %-35sModify calendar's access list.\n"),
				 catgets(catd, 1, 13, 
					"-p <symbolic-mode-list>"));
		fprintf (stderr, catgets(catd, 1, 14, 
					"\t[ -c <calendar@host> ]\n"));
		fprintf (stderr, catgets(catd, 1, 15, "\t[ -n <name> ]\n"));
		fprintf (stderr, catgets(catd, 1, 20, 
				"  %-35sList calendar's access and rights.\n"),
				 catgets(catd, 1, 21, 
					"-x [ -c <calendar@host> ]"));
		fprintf (stderr, catgets(catd, 1, 22, 
				"  %-35sRemove name from access list.\n"),
				 catgets(catd, 1, 23, 
					"-z [ -c <calendar@host> ]"));
		fprintf (stderr, catgets(catd, 1, 24, "\t[ -n <name> ]\n"));
			
		exit (2);
	}

	cm_tty_load_props(&props);

	if (flags & ADD_FLG) {
		cal_name = get_cal_name(cal_name, props);
		add_cal(cal_name);
	} else if (flags & DEL_FLG) {
		cal_name = get_cal_name(cal_name, props);
		if ((status = cal_logon(cal_name, &cal)) != CSA_SUCCESS) {
			fprintf(stderr, 
				catgets(catd, 1, 25, 
				"%s: Could not delete calendar because: %s.\n"),
					prog_name, handle_error(status));
			exit(1);
		}
		delete_cal(props, cal, cal_name, dont_ask);
	} else if (flags & INF_FLG) {
		cal_name = get_cal_name(cal_name, props);
		if ((status = cal_logon(cal_name, &cal)) != CSA_SUCCESS) {
			fprintf(stderr, 
			       catgets(catd, 1, 26, 
			       "%s: Could not get calendar info because: %s.\n"),
					prog_name, handle_error(status));
			exit(1);
		}
		cal_info(cal, cal_name, flags);
	} else if (flags & LST_FLG) {
		list_cal(host);
	} else if (flags & (PRM_FLG | RM_FLG)) {
		if (!name) {
			fprintf (stderr, catgets(catd, 1, 27, 
						"-n <name> must be used in "));
			fprintf (stderr, 
				 catgets(catd, 1, 28, 
				"conjuction with the -p, -r or -z options.\n"));
			exit(1);
		}

		cal_name = get_cal_name(cal_name, props);
		if ((status = cal_logon(cal_name, &cal)) != CSA_SUCCESS) {
			fprintf(stderr, 
			      catgets(catd, 1, 29, 
			      "%s: Could not change permission because: %s.\n"),
			      		prog_name, handle_error(status));
			exit(1);
		}

		mod_access(cal, name, new_rights_string, flags);
	} else if (flags & PRT_FLG) {
		cal_name = get_cal_name(cal_name, props);
		if ((status = cal_logon(cal_name, &cal)) != CSA_SUCCESS) {
			fprintf(stderr, 
			      catgets(catd, 1, 30, 
			      "%s: Could not list permissions because: %s.\n"),
			      		prog_name, handle_error(status));
			exit(1);
		}
		list_access(cal);
	}

	return(0);
}

static boolean_t
mod_access(
	CSA_session_handle	 cal,
	char			*name,
	char			*new_rights_string,
	u_long			 flags)
{
	CSA_uint32		 num_attrs, num_attrs_ret;
	CSA_attribute_reference	 attr_names[2];
	CSA_attribute		*attrs;
	CSA_return_code		 status;
	CSA_access_rights	*acl_list;
	int			 i;
	CSA_uint32               data_version = 3;

	attr_names[0] = CSA_CAL_ATTR_ACCESS_LIST;
	attr_names[1] = CSA_X_DT_CAL_ATTR_DATA_VERSION;

	if ((status = csa_read_calendar_attributes(cal, 2, attr_names, 
						   &num_attrs_ret, &attrs, 
						   NULL)) != CSA_SUCCESS) {
		fprintf(stderr,
			catgets(catd, 1, 29, 
			"%s: Could not change permission because: %s.\n"), 
				prog_name, handle_error(status));
		exit(1);
	}	

	for (i = 0; i < num_attrs_ret; i++) {
		if (!strcmp(attrs[i].name, CSA_CAL_ATTR_ACCESS_LIST)) {
			acl_list = attrs[i].value->item.access_list_value;
		} else if (!strcmp(attrs[i].name, 
					CSA_X_DT_CAL_ATTR_DATA_VERSION)) {
			data_version = attrs[i].value->item.uint32_value;
		}
	}

	if (flags & PRM_FLG)
		update_acl(cal, acl_list, attrs, name, 
			   new_rights_string, data_version);
	else
		remove_from_acl(cal, acl_list, attrs, name, data_version);

	return(_B_TRUE);
}

static boolean_t
delete_cal(
	Props			*props,
	CSA_session_handle       cal,
	char			*cal_name,
	int			 dont_ask)
{
	CSA_return_code		 status;
	char			 input_buf[8];

	if (!dont_ask) {
		printf (catgets(catd, 1, 32, "Delete the calendar %s (y/n)? "), 
				cal_name);
		gets(input_buf);
		if (input_buf[0] != 'y') {
			printf (catgets(catd, 1, 33, "Delete aborted.\n"));
			exit(1);
		}
	}
	
	if ((status = csa_delete_calendar(cal, NULL)) != CSA_SUCCESS) {
		fprintf(stderr, catgets(catd, 1, 34, 
				"%s: Could not delete calendar because: %s.\n"),
					prog_name, handle_error(status));
		exit(1);
	}
	return(_B_TRUE);
}

static boolean_t
add_cal(char	*cal_name)
{
	char			*locale;
	CSA_session_handle       cal;
	CSA_attribute 		 attrs[1];
	CSA_attribute_value 	 val[1];
	CSA_calendar_user 	 user;
	CSA_return_code		 status;

	user.user_name = cm_get_uname();
	user.user_type = CSA_USER_TYPE_INDIVIDUAL;
	user.calendar_address = cal_name;
	user.calendar_user_extensions = NULL;

	/* Get the CDE platform independent locale name of the locale
	 * the application is running in.
	 */
	_DtI18NXlateOpToStdLocale(DtLCX_OPER_SETLOCALE,
				  setlocale(LC_CTYPE, NULL), 
				  &locale, NULL, NULL);
	
	if (locale) {
		attrs[0].name = CSA_CAL_ATTR_CHARACTER_SET;
		attrs[0].value = &val[0];
		attrs[0].attribute_extensions = NULL;
		val[0].type = CSA_VALUE_STRING;
		val[0].item.string_value = locale;
	}

	if ((status = csa_add_calendar(NULL, &user, 1, attrs, NULL)) != 
								CSA_SUCCESS) { 

		if (locale && status == CSA_E_INVALID_ATTRIBUTE) {
			/* A server of version 4 or less will not support
			 * the CSA_CAL_ATTR_CHARACTER_SET cal attribute.
			 */
			status = csa_add_calendar(NULL, &user, 0, NULL, NULL);
		}

		if (status != CSA_SUCCESS) {
			fprintf(stderr, catgets(catd, 1, 35, 
				"%s: Could not create calendar because: %s.\n"),
					prog_name, handle_error(status));
			exit(1);
		}
	}
	return (0);
}

static void
list_cal(
	char	*cal_name)
{
	CSA_uint32		 nusers;
	CSA_calendar_user	*users;
	CSA_return_code		 status;
	int			 i;

	if (!cal_name)
                cal_name = cm_get_local_host();

	if ((status = csa_list_calendars((CSA_service_reference)cal_name,
				&nusers, &users, NULL)) != CSA_SUCCESS) {
		fprintf(stderr, catgets(catd, 1, 36, 
				"%s: Could not list calendars because: %s.\n"), 
					prog_name, handle_error(status));
		exit(1);
	}

	if (nusers == 0) {
		printf (catgets(catd, 1, 37, "No calendars to list.\n"));
		exit(0);
	}

	for (i = 0; i < nusers; i++) {
		/* If string begins with callog. then it is a Sun impl and
		 * we remove the prefix.
		 */
		if (strncmp(users[i].calendar_address, "callog.", 7) == 0) {
			users[i].calendar_address += 7;
		}
		printf ("%s\n", users[i].calendar_address);
	}

	
}

static void
cal_info(
	CSA_session_handle       cal,
	char			*cal_name,
	u_long			 flags)
{
	CSA_uint32		 num_attrs, num_attrs_ret;
	CSA_attribute_reference	*names;
	CSA_attribute		*attrs;
	CSA_return_code		 status;
	int			 i;
	Cal_Info		 cali;

	memset(&cali, '\0', sizeof(Cal_Info));
	cali.num_entries = -1;
	cali.size = -1;
	cali.data_version = -1;

	if ((status = csa_list_calendar_attributes(cal, &num_attrs, &names, 
						   NULL)) != CSA_SUCCESS) {
		fprintf(stderr,
			catgets(catd, 1, 38, 
			"%s: Could not get calendar info because: %s.\n"), 
				prog_name, handle_error(status));
		exit(1);
	}
	if ((status = csa_read_calendar_attributes(cal, num_attrs, names, 
						   &num_attrs_ret, &attrs, 
						   NULL)) != CSA_SUCCESS) {
		fprintf(stderr,
			catgets(catd, 1, 38, 
			"%s: Could not get calendar info because: %s.\n"), 
				prog_name, handle_error(status));
		exit(1);
	}

	for (i = 0; i < num_attrs_ret; i++) {
		if (!strcmp(attrs[i].name, CSA_CAL_ATTR_ACCESS_LIST)) {
			cali.acls = attrs[i].value->item.access_list_value;
		} else if (!strcmp(attrs[i].name, CSA_CAL_ATTR_CALENDAR_NAME)) {
			cali.cal_name = attrs[i].value->item.string_value;
		} else if (!strcmp(attrs[i].name, CSA_CAL_ATTR_CALENDAR_OWNER)){
			cali.cal_owner = 
			    attrs[i].value->item.calendar_user_value->user_name;
		} else if (!strcmp(attrs[i].name, CSA_CAL_ATTR_CALENDAR_SIZE)) {
			cali.size = attrs[i].value->item.uint32_value;
		} else if (!strcmp(attrs[i].name, CSA_CAL_ATTR_CHARACTER_SET)) {
			cali.char_set = attrs[i].value->item.string_value;
		} else if (!strcmp(attrs[i].name, CSA_CAL_ATTR_DATE_CREATED))  {
			cali.date_created = attrs[i].value->item.string_value;
		} else if (!strcmp(attrs[i].name, CSA_CAL_ATTR_NUMBER_ENTRIES)){
			cali.num_entries = attrs[i].value->item.uint32_value;
		} else if (!strcmp(attrs[i].name, CSA_CAL_ATTR_VERSION))       {
			cali.version = attrs[i].value->item.string_value;
		} else if (!strcmp(attrs[i].name, 
					CSA_CAL_ATTR_PRODUCT_IDENTIFIER))      {
			cali.product_id = attrs[i].value->item.string_value;
		} else if (!strcmp(attrs[i].name, 
					CSA_X_DT_CAL_ATTR_SERVER_VERSION))     {
			cali.server_version = attrs[i].value->item.uint32_value;
		} else if (!strcmp(attrs[i].name, 
					CSA_X_DT_CAL_ATTR_DATA_VERSION))       {
			cali.data_version = attrs[i].value->item.uint32_value;
		} else {
			printf (catgets(catd, 1, 40, "unsupported: %s\n"), 
								attrs[i].name);
		}
	}

	if (flags & INF_FLG) {
		time_t		 	 tick = 0;
		struct tm 		*tm;
		char		 	 time_buf[64];
		CSA_access_rights	*acl_list;

		if (cali.date_created) {
			_csa_iso8601_to_tick(cali.date_created, &tick);
			tm = localtime(&tick);
			/* NL_COMMENT

			   Message 41: This message represents the time the
			   calendar was created.  It is handed to strftime.
			   It will be later used in a string that looks 
			   like this:

			   date created:   Tue Sep 19 08:34:23 PDT 1995

			*/
			strftime(time_buf, 64, catgets(catd, 1, 41, "%C"), tm);
		}

		printf (catgets(catd, 1, 42, "calendar:       %s\n"), 
				cali.cal_name ? cali.cal_name : 
					catgets(catd, 1, 43, "unknown"));
		printf (catgets(catd, 1, 44, "owner:          %s\n"), 
				cali.cal_owner ? cali.cal_owner : 
					catgets(catd, 1, 43, "unknown"));
		printf (catgets(catd, 1, 46, "date created:   %s\n"), 
				tick ? time_buf : 
					catgets(catd, 1, 43, "unknown"));
		if (cali.data_version != -1)
			printf (catgets(catd, 1, 48, "data version:   %d\n"), 
					cali.data_version);
		else
			printf (catgets(catd, 1, 49, 
					"data version:   unknown\n"));
		if (cali.server_version)
			printf (catgets(catd, 1, 50, "server version: %d\n"), 
					cali.server_version);
		else
			printf (catgets(catd, 1, 51, 
					"server version: unknown\n"));
		if (cali.size != -1)
			printf (catgets(catd, 1, 52, "size (bytes):   %d\n"), 
					cali.size);
		else
			printf (catgets(catd, 1, 53, 
					"size (bytes):   unknown\n"));
		if (cali.num_entries != -1)
			printf (catgets(catd, 1, 54, 
					"entries:        %d\n"), 
						cali.num_entries);
		else
			printf (catgets(catd, 1, 55, 
					"entries:        unknown\n"));
		printf (catgets(catd, 1, 56, "character set:  %s\n"),
				cali.char_set ? cali.char_set : 
					catgets(catd, 1, 43, "unknown"));
		printf (catgets(catd, 1, 58, "product id:     %s\n"),
				cali.product_id ? cali.product_id : 
					catgets(catd, 1, 43, "unknown"));

		printf (catgets(catd, 1, 60, "access rights:\n"));
		acl_list = cali.acls;
		while (acl_list) {
			if (cali.data_version > 3) {
				printf ("--> %s\n", acl_list->user->user_name);
				/* NL_COMMENT

				   Message 61: The %20s %s conversions hold 
				   space for the strings:

				                Public: View Insert Change 
				           Semiprivate: --
					       Private: --
			           Calendar Attributes: --
					         Roles: --

				  The idea is that they are all colon aligned.

				   The field size is currently 20 but should
				   be changed to match the the length of the
				   ``Calendar Attributes:'' string when 
				   translated.  

				*/

				printf (catgets(catd, 1, 61, "%20s %s\n"), 
					catgets(catd, 1, 62, "Public:"), 
					print_acl(PUBLIC_RIGHTS, 
						acl_list->rights));
				printf (catgets(catd, 1, 63, "%20s %s\n"), 
					catgets(catd, 1, 64, "Semiprivate:"), 
					print_acl(CONFIDENTIAL_RIGHTS, 
						 acl_list->rights));
				printf (catgets(catd, 1, 65, "%20s %s\n"), 
					catgets(catd, 1, 66, "Private:"), 
					print_acl(PRIVATE_RIGHTS, 
						acl_list->rights));
				printf (catgets(catd, 1, 67, "%20s %s\n"), 
					catgets(catd, 1, 68, 
						"Calendar Attributes:"), 
					print_acl(CAL_RIGHTS, 
						acl_list->rights));
				printf (catgets(catd, 1, 69, "%20s %s\n\n"), 
					catgets(catd, 1, 70, "Roles:"), 
					print_acl(OWNER_RIGHTS, 
						acl_list->rights));
			} else {
				printf ("--> %s: %s\n", 
					acl_list->user->user_name,
					print_acl(OLD_RIGHTS, 
						  acl_list->rights));
			}
			acl_list = acl_list->next;
		}
	} else {
		if (cali.server_version)
			printf ("%d\n", cali.server_version);
		else
			printf (catgets(catd, 1, 71, "unknown\n"));
	}
}

static CSA_return_code
cal_logon(
	char			*cal_name,
	CSA_session_handle	*cal)
{
	char			*target;
	CSA_calendar_user        csa_user;
	CSA_return_code		 status;

	target = (cal_name) ? cal_name : cm_get_credentials();

	csa_user.user_name = target;
	csa_user.user_type = 0;
	csa_user.calendar_user_extensions = NULL;
	csa_user.calendar_address = target;

	status = csa_logon(NULL, &csa_user, NULL, NULL, NULL, cal, NULL);
					 
	return (status);
}

static char *
get_user_calendar(
	Props	*props)
{
        char    *name, 
		*uname, 
		*loc;
        boolean_t  needfree = _B_FALSE;
 
        uname = cm_get_uname();
        if ((loc = get_char_prop(props, CP_CALLOC)) && *loc != NULL) {
                loc = strdup(loc);
                needfree = _B_TRUE;
        } else
                loc = cm_get_local_host();

        name = malloc(strlen(uname) + strlen(loc) + 2);
        sprintf(name, "%s@%s", uname, loc);

        if (needfree) free(loc);

        return (name);
}

static char *
handle_error(
	CSA_return_code		status)
{
	static char buf[256];

	memset(buf, '\0', 256);

	switch (status) {
	case CSA_SUCCESS:
		break;
	case CSA_E_CALENDAR_EXISTS:
		sprintf(buf, catgets(catd, 1, 72, "Calendar already exists")); 
		break;
	case CSA_X_DT_E_ENTRY_NOT_FOUND:
		sprintf(buf, catgets(catd, 1, 73, "Entry was not found")); 
		break;
	case CSA_E_CALENDAR_NOT_EXIST:
		sprintf(buf, catgets(catd, 1, 74, "Calendar does not exist")); 
		break;
	case CSA_E_INVALID_ENTRY_HANDLE:
		sprintf(buf, catgets(catd, 1, 75, "Invalid entry handle")); 
		break;
	case CSA_E_INVALID_SESSION_HANDLE:
		sprintf(buf, catgets(catd, 1, 76, "Invalid session handle")); 
		break;
	case CSA_E_NO_AUTHORITY:
		sprintf(buf, catgets(catd, 1, 77, "Permission denied")); 
		break;
	case CSA_E_INVALID_PARAMETER:
		sprintf(buf, catgets(catd, 1, 78, "Invalid parameter")); 
		break;
	case CSA_E_READONLY:
		sprintf(buf, catgets(catd, 1, 79, "Read only")); 
		break;
	case CSA_E_INVALID_ATTRIBUTE_VALUE:
		sprintf(buf, catgets(catd, 1, 80, "Invalid attribute value")); 
		break;
	case CSA_E_UNSUPPORTED_ATTRIBUTE:
		sprintf(buf, catgets(catd, 1, 81, "Unsupported attribute")); 
		break;
	case CSA_E_NOT_SUPPORTED:
		sprintf(buf, catgets(catd, 1, 82, "Not supported")); 
		break;
	case CSA_E_INVALID_ENUM:
		sprintf(buf, catgets(catd, 1, 83, "Invalid enumerator")); 
		break;
	case CSA_E_INSUFFICIENT_MEMORY:
		sprintf(buf, catgets(catd, 1, 84, "Insufficient memory")); 
		break;
	case CSA_E_DISK_FULL:
		sprintf(buf, catgets(catd, 1, 85, "Disk is full")); 
		break;
	case CSA_E_SERVICE_UNAVAILABLE:
		sprintf(buf, catgets(catd, 1, 86, "Service is unavailable")); 
		break;
	case CSA_X_DT_E_INVALID_SERVER_LOCATION:
		sprintf(buf, catgets(catd, 1, 87, "Invalid server location")); 
		break;
	case CSA_X_DT_E_SERVICE_NOT_REGISTERED:
		sprintf(buf, catgets(catd, 1, 88, 
					"Calendar service is not registered")); 
		break;
	case CSA_X_DT_E_SERVER_TIMEOUT:
		sprintf(buf, catgets(catd, 1, 89, "Server is not responding")); 
		break;
	case CSA_E_FAILURE:
		sprintf(buf, catgets(catd, 1, 90, "Unknown failure")); 
		break;
	default:
		sprintf(buf, catgets(catd, 1, 91, "Unsupported error code")); 
		break;
	}

	return (buf);

}

static char *
print_acl(
	Acl_Level	type,
	CSA_flags	rights)
{
	static char buf[32];

	memset(buf, '\0', 32);

	if (type == PUBLIC_RIGHTS) {
		if (rights & CSA_VIEW_PUBLIC_ENTRIES)
			strcat(buf, catgets(catd, 1, 92, "View "));
		if (rights & CSA_INSERT_PUBLIC_ENTRIES)
			strcat(buf, catgets(catd, 1, 93, "Insert "));
		if (rights & CSA_CHANGE_PUBLIC_ENTRIES)
			strcat(buf, catgets(catd, 1, 94, "Change "));
	} else if (type == CONFIDENTIAL_RIGHTS) {
		if (rights & CSA_VIEW_CONFIDENTIAL_ENTRIES)
			strcat(buf, catgets(catd, 1, 92, "View "));
		if (rights & CSA_INSERT_CONFIDENTIAL_ENTRIES)
			strcat(buf, catgets(catd, 1, 93, "Insert "));
		if (rights & CSA_CHANGE_CONFIDENTIAL_ENTRIES)
			strcat(buf, catgets(catd, 1, 94, "Change "));
	} else if (type == PRIVATE_RIGHTS) {
		if (rights & CSA_VIEW_PRIVATE_ENTRIES)
			strcat(buf, catgets(catd, 1, 92, "View "));
		if (rights & CSA_INSERT_PRIVATE_ENTRIES)
			strcat(buf, catgets(catd, 1, 93, "Insert "));
		if (rights & CSA_CHANGE_PRIVATE_ENTRIES)
			strcat(buf, catgets(catd, 1, 94, "Change "));
	} else if (type == CAL_RIGHTS) {
		if (rights & CSA_VIEW_CALENDAR_ATTRIBUTES)
			strcat(buf, catgets(catd, 1, 92, "View "));
		if (rights & CSA_INSERT_CALENDAR_ATTRIBUTES)
			strcat(buf, catgets(catd, 1, 93, "Insert "));
		if (rights & CSA_CHANGE_CALENDAR_ATTRIBUTES)
			strcat(buf, catgets(catd, 1, 94, "Change "));
	} else if (type == OWNER_RIGHTS) {
		if (rights & CSA_OWNER_RIGHTS)
			strcat(buf, catgets(catd, 1, 104, "Owner "));
		if (rights & CSA_SPONSOR_RIGHTS)
			strcat(buf, catgets(catd, 1, 105, "Sponsor "));
		if (rights & CSA_ORGANIZER_RIGHTS)
			strcat(buf, catgets(catd, 1, 106, "Author "));
	} else if (type == OLD_RIGHTS) {
		if (rights & CSA_X_DT_BROWSE_ACCESS)
			strcat(buf, catgets(catd, 1, 107, "Browse "));
		if (rights & CSA_X_DT_INSERT_ACCESS)
			strcat(buf, catgets(catd, 1, 108, "Insert "));
		if (rights & CSA_X_DT_DELETE_ACCESS)
			strcat(buf, catgets(catd, 1, 109, "Delete "));
	}

/* XXX: Why did I do it this way???
	if (type == OWNER_RIGHTS) { 
		if (buf[0] == '\0')
			strcat(buf, catgets(catd, 1, 110, "None"));
	} else {
		if (buf[0] == '\0')
			strcat(buf, catgets(catd, 1, 111, "--"));
	}
*/

	if (buf[0] == '\0')
		strcat(buf, catgets(catd, 1, 111, "--"));

	return (buf);
}

/* Assumes token is not greater than two characters */
static Perm_Token
get_token(
	char 	*string,
	int	 str_len,
	int	*pos)
{
	char	 buf[16];
	int	 i = 0;

	memset(buf, '\0', 16);

	if (*pos >= str_len) return (T_END);

	buf[i] = string[(*pos)++];
	if (*pos < str_len) 
		buf[i+1] = string[(*pos)];

	if (buf[i] == '=') return (T_EQUAL);
	if (buf[i] == '+') return (T_PLUS);
	if (buf[i] == '-') return (T_MINUS);
	if (buf[i] == ',') return (T_COMMA);
	if (buf[i] == 'v') return (T_VIEW);
	if (buf[i] == 'i') return (T_INSERT);
	if (buf[i] == 'c') return (T_CHANGE);
	if (buf[i] == 'b') return (T_BROWSE);
	if (buf[i] == 'd') return (T_DELETE);
	if (buf[i] == 'o') return (T_OWNER);
	if ((buf[i] == 'a') && 
	    (buf[i+1] != 't') && (buf[i+1] != 'r')) 
		return (T_AUTHOR);

	if (*pos >= str_len) return (T_END);

	(*pos)++;

	if (!strcmp(buf, "pu")) return (T_PUBLIC);
	if (!strcmp(buf, "sp")) return (T_SEMIPRIV);
	if (!strcmp(buf, "pr")) return (T_PRIVATE);
	if (!strcmp(buf, "at")) return (T_ATTRS);
	if (!strcmp(buf, "ar")) return (T_ROLE);

	return (T_UNKNOWN);
}

static boolean_t
update_access(
	CSA_flags 	*old_rights, 
	char		*new_rights_string)
{
	Perm_Token	 token,
			 access_type,
			 access_mode;
	u_int		 access_flag = 0;
	int		 pos = 0;
	int		 str_len = strlen(new_rights_string);
	boolean_t	 at_end = _B_FALSE,
			 get_next_token;

	if (!strcmp("noaccess", new_rights_string)) {
		*old_rights = (CSA_flags)0;
		return (_B_TRUE);
	}

	while (!at_end) {
		/* Expect "pu", "sp", "pr", "at" or "ar" */
		token = get_token(new_rights_string, str_len, &pos);
		switch (token) {
		case T_PUBLIC:
		case T_SEMIPRIV:
		case T_PRIVATE:
		case T_ATTRS:
		case T_ROLE:
			/* Do nothing, this was what we were expecting */
			access_type = token;
			break;
		default:
			return _B_FALSE;
		}

		/* Expect "+", "=", "-" */
		token = get_token(new_rights_string, str_len, &pos);
		switch (token) {
		case T_EQUAL:
		case T_PLUS:
		case T_MINUS:
			/* Do nothing, this was what we were expecting */
			access_mode = token;
			break;
		default:
			return _B_FALSE;
		}

		get_next_token = _B_TRUE;
		access_flag = 0;

		/* Expect "v", "i", "c", ",", "a", "o" or the end of line */
		while (get_next_token) {
			token = get_token(new_rights_string, str_len, &pos);

			switch (token) {
			case T_VIEW:
				if (access_type == T_ROLE) return _B_FALSE;
				access_flag |= VIEW_PERM;
				break;
			case T_INSERT:
				if (access_type == T_ROLE) return _B_FALSE;
				access_flag |= INSERT_PERM;
				break;
			case T_CHANGE:
				if (access_type == T_ROLE) return _B_FALSE;
				access_flag |= CHANGE_PERM;
				break;
			case T_OWNER:
				if (access_type != T_ROLE) return _B_FALSE;
				access_flag |= OWNER_PERM;
				break;
			case T_AUTHOR:
				if (access_type != T_ROLE) return _B_FALSE;
				access_flag |= AUTHOR_PERM;
				break;
			case T_END:
				at_end = _B_TRUE;
			case T_COMMA:
				get_next_token = _B_FALSE;
				if (!access_flag) return _B_FALSE;
				break;
			default:
				return _B_FALSE;
			}
		}

		if (access_mode == T_EQUAL)
			reset_mode(old_rights, access_type, access_flag);
		else if (access_mode == T_PLUS)
			add_mode(old_rights, access_type, access_flag);
		else
			remove_mode(old_rights, access_type, access_flag);

	}
	return (_B_TRUE);
}

static boolean_t
update_old_access(
	CSA_flags 	*old_rights, 
	char		*new_rights_string)
{
	Perm_Token	 token;
	u_int		 access_flag = 0;
	int		 pos = 0;
	int		 str_len = strlen(new_rights_string);
	boolean_t	 at_end = _B_FALSE;

	*old_rights = (CSA_flags)0;

	if (!strcmp("noaccess", new_rights_string)) {
		*old_rights = (CSA_flags)0;
		return (_B_TRUE);
	}

	while (!at_end) {
		/* Expect "b", "i", "d" */
		token = get_token(new_rights_string, str_len, &pos);
		switch (token) {
		case T_BROWSE:
			access_flag |= BROWSE_OPERM;
			break;
		case T_INSERT:
			access_flag |= INSERT_OPERM;
			break;
		case T_DELETE:
			access_flag |= DELETE_OPERM;
			break;
		case T_END:
			at_end = _B_TRUE;
			break;
		default:
			return _B_FALSE;
		}
	}

	set_old_mode(old_rights, access_flag);
	return (_B_TRUE);
}

static void
set_old_mode(
	CSA_flags	*old_rights,
	u_int	 	 access_flag)
{
	if (access_flag & BROWSE_OPERM)
		*old_rights |= CSA_X_DT_BROWSE_ACCESS;
	if (access_flag & INSERT_OPERM)
		*old_rights |= CSA_X_DT_INSERT_ACCESS;
	if (access_flag & DELETE_OPERM)
		*old_rights |= CSA_X_DT_DELETE_ACCESS;
}


static void
reset_mode(
	CSA_flags	*old_rights,
	Perm_Token	 access_type,
	u_int	 	 access_flag)
{

	switch (access_type) {
	case T_PUBLIC:
		*old_rights &= ~(CSA_VIEW_PUBLIC_ENTRIES | 
				 CSA_INSERT_PUBLIC_ENTRIES | 
				 CSA_CHANGE_PUBLIC_ENTRIES); 
		if (access_flag & VIEW_PERM)
			*old_rights |= CSA_VIEW_PUBLIC_ENTRIES;
		if (access_flag & INSERT_PERM)
			*old_rights |= CSA_INSERT_PUBLIC_ENTRIES;
		if (access_flag & CHANGE_PERM)
			*old_rights |= CSA_CHANGE_PUBLIC_ENTRIES;
		break;
	case T_SEMIPRIV:
		*old_rights &= ~(CSA_VIEW_CONFIDENTIAL_ENTRIES | 
				 CSA_INSERT_CONFIDENTIAL_ENTRIES | 
				 CSA_CHANGE_CONFIDENTIAL_ENTRIES); 
		if (access_flag & VIEW_PERM)
			*old_rights |= CSA_VIEW_CONFIDENTIAL_ENTRIES;
		if (access_flag & INSERT_PERM)
			*old_rights |= CSA_INSERT_CONFIDENTIAL_ENTRIES;
		if (access_flag & CHANGE_PERM)
			*old_rights |= CSA_CHANGE_CONFIDENTIAL_ENTRIES;
		break;
	case T_PRIVATE:
		*old_rights &= ~(CSA_VIEW_PRIVATE_ENTRIES | 
				 CSA_INSERT_PRIVATE_ENTRIES | 
				 CSA_CHANGE_PRIVATE_ENTRIES); 
		if (access_flag & VIEW_PERM)
			*old_rights |= CSA_VIEW_PRIVATE_ENTRIES;
		if (access_flag & INSERT_PERM)
			*old_rights |= CSA_INSERT_PRIVATE_ENTRIES;
		if (access_flag & CHANGE_PERM)
			*old_rights |= CSA_CHANGE_PRIVATE_ENTRIES;
		break;
	case T_ATTRS:
		*old_rights &= ~(CSA_VIEW_CALENDAR_ATTRIBUTES | 
				 CSA_INSERT_CALENDAR_ATTRIBUTES | 
				 CSA_CHANGE_CALENDAR_ATTRIBUTES); 
		if (access_flag & VIEW_PERM)
			*old_rights |= CSA_VIEW_CALENDAR_ATTRIBUTES;
		if (access_flag & INSERT_PERM)
			*old_rights |= CSA_INSERT_CALENDAR_ATTRIBUTES;
		if (access_flag & CHANGE_PERM)
			*old_rights |= CSA_CHANGE_CALENDAR_ATTRIBUTES;
		break;
	case T_ROLE:
		*old_rights &= ~(CSA_ORGANIZER_RIGHTS | 
				 CSA_SPONSOR_RIGHTS | 
				 CSA_OWNER_RIGHTS); 
		if (access_flag & OWNER_PERM)
			*old_rights |= CSA_OWNER_RIGHTS;
		if (access_flag & AUTHOR_PERM)
			*old_rights |= CSA_ORGANIZER_RIGHTS;
		break;
	}
}

static void
add_mode(
	CSA_flags	*old_rights,
	Perm_Token	 access_type,
	u_int	 	 access_flag)
{
	switch (access_type) {
	case T_PUBLIC:
		if (access_flag & VIEW_PERM)
			*old_rights |= CSA_VIEW_PUBLIC_ENTRIES;
		if (access_flag & INSERT_PERM)
			*old_rights |= CSA_INSERT_PUBLIC_ENTRIES;
		if (access_flag & CHANGE_PERM)
			*old_rights |= CSA_CHANGE_PUBLIC_ENTRIES;
		break;
	case T_SEMIPRIV:
		if (access_flag & VIEW_PERM)
			*old_rights |= CSA_VIEW_CONFIDENTIAL_ENTRIES;
		if (access_flag & INSERT_PERM)
			*old_rights |= CSA_INSERT_CONFIDENTIAL_ENTRIES;
		if (access_flag & CHANGE_PERM)
			*old_rights |= CSA_CHANGE_CONFIDENTIAL_ENTRIES;
		break;
	case T_PRIVATE:
		if (access_flag & VIEW_PERM)
			*old_rights |= CSA_VIEW_PRIVATE_ENTRIES;
		if (access_flag & INSERT_PERM)
			*old_rights |= CSA_INSERT_PRIVATE_ENTRIES;
		if (access_flag & CHANGE_PERM)
			*old_rights |= CSA_CHANGE_PRIVATE_ENTRIES;
		break;
	case T_ATTRS:
		if (access_flag & VIEW_PERM)
			*old_rights |= CSA_VIEW_CALENDAR_ATTRIBUTES;
		if (access_flag & INSERT_PERM)
			*old_rights |= CSA_INSERT_CALENDAR_ATTRIBUTES;
		if (access_flag & CHANGE_PERM)
			*old_rights |= CSA_CHANGE_CALENDAR_ATTRIBUTES;
		break;
	case T_ROLE:
		if (access_flag & OWNER_PERM)
			*old_rights |= CSA_OWNER_RIGHTS;
		if (access_flag & AUTHOR_PERM)
			*old_rights |= CSA_ORGANIZER_RIGHTS;
		break;
	}
}

static void
remove_mode(
	CSA_flags	*old_rights,
	Perm_Token	 access_type,
	u_int	 	 access_flag)
{
	switch (access_type) {
	case T_PUBLIC:
		if (access_flag & VIEW_PERM)
			*old_rights &= ~CSA_VIEW_PUBLIC_ENTRIES;
		if (access_flag & INSERT_PERM)
			*old_rights &= ~CSA_INSERT_PUBLIC_ENTRIES;
		if (access_flag & CHANGE_PERM)
			*old_rights &= ~CSA_CHANGE_PUBLIC_ENTRIES;
		break;
	case T_SEMIPRIV:
		if (access_flag & VIEW_PERM)
			*old_rights &= ~CSA_VIEW_CONFIDENTIAL_ENTRIES;
		if (access_flag & INSERT_PERM)
			*old_rights &= ~CSA_INSERT_CONFIDENTIAL_ENTRIES;
		if (access_flag & CHANGE_PERM)
			*old_rights &= ~CSA_CHANGE_CONFIDENTIAL_ENTRIES;
		break;
	case T_PRIVATE:
		if (access_flag & VIEW_PERM)
			*old_rights &= ~CSA_VIEW_PRIVATE_ENTRIES;
		if (access_flag & INSERT_PERM)
			*old_rights &= ~CSA_INSERT_PRIVATE_ENTRIES;
		if (access_flag & CHANGE_PERM)
			*old_rights &= ~CSA_CHANGE_PRIVATE_ENTRIES;
		break;
	case T_ATTRS:
		if (access_flag & VIEW_PERM)
			*old_rights &= ~CSA_VIEW_CALENDAR_ATTRIBUTES;
		if (access_flag & INSERT_PERM)
			*old_rights &= ~CSA_INSERT_CALENDAR_ATTRIBUTES;
		if (access_flag & CHANGE_PERM)
			*old_rights &= ~CSA_CHANGE_CALENDAR_ATTRIBUTES;
		break;
	case T_ROLE:
		if (access_flag & OWNER_PERM)
			*old_rights &= ~CSA_OWNER_RIGHTS;
		if (access_flag & AUTHOR_PERM)
			*old_rights &= ~CSA_ORGANIZER_RIGHTS;
		break;
	}
}

static void
update_acl(
	CSA_session_handle	 cal,
	CSA_access_rights	*acl_list,
	CSA_attribute		*attrs,
	char			*name,
	char			*new_rights_string,
	CSA_uint32		 version)
{
	CSA_access_rights	*acl_list_head,
				*acl_list_tail = NULL,
				 new_acl;
	CSA_attribute_value 	 val[1];
	CSA_return_code		 status;
	CSA_flags		 old_rights = (CSA_flags)0;
	CSA_calendar_user	 user;
	boolean_t		 matched_name = _B_FALSE;

	/* See if the name is already in the access list */
	acl_list_head = acl_list;
	while (acl_list) {
		if (!strcmp(name, acl_list->user->user_name)) {
			old_rights = acl_list->rights;
			if (version > 3) {
				if (!update_access(&old_rights, 
							new_rights_string)) {
					fprintf(stderr, catgets(catd, 1, 112, 
							"%s: Syntax error. "), 
						prog_name);
					fprintf(stderr, catgets(catd, 1, 113, 
					"Check your command options format.\n")
									      );
					exit(1);
				}
			} else {
				if (!update_old_access(&old_rights, 
							new_rights_string)) {
					fprintf(stderr, catgets(catd, 1, 112, 
							"%s: Syntax error. "), 
							prog_name);
					fprintf(stderr, catgets(catd, 1, 113, 
					"Check your command options format.\n")
									      );
					exit(1);
				}
			}
			acl_list->rights = old_rights;
			matched_name = _B_TRUE;
			break;
		}
	acl_list_tail = acl_list;
	acl_list = acl_list->next;
	}

	/* We didn't match any existing names in the acl list so we add a
	 * new one.
	 */
	if (!matched_name) {

		if (version > 3) {
			if (!update_access(&old_rights, new_rights_string)) {
				fprintf(stderr, catgets(catd, 1, 112, 
						"%s: Syntax error. "), 
					prog_name);
				fprintf(stderr, catgets(catd, 1, 113, 
					"Check your command options format.\n")
									      );
				exit(1); 
			}
		} else {
			if (!update_old_access(&old_rights, 
							new_rights_string)) {
				fprintf(stderr, catgets(catd, 1, 112, 
						"%s: Syntax error. "), 
					prog_name);
				fprintf(stderr, catgets(catd, 1, 113, 
					"Check your command options format.\n")
									      );
				exit(1);
			}
		}
		user.user_name = name;
		user.user_type = CSA_USER_TYPE_INDIVIDUAL;
		new_acl.user = &user;
		new_acl.rights = old_rights;
		new_acl.next = NULL;
		if (!acl_list_tail) 
			acl_list_head = &new_acl;
		else
			acl_list_tail->next = &new_acl;
	}

	/* Update the calendar attributes */
	attrs[0].name = CSA_CAL_ATTR_ACCESS_LIST;
	attrs[0].value = &val[0];
	val[0].type = CSA_VALUE_ACCESS_LIST;
	val[0].item.access_list_value = acl_list_head;
	if ((status = csa_update_calendar_attributes(cal, 1, 
					attrs, NULL)) != CSA_SUCCESS) {
		fprintf(stderr, catgets(catd, 1, 120, 
					"%s: Could not change permission"), 
					prog_name);
		fprintf(stderr, catgets(catd, 1, 121, " because: %s.\n"), 
					handle_error(status));
		exit(1);
	}        
}

static void
remove_from_acl(
	CSA_session_handle	 cal,
	CSA_access_rights	*acl_list,
	CSA_attribute		*attrs,
	char			*name,
	CSA_uint32		 version)
{
	CSA_access_rights	*acl_list_head,
				*acl_list_last;
	CSA_attribute_value 	 val[1];
	CSA_return_code		 status;
	boolean_t		 found_it = _B_FALSE;

	/* See if the name is already in the access list */
	acl_list_head = acl_list;
	acl_list_last = acl_list;
	while (acl_list) {
		/* If we found the name, remove it from the list. */
		if (!strcmp(name, acl_list->user->user_name)) {
			acl_list_last->next = acl_list->next;
			found_it = _B_TRUE;
			break;
		}
		acl_list_last = acl_list;
		acl_list = acl_list->next;
	}

	/* We didn't find the name in the list */
	if (!found_it) {
		fprintf(stderr, catgets(catd, 1, 122, 
				"%s: Could not remove permission"), prog_name);
		fprintf(stderr, 
		     catgets(catd, 1, 123, " because: %s not in access list.\n"),
				name);
		exit(1);
	}

	/* Update the calendar attributes */
	attrs[0].name = CSA_CAL_ATTR_ACCESS_LIST;
	attrs[0].value = &val[0];
	val[0].type = CSA_VALUE_ACCESS_LIST;
	val[0].item.access_list_value = acl_list_head;
	if ((status = csa_update_calendar_attributes(cal, 1, 
					attrs, NULL)) != CSA_SUCCESS) {
		fprintf(stderr, catgets(catd, 1, 122, 
				"%s: Could not remove permission"), prog_name);
		fprintf(stderr, catgets(catd, 1, 121, " because: %s.\n"), 
				handle_error(status));
		exit(1);
	}        
}

static void
list_access(
	CSA_session_handle	 cal)
{
	CSA_uint32		 num_attrs, num_attrs_ret;
	CSA_attribute_reference	 attr_names[2];
	CSA_attribute		*attrs;
	CSA_return_code		 status;
	CSA_access_rights	*acl_list;
	int			 i;
	CSA_uint32               data_version = 3;

	attr_names[0] = CSA_CAL_ATTR_ACCESS_LIST;
	attr_names[1] = CSA_X_DT_CAL_ATTR_DATA_VERSION;

	if ((status = csa_read_calendar_attributes(cal, 2, attr_names, 
						   &num_attrs_ret, &attrs, 
						   NULL)) != CSA_SUCCESS) {
		fprintf(stderr, catgets(catd, 1, 126, 
			"%s: Could not change permission because: %s.\n"), 
			prog_name, handle_error(status));
		exit(1);
	}	

	for (i = 0; i < num_attrs_ret; i++) {
		if (!strcmp(attrs[i].name, CSA_CAL_ATTR_ACCESS_LIST)) {
			acl_list = attrs[i].value->item.access_list_value;
		} else if (!strcmp(attrs[i].name, 
					CSA_X_DT_CAL_ATTR_DATA_VERSION)) {
			data_version = attrs[i].value->item.uint32_value;
		}
	}

	while (acl_list) {
		print_short_acl(acl_list->user->user_name, acl_list->rights,
			  data_version);
		acl_list = acl_list->next;
	}
}

static void
print_short_acl(
	char		*name,
	CSA_flags 	 flags,
	CSA_uint32	 version)
{
	int		 need_comma = _B_FALSE;
	char		 buf[256];

	memset(buf, '\0', 256);

	printf ("%-25s", name);
	if (!flags) {
		printf (catgets(catd, 1, 127, "no access\n"));
		return;
	}

	if (version > 3) {
		if (flags & (CSA_VIEW_PUBLIC_ENTRIES | 
			     CSA_INSERT_PUBLIC_ENTRIES | 
			     CSA_CHANGE_PUBLIC_ENTRIES)) {
			strcat (buf, "pu=");
			if (flags & CSA_VIEW_PUBLIC_ENTRIES)
				strcat (buf, "v");
			if (flags & CSA_INSERT_PUBLIC_ENTRIES)
				strcat (buf, "i");
			if (flags & CSA_CHANGE_PUBLIC_ENTRIES)
				strcat (buf, "c");
			need_comma = _B_TRUE;
		}
		if (flags & (CSA_VIEW_CONFIDENTIAL_ENTRIES | 
			     CSA_INSERT_CONFIDENTIAL_ENTRIES | 
			     CSA_CHANGE_CONFIDENTIAL_ENTRIES)) {
			if (need_comma) strcat (buf, ",");
			strcat (buf, "sp=");
			if (flags & CSA_VIEW_CONFIDENTIAL_ENTRIES)
				strcat (buf, "v");
			if (flags & CSA_INSERT_CONFIDENTIAL_ENTRIES)
				strcat (buf, "i");
			if (flags & CSA_CHANGE_CONFIDENTIAL_ENTRIES)
				strcat (buf, "c");
			need_comma = _B_TRUE;
		}
		if (flags & (CSA_VIEW_PRIVATE_ENTRIES | 
			     CSA_INSERT_PRIVATE_ENTRIES | 
			     CSA_CHANGE_PRIVATE_ENTRIES)) {
			if (need_comma) strcat (buf, ",");
			strcat (buf, "pr=");
			if (flags & CSA_VIEW_PRIVATE_ENTRIES)
				strcat (buf, "v");
			if (flags & CSA_INSERT_PRIVATE_ENTRIES)
				strcat (buf, "i");
			if (flags & CSA_CHANGE_PRIVATE_ENTRIES)
				strcat (buf, "c");
			need_comma = _B_TRUE;
		}
		if (flags & (CSA_VIEW_CALENDAR_ATTRIBUTES | 
			     CSA_INSERT_CALENDAR_ATTRIBUTES | 
			     CSA_CHANGE_CALENDAR_ATTRIBUTES)) {
			if (need_comma) strcat (buf, ",");
			strcat (buf, "at=");
			if (flags & CSA_VIEW_CALENDAR_ATTRIBUTES)
				strcat (buf, "v");
			if (flags & CSA_INSERT_CALENDAR_ATTRIBUTES)
				strcat (buf, "i");
			if (flags & CSA_CHANGE_CALENDAR_ATTRIBUTES)
				strcat (buf, "c");
			need_comma = _B_TRUE;
		}
		if (flags & (CSA_OWNER_RIGHTS | 
			     CSA_SPONSOR_RIGHTS | 
			     CSA_ORGANIZER_RIGHTS)) {
			if (need_comma) strcat (buf, ",");
			strcat (buf, "ar=");
			if (flags & CSA_OWNER_RIGHTS)
				strcat (buf, "o");
			if (flags & CSA_SPONSOR_RIGHTS)
				strcat (buf, "s");
			if (flags & CSA_ORGANIZER_RIGHTS)
				strcat (buf, "a");
			need_comma = _B_TRUE;
		}
		printf ("%-30s\n", buf);
	} else {
		if (flags & CSA_X_DT_BROWSE_ACCESS)
			printf ("b");
		if (flags & CSA_X_DT_INSERT_ACCESS)
			printf ("i");
		if (flags & CSA_X_DT_DELETE_ACCESS)
			printf ("d");
		printf ("\n");
	}
}

static char *
get_cal_name(
	char	*cal_name,
	Props	*props)
{
	if (cal_name)
		return (cal_name);

	if ((cal_name = get_user_calendar(props)) == NULL)
		return (cm_get_credentials());

	return (cal_name);
}
