/* HEADER FILE INCLUDES */
#include <sys/socket.h>	/* for socket(), bind(), connect(), recv() and send() */
#include <netdb.h>		/* definitions for network database operations */
#include <netinet/in.h>	/* Internet Protocol family */
#include <arpa/inet.h>	/* definitions for internet operations */

#include <stdio.h>		/* for printf() and fprintf() */
#include <string.h>		/* different manipulations over a char arrays, such as memset() */
#include <fcntl.h>		/* file control options such as open() */
#include <unistd.h>		/* standard symbolic constants and types such as close(), read() */
#include <stdlib.h>		/* malloc() , calloc(), free() , atof(), atoi(), atol() */

#include <sys/types.h>	/* data types */
#include <errno.h>		/* reporting and retrieving error conditions */
#include <sys/wait.h>	/* for waitpid() */
#include <sys/ipc.h>	/* interprocess communication access structure - ftok() */
#include <sys/shm.h>	/* headers for shared memory mechanism */

#include <time.h>		/* time() */
#include <pthread.h>	/* Threading */
#include "server_listen_threads.h"	/* include my custom headers and definitions */


/* ----------------------------------- MAIN FUNCTION ----------------------------------- */
int main()
{
	printf("INFO: DZChat (terminal C based chat) Version 1.1.0 \n");

	get_time currentTime;

	int sock_desc=0, serverPort=0, clientSock=0;	// socket for server connections; server listening port; socket for client processing;
	unsigned int threadCounter=0;		// Threads counter
	socklen_t clientLength;				// a length of a client socket
	struct sockaddr_in server,client;	// add structures for client and server
	handle_client initalParameters;		// for passing ip and socket to a handling thread
	pthread_t threadID;					// thread id used to process threads

	memset(&server,0,sizeof(server));	// null out a server structure
	memset(&client,0,sizeof(client));	// null out a client structure

	/* server related processing */
	serverPort=addPort();							// ask a user for a port
	sock_desc=socketDescriptor();					// open a socket descriptor for a server
	sinStructure(&server,serverPort);				// define ip parameters for a server structure
	bindServer(sock_desc,&server,sizeof(server));	// associate and reserve a port for a socket
	listenServer(sock_desc);						// start listening for connections

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


/* ----------------------------------- HANDLER FOR CLIENTS ----------------------------------- */
void * handleTCPClient(void * arguments)
{

	get_time currentTime;

	/* passed structure to a thread */
	handle_client * initial = (handle_client *) arguments;
	int socket = (int ) initial->socket;		// socket used to process clients
	char * ip = (char *) initial->ip;			// ip address of a connected client
	unsigned int counter = (unsigned int) initial->counter;			// ip address of a connected client

	client_parameters * clientStructuresList;				// for client parameters: name, id, socket, ip, port
	int membersOnline=0, myID=0, recvMsgSize=0;				// members online counter; id for a current client; size of a received message;
	char echoBuffer[RCVBUFSIZE], name[NAME_SIZE], port[6];	// Buffer for echo string; acceptable name size; clients binded port;

	memset(name,0,sizeof(name));
	memset(echoBuffer,0,sizeof(echoBuffer));
	memset(port,0,sizeof(port));

	// Frist we expect client to send an identification: his name, listening port (not the port used to connect to us)
	if ((recvMsgSize = recv(socket, echoBuffer, RCVBUFSIZE, 0)) < 0) printf("ERROR <%d>: Failed in receiving an identification from client\n", counter);

	// User identification: get user name, user listening port
	if (strstr(echoBuffer,ID_STRING)) {
		userIdentification(echoBuffer,sizeof(echoBuffer),name,port);		// echoBuffer is cleared in the identification function
		CURTIME;
		printf("%s INFO <%d>: The client identified himself as %s\n", currentTime.buffer, counter, name);
	}
	memset(echoBuffer,0,sizeof(echoBuffer));
	//
	myID=randomID();					// generate new id for a current client
	membersOnline=onlineCounter(1);		// keep an online counter actual
	//
	clientStructuresList=clientsStructure(name, &myID, &socket, port, ip, membersOnline, &counter);	// update array of structures
	int * currentList=setCurrentList(&myID, 1, clientStructuresList);								// add client to a list of IDs
	notificationOnline(currentList, clientStructuresList, &socket, membersOnline, &counter);			// Notify user, about who is online now
	chatting(echoBuffer, &socket, clientStructuresList, currentList , &counter);						// Receive/send messages
	//
	onlineCounter(0);									// keep an online counter actual
	setCurrentList(&myID, 0, clientStructuresList);		// delete user ID from the list of IDs and delete an information from the structure
	// clear variables
	memset(name,0,sizeof(name));
	memset(echoBuffer,0,sizeof(echoBuffer));
	memset(port,0,sizeof(port));
	membersOnline=0, myID=0, recvMsgSize=0;
}
/* ----------------------------------- HANDLER FOR CLIENTS END ----------------------------------- */


/* It allows a user to insert his own parameters in array of structures */
/*----------------------------------------------------------------------------------------------------------------------------*/
client_parameters * clientsStructure(char * name, int * myID, int * socket, char * portListening, char * ip, int usersOnline, unsigned int * counter) {
	static client_parameters params[1000];		// structure for client parameters

	if (params[*myID].client_id != 0) {			// be sure that this id is not allocated yet for an other client
		while(params[*myID].client_id != 0) *myID=randomID();
	}

	strcpy(params[*myID].nickname,name);			// set a client name to a structure
	params[*myID].client_id = *myID;				// set a client id to a structure
	params[*myID].socket = *socket;					// set a client socket to a structure
	params[*myID].port = atoi(portListening);		// set a client port to a structure
	strcpy(params[*myID].ip,ip);					// set a client name to a structure
	params[*myID].counter = *counter;				// set a thread number to a structure

	return params;		// return a shared memory with arrays of structures
}
/*----------------------------------------------------------------------------------------------------------------------------*/

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


/* Notify connected user how many online user do we have */
/*----------------------------------------------------------------------------------------------------------------------------*/
void notificationOnline(int * list, client_parameters * params, int * socket, int online, unsigned int * counter) {

	get_time currentTime;

	int id=0, temp=0, len=0;
	// buffer for announces to a currently connected client
	char notification[500], append[100];
	char connectedUser[NAME_SIZE], connectedUserIP[16], connectedUserListen[6], connectedUserID[5];

	// clear char arrays before using
	memset(notification,0,sizeof(notification));
	memset(append,0,sizeof(append));
	memset(connectedUser,0,sizeof(connectedUser));
	memset(connectedUserIP,0,sizeof(connectedUserIP));
	memset(connectedUserListen,0,sizeof(connectedUserListen));

	CURTIME;
	printf("%s INFO <%d>: The current list of clients online (currently: %d) :\n", currentTime.buffer, *counter, online);
	for(int i=0;i<MAX_AVAILABLE;i++)
	{
		id = list[i];
		if(id > 0) {	// check if a list value is set
			snprintf(append,sizeof(append),"Users online <%d>: ",online);
			snprintf(append,sizeof(append),"%s has id %d; ",params[id].nickname,params[id].client_id);
			strcat(notification,append);		// append the gotten value to the notification char array
			memset(append,0,sizeof(append));	// clear the append array
			id=0;	// clear id
		}
	}
	
	if (notification != 0) {	// send a notice how many users are online to a client
		strcat(notification,"\n");
		len = strlen(notification);
		if (send(*socket, notification, len, 0) != len) {
				printf("ERROR <%d>: Error to send() a notification to client - %s\n", *counter, strerror(errno));
		} else {
				CURTIME;
				printf("%s MESSAGE OUT <%d>: online list is sent\n", currentTime.buffer, *counter);
		}
	}
	memset(notification,0,sizeof(notification));

	/* Set a name of just connected user */
	for (int i=0; i<MAX_AVAILABLE; i++)
	{
		temp=list[i];
		if (*socket == params[temp].socket) {
			strcpy(connectedUser,params[temp].nickname);
			strcpy(connectedUserIP,params[temp].ip);
			snprintf(connectedUserListen,sizeof(connectedUserListen),"%d:",params[temp].port);
			snprintf(connectedUserID,sizeof(connectedUserID),"%d:",params[temp].client_id);
			break;
		}
	}

	/* Notify others who has connected */
	for(int i=0; i<MAX_AVAILABLE; i++)
	{
		// notify clients about a disconnection
		if (list[i]!=0) {
			temp=list[i];
			if (*socket!=params[temp].socket) {	// check if user from the list is not a just connected user, not to notify him about hiself connection
				snprintf(notification,sizeof(notification),"SYSTEM:SERVER_UPDATE:connected:%s:",connectedUser);
				strcat(notification, connectedUserID);
				strcat(notification, "0:");
				strcat(notification, connectedUserListen);
				strcat(notification, connectedUserIP); strcat(notification, ":");	

				len = strlen(notification);
				if(send(params[temp].socket, notification, len, 0) != len) {
					printf("ERROR <%d>: Error to send() a notification to client - %s\n", *counter, strerror(errno));
				} else {
					CURTIME;
					printf("%s MESSAGE OUT <%d>: notification about a new connection is sent to <%s:%d>\n", currentTime.buffer, *counter, params[temp].ip, params[temp].port);
				}
			}
		}

	}
	memset(notification,0,sizeof(notification));	// clear the buffer before exit
	memset(append,0,sizeof(append));
}
/*----------------------------------------------------------------------------------------------------------------------------*/

/* Retrieve from a user name and his listening port (not the port used to connect to us) */
/*----------------------------------------------------------------------------------------------------------------------------*/
char userIdentification(char * buffer,int size,char * name,char * port) {
	char *p=0; p=buffer;	// set a pointer to a buffer, to catch a name of the user in a char array
	char *n=0; n=name;		// set a pointer to a name array
	char *po=0; po=port;	// set a pointer to a port array
	int length=NAME_SIZE, namesize=0, portLengthAcceptable=6, portLengthReal=0;	// set limiter for while loop ; used to count a name size ; size of the port;
	
	int moveto = sizeof(ID_STRING);	// moving integer
	p += moveto-1;	// move the pointer to the beginning of a user's name

	/* get a name */
	while(*p && *p != '\"' && *p != '\'' && *p != ';' && *p != ' ' && *p != ':' && length-->0)
	{
		if(strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-0123456789_",*p)) {
			*n++=*p++;		// move the gotten char to name char array
			namesize++;		// count a name size
		} else {
			p++;			// if the char is not acceptable just move to another one
		}
	}

	/* get client port */
	if ( strchr(":",*p) ) {		// colon is a delimiter for name and port
		p++;				// move pointer from colon to a beginning port symbol
		while (*p && portLengthAcceptable-->0)
		{
			if(strchr("0123456789",*p)) {
				*po++=*p++;	// copy port symbols to a port variable
				portLengthReal++;
			} else {
				p++;		// if the char is not acceptable just move to another one
			}
		}
	}
	p -= (namesize+moveto+portLengthReal);	// move a pointer back to a beginning
	memset(p,0,size);		// set passed buffer to null
}
/*----------------------------------------------------------------------------------------------------------------------------*/


/* Functionaly for providing updates to users */
/* It is also cab be used to process as intermideary proxy for sending messages, but for now it is deprecated */
/*----------------------------------------------------------------------------------------------------------------------------*/
void chatting(char * buffer, int * socket, client_parameters * list, int * idList, unsigned int * counter) {

	get_time currentTime;

	int recvMsgSize=0;						// sive of a received message; an acceptable length of a name; an acceptable length of a message; 
	memset((char *)buffer,0,RCVBUFSIZE);	// clear the buffer before to obtain a new data

	// Start receiving a text message from a client
	if ((recvMsgSize = recv(*socket, (char *)buffer, RCVBUFSIZE, 0)) < 0) printf("ERROR <%d>: Failed in receiving a message from client\n", *counter);

	// Send received string and receive again until end of transmission
	while (recvMsgSize > 0)		// zero indicates end of transmission
	{
		char *p=0; p=buffer;
		int id=0;

		// check if a user asks us for updates
		if (strstr(buffer, "SYSTEM:USER_UPDATE")) {
			printf("SYSTEM:USER_UPDATE: User <%s> requires updates\n",list->nickname);
			giveUpdates(buffer, socket, list, idList, counter);
			goto ALIAS_RECEIVE;	// no wait for new messsages from a client
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
				strcpy(answer, beginning);																// "SYSTEM:USER_UPDATE:"
				strcat(answer, parameters[temp].nickname); strcat(answer, ":");							// user nickname
				snprintf(id, sizeof(id), "%d:", parameters[temp].client_id); strcat(answer, id); 		// id
				strcat(answer, "0:");																	// his socket, must be empty there
				snprintf(port,sizeof(port),"%d:",parameters[temp].port); strcat(answer, port);			// user listening port
				strcat(answer, parameters[temp].ip); strcat(answer, ":");								// user ip address
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

// Generate an unique ID for a new client
int randomID(void) {
	int id=0;
	time_t t;

	/* Intializes random number generator */
	srand((unsigned) time(&t));
	id = rand() % 1000 ;
	return id;
}
/*----------------------------------------------------------------------------------------------------------------------------*/

/* It allows us to keep the actual value of current members online */
int onlineCounter(int action) {
	// action=1 - increase counter
	// action=0 - decrease counter
	static int counter;

	switch(action) {
		case 1:				//increase
			if (counter != 0) {
				counter++;
			} else {
				counter = 0;
				counter++;
			}
			break;
		case 0:				// dercrease
			if (counter != 0) counter--;
			break;
		default:
			printf("WARNING: Wrong action definition in online users counter - ingoring\n");
	}

	return counter;	//return users online
}
/*----------------------------------------------------------------------------------------------------------------------------*/
