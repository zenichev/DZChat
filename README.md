# DZChat
Sep 2018 @Donat Zenichev

This is a C based chat for a linux terminal.

Indeed it's a shmall project, that born from my attempts to improve C skills.
But after a while I decided that this could be a chat with a versatile functionality.

>------------------------------------------------------------------------------------------------------------------->
## A short description
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
>------------------------------------------------------------------------------------------------------------------->

>------------------------------------------------------------------------------------------------------------------->
# To do:
- <Add a new command> '>online;'  - this command will allow users to see the list of online participants anytime.
- <Add a new transmission functionality> This will allow a newly connected user to get all the data about other users from scratch and not to ask the server for it in the future.
- <Feature> Data encryption mechanism.
- <Feature> Possiblity to facilitate NAT traversal betweeb users. And between a user and a server.
>------------------------------------------------------------------------------------------------------------------->
  
>------------------------------------------------------------------------------------------------------------------->
# Other:
- New ideas are highly appriciated, and of course the contribution to the project too.
>------------------------------------------------------------------------------------------------------------------->
