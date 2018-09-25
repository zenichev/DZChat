/* DZChat - Client. Sep 2018 @Donat Zenichev */
/* This functionality is responsible for sending messages to the server and clients */
/* Also the thread is responsible for obtaining data from the server starts there */
/* The other thing that is performed there is data-updating - data needed to connect to other members */

/* HEADER FILE INCLUDES */
#include <netdb.h>      /* definitions for network database operations */
#include <inttypes.h>   /* intptr_t */
#include <stdio.h>      /* for printf() and fprintf() */
#include <string.h>     /* different manipulations over a char arrays, such as memset() */
#include <stdlib.h>     /* malloc() , calloc(), free() , atof(), atoi(), atol() */
#include <pthread.h>    /* Threading */
#include <unistd.h>     /* standard symbolic constants and types such as close(), read() */
#include "main.h"       /* Include defenitions */

/* ---------------------------------- DATA RECEIVE/SEND ------------------------------------ */
void chatting(int * descriptor, int * descriptorUAS, char * nickname, receive_handler * passStructure) {

	pthread_t threadID;                             // thread id
	struct sockaddr_in client_direct;               // for direct connections to other clients
	int echoStringLen=0, temp=0, tempPort=0, userFound=0, recvMsgSize=0, id=0, sock_new_connection=0, clientLength=0;
	//
	//receive_handler passStructure;                // needed for passing different values to a thread
	passStructure->socket = *descriptor;            // a socket for a messaging with a data server
	//
	char echoString[RCVBUFSIZE], name[NICK_SIZE], myMessage[RCVBUFSIZE]={0}, systemMessage[RCVBUFSIZE], myMessageFull[RCVBUFSIZE]={0};

	memset(echoString,0,strlen(echoString));        // a whole data entered by a user
	memset(name,0,strlen(name));                    // a name of the destination user
	memset(systemMessage,0,strlen(systemMessage));  // system messsage to a data server
	memset(&client_direct,0,sizeof(client_direct)); // sin structure for outgoing connections with users

	// thread for messages from a server
	pthread_create(&threadID, NULL, dataReceiving, (void *) passStructure);

	/* start getting a messages */
	fgets(echoString, RCVBUFSIZE, stdin);
	for (;;) 
	{
		printf("*** send a message> ");                       // Prompt to send a message
		if ( fgets(echoString, RCVBUFSIZE, stdin) == NULL ) { // get a message
			continue;                                           // if message is null, ignore entering
		}

		// subtract a returning symbol
		if ((strlen(echoString) > 0) && (echoString[strlen (echoString) - 1] == '\n')) echoString[strlen (echoString) - 1] = '\0';

		// check if this is a null message
		if (strlen(echoString) == 0) continue;

		// if we need to quit
		if (strstr(echoString, "quit;")) {
			printf("INFO: Execution stopped, closing socket\n");
			exit(0);
		}

		// clear a terminal
		if (strstr(echoString, "clear;")) {
			system("clear");
			memset(echoString,0,echoStringLen);
			continue;// return to the beginning
		}

		// if a user sent a wrong syntax message
		if( !strstr(echoString,":") && !strstr(echoString, "clear;") && !strstr(echoString, "quit;") ) {
			printf("\nWARNING: Wrong syntax, use: <name>: <text-message> (without angle brackets)\n");
			printf("***Try again: ");
			fgets(echoString, RCVBUFSIZE, stdin);
		} else {
			getDestName(echoString, name);              // set a destination name
			getMessageBody(echoString, myMessage);      // set a message
			strcpy(myMessageFull, nickname); strcat(myMessageFull, ":");
			strcat(myMessageFull, myMessage);
		}

		//--------------------------------------------------------------------------------------------------------------------------------
		// Check if we have user in our local structure
		for(int i=0; i<MAX_AVAILABLE_CLIENTS; i++)
		{
			if (passStructure->ids[i] > 0) {
				temp=passStructure->ids[i];		// store id parameter in a temporary variable
				// try to find a needed name
				if ( strstr(passStructure->parameters[temp].nickname, name) && strlen(passStructure->parameters[temp].ip) != 0 && passStructure->parameters[temp].port != 0) {
					userFound=1;
					// try to connect to a given user
					sock_new_connection=socketDescriptor();
					tempPort=(intptr_t)passStructure->parameters[temp].port;
					sinStructure(&client_direct,tempPort,passStructure->parameters[temp].ip);
					clientLength=sizeof(client_direct);
					connection(sock_new_connection,&client_direct,clientLength);
					if (send(sock_new_connection, myMessageFull, sizeof(myMessageFull), 0) != sizeof(myMessageFull)) printf("ERROR: send() sent a different number of bytes than expected\n");
					close(sock_new_connection);	// do not forget to close the socket
					break;
				}
			}
		}
		//--------------------------------------------------------------------------------------------------------------------------------

		//--------------------------------------------------------------------------------------------------------------------------------
		// user was not found in my array - try to get an update from the server
		if ( userFound == 0) {
			strcpy(systemMessage,"SYSTEM:USER_UPDATE:");	// let the server identify that we are asking for a user parameters
			strcat(systemMessage, name); strcat(systemMessage, ":");
			// Send the asking string to the server
			if (send(*descriptor, systemMessage, sizeof(systemMessage), 0) != sizeof(systemMessage)) {
				printf("ERROR: send() sent a different number of bytes than expected\n");
			}
			printf("INFO: Updating users information..\n");
			for(int wait=0; wait<3; wait++)	// wait 3 seconds and close the cycle
			{
				sleep(1);
				// check if we got needed update
				for(int i=0; i<MAX_AVAILABLE_CLIENTS; i++)
				{
					if (passStructure->ids[i] > 0) {
						temp=passStructure->ids[i];
						if ( strstr(passStructure->parameters[temp].nickname, name) && strlen(passStructure->parameters[temp].ip) != 0 && passStructure->parameters[temp].port != 0) {
							userFound=1;
							sock_new_connection=socketDescriptor();
							tempPort=(intptr_t)passStructure->parameters[temp].port;
							sinStructure(&client_direct,tempPort,passStructure->parameters[temp].ip);
							clientLength=sizeof(client_direct);
							connection(sock_new_connection,&client_direct,clientLength);
							if (send(sock_new_connection, myMessageFull, sizeof(myMessageFull), 0) != sizeof(myMessageFull)) printf("ERROR: send() sent a different number of bytes than expected\n");
							close(sock_new_connection);	// do not forget to close the socket
							break;
						}
					}
				}
				if ( strstr(passStructure->parameters[temp].nickname, name) && strlen(passStructure->parameters[temp].ip) != 0 && passStructure->parameters[temp].port != 0) break;
			}
			memset(systemMessage,0,sizeof(systemMessage));
		}
		//--------------------------------------------------------------------------------------------------------------------------------
		
		// clear values
		memset(echoString,0,sizeof(echoString));
		memset(name,0,sizeof(name));
		memset(&client_direct,0,sizeof(client_direct));
		memset(&myMessage,0,sizeof(myMessage));
		memset(&myMessage,0,sizeof(myMessageFull));
		echoStringLen=0, temp=0, tempPort=0, userFound=0, recvMsgSize=0, id=0, sock_new_connection=0, clientLength=0;
	}
	pthread_exit(NULL);	// close a thread
}
/* ----------------------------------------------------------------------------------------- */