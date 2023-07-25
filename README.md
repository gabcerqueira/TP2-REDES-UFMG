# TP2-REDES-UFMG

Practical task 2 aimed to implement a system that coordinates multiple simultaneous connections between clients and allows communication among them.

Challenges
During the implementation of the task, several challenges arose, including:

Understanding the socket structure in C, which required knowledge of data structures, functions, and system calls related to sockets. This included socket creation, address binding, connection listening, establishment and termination of connections, data sending and receiving, among others.
Establishing and disconnecting connections correctly, involving the proper configuration of socket parameters such as protocol family (IPv4 or IPv6), socket, and port. Handling possible errors and exceptions during the connection process was necessary.
Implementing threads to allow multiple connections on the server-side and achieve parallelism between keyboard commands and client message reception. Additionally, sharing information between the created threads posed a challenge.
Solution
Server
The server part utilized threads to establish multiple connections, limited by the "threadCount" counter and the definition "MAX_CONNECTIONS" set to 15. The "connectionHandler" function runs in a thread to handle a client connection. It receives an argument of type void*, converted into a pointer of type ThreadArgs, containing relevant information about the connection and user states, such as the client socket and a list of users defined by the "User" structure. This function also handles the reception and processing of client messages through the "listenToClient" function.
The "listenToClient" function manages different client commands, such as "close connection," "list users," "send to," and "send all," performing appropriate treatments for each one. Messages are formatted before being sent in broadcast or unicast using the "formatMessage" function.
Several auxiliary functions and structures were utilized on the server-side to manage connected users, send and receive messages, and perform other operations related to client-server communication.

Client
The client could send messages to all connected users through the chat, send private messages to specific users, list connected users, and terminate the connection with the server. Auxiliary functions handled the manipulation of integer arrays, sorting, and searching. Other functions were responsible for parsing and formatting messages to ensure proper communication with the server and legible message reception by users.
The "runClient()" function managed the client's execution and communication with the server, receiving a socket descriptor previously established for server communication. It used a continuous loop to process user commands until the "disconnect" variable was set to 1, indicating the client's desire to terminate the connection with the server. The function switched between appropriate actions based on the received command, such as sending messages, listing users, or closing the connection.
Various auxiliary functions and structures were utilized on the client-side to handle server responses, parse messages, and perform necessary actions accordingly. Additionally, threads were used to receive and process messages asynchronously while keeping the client running.

The task successfully achieved its objective of implementing a system to coordinate multiple connections and facilitate communication among clients.
