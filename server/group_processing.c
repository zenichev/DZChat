/* DZChat - Server. Dec 2018 @Donat Zenichev */
/* This is all about groups: creations, joins, deletions etc. */

#include <stdlib.h>		/* malloc() , calloc(), free() , atof(), atoi(), atol() */
#include <stdio.h>		/* for printf() and fprintf() */
#include <string.h>		/* different manipulations over a char arrays, such as memset() */
#include <sys/socket.h>	/* for socket(), bind(), connect(), recv() and send() */
#include <sys/types.h>		/* data types */
#include "main.h"		/* include my custom headers and definitions */
#include <unistd.h>		/* standard symbolic constants and types such as close(), read() */
#include <errno.h>		/* reporting and retrieving error conditions */

// main implementer function for groups
void groupsImplementer(char * buffer, client_parameters * parameters, int * ids, group * groups, int * socket, unsigned int * counter)
{
    // possible commands:
    // ADD - adding new group
    // DEL - deletion of a group
    // JOIN - somebody joined
    // QUIT - somebody quit
    // GET - get updates to user

    if (strstr(buffer, "SYSTEM:GROUP_UPDATE:ADD:")) groupAdd( buffer, parameters, ids, groups, counter );
    if (strstr(buffer, "SYSTEM:GROUP_UPDATE:DEL:")) groupDel( buffer, parameters, ids, groups, counter );
    if (strstr(buffer, "SYSTEM:GROUP_UPDATE:JOIN:")) groupJoin( buffer, parameters, ids, groups, counter );
    if (strstr(buffer, "SYSTEM:GROUP_UPDATE:QUIT:")) groupQuit( buffer, parameters, ids, groups, counter );
    if (strstr(buffer, "SYSTEM:GROUP_UPDATE:GET:")) groupSendUpdates( buffer, parameters, ids, groups, socket, counter );
}

void groupSendUpdates(char * buffer, client_parameters * parameters, int * ids, group * groups, int * socket, unsigned int * counter) {

    get_time currentTime;

    char notification[500] = {0};
    int groups_found = 0, group_members_found = 0;

	// send an information about current groups existing
	strcpy(notification, "SYSTEM:GROUP_UPDATE:");
	for (int i=0; i<MAX_AVAILABLE_GROUPS; i++)
	{
		if ( !strstr(groups[i].groupName, "NULL") ) {
			groups_found = 1;
			strcat(notification, groups[i].groupName);
			char tempid[5] = {0};
			for (int j=0; j<MAX_AVAILABLE_GROUP_MEMBERS; j++)
			{
				if (groups[i].groupIDs[j] != 0) {
					group_members_found = 1;
					snprintf(tempid,sizeof(tempid),":%d", groups[i].groupIDs[j]);
					strcat(notification, tempid);
					memset(tempid, 0, sizeof(tempid));
				}
			}
			if ( group_members_found == 0 ) strcat(notification, ":0;");	// in case no members were found
			if ( group_members_found == 1 ) strcat(notification, ";");	// in case members were found
		}
	}
    // send a notice how many users are online to a client
	if (groups_found != 0) {
		if (send(*socket, notification, strlen(notification), 0) != strlen(notification)) {
				CURTIME;
				printf("%s ERROR <%d>: Error to send() a notification to client\n", currentTime.buffer, *counter);
		} else {
				CURTIME;
				printf("%s MESSAGE OUT <%d>: SYSTEM:GROUP_UPDATE sent\n", currentTime.buffer, *counter);
		}
	}
	//
	if (groups_found == 0) {
		strcat(notification,"GROUPS_FOUND=0;");
		if (send(*socket, notification, strlen(notification), 0) != strlen(notification)) {
				CURTIME;
				printf("%s ERROR <%d>: Error to send() a notification to client\n", currentTime.buffer, *counter );
		} else {
				CURTIME;
				printf("%s MESSAGE OUT <%d>: SYSTEM:GROUP_UPDATE sent (no groups) \n", currentTime.buffer, *counter);
		}
	}
    printf("%s INFO <%d>: Group updates given \n", currentTime.buffer, *counter);
    //clear variables
    groups_found = 0, group_members_found = 0;
    memset(notification,0,sizeof(notification));
}

void groupAdd(char * buffer, client_parameters * parameters, int * ids, group * groups, unsigned int * counter) {

    get_time currentTime;

    int nameLength = NAME_SIZE;

    char * ptrSrc; ptrSrc = buffer;
    ptrSrc += 24; //"SYSTEM:GROUP_UPDATE:ADD:"

    char name[NAME_SIZE] = {0};
    char * nPtr; nPtr = name;

    char action[25] = "SYSTEM:GROUP_UPDATE:ADD:";
    char id[2] = "0";

	/* Get a group name*/
	while(*ptrSrc && *ptrSrc != ':' && *ptrSrc != ';' && nameLength-->0)
	{
		if(strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-0123456789_",*ptrSrc)) { 
			*nPtr++=*ptrSrc++;      // move the gotten char to name char array
		} else {
			ptrSrc++;               // if the char is not acceptable just move to another one
		}
	}

    // find a new place in array for a new group
    int c = 0;
    if ( !strstr(groups[c].groupName, "NULL") ) {          // be sure that this array id is not allocated yet for another group
		while( !strstr(groups[c].groupName, "NULL") ) c++;
	}

    // check if we do not reach the max available group amount
    if (c > MAX_AVAILABLE_GROUPS) {
        CURTIME;
        printf("%s INFO <%d>: Not enough space for group allocation! Increase the MAX_AVAILABLE_GROUPS definition\n", currentTime.buffer, *counter);
        goto ALIAS_CLEAR_VARIABLES;
    }

    // check if we do not have the group with the same name
    for (int i=0; i<MAX_AVAILABLE_GROUPS; i++)
    {
        if ( strstr( groups[i].groupName, name ) && strlen(groups[i].groupName) == strlen(name) ) {
            CURTIME;
            printf("%s WARNING <%d>: Group <%s> already exists!\n", currentTime.buffer, *counter, groups[i].groupName);
            goto ALIAS_CLEAR_VARIABLES;
        }
    }

    memset(groups[c].groupName,0,sizeof(groups[c].groupName));
    strcpy(groups[c].groupName, name);                 // add a new name
    // initialize an array with the client ids
    for(int i=0; i<MAX_AVAILABLE_GROUP_MEMBERS; i++) { groups[c].groupIDs[i]=0; }
    CURTIME;
    printf("%s INFO <%d>: Group added <%s> \n", currentTime.buffer, *counter, name);

    // send updates to others
    groupSendUniqueUpdate( action, name, id, parameters, ids, counter);

    //clear variables
    ALIAS_CLEAR_VARIABLES: ptrSrc=0; nPtr=0; nameLength=0;
    memset(action,0,sizeof(action));
    memset(name,0,sizeof(name));
    memset(id,0,sizeof(id));
}

void groupDel(char * buffer, client_parameters * parameters, int * ids, group * groups, unsigned int * counter) {

    get_time currentTime;

    int nameLength = NAME_SIZE;
    int groupFound = 0;

    char * ptrSrc; ptrSrc = buffer;
    ptrSrc += 24; //"SYSTEM:GROUP_UPDATE:DEL:"

    char name[NAME_SIZE] = {0};
    char * nPtr; nPtr = name;

    char action[25] = "SYSTEM:GROUP_UPDATE:DEL:";
    char id[2] = "0";

	/* Get a group name*/
	while(*ptrSrc && *ptrSrc != ':' && *ptrSrc != ';' && nameLength-->0)
	{
		if(strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-0123456789_",*ptrSrc)) { 
			*nPtr++=*ptrSrc++;      // move the gotten char to name char array
		} else {
			ptrSrc++;               // if the char is not acceptable just move to another one
		}
	}

    for (int i=0; i<MAX_AVAILABLE_GROUPS; i++)
    {
        if ( strstr( groups[i].groupName, name ) && strlen(groups[i].groupName) == strlen(name) ) {
            groupFound = 1;
            memset(groups[i].groupName,0,sizeof(groups[i].groupName));
            strcpy( groups[i].groupName, "NULL" );
            for (int j=0; j<MAX_AVAILABLE_GROUP_MEMBERS; j++) groups[i].groupIDs[j] = 0;
        }
    }
    CURTIME;
    if ( groupFound == 0 ) printf("%s INFO <%d>: No group <%s> found!\n", currentTime.buffer, *counter, name);
    if ( groupFound == 1 ) printf("%s INFO <%d>: Group deleted <%s> \n", currentTime.buffer, *counter, name);
    
    // send updates to others
    groupSendUniqueUpdate( action, name, id, parameters, ids, counter);

    //clear variables
    ptrSrc=0; nPtr=0; nameLength=0;
    memset(action,0,sizeof(action));
    memset(name,0,sizeof(name));
    memset(id,0,sizeof(id));
}

void groupJoin(char * buffer, client_parameters * parameters, int * ids, group * groups, unsigned int * counter) {

    get_time currentTime;

    int nameLength = NAME_SIZE;
    int groupFound = 0;

    char * ptrSrc; ptrSrc = buffer;
    ptrSrc += 25; //"SYSTEM:GROUP_UPDATE:JOIN:"

    char name[NAME_SIZE] = {0};
    char * nPtr; nPtr = name;

    char id[5] = {0};
    char * idPtr; idPtr = id;

    char action[26] = "SYSTEM:GROUP_UPDATE:JOIN:";

	/* Get a group name*/
	while(*ptrSrc && *ptrSrc != ':' && *ptrSrc != ';' && nameLength-->0)
	{
		if(strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-0123456789_",*ptrSrc)) { 
			*nPtr++=*ptrSrc++;      // move the gotten char to name char array
		} else {
			ptrSrc++;               // if the char is not acceptable just move to another one
		}
	}
    if (*ptrSrc == ':') ptrSrc++; nameLength = NAME_SIZE;
	/* Get a group member id*/
	while(*ptrSrc && *ptrSrc != ':' && *ptrSrc != ';' && nameLength-->0)
	{
		if(strchr("0123456789",*ptrSrc)) { 
			*idPtr++=*ptrSrc++;      // move the gotten char to id char array
		} else {
			ptrSrc++;               // if the char is not acceptable just move to another one
		}
	}

    for (int i=0; i<MAX_AVAILABLE_GROUPS; i++)
    {
        if ( strstr( groups[i].groupName, name ) && strlen(groups[i].groupName) == strlen(name) ) {
            groupFound = 1;
            int j=0;
            if ( groups[i].groupIDs[j] != 0 ) {
                while ( groups[i].groupIDs[j] != 0 ) j++;
            }
            groups[i].groupIDs[j] = atoi(id);
            break;
        }
    }
    CURTIME;
    if ( groupFound == 0 ) printf("%s INFO <%d>: No group <%s> found!\n", currentTime.buffer, *counter, name);
    if ( groupFound == 1 ) printf("%s INFO <%d>: <%s> joined the group <%s>\n", currentTime.buffer, *counter, id, name);

    // send updates to others
    groupSendUniqueUpdate( action, name, id, parameters, ids, counter);

    //clear variables
    ptrSrc=0; nPtr=0; idPtr=0; nameLength=0;
    memset(action, 0, sizeof(action));
    memset(name, 0, sizeof(name));
    memset(id, 0, sizeof(id));
}

void groupQuit(char * buffer, client_parameters * parameters, int * ids, group * groups, unsigned int * counter) {

    get_time currentTime;

    int nameLength = NAME_SIZE;
    int deleted = 0, groupFound = 0;

    char * ptrSrc; ptrSrc = buffer;
    ptrSrc += 25; //"SYSTEM:GROUP_UPDATE:QUIT:"

    char name[NAME_SIZE] = {0};
    char * nPtr; nPtr = name;

    char id[5] = {0};
    char * idPtr; idPtr = id;

    char action[26] = "SYSTEM:GROUP_UPDATE:QUIT:";

	/* Get a group name*/
	while(*ptrSrc && *ptrSrc != ':' && *ptrSrc != ';' && nameLength-->0)
	{
		if(strchr("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ-0123456789_",*ptrSrc)) { 
			*nPtr++=*ptrSrc++;      // move the gotten char to name char array
		} else {
			ptrSrc++;               // if the char is not acceptable just move to another one
		}
	}
    if (*ptrSrc == ':') ptrSrc++; nameLength = NAME_SIZE;
	/* Get a group member id*/
	while(*ptrSrc && *ptrSrc != ':' && *ptrSrc != ';' && nameLength-->0)
	{
		if(strchr("0123456789",*ptrSrc)) { 
			*idPtr++=*ptrSrc++;      // move the gotten char to id char array
		} else {
			ptrSrc++;               // if the char is not acceptable just move to another one
		}
	}

    for (int i=0; i<MAX_AVAILABLE_GROUPS; i++)
    {
        if ( strstr( groups[i].groupName, name ) && strlen(groups[i].groupName) == strlen(name) ) {
            groupFound = 1;
            for (int j=0; j<MAX_AVAILABLE_GROUP_MEMBERS; j++) {
                if ( groups[i].groupIDs[j] == atoi(id) ) {
                    groups[i].groupIDs[j] = 0;
                    deleted = 1;
                }
            }
        }
    }
    CURTIME;
    if ( groupFound == 0 ) printf("%s INFO <%d>: No group <%s> found!\n", currentTime.buffer, *counter, name);
    if ( deleted == 0 ) printf("%s INFO <%d>: No user <%d> found in the group <%s> !\n", currentTime.buffer, *counter, atoi(id), name);
    if ( groupFound == 1 ) printf("%s INFO <%d>: <%s> quit the group <%s>\n", currentTime.buffer, *counter, id, name);

    // send updates to others
    groupSendUniqueUpdate( action, name, id, parameters, ids, counter);

    // clear variables
    ptrSrc=0; nPtr=0; idPtr=0; nameLength=0;
    memset(action, 0, sizeof(action));
    memset(name, 0, sizeof(name));
    memset(id, 0, sizeof(id));
}

// delete user from all groups
void groupQuitAll(group * groups, int * myID, unsigned int * counter) {

    get_time currentTime;

    // start searching
    for (int i=0; i<MAX_AVAILABLE_GROUPS; i++) {
        if ( strstr(groups[i].groupName, "NULL") ) continue; // if place is NULL go further
        // try to find a quit user in an existing group
        for (int j=0; j<MAX_AVAILABLE_GROUP_MEMBERS; j++) {
            if ( groups[i].groupIDs[j] ==  *myID) {
                CURTIME;
                printf("%s INFO <%d>: %d quit the group <%s>\n", currentTime.buffer, *counter, groups[i].groupIDs[j], groups[i].groupName);
                groups[i].groupIDs[j] = 0;
            }
        }
    }
}

// actions gotten from users
void groupSendUniqueUpdate(char * action, char * groupName, char * memberID, client_parameters * parameters, int * ids, int * counter) {

    get_time currentTime;

    int temp = 0;
    char command[50] = {0};// notification[100] = "_<";
    strcpy(command, action);

    if (strstr(action, "SYSTEM:GROUP_UPDATE:ADD:")) goto ADD;
    if (strstr(action, "SYSTEM:GROUP_UPDATE:DEL:")) goto DEL;
    if (strstr(action, "SYSTEM:GROUP_UPDATE:JOIN:")) goto JOIN;
    if (strstr(action, "SYSTEM:GROUP_UPDATE:QUIT:")) goto QUIT;
    printf("%s WARNING <%d>: no possible actions for groupSendUniqueUpdate() - <%s>\n", currentTime.buffer, *counter, action);
    goto ALIAS_CLEAR_VARIABLES;//if no acceptable action is gotten

    ADD: strcat(command, groupName); strcat(command, ";");
    goto SEND;

    DEL: strcat(command, groupName); strcat(command, ";");
    goto SEND;

    JOIN: strcat(command, groupName); strcat(command, ":");
    strcat(command, memberID); strcat(command, ";");
    goto SEND;

    QUIT: strcat(command, groupName); strcat(command, ":");
    strcat(command, memberID); strcat(command, ";");
    goto SEND;

    SEND: for (int i=0; i<MAX_AVAILABLE_CLIENTS; i++) {
        if ( ids[i] != 0 ) {
            temp=ids[i];
            if(send(parameters[temp].socket, command, strlen(command), 0) != strlen(command)) {
                CURTIME;
                printf("%s ERROR <%d>: Error to send() a notification to client - %s\n", currentTime.buffer, *counter, strerror(errno));
            }
        }
    }

    ALIAS_CLEAR_VARIABLES: memset(command,0,sizeof(command)); temp = 0;
}
