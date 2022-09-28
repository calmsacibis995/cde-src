/* $XConsortium: strstr.c /main/cde1_maint/1 1995/07/17 21:31:22 drk $ */
char *strstr ( s1, s2 )
char *s1, *s2 ; {
	
	int x, y ;
	
	y = strlen ( s2 ) ;
	x = strlen ( s1 ) - y ;
	
	if ( x < 0 ) return ( ( char * ) 0L ) ;
	do {
		if ( strncmp ( s1 + x, s2, y ) == 0 ) return ( s1 + x ) ;
		}
	while ( --x >= 0 ) ;
	
	return ( ( char * ) 0L ) ;
	}

