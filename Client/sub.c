/* DZChat - Client. Dec 2018 @Donat Zenichev */
/* There "sub-functions" for different purposes are stored */

/* HEADER FILE INCLUDES */
#include <stdio.h>      /* for printf() and fprintf() */
#include <string.h>     /* different manipulations over a char arrays, such as memset() */
#include <stdlib.h>     /* malloc() , calloc(), free() , atof(), atoi(), atol() */
#include "main.h"       /* Include defenitions */

/* ---------------------------------- SHOW ONLINE LIST ------------------------------------ */
void getOnliners(client_parameters * parameters, int * ids) {

	int temp=0;

	printf(">------------------------------------------------------------------------------>\n");
	printf("| Users currently online:                                                      |\n");
	for(int i=0; i<MAX_AVAILABLE_CLIENTS; i++)
	{
		if (ids[i] > 0) {
			temp=ids[i];
			if ( strlen(parameters[temp].nickname) != 0 ) printf("| - %s                                                                      |\n", parameters[temp].nickname);
		}
	}
	printf(">------------------------------------------------------------------------------>\n");
	temp=0;
}
/* ---------------------------------------------------------------------------------------- */

/* ---------------------------------- GET MY OWN ID    ------------------------------------ */
int getMyID(client_parameters * parameters, int * ids,  char * name) {
	int myid=0, temp=0;
	for(int i=0; i<MAX_AVAILABLE_CLIENTS; i++) {
		if ( ids[i] > 0 ) {
			temp = ids[i];
			if ( strstr(parameters[temp].nickname, name) && strlen(parameters[temp].nickname) == strlen(name) ) {
				myid = parameters[temp].client_id;
				break;
			}
		}
	}
	return myid;
}
/* ---------------------------------------------------------------------------------------- */
