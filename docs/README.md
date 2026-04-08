# TICK
Terminal Interface Command Kiosk

### Team Members
This project was created by MREN 178 Team 1:

* James Colbourne (20523893)
* Beric Dengler (20515669)
* Nicolas Kaye (20506269)
* Lukas Nuzzi (20524153)

### System Overview
#### Server
The server handles all the sending, receiving, and storing of messages, user authentication, and channels. This must be running alongside the client. When a message is sent, the server receives it and puts it in a channel deque where it will be passed on when a client requests an update. Each channel has its own deque, which is how the server stores messages for each channel. The server also houses an array with all the user structs in it, and a BST which allows for rapid $O(logN)$ indexing of the array. This system is shown in Figure 1. In this approach, the server holds all of the important information, and chooses what the client can and cannot see. The server has no long term means of storing messages as they are stored in volatile memory, and messages are lost on shutdown. However, messages are stored as long as the server does not shutdown, and it was designed to be as robust as possible so that it would not crash.

#### Client

The client has two parts. The interface and the backend logic. The interface is what the user sees, and features a text input box, and a terminal to see messages in a channel. When the return key is pressed, the backend logic parses the written message into a string. The elements of this string are then passed via socket to the server, which reconstitutes the string for storage. When the user wishes to call new messages, the tilde key is pressed which sends a request for the newest messages from the server. The server passes these back along with the user who sent them. These messages are then displayed in the appropriate channel in the interface.

### List of Data Structures & Algorithms

All of these DSA functions are available in the DSAFunctions.c file. Other implementations of things like the server and interface are in their respective files.

A list of the major data structures & algorithms:

* Deque
* Binary Search Tree
* Lookup Table

### System Behavior

The inputs and outputs are similar to that of a normal text application (such as an IRC or Discord).

To start, the server should be initialized before a client is opened. Once it is initialized, it will print that it is available on port 3000, along with other debugging messages.

With the client, you should be able to type a Server IP, username, and then proceed to the client, where you are able to type messages and receive messages. An invalid Server IP will not be accepted, as the app will briefly freeze the main thread while it tries to call the socket, and then allow the user to type a valid one as a failsafe. All of these inputs will be communicated with the server over the custom TCP socket protocol created for this project, sending a 4 byte long identifier that tells the server which command to execute (send a message, create a user, etc.)

On the client, channels can be changed with `Tab` which recalls the server to update latest messages for the new channel ID. This behaviour is similar to pressing `~`, which refreshes the messages.

For a more detailed breakdown of all components, please see the report or `INSTALL.md` which has a more clear "how to use" section.