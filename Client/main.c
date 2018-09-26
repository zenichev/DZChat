/* DZChat - Client. Sep 2018 @Donat Zenichev */
/* File implementer */

/* HEADER FILE INCLUDES */
#include <sys/socket.h>   /* for socket(), bind(), connect(), recv() and send() */
#include <netdb.h>        /* definitions for network database operations */
#include <netinet/in.h>   /* Internet Protocol family */
#include <arpa/inet.h>    /* definitions for internet operations */

#include <stdio.h>        /* for printf() and fprintf() */
#include <string.h>       /* different manipulations over a char arrays, such as memset() */

#include <stdlib.h>       /* malloc() , calloc(), free() , atof(), atoi(), atol() */
#include <sys/types.h>    /* data types */
#include <inttypes.h>     /* intptr_t */
#include <errno.h>        /* reporting and retrieving error conditions */

#include <fcntl.h>        /* file control options such as open() */
#include <unistd.h>       /* standard symbolic constants and types such as close(), read() */

#include <pthread.h>      /* Threading */

#include "main.h"         /* Include defenitions */


/* ----------------------------------- MAIN FUNCTION ----------------------------------- */
int main()
{
	printf("-------------------------------------------------");
	printf("\n| DZChat (terminal C based chat) Version 1.1.0  |");
	printf("\n| Updates: https://github.com/zenichev/DZChat   |");
	printf("\n-------------------------------------------------\n");
	
	char nickname[NICK_SIZE];                 // my own name
	receive_handler passStructure;            // needed for passing different values to a thread (in the chatting function)

	pthread_t threadID;                       // thread id for a listening thread
	recepient passRecepient;                  // pass values to a listening thread
	struct sockaddr_in uas;                   // a structure to receive messages from other clients
	int sock_desc=0, port=0;                  // socket for listening; our listening port
	memset(&uas,0,sizeof(uas));

	int sock_desc_client, serverPort, clientLength, counter=0;	// socket for sending; server port; - ; - ;
	char serverAddress[17];                   // the format is with quotes and dotes: "000.000.000.000"
	struct sockaddr_in client;                // a structure to send messages
	memset(&client,0,sizeof(client));
	
	/* uas - direct messaging with other clients */
	sock_desc=socketDescriptor();             // open a socket descriptor for listening
	port=randomPort();                        // randomized port for listening
	sinStructure(&uas,port,'\0');             // define ip parameters for a server structure
	bindServer(sock_desc,&uas,sizeof(uas));   // associate and reserve a port for a socket
	listenServer(sock_desc);                  // start listening for connections from other clients - for direct messaging

	/* For connections to a data server */
	addAddress(serverAddress);                // ask a user for a server address
	serverPort=addPort();                     // ask a user for a server port
	sock_desc_client=socketDescriptor();      // open a socket descriptor for a connection to a server
	sinStructure(&client,serverPort,serverAddress);     // define ip parameters for a client structure
	clientLength=sizeof(client);
	connection(sock_desc_client,&client,clientLength);	// connect to a server
	//
	identification(&sock_desc_client, &port, nickname, &passStructure); // identify myself to a server
	introduction();                                                     // show introduction to a user

	// pass needed values to a listening thread
	passRecepient.socket = sock_desc;
	passRecepient.uas = &uas;
	// thread for messages from other clients
	pthread_create(&threadID, NULL, dataReceivingFromClients, (void *) &passRecepient);
	// start chatting
	chatting(&sock_desc_client,&sock_desc, nickname, &passStructure);

	pthread_exit(NULL);                   // close a thread
	close(sock_desc_client);              // close a client socket
	close(sock_desc);                     // close a uas socket

	exit(0);
	return 0;
}
/* ----------------------------------- MAIN FUNCTION END ----------------------------------- */
