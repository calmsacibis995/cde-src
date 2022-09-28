typedef unsigned char Byte;

typedef struct _GlyphRec {
	Byte	width;
	Byte	height;
	Byte	xoff;
	Byte	yoff;
	Pixmap	pixmap;
} Glyph;

typedef struct _GlyphFontRec {
	Byte	maxWidth;
	Byte	maxHeight;
	Byte	numGlyphs;
	char	baseChar;
	Glyph   *data;
} GlyphFont;

TermFont _DtTermPrimRenderGlyphFontCreate(Widget w, GlyphFont **font);
