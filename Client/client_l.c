/* HEADER FILE INCLUDES */
#include <sys/socket.h>	/* for socket(), bind(), connect(), recv() and send() */
#include <netdb.h>		/* definitions for network database operations */
#include <netinet/in.h>	/* Internet Protocol family */
#include <arpa/inet.h>		/* definitions for internet operations */

#include <stdio.h>		/* for printf() and fprintf() */
#include <string.h>		/* different manipulations over a char arrays, such as memset() */

#include <stdlib.h>		/* malloc() , calloc(), free() , atof(), atoi(), atol() */
#include <sys/types.h>		/* data types */
#include <inttypes.h>		/* intptr_t */
#include <errno.h>		/* reporting and retrieving error conditions */

#include <fcntl.h>		/* file control options such as open() */
#include <unistd.h>		/* standard symbolic constants and types such as close(), read() */

#include <pthread.h>		/* Threading */

#include "client_listen.h"	/* Include defenitions */


/* ----------------------------------- MAIN FUNCTION ----------------------------------- */
int main()
{
	char nickname[NICK_SIZE];							// my own name

	pthread_t threadID;								// thread id for a listening thread
	recepient passRecepient;							// pass values to a listening thread
	struct sockaddr_in uas;								// a structure to receive messages from other clients
	int sock_desc=0, port=0;							// socket for listening; our listening port
	memset(&uas,0,sizeof(uas));

	int sock_desc_client, serverPort, clientLength, counter=0;			// socket for sending; server port; - ; - ;
	char serverAddress[17];								// the format is with quotes and dotes: "000.000.000.000"
	struct sockaddr_in client;							// a structure to send messages
	memset(&client,0,sizeof(client));
	
	/* uas - direct messaging with other clients */
	sock_desc=socketDescriptor();							// open a socket descriptor for listening
	port=randomPort();								// randomized port for listening
	sinStructure(&uas,port,'\0');							// define ip parameters for a server structure
	bindServer(sock_desc,&uas,sizeof(uas));						// associate and reserve a port for a socket
	listenServer(sock_desc);							// start listening for connections from other clients - for direct messaging

	/* For connections to a data server */
	addAddress(serverAddress);							// ask a user for a server address
	serverPort=addPort();								// ask a user for a server port
	sock_desc_client=socketDescriptor();						// open a socket descriptor for a connection to a server
	sinStructure(&client,serverPort,serverAddress);					// define ip parameters for a client structure
	clientLength=sizeof(client);
	connection(sock_desc_client,&client,clientLength);				// connect to a server
	//
	identification(&sock_desc_client,&port,nickname);				// identify myself to a server
	introduction();									// show introduction to a user

	// pass needed values to a listening thread
	passRecepient.socket = sock_desc;
	passRecepient.uas = &uas;
	// thread for messages from other clients
	pthread_create(&threadID, NULL, dataReceivingFromClients, (void *) &passRecepient);
	// start chatting
	chatting(&sock_desc_client,&sock_desc, nickname);

	pthread_exit(NULL);								// close a thread
	close(sock_desc_client);							// close a client socket
	close(sock_desc);								// close a uas socket

	exit(0);
	return 0;
}
/* ----------------------------------- MAIN FUNCTION END ----------------------------------- */

/* ---------------------------------- DATA RECEIVE/SEND ------------------------------------ */
void chatting(int * descriptor, int * descriptorUAS, char * nickname) {

	pthread_t threadID;								// thread id
	struct sockaddr_in client_direct;						// for direct connections to other clients
	int echoStringLen=0, temp=0, tempPort=0, userFound=0, recvMsgSize=0, id=0, sock_new_connection=0, clientLength=0;
	//
	receive_handler passStructure;							// needed for passing different values to a thread
	passStructure.socket = *descriptor;						// a socket for a messaging with a data server
	//
	char echoString[RCVBUFSIZE], name[NICK_SIZE], myMessage[RCVBUFSIZE]={0}, systemMessage[RCVBUFSIZE], myMessageFull[RCVBUFSIZE]={0};

	memset(echoString,0,strlen(echoString));					// a whole data entered by a user
	memset(name,0,strlen(name));							// a name of the destination user
	memset(systemMessage,0,strlen(systemMessage));					// system messsage to a data server
	memset(&client_direct,0,sizeof(client_direct));					// sin structure for outgoing connections with users

	// initialize an array with the client ids
	for(int i=0; i<MAX_AVAILABLE_CLIENTS; i++)
	{
		passStructure.ids[i]=0;
	}

	// thread for messages from a server
	pthread_create(&threadID, NULL, dataReceiving, (void *) &passStructure);

	/* start getting a messages */
	fgets(echoString, RCVBUFSIZE, stdin);
	for (;;) 
	{
		printf("*** send a message> ");						// Prompt to send a message
		if ( fgets(echoString, RCVBUFSIZE, stdin) == NULL ) {			// get a message
			continue;							// if message is null, ignore entering
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
			getDestName(echoString, name);					// set a destination name
			getMessageBody(echoString, myMessage);				// set a message
			strcpy(myMessageFull, nickname); strcat(myMessageFull, ":");
			strcat(myMessageFull, myMessage);
		}

		//--------------------------------------------------------------------------------------------------------------------------------
		// Check if we have user in our local structure
		for(int i=0; i<MAX_AVAILABLE_CLIENTS; i++)
		{
			if (passStructure.ids[i] > 0) {
				temp=passStructure.ids[i];				// store id parameter in a temporary variable
				// try to find a needed name
				if ( strstr(passStructure.parameters[temp].nickname, name) && strlen(passStructure.parameters[temp].ip) != 0 && passStructure.parameters[temp].port != 0) {
					userFound=1;
					// try to connect to a given user
					sock_new_connection=socketDescriptor();
					tempPort=(intptr_t)passStructure.parameters[temp].port;
					sinStructure(&client_direct,tempPort,passStructure.parameters[temp].ip);
					clientLength=sizeof(client_direct);
					connection(sock_new_connection,&client_direct,clientLength);
					if (send(sock_new_connection, myMessageFull, sizeof(myMessageFull), 0) != sizeof(myMessageFull)) printf("ERROR: send() sent a different number of bytes than expected\n");
					close(sock_new_connection);			// do not forget to close the socket
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
					if (passStructure.ids[i] > 0) {
						temp=passStructure.ids[i];
						if ( strstr(passStructure.parameters[temp].nickname, name) && strlen(passStructure.parameters[temp].ip) != 0 && passStructure.parameters[temp].port != 0) {
							userFound=1;
							sock_new_connection=socketDescriptor();
							tempPort=(intptr_t)passStructure.parameters[temp].port;
							sinStructure(&client_direct,tempPort,passStructure.parameters[temp].ip);
							clientLength=sizeof(client_direct);
							connection(sock_new_connection,&client_direct,clientLength);
							if (send(sock_new_connection, myMessageFull, sizeof(myMessageFull), 0) != sizeof(myMessageFull)) printf("ERROR: send() sent a different number of bytes than expected\n");
							close(sock_new_connection);	// do not forget to close the socket
							break;
						}
					}
				}
				if ( strstr(passStructure.parameters[temp].nickname, name) && strlen(passStructure.parameters[temp].ip) != 0 && passStructure.parameters[temp].port != 0) break;
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

/* ------------------------------ RECEIVE MESSAGES FROM A SERVER --------------------------- */
void * dataReceiving(void * arguments) {

	get_time currentTime;

	/* passed structure to a thread */
	receive_handler * passed = (receive_handler *) arguments;
	int socket = (int ) passed->socket;
	client_parameters * parameters = (client_parameters *) passed->parameters;
	int *idList = (int*) passed->ids;

	int recvMsgSize=0, id=0, temp=0, deleted=0;
	char echoBuffer[RCVBUFSIZE];
	memset(echoBuffer,0,sizeof(echoBuffer));

	for (;;) 
	{
		// try to receive an echo returned from a server
		if ((recvMsgSize = recv(socket, echoBuffer, RCVBUFSIZE, 0)) < 0) {
			sleep(1);
		} else {

			// check if this is a null message
			if (strlen(echoBuffer) == 0) {
				continue;
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
			sleep(1);
		} else {
			if (strlen(echoBuffer) == 0) continue; 		// check if this is a null message
			if (!strstr(echoBuffer, "SYSTEM:USER_UPDATE")) {
				CURTIME;
				printf("\n> %s MESSAGE: %s", currentTime.buffer, echoBuffer); // if it is not a system message
				printf("\n*** send a message>");
				fflush( stdout );
			}
			memset(echoBuffer,0,sizeof(echoBuffer));	// null out a buffer
			continue;
		}
		close(incomingSocket);							// close a used socket
	}
}
/* ----------------------------------------------------------------------------------------- */

/* ------------------------ IDENTIFY YOURSELF TO THE DATA SERVER --------------------------- */
void identification(int * descriptor, int * port, char * nickname) {

	get_time currentTime;

	int identityLen=0, recvMsgSize=0;
	char echoBuffer[RCVBUFSIZE], addPort[6];
	char identityString[33] = "z9HG4bk:";
	memset(nickname,0,sizeof(nickname));
	memset(echoBuffer,0,sizeof(echoBuffer));
	memset(addPort,0,sizeof(addPort));
	
	printf("*** Add a nickname (maximum 20 characters):");
	scanf("%s",nickname);							// ask user for his username
	printf("INFO: Trying identify myself to a server as: %s\n",nickname);
	strcat(identityString, nickname);				// add an identification string

	snprintf(addPort,sizeof(addPort),":%d",*port);
	strcat(identityString, addPort);				// add a port at the end
	// Send the string to the server
	identityLen = strlen(identityString);
	if (send(*descriptor, identityString, identityLen, 0) != identityLen) {
		printf("ERROR: Cannot send an identification - %s\n",strerror(errno));
	}
	memset(identityString,0,identityLen);

	// Receive a current information from a server
	if ((recvMsgSize = recv(*descriptor, echoBuffer, sizeof(echoBuffer), 0)) < 0) printf("ERROR: Error in receiving information from a server\n");
	CURTIME;
	printf("> %s MESSAGE: List of clients online: %s", currentTime.buffer, echoBuffer);
	memset(echoBuffer,0,sizeof(echoBuffer));
	memset(addPort,0,sizeof(addPort));
	identityLen=0, recvMsgSize=0;
}
/* ----------------------------------------------------------------------------------------- */

/* ------------------------------ ADD NEW USER TO THE LOCAL ARRAY -------------------------- */
int structureAdd(char * sourceString, client_parameters * params, int * ids ) {

	get_time currentTime;

	// returning value
	int result=0;

	// arrays to store an information
	char name[21]={0}, id[5]={0}, port[6]={0}, socket[2]={0}, ip[15]={0};

	// acceptable length of values
	int nameLength=20, idLength=4, socketLegth=1, portLength=6, ipLength=15, movePtr=0, needNotice=0;

	// pointers
	char *ptrSrc, *nPtr, *idPtr, *sPtr, *portPtr, *ipPtr;
	ptrSrc=sourceString; nPtr=name; idPtr=id; sPtr=socket; portPtr=port, ipPtr=ip;

	if (strstr(sourceString, "SYSTEM:SERVER_UPDATE")) {
		movePtr=31;	//SYSTEM:SERVER_UPDATE:connected:
		needNotice=1;
	}
	if (strstr(sourceString, "SYSTEM:USER_UPDATE")) movePtr=19;		//SYSTEM:USER_UPDATE:

	ptrSrc+= movePtr;						// move a pointer to a starting of data

	if ( strchr("0",*ptrSrc) ) return 0;	// nothing found

	/* Get a recipient name */
	while(*ptrSrc && *ptrSrc != ':' && nameLength-->0)
	{
		if(strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-0123456789_",*ptrSrc)) { 
			*nPtr++=*ptrSrc++;		// move the gotten char to name char array
		} else {
			ptrSrc++;       			// if the char is not acceptable just move to another one
		}
	}
	ptrSrc++; 

	/* Get a recipient id */
	while(*ptrSrc && *ptrSrc != ':' && idLength-->0)
	{
		if(strchr("0123456789",*ptrSrc)) {    
			*idPtr++=*ptrSrc++;		// move the gotten char to name char array
		} else {
			ptrSrc++;       			// if the char is not acceptable just move to another one
		}
	}
	ptrSrc++;

	/* Get a recipient socket */
	while(*ptrSrc && *ptrSrc != ':' && socketLegth-->0)
	{
		if(strchr("0123456789",*ptrSrc)) {    
			*sPtr++=*ptrSrc++;		// move the gotten char to name char array
		} else {
			ptrSrc++;       			// if the char is not acceptable just move to another one
		}
	} 
	ptrSrc++; 

	/* Get a recipient port */
	while(*ptrSrc && *ptrSrc != ':' && portLength-->0)
	{
		if(strchr("0123456789",*ptrSrc)) {
			*portPtr++=*ptrSrc++;		// move the gotten char to name char array
		} else {
			ptrSrc++;       			// if the char is not acceptable just move to another one
		} 
	} 
	ptrSrc++; 

	/* Get a recipient ip */
	while(*ptrSrc && *ptrSrc != ':' && ipLength-->0)
	{
		if(strchr("0123456789.",*ptrSrc)) {
			*ipPtr++=*ptrSrc++;		// move the gotten char to name char array
		} else {
			ptrSrc++;       			// if the char is not acceptable just move to another one
		}
	}

	ptrSrc=0; nPtr=0; idPtr=0; sPtr=0; portPtr=0, ipPtr=0;	// clear pointers values

	strcpy(params[atoi(id)].nickname, name);			// set a client name to a structure
	params[atoi(id)].client_id = atoi(id);				// set a client id to a structure
	params[atoi(id)].socket = atoi(socket);				// set a client socket to a structure
	params[atoi(id)].port = atoi(port);					// set a client port to a structure
	strcpy(params[atoi(id)].ip, ip);					// set a client ip to a structure

	if (needNotice == 1) {
		CURTIME;
		printf("\n> %s MESSAGE: New user connected - <%s>", currentTime.buffer, params[atoi(id)].nickname);	// notify a user that a given client has disconnected
		printf("\n*** send a message>");
		fflush( stdout );
	}

	setCurrentList(&params[atoi(id)].client_id,params,ids);					// add client to a list of IDs
	return atoi(id);
}
/* ----------------------------------------------------------------------------------------- */

/* ------------------ NOTIFICATION TO USER (LOCALY GENERATED) ------------------------------ */
void introduction(void) {
	printf("\n>----------------------------------------------------------------------------->\n");
	printf("INFO: Now you can send messages to online participants                         |\n");
	printf("INFO: Use the following syntax(without angle brackets): <name>: <text-message> |\n");
	printf("INFO: If you want to quit the chat just type> quit;                            |\n");
	printf("INFO: If you want to clear a terminal just type> clear;                        |");
	printf("\n>----------------------------------------------------------------------------->\n");
	printf("\n");
}
/* ----------------------------------------------------------------------------------------- */

/* ------------------ RETRIEVE DATA FROM AN USER MESSAGE ----------------------------------- */
// get a recipient name
char * getDestName(char * string, char * name) {
	char *p; p=string;
	char *n; n=name;
	int nameLength=NICK_SIZE;

	while(*p && *p != ':' && nameLength-->0)
	{
		if(strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-0123456789_",*p)) {	// acceptable symbols for a name
			*n++=*p++;		// move the gotten char to name char array
		} else {
			p++;			// if the char is not acceptable just move to another one
		}
	}
	p=0; n=0;
}
// get a message body
char * getMessageBody(char * string, char * message) {
	char *p; p=string;
	char *m; m=message;
	int messageLength=RCVBUFSIZE;

	// move the pointer until a message body
	while(*p && *p != ':')
	{
		p++;
	}
	p++;

	while(*p && *p != ':' && messageLength-->0)
	{
		if(strchr("'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-0123456789_!? ",*p)) {	// acceptable symbols for a name
			*m++=*p++;		// move the gotten char to name char array
		} else {
			p++;			// if the char is not acceptable just move to another one
		}
	}
	p=0; m=0;
}
/* ----------------------------------------------------------------------------------------- */

/* ----------------------------------------------------------------------------------------- */
/* This array stores an acutal IDs of clients */
int setCurrentList(int * passedID, client_parameters * infoClients, int * list) {
	int temp=0;
	for(int i=0; i<MAX_AVAILABLE_CLIENTS; i++)
	{
		if ((temp=list[i]) == 0) {
			list[i]=*passedID;
			break;
		}
	}
	if(temp!=0) printf("ERROR: The ID of a client was not stored in a list!\n");	//seems that id was not stored
	// clear values
	temp=0;

}

/* Delete a disconnected user */
int delCurrentList(char * buffer, client_parameters * infoClients, int * list) {

	get_time currentTime;

	int deletion=0, nameLength=NICK_SIZE;
	char beginning[21]="SYSTEM:SERVER_UPDATE:", passedID[NICK_SIZE]={0};
	char *nPtr; nPtr=passedID;
	char *ptrSrc; ptrSrc=buffer;
	ptrSrc += sizeof(beginning);

	/* Get a disconnected ID */
	while(*ptrSrc && *ptrSrc != ':' && nameLength-->0)
	{
		if(strchr("0123456789",*ptrSrc)) { 
			*nPtr++=*ptrSrc++;		// move the gotten char to name char array
		} else {
			ptrSrc++;       			// if the char is not acceptable just move to another one
		}
	}
	ptrSrc=0;

	for(int i=0; i<MAX_AVAILABLE_CLIENTS; i++)
	{
		if (list[i]==atoi(passedID) ) {
			CURTIME;
			printf("\n> %s MESSAGE: User <%s> disconnected", currentTime.buffer, infoClients[atoi(passedID)].nickname);	// notify a user that a given client has disconnected
			printf("\n*** send a message>");
			fflush( stdout );
			// clear client information
			infoClients[atoi(passedID)].client_id=0;
			memset(infoClients[atoi(passedID)].nickname,0,sizeof(infoClients[atoi(passedID)].nickname));
			memset(infoClients[atoi(passedID)].ip,0,sizeof(infoClients[atoi(passedID)].ip));
			infoClients[atoi(passedID)].socket=0;
			infoClients[atoi(passedID)].port=0;
			list[i] = 0;	// delete myself from the IDs list
			deletion=1;
		}
	}
	// clear values
	nameLength=0;
	passedID[NICK_SIZE]='\0';
	nPtr=0; ptrSrc=0;

	return deletion;	
}
/* ----------------------------------------------------------------------------------------- */


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
