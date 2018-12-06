/* DZChat - Server. Dec 2018 @Donat Zenichev */
/* This functionality is responsible for notifying other members about newly connected user */
/* Also a user that just connected get a list of online participants */

#include <stdio.h>		/* for printf() and fprintf() */
#include <string.h>		/* different manipulations over a char arrays, such as memset() */
#include "main.h"		/* include my custom headers and definitions */
#include <errno.h>		/* reporting and retrieving error conditions */

/* Notify connected user how many online user do we have */
/*----------------------------------------------------------------------------------------------------------------------------*/
void notificationOnline(int * list, client_parameters * params, int * socket, int online, unsigned int * counter, group * groups) {

	get_time currentTime;

	int id=0, temp=0, len=0, group_id=0, groups_found=0, group_members_found=0;
	// buffer for announces to a currently connected client
	char notification[1000], append[100], header[29]="SYSTEM:SERVER_UPDATE:ONLINE=", header_groups[21]="SYSTEM:GROUP_UPDATE:";
	char connectedUser[NAME_SIZE], connectedUserIP[16], connectedUserListen[6], connectedUserID[5];

	// clear char arrays before using
	memset(notification,0,sizeof(notification));
	memset(append,0,sizeof(append));
	memset(connectedUser,0,sizeof(connectedUser));
	memset(connectedUserIP,0,sizeof(connectedUserIP));
	memset(connectedUserListen,0,sizeof(connectedUserListen));

	CURTIME;
	printf("%s INFO <%d>: The current list of clients online (currently: %d) :\n", currentTime.buffer, *counter, online);

	// update user with current clients online
	snprintf(notification,sizeof(notification),"SYSTEM:SERVER_UPDATE:ONLINE=%d;",online);
	for(int i=0;i<MAX_AVAILABLE;i++)
	{
		id = list[i];
		if(id > 0) {	// check if a list value is set
			snprintf(append,sizeof(append),"%s:%d:0:%d:%s;",params[id].nickname, params[id].client_id, params[id].port, params[id].ip);
			strcat(notification,append);		// append the gotten value to the notification char array
			memset(append,0,sizeof(append));	// clear the append array
			id=0;	// clear id
		}
	}
	// send a notice how many users are online to a client
	if (notification != 0) {
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
					CURTIME;
					printf("%s ERROR <%d>: Error to send() a notification to client - %s\n", currentTime.buffer, *counter, strerror(errno));
				} else {
					CURTIME;
					printf("%s MESSAGE OUT <%d>: notification about a new connection is sent to <%s> socket:%d\n", currentTime.buffer, *counter, params[temp].ip, params[temp].socket);
				}
			}
		}

	}
	memset(notification,0,sizeof(notification));	// clear the buffer before exit
	memset(append,0,sizeof(append));
}
/*----------------------------------------------------------------------------------------------------------------------------*/
