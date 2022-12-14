/* $XConsortium: tex.h /main/cde1_maint/1 1995/07/17 22:21:21 drk $ */
EXTERN char *special[256] 
#if defined(DEFINE)
= {
/* Char Name                        TeX Expansion */
/*                                                */
/*   0: ^@  NUL                         */ 0,
/*   1: ^A  SOH                         */ 0,
/*   2: ^B  STX                         */ 0,
/*   3: ^C  ETX                         */ 0,
/*   4: ^D  EOT                         */ 0,
/*   5: ^E  ENQ                         */ 0,
/*   6: ^F  ACK                         */ 0,
/*   7: ^G  BEL                         */ 0,
/*   8: ^H  BS                          */ 0,
/*   9: ^I  HT                          */ 0,
/*  10: ^J  LF                          */ 0,
/*  11: ^K  VT                          */ 0,
/*  12: ^L  FF                          */ 0,
/*  13: ^M  CR                          */ 0,
/*  14: ^N  SO                          */ 0,
/*  15: ^O  SI                          */ 0,
/*  16: ^P  DLE                         */ 0,
/*  17: ^Q  DC1                         */ 0,
/*  18: ^R  DC2                         */ 0,
/*  19: ^S  DC3                         */ 0,
/*  20: ^T  DC4                         */ 0,
/*  21: ^U  NAK                         */ 0,
/*  22: ^V  SYN                         */ 0,
/*  23: ^W  ETB                         */ 0,
/*  24: ^X  CAN                         */ 0,
/*  25: ^Y  EM                          */ 0,
/*  26: ^Z  SUB                         */ 0,
/*  27: ^[  ESC                         */ 0,
/*  28: ^\  FS                          */ 0,
/*  29: ^]  GS                          */ 0,
/*  30: ^^  RS                          */ 0,
/*  31: ^_  US                          */ 0,
/*  32: SP                              */ 0,
/*  33: !                               */ 0,
/*  34: "                               */ 0,
/*  35: #                               */ "\\#",
/*  36: $                               */ "{\\dollar}",
/*  37: %                               */ "\\%",
/*  38: &                               */ "\\&",
/*  39: '                               */ "{'}",
/*  40: (                               */ 0,
/*  41: )                               */ 0,
/*  42: *                               */ 0,
/*  43: +                               */ 0,
/*  44: ,                               */ 0,
/*  45: -                               */ "{-}",
/*  46: .                               */ 0,
/*  47: /                               */ 0,
/*  48: 0                               */ 0,
/*  49: 1                               */ 0,
/*  50: 2                               */ 0,
/*  51: 3                               */ 0,
/*  52: 4                               */ 0,
/*  53: 5                               */ 0,
/*  54: 6                               */ 0,
/*  55: 7                               */ 0,
/*  56: 8                               */ 0,
/*  57: 9                               */ 0,
/*  58: :                               */ 0,
/*  59: ;                               */ 0,
/*  60: <                               */ "{\\lthan}",
/*  61: =                               */ 0,
/*  62: >                               */ "{\\gthan}",
/*  63: ?                               */ 0,
/*  64: @                               */ 0,
/*  65: A                               */ 0,
/*  66: B                               */ 0,
/*  67: C                               */ 0,
/*  68: D                               */ 0,
/*  69: E                               */ 0,
/*  70: F                               */ 0,
/*  71: G                               */ 0,
/*  72: H                               */ 0,
/*  73: I                               */ 0,
/*  74: J                               */ 0,
/*  75: K                               */ 0,
/*  76: L                               */ 0,
/*  77: M                               */ 0,
/*  78: N                               */ 0,
/*  79: O                               */ 0,
/*  80: P                               */ 0,
/*  81: Q                               */ 0,
/*  82: R                               */ 0,
/*  83: S                               */ 0,
/*  84: T                               */ 0,
/*  85: U                               */ 0,
/*  86: V                               */ 0,
/*  87: W                               */ 0,
/*  88: X                               */ 0,
/*  89: Y                               */ 0,
/*  90: Z                               */ 0,
/*  91: [                               */ 0,
/*  92: \\                              */ "{\\bslash}",
/*  93: ]                               */ 0,
/*  94: ^                               */ "\\string^",
/*  95: _                               */ "{\\under}",
/*  96: `                               */ "{`}",
/*  97: a                               */ 0,
/*  98: b                               */ 0,
/*  99: c                               */ 0,
/* 100: d                               */ 0,
/* 101: e                               */ 0,
/* 102: f                               */ 0,
/* 103: g                               */ 0,
/* 104: h                               */ 0,
/* 105: i                               */ 0,
/* 106: j                               */ 0,
/* 107: k                               */ 0,
/* 108: l                               */ 0,
/* 109: m                               */ 0,
/* 110: n                               */ 0,
/* 111: o                               */ 0,
/* 112: p                               */ 0,
/* 113: q                               */ 0,
/* 114: r                               */ 0,
/* 115: s                               */ 0,
/* 116: t                               */ 0,
/* 117: u                               */ 0,
/* 118: v                               */ 0,
/* 119: w                               */ 0,
/* 120: x                               */ 0,
/* 121: y                               */ 0,
/* 122: z                               */ 0,
/* 123: {                               */ "{\\lbrac}",
/* 124: |                               */ "{\\vbar}",
/* 125: }                               */ "{\\rbrac}",
/* 126: ~                               */ "\\string~",
/* 127: DEL                             */ 0,
/* 128: undefined                       */ 0,
/* 129: undefined                       */ 0,
/* 130: undefined                       */ 0,
/* 131: undefined                       */ 0,
/* 132: undefined                       */ 0,
/* 133: undefined                       */ 0,
/* 134: undefined                       */ 0,
/* 135: undefined                       */ 0,
/* 136: undefined                       */ 0,
/* 137: undefined                       */ 0,
/* 138: undefined                       */ 0,
/* 139: undefined                       */ 0,
/* 140: undefined                       */ 0,
/* 141: undefined                       */ 0,
/* 142: undefined                       */ 0,
/* 143: undefined                       */ 0,
/* 144: undefined                       */ 0,
/* 145: undefined                       */ 0,
/* 146: undefined                       */ 0,
/* 147: undefined                       */ 0,
/* 148: undefined                       */ 0,
/* 149: undefined                       */ 0,
/* 150: undefined                       */ 0,
/* 151: undefined                       */ 0,
/* 152: undefined                       */ 0,
/* 153: undefined                       */ 0,
/* 154: undefined                       */ 0,
/* 155: undefined                       */ 0,
/* 156: undefined                       */ 0,
/* 157: undefined                       */ 0,
/* 158: undefined                       */ 0,
/* 159: undefined                       */ 0,
/* 160: undefined                       */ 0,
/* 161: A   grave                       */ "\\`A",
/* 162: A   circumflex                  */ "\\^A",
/* 163: E   grave                       */ "\\`E",
/* 164: E   circumflex                  */ "\\^E",
/* 165: E   dieresis                    */ "\\\042E",
/* 166: I   circumflex                  */ "\\^I",
/* 167: I   dieresis                    */ "\\\042I",
/* 168: acute accent                    */ "\\'{ }",
/* 169: grave accent                    */ "\\`{ }",
/* 170: circumflex                      */ "\\^{ }",
/* 171: dieresis                        */ "\\\042{ }",
/* 172: tilde                           */ "\\~{ }",
/* 173: U   grave                       */ "\\`U",
/* 174: U   circumflex                  */ "\\^U",
/* 175: Italian Lira                    */ 0,
/* 176: overbar                         */ 0,
/* 177: Y   acute                       */ "\\'Y",
/* 178: y   acute                       */ "\\'y",
/* 179: degree                          */ 0,
/* 180: C   cedilla                     */ "\\c{C}",
/* 181: c   cedilla                     */ "\\c{c}",
/* 182: N   tilde                       */ "\\~N",
/* 183: n   tilde                       */ "\\~n",
/* 184: inverted exclamation mark       */ "{!`}",
/* 185: inverted question mark          */ "{?`}",
/* 186: currency sign                   */ 0,
/* 187: pound sterling                  */ 0,
/* 188: Yen                             */ 0,
/* 189: Section sign                    */ 0,
/* 190: florin                          */ 0,
/* 191: Cent sign                       */ 0,
/* 192: a   circumflex                  */ "\\^a",
/* 193: e   circumflex                  */ "\\^e",
/* 194: o   circumflex                  */ "\\^o",
/* 195: u   circumflex                  */ "\\^u",
/* 196: a   acute                       */ "\\'a",
/* 197: e   acute                       */ "\\'e",
/* 198: o   acute                       */ "\\'o",
/* 199: u   acute                       */ "\\'u",
/* 200: a   grave                       */ "\\`a",
/* 201: e   grave                       */ "\\`e",
/* 202: o   grave                       */ "\\`o",
/* 203: u   grave                       */ "\\`u",
/* 204: a   dieresis                    */ "\\\042a",
/* 205: e   dieresis                    */ "\\\042e",
/* 206: o   dieresis                    */ "\\\042o",
/* 207: u   dieresis                    */ "\\\042u",
/* 208: A   angstrom                    */ "{\\AA}",
/* 209: i   circumflex                  */ "{\\^\\i}",
/* 210: O   slash                       */ "{\\O}",
/* 211: AE  ligature                    */ "{\\AE}",
/* 212: a   angstrom                    */ "{\\aa}",
/* 213: i   acute                       */ "{\\'\\i}",
/* 214: o   slash                       */ "{\\o}",
/* 215: ae  ligature                    */ "{\\ae}",
/* 216: A   dieresis                    */ "\\\042A",
/* 217: i   grave                       */ "{\\`\\i}",
/* 218: O   dieresis                    */ "\\\042O",
/* 219: U   dieresis                    */ "\\\042U",
/* 220: E   acute                       */ "\\'E",
/* 221: i   dieresis                    */ "{\\\042\\i}",
/* 222: SS  ligature                    */ "{\\ss}",
/* 223: O   circumflex                  */ "\\^O",
/* 224: A   acute                       */ "\\'A",
/* 225: A   tilde                       */ "\\~A",
/* 226: a   tilde                       */ "\\~a",
/* 227: Eth                             */ 0,
/* 228: eth                             */ 0,
/* 229: I   acute                       */ "\\'I",
/* 230: I   grave                       */ "\\`I",
/* 231: O   acute                       */ "\\'O",
/* 232: O   grave                       */ "\\`O",
/* 233: O   tilde                       */ "\\~O",
/* 234: o   tilde                       */ "\\~o",
/* 235: S   caron                       */ "\\v{S}",
/* 236: s   caron                       */ "\\v{s}",
/* 237: U   acute                       */ "\\'U",
/* 238: Y   dieresis                    */ "\\\042Y",
/* 239: y   dieresis                    */ "\\\042y",
/* 240: Thorn                           */ 0,
/* 241: thorn                           */ 0,
/* 242: undefined                       */ 0,
/* 243: mu                              */ 0,
/* 244: paragraph sign                  */ 0,
/* 245: fraction 3/4                    */ 0,
/* 246: long dash                       */ 0,
/* 247: fraction 1/4                    */ 0,
/* 248: fraction 1/2                    */ 0,
/* 249: Female ordinal                  */ 0,
/* 250: Male ordinal                    */ 0,
/* 251: French double open quote        */ 0,
/* 252: Solid square                    */ 0,
/* 253: French double close quote       */ 0,
/* 254: Plus over minus sign            */ 0,
/* 255: undefined                       */ 0
}
#endif
;

