/* DZChat - Server. Sep 2018 @Donat Zenichev */
/* thread handler - each client is processed in a separate thread */

#include <stdio.h>		/* for printf() and fprintf() */
#include <string.h>		/* different manipulations over a char arrays, such as memset() */
#include <sys/socket.h>		/* for socket(), bind(), connect(), recv() and send() */
#include <sys/types.h>		/* data types */
#include "main.h"		/* include my custom headers and definitions */

/* ----------------------------------- HANDLER FOR CLIENTS ----------------------------------- */
void * handleTCPClient(void * arguments)
{

	get_time currentTime;

	/* passed structure to a thread */
	handle_client * initial = (handle_client *) arguments;
	int socket = (int ) initial->socket;					// socket used to process clients
	char * ip = (char *) initial->ip;					// ip address of a connected client
	unsigned int counter = (unsigned int) initial->counter;			// ip address of a connected client

	client_parameters * clientStructuresList;				// for client parameters: name, id, socket, ip, port
	int membersOnline=0, myID=0, recvMsgSize=0;				// members online counter; id for a current client; size of a received message;
	char echoBuffer[RCVBUFSIZE], name[NAME_SIZE], port[6];			// Buffer for echo string; acceptable name size; clients binded port;

	memset(name,0,sizeof(name));
	memset(echoBuffer,0,sizeof(echoBuffer));
	memset(port,0,sizeof(port));

	// Frist we expect client to send an identification: his name, listening port (not the port used to connect to us)
	if ((recvMsgSize = recv(socket, echoBuffer, RCVBUFSIZE, 0)) < 0) printf("ERROR <%d>: Failed in receiving an identification from client\n", counter);

	// User identification: get user name, user listening port
	if (strstr(echoBuffer,ID_STRING)) {
		userIdentification(echoBuffer,sizeof(echoBuffer),name,port);	// echoBuffer is cleared in the identification function
		CURTIME;
		printf("%s INFO <%d>: The client identified himself as %s\n", currentTime.buffer, counter, name);
	}
	memset(echoBuffer,0,sizeof(echoBuffer));
	//
	myID=randomID();							// generate new id for a current client
	membersOnline=onlineCounter(1);						// keep an online counter actual
	//
	clientStructuresList=clientsStructure(name, &myID, &socket, port, ip, membersOnline, &counter);	// update array of structures
	int * currentList=setCurrentList(&myID, 1, clientStructuresList);								// add client to a list of IDs
	notificationOnline(currentList, clientStructuresList, &socket, membersOnline, &counter);	// Notify user, about who is online now
	chatting(echoBuffer, &socket, clientStructuresList, currentList , &counter);						// Receive/send messages
	//
	onlineCounter(0);										// keep an online counter actual
	setCurrentList(&myID, 0, clientStructuresList);							// delete user ID from the list of IDs and delete an information from the structure
	// clear variables
	memset(name,0,sizeof(name));
	memset(echoBuffer,0,sizeof(echoBuffer));
	memset(port,0,sizeof(port));
	membersOnline=0, myID=0, recvMsgSize=0;
}
/* ----------------------------------- HANDLER FOR CLIENTS END ----------------------------------- */
