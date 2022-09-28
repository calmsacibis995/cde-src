<chapter id=SgmlCompliance > Document Type Definition\ for HelpTag

&headerFix;

                  <idx|standard generalized markup language (SGML)|
                  <idx|SGML, compliance|
                  <idx|Document Type Definition (DTD)|
                  <idx|markup language, SGML|


<procedure>Document Type Definition
A Document Type Definition (DTD) defines a set of elements
to create a structured (or hierarchical) document.  The
DTD specifies the syntax for each element and governs
how and where elements can be used in a document.


The DTD for &TagAppName; 1.23 conforms to the Standard Generalized Markup
Language (SGML) ISO specification 8879:1986.  This means that you can use the
&HelpSystemName; markup language to create help files that are SGML
compliant.  


<procedure> Avoid Shorthand\ Markup

Certain abbreviated markup conventions supported in
&TagAppName; are not valid when producing SGML compliant files.
For example, an author can use shorthand markup for certain
inline font changes.  Shorthand markup eliminates the
start and end tags of the normal syntax. For example, this markup:

<ex>
&<emph>...%%text%%...&<\emph>
<\ex>
can be replaced with this shorthand markup:

<ex>...!!%%text%%!!...<\ex>


<note>
&TagAppName; can process !!either!! markup.  Shorthand markup, although
convenient, is not standard SGML markup.  Therefore, if you want fully
compliant SGML files, do not use shorthand markup.
<\note>

In some instances, &TagAppName; also interprets "implied" element tags.
For example, you can enter ``&<chapter>'' without using ``&<head>'' and
``&<\head>'' to denote the chapter title.  The chapter tag implies there
is a chapter head. &TagAppName; interprets the implied
markup.  SGML standard syntax often requires explicit start and end
tags for "implied" elements, although there are exceptions. 


<procedure>Example:\ SGML Markup

This markup adds a required head tag for the hometopic's title.
It shows using ``&<head>'' and ``&<\head>'' for a paragraph and a list
that contain a title.  The list includes two additional tags, ``&<item>''
and ``&<p>'', which are components of the list element.

<ex smaller>
&<hometopic>&<head> %%Home Topic Title%% &<\head>

&<p> %%first paragraph%%
&<p> %%second paragraph%%
&<p>&<head> %%Paragraph Head%% &<\head>
&<p> %%paragraph text%%

&<list>&<head> %%List Head%% &<\head>
  &<item>&<p> %%first list item%%
  &<item>&<p> %%second list item%%
  &<item>&<p> %%third list item%%
  &<\list>
<\ex>

<procedure>DTD for \CDE HelpTag 1.23

The Document Type Definition (DTD) identifies each element
and the syntax for its use.  If you are familiar with DTDs,
you can refer to the DTD specifications that follow.
Otherwise, if you would like to learn more about DTDs and the
fields that describe each element, an informative resource is <book>The SGML
Handbook<\book> by Charles F. Goldfarb.


<vex smallest>

<!SGML "ISO 8879:1986"

        ======================================================
                          CDE HelpTag 1.23 DTD

        ======================================================
 

SGML Declaration

CHARSET
BASESET "ISO 646-1983//CHARSET International Reference Version
          (IRV)//ESC 2/5 4/0"
DESCSET      0        9      UNUSED
             9        2      9
            11        2      UNUSED
            13        1      13
            14       18      UNUSED
            32       95      32
           127        1      UNUSED

BASESET "ISO Registration Number 100//CHARSET ECMA-94
          Right Part of Latin Alphabet  Nr. 1//ESC  2/13  4/1"
DESCSET    128       32      UNUSED
           160        5      32
           165        1      UNUSED
           166       88      38
           254        1      127
           255        1      UNUSED

CAPACITY SGMLREF
    TOTALCAP 350000
    ENTCAP   100000
    ENTCHCAP  50000
    ELEMCAP   50000
    GRPCAP   210000
    EXGRPCAP  50000
    EXNMCAP   50000
    ATTCAP    50000
    ATTCHCAP  50000
    AVGRPCAP  50000
    NOTCAP    50000
    NOTCHCAP  50000
    IDCAP     50000
    IDREFCAP  50000
    MAPCAP   210000
    LKSETCAP  50000
    LKNMCAP   50000

SCOPE DOCUMENT
SYNTAX -- The Core Reference Syntax except with ATTCNT, LITLEN, NAMELEN,
          GRPCNT, and GRPGTCNT changed --

SHUNCHAR CONTROLS   0   1   2   3   4   5   6   7   8   9
                   10  11  12  13  14  15  16  17  18  19
                   20  21  22  23  24  25  26  27  28  29
                   30  31 127 255

BASESET "ISO 646-1983//CHARSET International Reference Version
          (IRV)//ESC 2/5 4/0"
DESCSET      0      128      0

FUNCTION     RE      13
             RS      10
             SPACE   32
             TAB     SEPCHAR    9

NAMING
   LCNMSTRT ""
   UCNMSTRT ""
   LCNMCHAR "-."
   UCNMCHAR "-."
   NAMECASE
      GENERAL YES
      ENTITY   NO

DELIM
   GENERAL  SGMLREF ETAGO "<\"
   SHORTREF SGMLREF -- Removed short references --
   NAMES    SGMLREF SYSTEM FILE
   QUANTITY SGMLREF
       ATTCNT    140
       LITLEN   4096
       NAMELEN    64
       GRPCNT    100
       GRPGTCNT  253
       TAGLVL     48

FEATURES
    MINIMIZE
        DATATAG    NO
        OMITTAG    YES
        RANK       NO
        SHORTTAG   NO
    LINK
        SIMPLE     NO
        IMPLICIT   NO
        EXPLICIT   NO
    OTHER
        CONCUR     NO
        SUBDOC     NO
        FORMAL     NO
        APPINFO  NONE
>

<!DOCTYPE helpvolume [
<!ELEMENT helpvolume  O O (metainfo?,
                           hometopic?,
                           (chapter* | (s1*, rsect*)),
			   glossary?)
                          +(memo | location | idx) >

<!ELEMENT metainfo    - O (idsection, abstract?, otherfront*) >

<!ELEMENT idsection   O O (title, copyright?) >

<!ELEMENT title       - O (hline)
                          -(memo | location | idx) >

<!ELEMENT hline       O O (partext) >

<!ELEMENT partext     O O ((#PCDATA | emph  | computer  | user | term |
                            var     | quote | graphic   | book | xref |
                            link    | esc   | location) ) >

<!ELEMENT emph        - - (partext) -(emph) >

<!ELEMENT computer    - - ((#PCDATA | quote | var | user | esc)*) >

<!ELEMENT user        - - ((#PCDATA | var | esc)*) >

<!ELEMENT term        - - (partext)
                          -(emph  | computer | term | var |
                            quote | user     | book ) >
<!ATTLIST term            base        CDATA             #IMPLIED
                          gloss       (gloss | nogloss) gloss >

<!ELEMENT var         - - ((#PCDATA | esc)*) >

<!ELEMENT quote       - - (partext) -(quote) >

<!ELEMENT graphic     - O EMPTY >
<!ATTLIST graphic         id          ID                #IMPLIED
                          file        ENTITY            #REQUIRED >

<!ELEMENT book        - - (partext) -(book) >

<!ELEMENT xref        - O EMPTY >
<!ATTLIST xref            id          IDREF             #REQUIRED >

<!ELEMENT link        - - (partext) -(link) >
<!ATTLIST link            hyperlink   CDATA             #REQUIRED
                          type        (jump        |
                                       jumpnewview |
                                       definition  |
                                       execute     |
                                       appdefined  |
                                       man         )    jump
                           description CDATA           #IMPLIED >

<!ELEMENT esc         - - CDATA >

<!ELEMENT location    - - (partext) -(location) >
<!ATTLIST location        id           ID              #REQUIRED >

<!ELEMENT copyright   - O (text)
                           -(memo | location | idx) >

<!ELEMENT text        O O ((p       | note      | caution   | warning |
                            lablist | list      | ex        | vex     |
                            esc     | otherhead | procedure | figure  |
                            image)* ) >

<!ELEMENT p           - O (head?, partext)
                          +(newline) >
<!ATTLIST (p | image)     indent       (indent)        #IMPLIED
                          id           ID              #IMPLIED
                          gentity      ENTITY          #IMPLIED
                          gposition    (left | right)  left
                          ghyperlink   CDATA           #IMPLIED
                          glinktype    (jump        |
                                        jumpnewview |
                                        definition  |
                                        execute     |
                                        appdefined  |
                                        man         )  jump
                          gdescription  CDATA          #IMPLIED >

<!ELEMENT head        - - (hline)
                          -(memo | location | idx) >

<!ELEMENT newline     - O EMPTY >

<!ELEMENT note        - - (head?, text)
                           -(note | caution | warning) >

<!ELEMENT caution     - - (head?, text)
                          -(note | caution | warning) >

<!ELEMENT warning     - - (head?, text)
                          -(note | caution | warning) >

<!ELEMENT lablist     - - (head?, labheads?, (label, text)+) >
<!ATTLIST lablist         spacing     (loose | tight) loose >

<!ELEMENT labheads    - O (labh, labhtext)
                          -(memo | location | idx) >

<!ELEMENT labh        O - (partext) >

<!ELEMENT labhtext    O O (partext) >

<!ELEMENT label       - - (partext) >

<!ELEMENT list        - - (head?, item+) >
<!ATTLIST list            type        (order   |
                                       bullet  |
                                       plain   |
                                       check   )      bullet
                          ordertype   (ualpha  |
                                       lalpha  |
                                       arabic  |
                                       uroman  |
                                       lroman  )      arabic
                          spacing     (tight   |
                                       loose   )      tight
                          continue    (continue)      #IMPLIED >

<!ELEMENT item        - O (text) >
<!ATTLIST item            id           ID             #IMPLIED >

<!ELEMENT ex          - - (head?, (exampleseg, annotation?)+)
                          -(ex | vex | note | caution | warning) >
<!ATTLIST ex              notes       (side | stack)  stack
                          lines       (number   |
                                       nonumber )     nonumber
                          textsize    (normal   |
                                       smaller  |
                                       smallest )     normal >

<!ELEMENT exampleseg  O O (partext) +(lineno) >

<!ELEMENT annotation  - - (partext) +(newline) >

<!ELEMENT lineno      - O EMPTY >
<!ATTLIST lineno          id           ID             #IMPLIED >

<!ELEMENT vex         - - CDATA >
<!ATTLIST vex             lines       (number   |
                                       nonumber )     nonumber
                          textsize    (normal   |
                                       smaller  |
                                       smallest )     normal >

<!ELEMENT otherhead   - O (head, text) >

<!ELEMENT procedure   - O (chaphead, text)
                          -(procedure) >

<!ELEMENT chaphead    O O (head, abbrev?)
                          -(memo | location | idx) >

<!ELEMENT abbrev      - O (hline) >

<!ELEMENT figure      - - (caption?)
                          -(figure | graphic) >
<!ATTLIST figure          number      NUMBER          #IMPLIED
                          tonumber    (number  |
                                       nonumber)      number
                          id          ID              #IMPLIED
                          entity      ENTITY          #REQUIRED
                          ghyperlink  CDATA           #IMPLIED
                          glinktype   (jump        |
                                       jumpnewview |
                                       definition  |
                                       execute     |
                                       appdefined  |
                                       man         )  jump
                          gdescription CDATA          #IMPLIED >

<!ELEMENT caption     O O (hline, abbrev?)
                          -(memo | location | idx) >

<!ELEMENT image       - - (head?, imagetext) >

<!ELEMENT imagetext   O O (partext) >

<!ELEMENT abstract    - O (head?, text, frontsub*) >

<!ELEMENT frontsub    - O (head, text) >

<!ELEMENT otherfront  - O (head?, text, frontsub*) >

<!ELEMENT hometopic   - O (chaphead, text) >

<!ELEMENT chapter     - O (chaphead, text, (s1*, rsect*)) >
<!ATTLIST (chapter |
           s1      |
           s2      |
           s3      |
           s4      |
           s5      |
           s6      |
           s7      |
           s8      |
           s9      )      id          ID              #IMPLIED >

<!ELEMENT s1          - O (chaphead, text, s2*, rsect*) >

<!ELEMENT s2          - O (chaphead, text, s3*, rsect*) >

<!ELEMENT s3          - O (chaphead, text, s4*, rsect*) >

<!ELEMENT s4          - O (chaphead, text, s5*, rsect*) >

<!ELEMENT s5          - O (chaphead, text, s6*, rsect*) >

<!ELEMENT s6          - O (chaphead, text, s7*, rsect*) >

<!ELEMENT s7          - O (chaphead, text, s8*, rsect*) >

<!ELEMENT s8          - O (chaphead, text, s9*, rsect*) >

<!ELEMENT s9          - O (chaphead, text) >

<!ELEMENT rsect       - O (chaphead, text, rsub*) >
<!ATTLIST rsect           id          ID              #IMPLIED >

<!ELEMENT rsub        - O (chaphead, text) >

<!ELEMENT glossary    - O (text?, (dterm, definition)+) >

<!ELEMENT dterm       - O (partext) -(term) >

<!ELEMENT definition  O O (text) >

<!ELEMENT idx         - - (partext, sort?) >

<!ELEMENT sort        - O ((#PCDATA | esc)+) >

<!ELEMENT memo        - - CDATA >

] >

<\vex>


