/* DZChat - Client. Dec 2018 @Donat Zenichev */
/* This functionality is responsible for updating of the local array of structures */
/* This array keeps the data of other clients needed to connect to them */

/* HEADER FILE INCLUDES */
#include <stdio.h>      /* for printf() and fprintf() */
#include <string.h>     /* different manipulations over a char arrays, such as memset() */
#include "main.h"       /* Include defenitions */
#include <stdlib.h>     /* malloc() , calloc(), free() , atof(), atoi(), atol() */

/* ------------------------------ ADD NEW USER TO THE LOCAL ARRAY -------------------------- */
// the entering format for a passed sourceString:
// SYSTEM:SERVER_UPDATE:connected:name:111:0:5000:127.0.0.1 - for recently connected user
// SYSTEM:USER_UPDATE:name:111:0:5000:127.0.0.1	- for user updates generated from us
// SYSTEM:SERVER_UPDATE:online:name:111:0:5000:127.0.0.1 - for getting only while connecting first time 
int structureAdd(char * sourceString, client_parameters * params, int * ids ) {

	get_time currentTime;

	// returning value
	int result=0, temp=0;

	// arrays to store an information
	char name[21]={0}, id[5]={0}, port[6]={0}, socket[2]={0}, ip[15]={0};

	// acceptable length of values
	int nameLength=20, idLength=4, socketLegth=1, portLength=6, ipLength=15, movePtr=0, needNotice=0;

	// pointers
	char *ptrSrc, *nPtr, *idPtr, *sPtr, *portPtr, *ipPtr;
	ptrSrc=sourceString; nPtr=name; idPtr=id; sPtr=socket; portPtr=port, ipPtr=ip;

	if (strstr(sourceString, "SYSTEM:SERVER_UPDATE:connected:")) {
		movePtr=31;	//SYSTEM:SERVER_UPDATE:connected:
		needNotice=1;
	}

	if (strstr(sourceString, "SYSTEM:SERVER_UPDATE:online:")) {
		movePtr=28;	//SYSTEM:SERVER_UPDATE:online:
	}

	if (strstr(sourceString, "SYSTEM:USER_UPDATE")) movePtr=19;		//SYSTEM:USER_UPDATE:

	ptrSrc+= movePtr;           // move a pointer to a starting of data

	if ( strchr("0",*ptrSrc) ) return 0;	// nothing found

	/* Get a recipient name */
	while(*ptrSrc && *ptrSrc != ':' && nameLength-->0)
	{
		if(strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-0123456789_",*ptrSrc)) { 
			*nPtr++=*ptrSrc++;      // move the gotten char to name char array
		} else {
			ptrSrc++;               // if the char is not acceptable just move to another one
		}
	}
	ptrSrc++; 

	/* Get a recipient id */
	while(*ptrSrc && *ptrSrc != ':' && idLength-->0)
	{
		if(strchr("0123456789",*ptrSrc)) {    
			*idPtr++=*ptrSrc++;     // move the gotten char to name char array
		} else {
			ptrSrc++;               // if the char is not acceptable just move to another one
		}
	}
	ptrSrc++;

	/* Get a recipient socket */
	while(*ptrSrc && *ptrSrc != ':' && socketLegth-->0)
	{
		if(strchr("0123456789",*ptrSrc)) {    
			*sPtr++=*ptrSrc++;      // move the gotten char to name char array
		} else {
			ptrSrc++;               // if the char is not acceptable just move to another one
		}
	} 
	ptrSrc++; 

	/* Get a recipient port */
	while(*ptrSrc && *ptrSrc != ':' && portLength-->0)
	{
		if(strchr("0123456789",*ptrSrc)) {
			*portPtr++=*ptrSrc++;   // move the gotten char to name char array
		} else {
			ptrSrc++;               // if the char is not acceptable just move to another one
		} 
	} 
	ptrSrc++; 

	/* Get a recipient ip */
	while(*ptrSrc && *ptrSrc != ':' && ipLength-->0)
	{
		if(strchr("0123456789.",*ptrSrc)) {
			*ipPtr++=*ptrSrc++;     // move the gotten char to name char array
		} else {
			ptrSrc++;               // if the char is not acceptable just move to another one
		}
	}

	ptrSrc=0; nPtr=0; idPtr=0; sPtr=0; portPtr=0, ipPtr=0;	// clear pointers values

	// if we already have a user in our local array - do not readd it again
	for(int i=0; i<MAX_AVAILABLE_CLIENTS; i++)
	{
		if ( (ids[i] > 0) && (ids[i] == atoi(id)) ) {
			temp=ids[i];
			if ( strstr(params[temp].nickname, name) && strlen(params[temp].nickname) == strlen(name) ) {
				printf("DEBUG: we do not need to add a user, he already exists \n");
				return atoi(id);
			}
		}
	}

	strcpy(params[atoi(id)].nickname, name);      // set a client name to a structure
	params[atoi(id)].client_id = atoi(id);        // set a client id to a structure
	params[atoi(id)].socket = atoi(socket);       // set a client socket to a structure
	params[atoi(id)].port = atoi(port);           // set a client port to a structure
	strcpy(params[atoi(id)].ip, ip);              // set a client ip to a structure

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
/* We delete all the data about currently disconnected user from the array IDs, and from the array of structures */
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
