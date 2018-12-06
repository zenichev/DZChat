/* DZChat - Client. Dec 2018 @Donat Zenichev */
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

	get_time currentTime;

	pthread_t threadID;                             // thread id
	struct sockaddr_in client_direct;               // for direct connections to other clients
	int echoStringLen=0, temp=0, tempPort=0, userFound=0, recvMsgSize=0, id=0, sock_new_connection=0, clientLength=0, myID=0;
	//
	//passStructure->socket = *descriptor;            // a socket for a messaging with a data server
	//
	char echoString[RCVBUFSIZE], name[NICK_SIZE], myMessage[RCVBUFSIZE]={0}, systemMessage[RCVBUFSIZE], myMessageFull[RCVBUFSIZE]={0}, newGroupName[NICK_SIZE]={0}, group_ids[100]={0};

	memset(echoString,0,strlen(echoString));        // a whole data entered by a user
	memset(name,0,strlen(name));                    // a name of the destination user
	memset(systemMessage,0,strlen(systemMessage));  // system messsage to a data server
	memset(&client_direct,0,sizeof(client_direct)); // sin structure for outgoing connections with users

	// initialize arrays of structures for groups
	for(int i=0; i<MAX_AVAILABLE_GROUPS; i++) {
		strcpy(passStructure->newGroups[i].groupName, "NULL");
		for(int j=0; j<MAX_AVAILABLE_GROUP_MEMBERS; j++) {
			passStructure->newGroups[i].groupIDs[j] = 0;
		}
	}

	// my ID
	myID = getMyID(passStructure->parameters, passStructure->ids , nickname );

	/* start getting a messages */
	fgets(echoString, RCVBUFSIZE, stdin);
	for (;;) 
	{
		printf("*** send a message> ");				// Prompt to send a message
		if ( fgets(echoString, RCVBUFSIZE, stdin) == NULL ) {	// get a message
			continue;					// if message is null, ignore entering
		}
		
		// subtract a returning symbol
		if ((strlen(echoString) > 0) && (echoString[strlen (echoString) - 1] == '\n')) echoString[strlen (echoString) - 1] = '\0';

		// check if this is a null message
		if (strlen(echoString) == 0) continue;

		// if we need to quit
		if (strstr(echoString, "quit;")) {
			CURTIME;
			printf("%s INFO: Execution stopped, closing socket\n", currentTime.buffer);
			exit(0);
		}

		// clear a terminal
		if (strstr(echoString, "clear;")) {
			system("clear");
			memset(echoString,0,sizeof(echoString));
			continue;// return to the beginning
		}

		// get online user list
		if (strstr(echoString, "online;")) {
			getOnliners(passStructure->parameters, passStructure->ids);
			memset(echoString,0,sizeof(echoString));
			continue;// return to the beginning
		}

		// get help intro
		if (strstr(echoString, "help;")) {
			help();
			memset(echoString,0,sizeof(echoString));
			continue;// return to the beginning
		}

		// process group functionality
		if ( (strstr(echoString, ";") && strstr(echoString, "group")) || (strstr(echoString, "group") && strstr(echoString, "say:") && echoString[0] == 'g') ) {
			groupsImplementer( echoString, newGroupName, group_ids, passStructure->newGroups, descriptor, passStructure->ids, passStructure->parameters, &myID );
			continue;
		}

		// if a user sent a wrong syntax message
		if( !strstr(echoString,":") ) {
			printf("WARNING: Wrong syntax, use: <name>: <text-message> (without angle brackets)\n");
			memset(echoString,0,sizeof(echoString));
			continue;// return to the beginning
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
				if ( strstr(passStructure->parameters[temp].nickname, name) && strlen(passStructure->parameters[temp].nickname) == strlen(name) && strlen(passStructure->parameters[temp].ip) != 0 && passStructure->parameters[temp].port != 0) {
					userFound=1;
					// try to connect to a given user
					sock_new_connection=socketDescriptor();
					tempPort=(intptr_t)passStructure->parameters[temp].port;
					sinStructure(&client_direct,tempPort,passStructure->parameters[temp].ip);
					clientLength=sizeof(client_direct);
					connection(sock_new_connection,&client_direct,clientLength);
					CURTIME;
					if (send(sock_new_connection, myMessageFull, sizeof(myMessageFull), 0) != sizeof(myMessageFull)) printf("%s ERROR: send() sent a different number of bytes than expected\n", currentTime.buffer);
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
				CURTIME;
				printf("%s ERROR: send() sent a different number of bytes than expected\n", currentTime.buffer);
			}
			CURTIME;
			printf("%s INFO: Updating users information..\n", currentTime.buffer);
			for(int wait=0; wait<3; wait++)	// wait 3 seconds and close the cycle
			{
				sleep(1);
				// check if we got needed update
				for(int i=0; i<MAX_AVAILABLE_CLIENTS; i++)
				{
					if (passStructure->ids[i] > 0) {
						temp=passStructure->ids[i];
						if ( strstr(passStructure->parameters[temp].nickname, name) && strlen(passStructure->parameters[temp].nickname) == strlen(name) && strlen(passStructure->parameters[temp].ip) != 0 && passStructure->parameters[temp].port != 0) {
							userFound=1;
							sock_new_connection=socketDescriptor();
							tempPort=(intptr_t)passStructure->parameters[temp].port;
							sinStructure(&client_direct,tempPort,passStructure->parameters[temp].ip);
							clientLength=sizeof(client_direct);
							connection(sock_new_connection,&client_direct,clientLength);
							CURTIME;
							if (send(sock_new_connection, myMessageFull, sizeof(myMessageFull), 0) != sizeof(myMessageFull)) printf("%s ERROR: send() sent a different number of bytes than expected\n", currentTime.buffer);
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
