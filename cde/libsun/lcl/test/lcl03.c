#include <stdio.h>
#include <locale.h>

#include "lcl.h"

static const char *header =
"Mime-Version: 1.0\n"
"Content-Type:Plain/Text:charset=ISO-2022-JP\n";

static const char *body =
"\
えーと、このネタは既に fj.mail で尽きたかと思ってたのですが… (^_^;)\
ぼくの考えは最初の記事、\
\
 ARTICLE-ID: 10/13/93(05:38:51) fj.mail:2126/etlss2\
    Subject: Why MIME ? なんでＭＩＭＥなんや？\
       From: ysato@etl.go.jp (Yutaka Sato 佐藤豊)\
 Message-ID: <ikmHM.ysato@etl.go.jp>\
\
に書いたとおりです。\
\
On 10/13/93(05:38) I wrote in <ikmHM.ysato@etl.go.jp>\
 |わたしは、MIME-PART2(RFC1342改めRFC1522)による、ヘッダ中の非ASCIIの表現は、\
 |\
 |　・ASCIIベースで定義されたRFC821/822に基づくインターネットnews/mailで、\
 |　・任意のヘッダフィールドにおいて、\
 |　・任意の（複数の）文字セットを含むテキストを、\
 |　・任意の（複数の）文字エンコーディングシステムで表現し、\
 |　・EBCDICなどのゲートウェイなどもかいくぐりつつ、\
 |\
 |転送する手段を実現するための、現状においてはまともな便法だと考えています。\
\
\
言語処理系的に考えるとRFC822は、comment, folding, quoting, encoding ...\
などの字句(lexical)レベルと、フィールド並びや構造化フィールドなどの\
構文(syntax)レベルからなっています。\
コメントに代表される字句レベルの規則は、原則として全ての構文規則に適用\
されます。\
\
RFC1522は、構文レベルには手を入れず、字句レベルの規則としてエンコー\
ディング規則を導入するというそれだけの、シンプルで適用範囲の広い方法\
をとりました。\
カバーする範囲が広いことは一般に、個々のケースで最適で無くなるという面\
を持ちます。で、私はこれを、容認できる代償だと思っています。\
\
Subjectフィールドだけなら、7bitsのISO-2022だけなら、JISだけなら、直接\
SMTPなら、、、、などなどの話は出ていました。目指しているゴールは人に\
よって異なるとは思います。私のゴールは、上に列挙したようなもので、\
この目的に RFC1522 は適合しています。\
\
　　　　　　　　　　　　　　　　 @ @\
┰── ─┰─ ┰　　  ／／＼^^　( - )　{天職マガジン}\
┠─++佐藤＠ＥＴＬ　／　876m＼ _<   >_\
┸── 　┸　 ┸────────────────────────────┘\
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
