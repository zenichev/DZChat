/* DZChat - Server. Dec 2018 @Donat Zenichev */
/* file implementer */

/* HEADER FILE INCLUDES */
#include <sys/socket.h>	/* for socket(), bind(), connect(), recv() and send() */
#include <netdb.h>		/* definitions for network database operations */
#include <netinet/in.h>	/* Internet Protocol family */
#include <arpa/inet.h>		/* definitions for internet operations */

#include <stdio.h>		/* for printf() and fprintf() */
#include <string.h>		/* different manipulations over a char arrays, such as memset() */
#include <fcntl.h>		/* file control options such as open() */
#include <unistd.h>		/* standard symbolic constants and types such as close(), read() */
#include <stdlib.h>		/* malloc() , calloc(), free() , atof(), atoi(), atol() */

#include <sys/types.h>		/* data types */
#include <errno.h>		/* reporting and retrieving error conditions */
#include <sys/wait.h>		/* for waitpid() */
#include <sys/ipc.h>		/* interprocess communication access structure - ftok() */
#include <sys/shm.h>		/* headers for shared memory mechanism */

#include <time.h>		/* time() */
#include <pthread.h>		/* Threading */
#include "main.h"		/* include my custom headers and definitions */


/* ----------------------------------- MAIN FUNCTION ----------------------------------- */
int main()
{
	printf("-------------------------------------------------");
	printf("\n| Data server                                   |");
	printf("\n| DZChat (terminal C based chat) Version 1.1.0  |");
	printf("\n| Updates: https://github.com/zenichev/DZChat   |");
	printf("\n-------------------------------------------------\n");

	get_time currentTime;

	char serverAddress[17];                 	// the format is with quotes and dotes: "000.000.000.000"
	int sock_desc=0, serverPort=0, clientSock=0;	// socket for server connections; server listening port; socket for client processing;
	unsigned int threadCounter=0;			// Threads counter
	socklen_t clientLength;				// a length of a client socket
	struct sockaddr_in server,client;		// add structures for client and server
	handle_client initalParameters;			// for passing ip and socket to a handling thread
	pthread_t threadID;				// thread id used to process threads

	memset(&server,0,sizeof(server));		// null out a server structure
	memset(&client,0,sizeof(client));		// null out a client structure

	/* server related processing */
	addAddress(serverAddress);              	// ask a user for a listening address
	serverPort=addPort();				// ask a user for a port
	sock_desc=socketDescriptor();			// open a socket descriptor for a server
	sinStructure(&server,serverPort,serverAddress);	// define ip parameters for a server structure
	bindServer(sock_desc,&server,sizeof(server));	// associate and reserve a port for a socket
	listenServer(sock_desc);			// start listening for connections

	// initialize arrays of structures for groups
	for(int i=0; i<MAX_AVAILABLE_GROUPS; i++) {
		strcpy(initalParameters.groups[i].groupName, "NULL");
		for(int j=0; j<MAX_AVAILABLE_GROUP_MEMBERS; j++) {
			initalParameters.groups[i].groupIDs[j] = 0;
		}
	}

	printf("INFO: Start the server on the port %d\n",serverPort);
	for (;;)	// run forever
	{
		threadCounter++;// count client threads
		clientLength=sizeof(client);

		// get a socket for an incoming client connection - server waits there until client will connect
		clientSock=acceptConnection(sock_desc,&client,&clientLength);

		// copy parameters to a passing stucture to use it in a handling thread
		strcpy(initalParameters.ip, inet_ntoa(client.sin_addr) );
		initalParameters.socket=clientSock;
		initalParameters.counter=threadCounter;

		CURTIME;
		printf("----------------------------------\n");
		printf("%s INFO <%d>: Handling a new client = <%s>, thread number = <%d>\n", currentTime.buffer, threadCounter, inet_ntoa(client.sin_addr), threadCounter);

		pthread_create(&threadID, NULL, handleTCPClient, (void *) &initalParameters);	// start a new thread here
	}
	pthread_exit(NULL);

	exit(0);
	return 0;
}
/* ----------------------------------- MAIN FUNCTION END ----------------------------------- */
