/* DZChat - Client. Sep 2018 @Donat Zenichev */
/* This functionality is responsible for identification of myself to the data server */

/* HEADER FILE INCLUDES */
#include <stdio.h>        /* for printf() and fprintf() */
#include <string.h>       /* different manipulations over a char arrays, such as memset() */
#include "main.h"         /* Include defenitions */
#include <sys/socket.h>   /* for socket(), bind(), connect(), recv() and send() */
#include <errno.h>        /* reporting and retrieving error conditions */
#include <stdlib.h>       /* malloc() , calloc(), free() , atof(), atoi(), atol() */

/* ------------------------ IDENTIFY YOURSELF TO THE DATA SERVER --------------------------- */
void identification(int * descriptor, int * port, char * nickname, receive_handler * passStructure) {
	// passStructure.parameters
	// passStructure.ids

	get_time currentTime;

	int identityLen=0, recvMsgSize=0, movePtr=0, online=0;
	char echoBuffer[RCVBUFSIZE], addPort[6];
	char identityString[33] = "z9HG4bk:";

	memset(nickname,0,sizeof(nickname));        // my nickname
	memset(echoBuffer,0,sizeof(echoBuffer));    // buffer for a response
	memset(addPort,0,sizeof(addPort));          // my listening port
	
	printf("*** Add a nickname (maximum 20 characters):");
	scanf("%s",nickname);                       // ask user for his username
	strcat(identityString, nickname);           // add an identification string

	snprintf(addPort,sizeof(addPort),":%d",*port);
	strcat(identityString, addPort);            // add a port at the end
	// Send the string to the server
	identityLen = strlen(identityString);
	if (send(*descriptor, identityString, identityLen, 0) != identityLen) {
		printf("ERROR: Cannot send an identification - %s\n",strerror(errno));
	}
	memset(identityString,0,identityLen);

	// Receive a current information from a server
	if ((recvMsgSize = recv(*descriptor, echoBuffer, sizeof(echoBuffer), 0)) < 0) printf("ERROR: Error in receiving information from a server\n");

	// update local data storage and get an amount of online participatns
	if (strstr(echoBuffer, "SYSTEM:SERVER_UPDATE:ONLINE="))
	online=getMembersData(echoBuffer, passStructure);
	CURTIME;
	printf("\nINFO: List of clients online: %d", online);
	memset(echoBuffer,0,sizeof(echoBuffer));
	memset(addPort,0,sizeof(addPort));
	identityLen=0, recvMsgSize=0;
}
/* ----------------------------------------------------------------------------------------- */

/* Update local array of structures, and return amount of online participants */
int getMembersData(char * echoBuffer, receive_handler * passStructure) {

		int maxLength = 1000;                     // just to restrict the possible infinite loop
		int maxFieldSize = 50;                    // maximum size of each field
		int clientsAdded = 0;                     // restrict the cycle

		char online[4] = {0}, client_id[5] = {0}, name[NICK_SIZE] = {0}, ipAddr[16] = {0}, port[6] = {0}, addMember[100] = {0}, clSocket[3] = {0};
		char * ptrSrc; ptrSrc = echoBuffer;       // point on the receiving buffer
		char * ptrOnline; ptrOnline = online;     // point on the string that stores online users
		char * ptrID; ptrID = client_id;          // for a client id
		char * ptrN; ptrN = name;                 // for a client name
		char * ptrIP; ptrIP = ipAddr;             // for an ip address
		char * ptrP; ptrP = port;                 // for a port
		char * ptrSct; ptrSct = clSocket;         // for a socket (not used now)

		// move the pointer to the online members amount
		int movePtr=27;	//SYSTEM:SERVER_UPDATE:ONLINE=
		ptrSrc += movePtr;

		// initialize an array with the client ids
		for(int i=0; i<MAX_AVAILABLE_CLIENTS; i++)
		{
			passStructure->ids[i]=0;
		}

		// get amount of online members
		while(*ptrSrc && *ptrSrc != ';' && *ptrSrc != '\0' && *ptrSrc != '\n')
		{
			if(strchr("0123456789",*ptrSrc)) {      // acceptable symbols for a name
				*ptrOnline++=*ptrSrc++;               // move the gotten char to name char array
			} else {
				ptrSrc++;                             // if the char is not acceptable just move to another one
			}
		}

		// start spliting each client block: "942:donat:127.0.0.1:5112;"
		// at a first cycle, a data block enters here without a semicolon at the beginning
		// then each following cycle, we have to move from the ';' by checks - if ptrSrc = ';'
		while(*ptrSrc && *ptrSrc !='\0' && *ptrSrc !='\n' && maxLength-->0 && clientsAdded++<MAX_AVAILABLE_CLIENTS)
		{
			if (*ptrSrc == ';') ptrSrc++;           // in case we stopped at ';' move further
			if (*ptrSrc == '\0' || *ptrSrc == ' ' || *ptrSrc =='\n') break;	// this stops execution when data body is finished

			// each client data block is processed by one cycle, below
			//---------------------------------------------------------------------------------------------------	
			while( *ptrSrc != ';' && *ptrSrc != ' ' && *ptrSrc != '\0' && *ptrSrc != '\n')	// a last processed filed (for one user) is followed by ';'
			{
				// get a name - store it in the 'name'
				while(*ptrSrc && *ptrSrc != ':' && *ptrSrc != ';' && maxFieldSize-->0)
				{
					if(strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-0123456789_",*ptrSrc)) {	// acceptable symbols for a name
						*ptrN++=*ptrSrc++;                // move the gotten char to name char array
					} else {
						ptrSrc++;                         // if the char is not acceptable just move to another one
					}			
				}
				maxFieldSize=50;	// reset max field size back to 50
				ptrSrc++;			// move pointer to a next data field

				// get a client_id - store it in the 'client_id'
				while(*ptrSrc && *ptrSrc != ':' && *ptrSrc != ';' && maxFieldSize-->0)
				{
					if(strchr("0123456789",*ptrSrc)) {	// acceptable symbols for an ID
						*ptrID++=*ptrSrc++;               // move the gotten char to name char array
					} else {
						ptrSrc++;                         // if the char is not acceptable just move to another one
					}			
				}
				maxFieldSize=50;                      // reset max field size back to 50
				ptrSrc++;                             // move pointer to a next data field

				// get a client socket - store it in the 'clSocket'
				while(*ptrSrc && *ptrSrc != ':' && *ptrSrc != ';' && maxFieldSize-->0)
				{
					if(strchr("0123456789",*ptrSrc)) {	// acceptable symbols for an ID
						*ptrSct++=*ptrSrc++;              // move the gotten char to name char array
					} else {
						ptrSrc++;                         // if the char is not acceptable just move to another one
					}			
				}
				maxFieldSize=50;                      // reset max field size back to 50
				ptrSrc++;                             // move pointer to a next data field

				// get a port - store it in the 'port'
				while(*ptrSrc && *ptrSrc != ':' && *ptrSrc != ';' && maxFieldSize-->0)
				{
					if(strchr("0123456789",*ptrSrc)) {  // acceptable symbols for an ID
						*ptrP++=*ptrSrc++;                // move the gotten char to name char array
					} else {
						ptrSrc++;                         // if the char is not acceptable just move to another one
					}			
				}
				maxFieldSize=50;                      // reset max field size back to 50
				ptrSrc++;                             // move pointer to a next data field

				// get an ip address - store it in the 'ipAddr'
				while(*ptrSrc && *ptrSrc != ':' && *ptrSrc != ';' && maxFieldSize-->0)
				{
					if(strchr("0123456789.",*ptrSrc)) {	// acceptable symbols for an ip address
						*ptrIP++=*ptrSrc++;               // move the gotten char to name char array
					} else {
						ptrSrc++;                         // if the char is not acceptable just move to another one
					}			
				}
				maxFieldSize=50;                      // reset max field size back to 50

				// here prtSrc stopped at last ';' before a next client data field and a cycle for a current data block is stopped
				// we return back to the previouos cycle

				// pass string to a structureAdd() as follows: "SYSTEM:SERVER_UPDATE:online:name:111:0:5000:127.0.0.1"
				// addMember is a string used to store a full member information when passing it to structureAdd()
				strcpy(addMember, "SYSTEM:SERVER_UPDATE:online:");
				strcat(addMember, name); strcat(addMember, ":");
				strcat(addMember, client_id); strcat(addMember, ":");
				strcat(addMember, "0:");
				strcat(addMember, port); strcat(addMember, ":");
				strcat(addMember, ipAddr); strcat(addMember, ":");
				structureAdd(addMember, passStructure->parameters, passStructure->ids);	// add new user to local arrays

				// clear arrays
				memset(name,0,strlen(name));
				memset(client_id,0,strlen(client_id));
				memset(port,0,strlen(port));
				memset(ipAddr,0,strlen(ipAddr));
				memset(addMember,0,strlen(addMember));

				// reset pointers
				ptrID = 0;	ptrID = client_id;        // for a client id
				ptrN = 0;	ptrN = name;                // for a client name
				ptrIP = 0;	ptrIP = ipAddr;           // for an ip address
				ptrP = 0;	ptrP = port;                // for a port
				ptrSct = 0;	ptrSct = clSocket;        // for a socket (not used now)
			}//---------------------------------------------------------------------------------------------------
		}

		//return back the quantity of online users
		return( atoi(online) );
}
