# TCP-IP-Implementation
Concurrent TCP IP implemention  for handling multiple client request.


INSTRUCTION TO CHECK THE IMPLEMENTATION OF TCP PROTOCOL :
Associated files : server_tcp.c and client_tcp.c

Role of Server Process in TCP protocol program - server_tcp.c
*********************************************************************************
1. Creates the socket.
2. Binds the socket
3. listen to the client request.
4. Concurrent client is implemented to accept the client request.
5. Recieves the linux command sent from client.
6. Executes linux commands
7. Executed command is sent back to the client.

Steps For Execution :
---------------------------------------------------------------------------------
1. gcc server_tcp.c -o ser
2. ./ser 16184

//Following will be displayed :
Socket creation and binding information is displayed.
Associated port with respect to client connection is also displayed.

Role of Client Process in TCP protocal program - client_tcp.c
*********************************************************************************
1. Creates socket.
2. Binds socket
3. Connect request is raised to establish server conncetion.
4. Command entered is read and is sent to server.
5. Recieves the executed output from server and displays it.

Steps For Execution :
---------------------------------------------------------------------------------
1. gcc client_tcp.c -o cli
2. ./cli server_ip_addr 16184 

//Following will be displayed :
Socket creation and binding information is displayed.
Prompt is display to enter the linux command
Executed result is displayed
