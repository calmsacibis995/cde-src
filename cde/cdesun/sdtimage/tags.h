/*
 *	@(#)tags.h	1.3 96/03/07 Copyright (c) 1992 Sun Microsystems, Inc.
 *
 *	tags.h - cps/dps tags for ImageTool
 */

#define DONE_TAG		0
#define OUTPUT_TAG		1
#define ERROR_TAG		2
#define RENDER_PAGE_TAG		3
#define RENDER_DONE_TAG 	4
#define RENDER_SMALL_DONE_TAG	5
#define TOTAL_PAGES_TAG		6
#define ALL_DONE_TAG		7
#define ERROR_PAGE_TAG		8

#define MAXOUTPUT		4096

#define ERROR_STRING		"Error on page: "
#define PS_ERROR_STRING		(Error on page: )
#define PAGES_STRING		"Total pages"
#define PS_PAGES_STRING		(Total pages)
#define REVERSE_PAGES_STRING	"RTotal pages: "
#define PS_REVERSE_PAGES_STRING	(RTotal pages: )
#define END_OF_FILE		"\nImagetool EOF\n"
#define PS_END_OF_FILE		(Imagetool EOF)

#define PS_HAS_COLOR		(Color found)
#define HAS_COLOR		"Color found"

#define CONTINUE_STRING		"Continue"
#define RENDERED_PAGE_STRING	"RenderedPage"
#define PAGECOUNTER_END_STRING	"PagecounterEnd"
#define PAGECOUNTER_COLOR_STRING "PagecounterColor"
