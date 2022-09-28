#include <stdio.h>
#include <locale.h>

#include "lcl.h"

static const char *header =
"Mime-Version: 1.0\n"
"Content-Type:Plain/Text:charset=ISO-2022-JP\n";

static const char *body =
"\
�����ȡ����Υͥ��ϴ��� fj.mail �ǿԤ������ȻפäƤ��ΤǤ����� (^_^;)\
�ܤ��ιͤ��Ϻǽ�ε�����\
\
 ARTICLE-ID: 10/13/93(05:38:51) fj.mail:2126/etlss2\
    Subject: Why MIME ? �ʤ�ǣͣɣͣŤʤ�䡩\
       From: ysato@etl.go.jp (Yutaka Sato ��ƣ˭)\
 Message-ID: <ikmHM.ysato@etl.go.jp>\
\
�˽񤤤��Ȥ���Ǥ���\
\
On 10/13/93(05:38) I wrote in <ikmHM.ysato@etl.go.jp>\
 |�錄���ϡ�MIME-PART2(RFC1342����RFC1522)�ˤ�롢�إå������ASCII��ɽ���ϡ�\
 |\
 |����ASCII�١�����������줿RFC821/822�˴�Ť����󥿡��ͥå�news/mail�ǡ�\
 |����Ǥ�դΥإå��ե�����ɤˤ����ơ�\
 |����Ǥ�դΡ�ʣ���Ρ�ʸ�����åȤ�ޤ�ƥ����Ȥ�\
 |����Ǥ�դΡ�ʣ���Ρ�ʸ�����󥳡��ǥ��󥰥����ƥ��ɽ������\
 |����EBCDIC�ʤɤΥ����ȥ������ʤɤ⤫��������Ĥġ�\
 |\
 |ž��������ʤ�¸����뤿��Ρ������ˤ����ƤϤޤȤ����ˡ���ȹͤ��Ƥ��ޤ���\
\
\
���������Ū�˹ͤ����RFC822�ϡ�comment, folding, quoting, encoding ...\
�ʤɤλ���(lexical)��٥�ȡ��ե�������¤Ӥ乽¤���ե�����ɤʤɤ�\
��ʸ(syntax)��٥뤫��ʤäƤ��ޤ���\
�����Ȥ���ɽ���������٥�ε�§�ϡ���§�Ȥ������Ƥι�ʸ��§��Ŭ��\
����ޤ���\
\
RFC1522�ϡ���ʸ��٥�ˤϼ�����줺�������٥�ε�§�Ȥ��ƥ��󥳡�\
�ǥ��󥰵�§��Ƴ������Ȥ�����������Ρ�����ץ��Ŭ���ϰϤι�����ˡ\
��Ȥ�ޤ�����\
���С������ϰϤ��������Ȥϰ��̤ˡ��ġ��Υ������Ǻ�Ŭ��̵���ʤ�Ȥ�����\
������ޤ����ǡ���Ϥ������ǧ�Ǥ���������ȻפäƤ��ޤ���\
\
Subject�ե�����ɤ����ʤ顢7bits��ISO-2022�����ʤ顢JIS�����ʤ顢ľ��\
SMTP�ʤ顢�������ʤɤʤɤ��äϽФƤ��ޤ������ܻؤ��Ƥ��르����Ͽͤ�\
��äưۤʤ�Ȥϻפ��ޤ�����Υ�����ϡ������󤷤��褦�ʤ�Τǡ�\
������Ū�� RFC1522 ��Ŭ�礷�Ƥ��ޤ���\
\
�������������������������������� @ @\
������ ������ ������  ������^^��( - )��{ŷ���ޥ�����}\
����++��ƣ���ţԣ̡�����876m�� _<   >_\
������ ������ ������������������������������������������������������������\
\
";

int
main(){
    char *locale;
    LCLd lcld;
    LCTd lctd;
    LclError	*ret;
    LclCharsetSegmentSet	*segs;

    setlocale(LC_ALL, "");

    if ((locale = setlocale(LC_CTYPE, NULL)) == NULL) {
	fprintf(stderr, "Locale is not set correctly.\n");
	return 1;
    }

    lcld = lcl_open(locale);
    if (!lcld) {
	fprintf(stderr, "Failed: lcl_open()\n");
	return 1;
    }

    lctd = lct_create(lcld, 
		      LctNSourceType, LctNMsgText, header, body,
		      LctNSourceForm, LctNInComingStreamForm,
		      LctNKeepReference, LctNKeepByReference,
		      NULL);

    if (!lctd) {
	fprintf(stderr, "Failed: lcl_create()\n");
	return 1;
    }

    ret = lct_getvalues(lctd,
			LctNDisplayForm,
			LctNBodySegment, &segs,
			NULL);

    if (ret) {
	fprintf(stderr, "Failed: lcl_getvalues()\n");
	lcl_destroy_error(ret);
	return 1;
    }

    fprintf(stderr, "segs->num = %d\n", segs->num);
    fprintf(stdout, "Charset: %s\n", segs->seg[0].charset);
    fprintf(stdout, 
	    "Body content:\n"
	    "%s\n", 
	    segs->seg[0].segment);

    lct_destroy(lctd);

    lcl_close(lcld);

    return 0;
}
