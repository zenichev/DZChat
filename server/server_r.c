#include <stdio.h>      /* for printf() and fprintf() */
#include <string.h>     /* different manipulations over a char arrays, such as memset() */
#include <sys/socket.h> /* for socket(), bind(), connect(), recv() and send() */
#include <sys/types.h>  /* data types */
#include "main.h"       /* include my custom headers and definitions */
#include <errno.h>      /* reporting and retrieving error conditions */
#include <stdlib.h>     /* malloc() , calloc(), free() , atof(), atoi(), atol() */
#include <sys/socket.h> /* for socket(), bind(), connect(), recv() and send() */
#include <netinet/in.h> /* Internet Protocol family */
#include <arpa/inet.h>  /* definitions for internet operations */

/* SERVER RELATED */
/*----------------------------------------------------------------------------------------------------------------------------*/
// add address for connection to a server
int addPort(void) {
	int serverPort,temp=0;
	printf("*** Add the port on which you want to listen:");	// enter a port on which you want to listen
	if ( (temp=scanf("%d",&serverPort)) <= 0) {
		serverPort=SERVER_PORT;
		printf("ERROR: Wrong or misspelled port number - set to the default %d\n",serverPort);
	}
	return serverPort;
}

// new socket descriptor
int socketDescriptor(void) {
	int descriptor;
	if ((descriptor=socket(AF_INET,SOCK_STREAM,0)) < 0) {	// open a socket, a TCP stream is 2-way
		printf("ERROR: Failed in a socket creation - %s\n",strerror(errno));
		exit(1);
	}
	return descriptor;
}

// ip address/port structure for clients/server
void sinStructure(struct sockaddr_in *structName,int port) {
	// set server structure parameters
	structName->sin_family=AF_INET;
	structName->sin_addr.s_addr=inet_addr(SERVER_IP);
	structName->sin_port=htons(port);
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
	if(listen(descriptor,MAX_AVAILABLE) < 0) {
		printf("ERROR: Failed in a socket listening - %s\n",strerror(errno));
		exit(1);
	}
}

// accept connections from other clients
int acceptConnection(int descriptor,struct sockaddr_in *structName,int *length) {
		get_time currentTime;
		int socketDescriptor;
		// get a socket for an incoming client connection
		if((socketDescriptor=accept(descriptor,(struct sockaddr *) structName, length))<0) {
			printf("ERROR: Failed in a temporary socket creation\n");
			exit(1);
		} else {
			CURTIME;
			printf("%s INFO: Socket for client is now created = <%d>\n", currentTime.buffer, socketDescriptor);
		}
		return socketDescriptor;
}
/*----------------------------------------------------------------------------------------------------------------------------*/
