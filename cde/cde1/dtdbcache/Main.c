/*
cc -g -o Main Main.c -I/usr/dt/include -I/usr/openwin/include -R/export2/build/SunOS/lib1/DtSvc:/usr/dt/lib:/usr/openwin/lib -L/export2/build/SunOS/lib1/DtSvc -L/usr/dt/lib -L/usr/openwin/lib -lDtSvc -lXt
 */
#include        <locale.h>
#include <Tt/tt_c.h>
#include <Xm/Form.h>
#include <Xm/Text.h>
#include <Dt/Dts.h>
#include <Tt/tt_c.h>
#include <Dt/UserMsg.h>
#include        <Dt/EnvControl.h>


#define ApplicationClass "dtdtscache"
#define RebuildMsg 	 "DtDtsCache"

int             build_db = 1;

int
main(int argc, char **argv)
{
	XtAppContext    appContext;
	Widget          toplevel, form;
	char           *tmp;
	int		c;
	int		more = 1;
	Tt_message	msg;
	int		count = 0;
	int		slp = 0;
	int		init_flag;
	int		i;
	char		*locale = setlocale(LC_ALL, "");


        if(!locale)
        {
                perror("setlocale");
        }
        _DtEnvControl(DT_ENV_SET);

	if(strcmp(argv[1], "-init") == 0)
	{
		init_flag = 1;
	}
	else
	{
		init_flag = 0;
	}

	if(!init_flag && tt_ptr_error(tt_open()) != TT_OK)
	{
		_DtSimpleError(ApplicationClass,
				DtFatalError,
				NULL ,
				"%s: Couldn't initialize ToolTalk\n",
				argv[0]);
	}
	if(!init_flag)
	{
		switch(tt_ptype_declare("SDTDTSCACHE"))
		{
			case	TT_OK:
				break;
			case	TT_ERR_NOMP:
				_DtSimpleError(ApplicationClass,
					DtFatalError,
					NULL ,
					"%s: Couldn't initialize ToolTalk\n",
					argv[0]);
				break;
			case	TT_ERR_PTYPE:
				_DtSimpleError(ApplicationClass,
					DtFatalError,
					NULL ,
					"%s: Ptype %s not installed\n",
					argv[0],
					RebuildMsg);
				break;
			default:
				_DtSimpleError(ApplicationClass,
					DtFatalError,
					NULL ,
				"%s: unknown error from tt_ptype_declare\n");
		}
	}

	toplevel = XtAppInitialize(&appContext, ApplicationClass, NULL, 0,
				   &argc, argv, NULL, NULL, 0);

	if (DtAppInitialize(appContext, XtDisplay(toplevel), toplevel, argv[0],
			    ApplicationClass) == False)
	{
		_DtSimpleError(ApplicationClass,
				DtFatalError,
				NULL ,
				"%s: Couldn't initialize Dt\n",
				argv[0]);
	}

	while(!init_flag && more)
	{
		msg = tt_message_receive();
		if(msg != 0)
		{
			char	*op = tt_message_op(msg);

			if(strcmp(op, RebuildMsg) != 0)
			{
				tt_message_reject(msg);
			}
			else
			{
				Tt_message m;

				_DtDtsMMInit(1);
				count = 0;
				slp = 0;
				tt_message_reply(msg);
/*
				m = tt_message_create();
				tt_message_scope_set(m, TT_SESSION);
				tt_message_class_set(m, TT_NOTICE);
				tt_message_op_set(m, "DtTypes_Reloaded");
				tt_message_address_set(m, TT_PROCEDURE);
				tt_message_send(m);
*/
			}
		}
		else
		{
			count++;
		}
		if(count >= 10)
		{
			if(slp)
			{
				more = 0;
				continue;
			}
			sleep(30);
			slp = 1;
		}
	}

	if(init_flag)
	{
		_DtDtsMMInit(1);
	}
	exit(0);
}
