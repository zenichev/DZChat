/* DZChat - Server. Sep 2018 @Donat Zenichev */
/* Identify user that connects to us */

#include <stdio.h>    /* for printf() and fprintf() */
#include <string.h>   /* different manipulations over a char arrays, such as memset() */
#include "main.h"     /* include my custom headers and definitions */

/* Retrieve from a user name and his listening port (not the port used to connect to us) */
/*----------------------------------------------------------------------------------------------------------------------------*/
char userIdentification(char * buffer,int size,char * name,char * port) {
	char *p=0; p=buffer;		// set a pointer to a buffer, to catch a name of the user in a char array
	char *n=0; n=name;		// set a pointer to a name array
	char *po=0; po=port;		// set a pointer to a port array
	int length=NAME_SIZE, namesize=0, portLengthAcceptable=6, portLengthReal=0;	// set limiter for while loop ; used to count a name size ; size of the port;
	
	int moveto = sizeof(ID_STRING);	// moving integer
	p += moveto-1;			// move the pointer to the beginning of a user's name

	/* get a name */
	while(*p && *p != '\"' && *p != '\'' && *p != ';' && *p != ' ' && *p != ':' && length-->0)
	{
		if(strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-0123456789_",*p)) {
			*n++=*p++;      // move the gotten char to name char array
			namesize++;     // count a name size
		} else {
			p++;            // if the char is not acceptable just move to another one
		}
	}

	/* get client port */
	if ( strchr(":",*p) ) {				// colon is a delimiter for name and port
		p++;                    		// move pointer from colon to a beginning port symbol
		while (*p && portLengthAcceptable-->0)
		{
			if(strchr("0123456789",*p)) {
				*po++=*p++;		// copy port symbols to a port variable
				portLengthReal++;
			} else {
				p++;			// if the char is not acceptable just move to another one
			}
		}
	}
	p -= (namesize+moveto+portLengthReal);		// move a pointer back to a beginning
	memset(p,0,size);				// set passed buffer to null
}
/*----------------------------------------------------------------------------------------------------------------------------*/
