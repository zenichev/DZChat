/* DZChat - Client. Sep 2018 @Donat Zenichev */
/* All the functionality is responsible for connections to remote server or members */
/* Also the functions are responsible for listening */

/* HEADER FILE INCLUDES */
#include <stdio.h>        /* for printf() and fprintf() */
#include <string.h>       /* different manipulations over a char arrays, such as memset() */
#include "main.h"         /* Include defenitions */

#include <sys/socket.h>   /* for socket(), bind(), connect(), recv() and send() */
#include <netdb.h>        /* definitions for network database operations */
#include <netinet/in.h>   /* Internet Protocol family */
#include <arpa/inet.h>    /* definitions for internet operations */

#include <stdlib.h>       /* malloc() , calloc(), free() , atof(), atoi(), atol() */
#include <errno.h>        /* reporting and retrieving error conditions */


/* ----------------------------------- CONNECTION RELATED ---------------------------------- */
// add port for connection to a server
int addPort(void) {
	int serverPort,temp=0;
	// enter a port on which you want to listen
	printf("*** Add a port of the server:");
	if ( (temp=scanf("%d",&serverPort)) <= 0) {
		serverPort=SERVER_PORT;
		printf("ERROR: Wrong or misspelled port number - set to default %d\n",serverPort);
	}
	return serverPort;
}

// random port for binding
int randomPort(void) {
	int id=0;
	time_t t;
	/* Intializes random number generator */
	srand((unsigned) time(&t));
	id = (rand()%(PORT_BIND_MAX-PORT_BIND_MIN))+PORT_BIND_MIN;
	return id;
}

// add address for connection to a server
char addAddress(char address[]) {
	int temp=0;
	// enter a port on which you want to listen
	printf("*** Add an address of the server:");
	if ( (temp=scanf("%s",address)) <= 0) {
		strcpy(address,SERVER_IP);
		printf("ERROR: Wrong or misspelled ip address - set to default %s\n",address);
	}
}

// new socket descriptor
int socketDescriptor(void) {
	int descriptor;
	// open a socket, a stream is 2-way
	if ((descriptor=socket(AF_INET,SOCK_STREAM,0)) < 0) {
		printf("ERROR: Failed in socket creation - %s\n",strerror(errno));
		exit(1);
	}
	return descriptor;
}

// ip address/port structure for clients/server
void sinStructure(struct sockaddr_in *structName,int port,char address[]) {
	if (!address) address = LISTEN_IP;	// for listening
	// set server structure parameters
	structName->sin_family=AF_INET;
	structName->sin_addr.s_addr=inet_addr(address);
	structName->sin_port=htons(port);
}

// try to connect to a server
void connection(int descriptor,struct sockaddr_in *structName,int length) {
	// try connection to a given server
	if(connect(descriptor,(struct sockaddr *) structName,length) < 0) {
		printf("ERROR: Failed in establishing the connection - %s\n",strerror(errno));
		exit(1);
	}
}

// binding for direct client messaging
void bindServer(int descriptor,struct sockaddr_in *structName,int size) {
	if(bind(descriptor, (struct sockaddr *) structName, size) < 0) {
		printf("ERROR: Failed in a socket binding - %s\n",strerror(errno));
		exit(1);
	}
}

// start listening
void listenServer(int descriptor) {
	if(listen(descriptor,MAX_AVAILABLE_CLIENTS) < 0) {
		printf("ERROR: Failed in a socket listening - %s\n",strerror(errno));
		exit(1);
	}
}

// accept connections from other clients
int acceptConnection(int descriptor,struct sockaddr_in *structName,int *length) {
		int socketDescriptor;
		// get a socket for an incoming client connection
		if((socketDescriptor=accept(descriptor,(struct sockaddr *) structName, length))<0) {
			printf("ERROR: Failed in a temporary socket creation\n");
			exit(1);
		}
		return socketDescriptor;
}
/* ----------------------------------------------------------------------------------------- */
