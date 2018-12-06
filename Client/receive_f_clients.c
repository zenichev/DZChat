/* DZChat - Client. Dec 2018 @Donat Zenichev */
/* This functionality is responsible for data receiving from other clients */
/* It's started from a separate thread */

/* HEADER FILE INCLUDES */
#include <stdio.h>      /* for printf() and fprintf() */
#include <string.h>     /* different manipulations over a char arrays, such as memset() */
#include "main.h"       /* Include defenitions */
#include <unistd.h>     /* standard symbolic constants and types such as close(), read() */
#include <sys/socket.h> /* for socket(), bind(), connect(), recv() and send() */

/* ------------------ RECEIVE MESSAGES FROM OTHER CLIENTS ---------------------------------- */
void * dataReceivingFromClients(void * arguments) {

	get_time currentTime;

	/* passed socket to a thread */
	recepient * passed = (recepient *) arguments;
	int socket = (int ) passed->socket;
	struct sockaddr_in *clientStructure = (struct sockaddr_in*) passed->uas;

	int recvMsgSize=0, id=0, incomingSocket=0, clientLength=0;
	char echoBuffer[RCVBUFSIZE];

	for (;;) 
	{
		clientLength=sizeof(clientStructure);
		incomingSocket=acceptConnection(socket,clientStructure,&clientLength);

		// try to receive an echo returned from a server
		if ((recvMsgSize = recv(incomingSocket, echoBuffer, sizeof(echoBuffer), 0)) < 0) {
			sleep(0.5);
		} else {
			if (strlen(echoBuffer) == 0) continue;                                // check if this is a null message
			if (!strstr(echoBuffer, "SYSTEM:")) {
				CURTIME;
				printf("\n> %s MESSAGE: %s", currentTime.buffer, echoBuffer); // if it is not a system message
				printf("\n*** send a message>");
				fflush( stdout );
			}
			memset(echoBuffer,0,sizeof(echoBuffer));                              // null out a buffer
			continue;
		}
		close(incomingSocket);                                                        // close a used socket
	}
}
/* ----------------------------------------------------------------------------------------- */
