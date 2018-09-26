/* DZChat - Server headers. Sep 2018 @Donat Zenichev */

#include <netinet/in.h>	/* Internet Protocol family */
#include <time.h>

/* ----- PRE-PROCESSOR DIRECTIVE ----- */
#define SERVER_PORT 5000		// default port for listening
#define SERVER_IP "127.0.0.1"		// ip to listen for
#define MAX_AVAILABLE 10		// maximum available clients
#define RCVBUFSIZE 50000		// receive buffer for each message
#define ID_STRING "z9HG4bk:"		// string used to identify identification process
#define NAME_SIZE 20			// acceptable name size
#define CURTIME time(&currentTime.rawtime);currentTime.info=localtime(&currentTime.rawtime);strftime(currentTime.buffer,80,"%I:%M:%S%p", currentTime.info)
/* ----------------------------------- */

/* ----- STRUCTURE DECLARATIONS ----- */

// store clients infomration is an array of structures
typedef struct clients
{
	char nickname[NAME_SIZE];		// client name
	int client_id;				// client id
	int socket;				// client socket openned
	char ip[15];				// ip address of a connected client
	int port;				// port where client listens for direct connections from others
	unsigned int counter;			// id for each client thread
} client_parameters;

// for passing client variables into a handleTCPClient function
typedef struct handleClient
{
	char ip[12];
	int socket;
	unsigned int counter;
} handle_client;

typedef struct getCurrentTime			// for timing in the messages
{
   time_t rawtime;
   struct tm *info;
   char buffer[80];
} get_time;
/* ----------------------------------- */


/* ----- FUNCTION DECLARATIONS ----- */

/* Server related */
int addPort(void);									// ask user of the server for port
char addAddress(char address[]);							// add ip address
int socketDescriptor(void);								// open socker descriptor
void bindServer(int descriptor,struct sockaddr_in *structName,int size);		// bind socket to a port
void listenServer(int descriptor);							// start listening
void sinStructure(struct sockaddr_in *structName,int port,char address[]);		// ip address/port structure for clients/server

/* Client processing related */
int acceptConnection(int descriptor,struct sockaddr_in *structName,int *length);	// wait for connection from a client
void * handleTCPClient(void * arguments);						// handle client in a new thread
char userIdentification(char * buffer,int size,char * name,char * port);		// get a client's name and a listening port (clients have separate port for listening)
void notificationOnline(int * list, struct clients * shm, int * socket, int online,  unsigned int * counter);			// notify a new client which online do we have now
//
void chatting(char * buffer, int * socket, client_parameters * list, int * idList, unsigned int * counter);			// get informational requests from clients and answer on them
void giveUpdates(char * sourceString, int * socket, client_parameters * parameters, int * idList, unsigned int * counter); 	// send user updates
//
int onlineCounter(int action);													// clients counter
int randomID(void);														// generate ID for clients
//
int * setCurrentList(int *passedID, int action, struct clients * infoClients);							// keep a list of client IDs
client_parameters * clientsStructure(char * name, int * myID, int * socket, char * portListening, char * ip, int usersOnline, unsigned int * counter);	// keep clients data: name, ids, socket etc. in an array of structures
/* ----------------------------------- */
