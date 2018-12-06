#include <time.h>

/* ----- PRE-PROCESSOR DIRECTIVE ----- */
#define LISTEN_IP "127.0.0.1"     	// where to listen for direct connections from other clients
#define SERVER_PORT 5000          	// a default port for connections with data server
#define SERVER_IP "127.0.0.1"     	// a default ip for connections with data server
#define RCVBUFSIZE 1000           	// deafult receive buffer size
#define NICK_SIZE 20              	// maximum nick size
#define PORT_BIND_MIN 5100        	// for listening
#define PORT_BIND_MAX 5200        	// for listening
#define MAX_AVAILABLE_CLIENTS 20  	// maximum clients for chatting
#define MAX_AVAILABLE_GROUP_MEMBERS 20	// maximum group members in a one group
#define MAX_AVAILABLE_GROUPS 10   	// maximum groups can be created
#define CURTIME time(&currentTime.rawtime);currentTime.info=localtime(&currentTime.rawtime);strftime(currentTime.buffer,80,"%I:%M:%S", currentTime.info)
/* ----------------------------------- */

/* ----- STRUCTURE DECLARATIONS ----- */
typedef struct groups
{
	char groupName[NICK_SIZE];			//name for the group
	int groupIDs[MAX_AVAILABLE_GROUP_MEMBERS];	//members in the group
} group;

typedef struct clients
{
	char nickname[NICK_SIZE];       // client name
	int client_id;                  // client id
	int socket;                     // client socket openned
	char ip[15];                    // ip address of a connected client
	int port;                       // port where client listens for direct connections from others
} client_parameters;

typedef struct chatHandler
{
	client_parameters parameters[1000];
	int socket;
	int ids[MAX_AVAILABLE_CLIENTS];
	group newGroups[MAX_AVAILABLE_GROUPS];			//new structure for group chatting
} receive_handler;

typedef struct uasRecepient       // needed for passing values into a thread
{
	int socket;
	struct sockaddr_in * uas;
} recepient;

typedef struct getCurrentTime     // for timing in the messages
{
   time_t rawtime;
   struct tm *info;
   char buffer[80];
} get_time;
/* ----------------------------------- */

/* ----- FUNCTION DECLARATIONS ----- */
int addPort(void);                // add port for connection to a server
char addAddress(char address[]);  // add address for connection to a server
int socketDescriptor(void);       // new socket descriptor
void sinStructure(struct sockaddr_in *structName,int port,char address[]);      // ip address/port structure for clients/server
void connection(int descriptor,struct sockaddr_in *structName,int length);      // try to connect to a server
void bindServer(int descriptor,struct sockaddr_in *structName,int size);        // binding for direct client messaging
void listenServer(int descriptor);                                              // start listening
int acceptConnection(int descriptor,struct sockaddr_in *structName,int *length);// accept connections from other clients
//
int randomPort(void);						// random port for binding
void introduction(void);					// start intro
void help(void);						// help intro
char * getDestName(char * string, char * name);		// get a destination name for a message
char * getMessageBody(char * string, char * message);		// get a destiantion text body for a message
void getOnliners(client_parameters * parameters, int * ids);	// get a list of online participants
int getMyID(client_parameters * parameters, int * ids,  char * name);	// get my own id
//
void identification(int * descriptor, int * port, char * nickname, receive_handler * passStructure);    // identify myself to a server
int getMembersData(char * echoBuffer, receive_handler * passStructure);                                 // get online users data
void chatting(int * descriptor, int * descriptorUAS, char * nickname, receive_handler * passStructure); // start chatting
//
int structureAdd(char * sourceString, client_parameters * params, int * ids );				// update array of IDs - update local array structure
int setCurrentList(int * passedID, client_parameters * infoClients, int * list);			// update array of IDs - adding
int delCurrentList(char * buffer, client_parameters * infoClients, int * list);			// update array of IDs - deletion
//
void * dataReceiving(void * arguments);                   	// threading for data receiving from a server
void * dataReceivingFromClients(void * arguments);        	// threading for data receiving from other clients
//
void groupsImplementer( char * echoString, char * newGroupName, char * group_ids, group * newGroups, int * descriptor, int * ids, client_parameters * parameters, int * myID  );
void groupAdd(group * newGroup, char * newName, char * ids, int * descriptor);				// create a new group
char * getGroupName(char * buffer);									// retrieve a group name from the field
void groupShow(group * newGroup);													// show a list of groups created
void groupDel(group * group, char * name, int * descriptor);						// delete a group
void groupJoin(group * group, char * name, int * myID, int * descriptor);				// join a group
void groupQuit(group * group, char * name, int * myID, int * descriptor);				// quit a group
void groupShowMembers(group * group, char * name, client_parameters * parameters, int * ids);									// show group members
void groupMessage(group * group, char * name, client_parameters * parameters, int * ids, char * message, int * myID);
void receiveGroupUpdates( char * echoBuffer, group * newGroups, client_parameters * parameters);
void receiveGroupUniqueUpdates( char * echoBuffer, group * newGroups, client_parameters * parameters );
/* ----------------------------------- */
