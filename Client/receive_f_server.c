/* DZChat - Client. Dec 2018 @Donat Zenichev */
/* This functionality is responsible for data receiving from the data server */
/* It's started from a separate thread */

/* HEADER FILE INCLUDES */
#include <stdio.h>			/* for printf() and fprintf() */
#include <string.h>			/* different manipulations over a char arrays, such as memset() */
#include "main.h"			/* Include defenitions */
#include <sys/socket.h>		/* for socket(), bind(), connect(), recv() and send() */
#include <unistd.h>			/* standard symbolic constants and types such as close(), read() */

/* ------------------------------ RECEIVE MESSAGES FROM A SERVER --------------------------- */
void * dataReceiving(void * arguments) {

	get_time currentTime;

	/* passed structure to a thread */
	receive_handler * passed = (receive_handler *) arguments;
	int socket = (int ) passed->socket;
	client_parameters * parameters = (client_parameters *) passed->parameters;
	int * idList = (int*) passed->ids;
	group * groups = (group*) passed->newGroups; 

	int recvMsgSize=0, id=0, temp=0, deleted=0;
	char echoBuffer[RCVBUFSIZE];
	memset(echoBuffer,0,sizeof(echoBuffer));

	for (;;) 
	{
		// try to receive an echo returned from a server
		if ((recvMsgSize = recv(socket, echoBuffer, RCVBUFSIZE, 0)) < 0) {
			sleep(0.5);
		} else {
			// check if this is a null message
			if (strlen(echoBuffer) == 0) {
				continue;
			}
			
			// group updates
			if (strstr(echoBuffer, "SYSTEM:GROUP_UPDATE")) {
				receiveGroupUpdates( echoBuffer, groups, parameters );
				memset(echoBuffer,0,sizeof(echoBuffer));
				continue;	// continue receiving messages
			}

			// user asks for updates
			if (strstr(echoBuffer, "SYSTEM:USER_UPDATE")) id = structureAdd(echoBuffer, parameters, idList);	// user update information
			if (strstr(echoBuffer, "SYSTEM:USER_UPDATE")) {
				if (id == 0) {
						CURTIME;
						printf("> %s MESSAGE: Sorry but the requested user seems offline.\n", currentTime.buffer);
						fflush( stdout );
						memset(echoBuffer,0,sizeof(echoBuffer));
						continue;	// continue receiving messages
				}
			}
			
			// notification about newly connected user
			if (strstr(echoBuffer, "SYSTEM:SERVER_UPDATE:connected:")) {	// server updates, connection
					structureAdd(echoBuffer, parameters, idList);
					memset(echoBuffer,0,sizeof(echoBuffer));
					continue;	// continue receiving messages
			}
			
			// notification about disconnected user
			if ( strstr(echoBuffer, "SYSTEM:SERVER_UPDATE") && strstr(echoBuffer, "disconnected") ) {	// server updates, disconnection
					delCurrentList(echoBuffer, parameters, idList);
					memset(echoBuffer,0,sizeof(echoBuffer));
					continue;
			}
			
			// if it is not a system message
			if (!strstr(echoBuffer, "SYSTEM:USER_UPDATE") && !strstr(echoBuffer, "SYSTEM:SERVER_UPDATE")) {
				CURTIME;
				printf("\n> %s MESSAGE: %s", currentTime.buffer, echoBuffer);	// notify a user that a given client has disconnected
				printf("\n*** send a message>");
				fflush( stdout );
			}
			
			memset(echoBuffer,0,sizeof(echoBuffer));
			continue;
		}
	}
}
/* ----------------------------------------------------------------------------------------- */
