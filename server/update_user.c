/* DZChat - Server. Sep 2018 @Donat Zenichev */
/* interactions with users - it keeps users updated */

#include <stdio.h>      /* for printf() and fprintf() */
#include <string.h>     /* different manipulations over a char arrays, such as memset() */
#include <sys/socket.h> /* for socket(), bind(), connect(), recv() and send() */
#include <sys/types.h>  /* data types */
#include "main.h"       /* include my custom headers and definitions */
#include <unistd.h>     /* standard symbolic constants and types such as close(), read() */

/* Functionaly for providing updates to users */
/* It is also cab be used to process as intermideary proxy for sending messages, but for now it is deprecated */
/*----------------------------------------------------------------------------------------------------------------------------*/
void chatting(char * buffer, int * socket, client_parameters * list, int * idList, unsigned int * counter) {

	get_time currentTime;

	int recvMsgSize=0;                    // sive of a received message; an acceptable length of a name; an acceptable length of a message; 
	memset((char *)buffer,0,RCVBUFSIZE);  // clear the buffer before to obtain a new data

	// Start receiving a text message from a client
	if ((recvMsgSize = recv(*socket, (char *)buffer, RCVBUFSIZE, 0)) < 0) printf("ERROR <%d>: Failed in receiving a message from client\n", *counter);

	// Send received string and receive again until end of transmission
	while (recvMsgSize > 0)               // zero indicates end of transmission
	{
		char *p=0; p=buffer;
		int id=0;

		// check if a user asks us for updates
		if (strstr(buffer, "SYSTEM:USER_UPDATE")) {
			printf("SYSTEM:USER_UPDATE: User <%s> requires updates\n",list->nickname);
			giveUpdates(buffer, socket, list, idList, counter);
			goto ALIAS_RECEIVE;               // no wait for new messsages from a client
		} else {
			CURTIME;
			printf("%s INFO: Message not for me. From user <%s>. \n", currentTime.buffer, list->nickname);
			goto ALIAS_RECEIVE;
		}

		ALIAS_RECEIVE: memset((char *)buffer,0,RCVBUFSIZE);	// clear the buffer before to obtain a new data

		// See if there is more data to receive
		if ((recvMsgSize = recv(*socket, (char *)buffer, RCVBUFSIZE, 0)) < 0) printf("ERROR <%d>: Failed to recv() message from a client\n", *counter);
	}

	// clear variables
	recvMsgSize=0;
	memset((char *)buffer,0,RCVBUFSIZE);

	close(*socket); // Close client socket
	CURTIME;
	printf("%s INFO <%d>: Clients socket <%d> closed\n", currentTime.buffer, *counter, *socket);
	return;
}
/*----------------------------------------------------------------------------------------------------------------------------*/


/* Give updates to user he asks for */
/*----------------------------------------------------------------------------------------------------------------------------*/
void giveUpdates(char * sourceString, int * socket, client_parameters * parameters, int * idList, unsigned int * counter) {

	int nameLength=NAME_SIZE, temp=0, updatesFound=0;
	char askingName[NAME_SIZE], beginning[19]="SYSTEM:USER_UPDATE:", answer[RCVBUFSIZE]={0}, port[6]={0}, id[5]={0};
	char * ptrSrc; ptrSrc=sourceString;
	char * nPtr; nPtr=askingName;
	ptrSrc += sizeof(beginning);
	memset(askingName,0,sizeof(askingName));
	memset(answer,0,sizeof(answer));

	/* Get an asking name */
	while(*ptrSrc && *ptrSrc != ':' && nameLength-->0)
	{
		if(strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-0123456789_",*ptrSrc)) { 
			*nPtr++=*ptrSrc++;		// move the gotten char to name char array
		} else {
			ptrSrc++;       			// if the char is not acceptable just move to another one
		}
	}
	
	for (int i=0; i<MAX_AVAILABLE; i++)
	{
		if (idList[i] != 0) {
			temp=idList[i];
			if ( strstr(parameters[temp].nickname,askingName) ) {
				strcpy(answer, beginning);                                                        // "SYSTEM:USER_UPDATE:"
				strcat(answer, parameters[temp].nickname); strcat(answer, ":");                   // user nickname
				snprintf(id, sizeof(id), "%d:", parameters[temp].client_id); strcat(answer, id);  // id
				strcat(answer, "0:");                                                             // his socket, must be empty there
				snprintf(port,sizeof(port),"%d:",parameters[temp].port); strcat(answer, port);    // user listening port
				strcat(answer, parameters[temp].ip); strcat(answer, ":");                         // user ip address
				updatesFound=1;
				break;
			}
		}
	}

	switch(updatesFound) {
		// found
		case 1:
			if (send(*socket, answer, RCVBUFSIZE, 0) != RCVBUFSIZE) {
				printf("ERROR <%d>: Failed to send() message to client, SYSTEM:USER_UPDATE\n", *counter);
			}
			break;
		// not found
		case 0:
			if (send(*socket, "SYSTEM:USER_UPDATE:0", RCVBUFSIZE, 0) != RCVBUFSIZE) {
				printf("ERROR <%d>: Failed to send() message to client, SYSTEM:USER_UPDATE\n", *counter);
			}
			break;
		default:
			if (send(*socket, "SYSTEM:USER_UPDATE:0", RCVBUFSIZE, 0) != RCVBUFSIZE) {
				printf("ERROR <%d>: Failed to send() message to client, SYSTEM:USER_UPDATE\n", *counter);
			}
			break;
	}

	// clear variables
	memset(askingName,0,sizeof(askingName));
	memset(answer,0,sizeof(answer));
	memset(beginning,0,sizeof(beginning));
	memset(port,0,sizeof(port));
	memset(id,0,sizeof(id));
	nameLength=0, temp=0, updatesFound=0;
	nPtr = '\0', ptrSrc = '\0';
}
/*----------------------------------------------------------------------------------------------------------------------------*/
