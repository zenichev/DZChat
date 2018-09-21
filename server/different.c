/* DZChat - Server. Sep 2018 @Donat Zenichev */
/* Other functionality */

#include <stdio.h>    /* for printf() and fprintf() */
#include <string.h>   /* different manipulations over a char arrays, such as memset() */
#include <stdlib.h>   /* malloc() , calloc(), free() , atof(), atoi(), atol() */
#include "main.h"     /* include my custom headers and definitions */

// Generate an unique ID for a new client
int randomID(void) {
	int id=0;
	time_t t;

	/* Intializes random number generator */
	srand((unsigned) time(&t));
	id = rand() % 1000 ;
	return id;
}
/*----------------------------------------------------------------------------------------------------------------------------*/

/* It allows us to keep the actual value of current members online */
int onlineCounter(int action) {
	// action=1 - increase counter
	// action=0 - decrease counter
	static int counter;

	switch(action) {
		case 1:				//increase
			if (counter != 0) {
				counter++;
			} else {
				counter = 0;
				counter++;
			}
			break;
		case 0:				// dercrease
			if (counter != 0) counter--;
			break;
		default:
			printf("WARNING: Wrong action definition in online users counter - ingoring\n");
	}

	return counter;	//return users online
}
/*----------------------------------------------------------------------------------------------------------------------------*/
