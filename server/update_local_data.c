/* DZChat - Server. Dec 2018 @Donat Zenichev */
/* Keep local arrays of structures updates */

#include <stdio.h>		/* for printf() and fprintf() */
#include <string.h>		/* different manipulations over a char arrays, such as memset() */
#include <sys/socket.h>	/* for socket(), bind(), connect(), recv() and send() */
#include <sys/types.h>		/* data types */
#include "main.h"		/* include my custom headers and definitions */
#include <errno.h>		/* reporting and retrieving error conditions */
#include <stdlib.h>		/* malloc() , calloc(), free() , atof(), atoi(), atol() */

/* This array stores an acutal IDs of clients */
/*----------------------------------------------------------------------------------------------------------------------------*/
int * setCurrentList(int *passedID, int action, client_parameters * infoClients) {
	// action=1 add a user to the list
	// action=0 delete a user from the list

	get_time currentTime;

	int temp=0, disconnected=0, socket=0, len=0;
	static int list[MAX_AVAILABLE];
	char notification[100], disconnected_name[NAME_SIZE];

	memset(notification,0,sizeof(notification));
	memset(disconnected_name,0,sizeof(disconnected_name));

	switch(action) {
		// add new id to the list
		case 1:
			for(int i=0; i<MAX_AVAILABLE; i++)
			{
				if ((temp=list[i]) == 0) {
					list[i]=*passedID;
					break;
				}
			}
			if(temp!=0) printf("ERROR <%d>: The ID of a client was not stored in a list!\n", infoClients[*passedID].counter);	//seems that id was not stored
			temp=0;
			return list;
		// delete an existant id from the list
		// and delete all information about client from infoClients structure
		case 0:
			for(int i=0; i<MAX_AVAILABLE; i++)
			{
				if (list[i]==*passedID) {
					disconnected=*passedID;
					strcpy(disconnected_name,infoClients[disconnected].nickname);
					// clear client information
					infoClients[disconnected].client_id=0;
					memset(infoClients[disconnected].nickname,0,sizeof(infoClients[disconnected].nickname));
					infoClients[disconnected].socket=0;
					list[i] = 0;	// delete myself from the IDs list
				}
			}

			/* Send notification about a disconnected user */
			for(int i=0; i<MAX_AVAILABLE; i++)
			{
				// notify clients about a disconnection
				if (list[i]!=0) {
					temp=list[i];
					socket=infoClients[temp].socket;	// find a socket for notification
					snprintf(notification,sizeof(notification),"SYSTEM:SERVER_UPDATE:%d:disconnected.",disconnected);
					len = strlen(notification);
					if(send(socket, notification, len, 0) != len) {
						printf("ERROR <%d>: Error to send() a notification to a client - %s\n", infoClients[disconnected].counter, strerror(errno));
					} else {
						CURTIME;
						printf("%s MESSAGE OUT <%d>: a notification about a disconnection is sent to <%s>\n", currentTime.buffer, infoClients[disconnected].counter, infoClients[temp].nickname);
					}
					memset(notification,0,sizeof(notification));
				}
			}
			// clear values
			memset(notification,0,sizeof(notification));
			memset(disconnected_name,0,sizeof(disconnected_name));
			temp=0; socket=0; disconnected=0, len=0;
	}
}
/*----------------------------------------------------------------------------------------------------------------------------*/

/* It allows a user to insert his own parameters in array of structures */
/*----------------------------------------------------------------------------------------------------------------------------*/
client_parameters * clientsStructure(char * name, int * myID, int * socket, char * portListening, char * ip, int usersOnline, unsigned int * counter) {
	static client_parameters params[1000];				// structure for client parameters

	if (params[*myID].client_id != 0) {				// be sure that this id is not allocated yet for an other client
		while(params[*myID].client_id != 0) *myID=randomID();
	}

	strcpy(params[*myID].nickname,name);				// set a client name to a structure
	params[*myID].client_id = *myID;				// set a client id to a structure
	params[*myID].socket = *socket;					// set a client socket to a structure
	params[*myID].port = atoi(portListening);			// set a client port to a structure
	strcpy(params[*myID].ip,ip);					// set a client name to a structure
	params[*myID].counter = *counter;				// set a thread number to a structure

	return params;							// return a shared memory with arrays of structures
}
/*----------------------------------------------------------------------------------------------------------------------------*/
