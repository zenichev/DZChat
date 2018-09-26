# DZChat
Sep 2018 @Donat Zenichev

This is a C based chat for a linux terminal.

Indeed it's a shmall project, that born from my attempts to improve C skills.
But after a while I decided that this could be a chat with a versatile functionality.

The idea is to built it completely using C.
First it worked using forking, then I decided to move it completely to threading.

### How to
You compile separately a server and a client.
Server is used as a data server to keep all around updated. Client used an entering point to the chat.
When you compile server/client do not forget to include an -lpthread option (in case you use gcc compiler).

### A short description
There is no demand to be located in the same broadcast domain with other participants.
Data transmission works over the TCP (I'm not looking into the UDP, since I prefer to let the TCP perform it's work, and not to build a custom functionality to support the data transmission).

So for now the topology is as following:
- Server for data storing;
- Client for chatting;

Server does not stay in the path, when clients chat with each other.
The only thing that server is responsible for, is to keep clients updated.

So for e.g., one client (client A) gets online, and then one more client (client B) gets online.
- The A client is notficated that the B client is now online. And also the A client got the connection data from the server (to use for connections towards the B client).
- Upon getting online, the B user also obtains (from the server) a list of online participants;
- After that both users can be connected directly without sending any information to the Server.
- After one of the users is disconnected, all online participants are notificated. When online participants get this message (about disconnection) they delete an appropriate data from their local arrays of structures. The same does the server when he already did notificate all members (so from this step no infromation about disconnected user is kept within the chat).

- If user doesn't have (for some reason) an infromation, that can be used to connect to a desired user, client sends an update request to the server, and server searches required data in his own array of structures. In a positive case, it answers with the needed infromation. In a negative case, it answers with zero '0' instead of a data, and that's how the client then understands that the user is offline (so he is able to notify user about it).

Server

![Image of Server](http://i.piccy.info/i9/8ebbfdea83f3d95962b0ccd9039467a6/1537955839/90782/1270172/Screenshot_from_2018_09_26_12_40_41.png)

Client

![Image of Client](http://i.piccy.info/i9/4ccc785255c19a01f49a8afec4642c20/1537955960/81912/1270172/Screenshot_from_2018_09_26_12_40_31.png)

### To do:
- |Refactoring| - rebuild the code to initalaize variables only with malloc() calloc() and release them with free()
- |Feature| Group chatting. This will allow us to create groups and use them to "broadcast" messages to the members.
- |Feature| A data encryption mechanism.
- |Feature| A possiblity to facilitate NAT traversal betweeb users. And between a user and a server.
- |Feature| Add a possility to use 'UP' arrow key to return to previos a message.

### Features done:
- |Add a new command| '>online;'  - this command will allow users to see the list of online participants anytime.
- |Add a new command| '>clear;'  - this command clears the terminal screen
- |Add a new command| '>quit;'  - this command allows user to quit the chat
- |Add a new transmission functionality| This allows a newly connected user to get all the data about other users from scratch and not to ask the server about each user(data) it in the future.
- |Add a new transmission functionality| This allows user to get an infromation about user(data) from the server, in case we don't have required infromation in our local array
- |Feature| - notify user when somebody disconnects
- |Feature| - notify user when somebody connects
- |Feature| - show to a newly connected user an online user list
- |Feature| - process the wrong entering syntax (that could obtained from a user).
  
### Other:
- New ideas are highly appriciated, and of course the contribution to the project too.
