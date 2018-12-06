/* DZChat - Client. Dec 2018 @Donat Zenichev */
/* Process a user's entered data and get a destination name from it and a message body */

/* HEADER FILE INCLUDES */
#include <stdio.h>      /* for printf() and fprintf() */
#include <string.h>     /* different manipulations over a char arrays, such as memset() */
#include "main.h"       /* Include defenitions */

/* ------------------ RETRIEVE DATA FROM AN USER MESSAGE ----------------------------------- */
// get a recipient name
char * getDestName(char * string, char * name) {
	char *p; p=string;
	char *n; n=name;
	int nameLength=NICK_SIZE;

	while(*p && *p != ':' && nameLength-->0)
	{
		if(strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-0123456789_",*p)) {	// acceptable symbols for a name
			*n++=*p++;      // move the gotten char to name char array
		} else {
			p++;            // if the char is not acceptable just move to another one
		}
	}
	p=0; n=0;
}
// get a message body
char * getMessageBody(char * string, char * message) {
	char *p; p=string;
	char *m; m=message;
	int messageLength=RCVBUFSIZE;

	// move the pointer until a message body
	while(*p && *p != ':')
	{
		p++;
	}
	p++;

	while(*p && *p != ':' && messageLength-->0)
	{
		if(strchr("'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-0123456789_!? ",*p)) {	// acceptable symbols for a name
			*m++=*p++;	// move the gotten char to name char array
		} else {
			p++;		// if the char is not acceptable just move to another one
		}
	}
	p=0; m=0;
}
/* ----------------------------------------------------------------------------------------- */
