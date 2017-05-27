//server tcp process
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/wait.h>
#include <ctype.h>
#include <netinet/in.h>

#define RETURN_FAIL    0

pid_t proc_pid;
static char* arg1[400];
static int no_cmd = 0;
static char buffer1[1000];

void pipe_method(char* buf);

int main(int argc, char* argv[])
{
	int simpleSocket, newsockfd, serLen, cliLen, port;
	char buffer[256];    				   /* Buffer creation */
	struct sockaddr_in serverAddress;               /* Server address */
	struct sockaddr* serverAddrPtr;
	struct sockaddr_in clientAddress;               /* Client address */
	struct sockaddr* clientAddrPtr;

	/* to prevent zombie processes */
	signal (SIGCHLD, SIG_IGN);

	if (argc != 2)
   	{
     		fprintf(stderr, "Usage: %s <port>\n", argv[0]);
     		exit(1);
   	}

	/* retrieve the port number for listening */
	port = atoi(argv[1]);
	/* Creating Internet socket */
	simpleSocket = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);
        if (simpleSocket == -1)
   	{
		fprintf(stderr, "Could not create a socket!\n");
     		exit(1);
   	}
   	else
   	{
     		fprintf(stderr, "Socket created!\n");
   	}

	/* set up the address structure */
	serLen = sizeof(serverAddress);
	bzero((char*)&serverAddress, serLen);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddress.sin_port = htons(port);
	serverAddrPtr = (struct sockaddr*) &serverAddress;

	/* bind to the address and port with our socket */
	if(bind(simpleSocket, serverAddrPtr, serLen) < 0)
	{
		fprintf(stderr, "Could not bind to address!\n");
		close(simpleSocket);
		exit(1);
	}
	else{
		fprintf(stderr, "Bind completed!\n");
	}

	/* let's listen on the socket for connections */
	if(listen(simpleSocket, 5) < 0)
	{
		fprintf(stderr, "Cannot listen on socket!\n");
		close(simpleSocket);
		exit(1);
	}

	clientAddrPtr = (struct sockaddr*) &clientAddress;
	cliLen = sizeof(clientAddress);

	printf("***** Server is associated with the port %d *****\n***** Press 'Ctrl+C' to quit from Server Process *****\n", port);

	while(1) {
		/* Implementing concurrent client */
		newsockfd = accept(simpleSocket,clientAddrPtr,&cliLen);
		if(newsockfd < 0)
		{
			fprintf(stderr, "Cannot accept connections!\n");
			close(simpleSocket);
			exit(1);
		}

		printf("Server %d: connection %d accepted\n", getpid(), newsockfd);

		if(fork() == 0) {   /* creating child to handle each client request */

			int a = dup2(newsockfd, STDOUT_FILENO);
			int b = dup2(newsockfd, STDERR_FILENO);
			if(a < 0 || b < 0)
			{
				fprintf(stderr, "Cannot DUP \n");
				exit(1);
			}

			while(1) {
				bzero(buffer, sizeof(buffer));

				/* Receiving  */
				if(recv(newsockfd, buffer, sizeof(buffer), 0) < 0)
				{
					fprintf(stderr, "Cannot receive the client request\n");
					exit(1);
				}

				if (strcmp(buffer, "exit\n") == 0) {
					close(newsockfd);
					exit(0);
				}

				pipe_method(buffer);    /* Transferring command to pipe_method for execution of pipe command */
			}
		}
		else
			close (newsockfd); /* Closing newsocket */
	}

	close(simpleSocket);

	return 0;
}

// Execution of command with previous pipe program :
static int execute_pipe(int val1, int val2, int val3)
{
	int fd[2];

	/* Invoking pipe and error checking */
	if(pipe(fd) < 0 ){
		perror(" ERROR *** PIPE INVOKING ERROR \n");
		exit(1);
	}

	/* Invoking child and error checking */
	if((proc_pid = fork()) < 0){
		perror(" ERROR *** CHILD INVOKING ERROR \n");
		exit(1);
	}

	/* child process executing commands */
	if (proc_pid == 0) {
		/* First command redirection */
		if (val2 == 1 && val3 == 0 && val1 == 0) {
			dup2( fd[1], 1);
		}
		/* Second command redirection */
		else if (val2 == 0 && val3 == 0 && val1 != 0) {
			dup2(val1, 0);
			dup2(fd[1], 1);

		}
		/* Third command redirection */
		 else {
			dup2( val1, 0);

		}

		/* Command execution */
		if (execvp( arg1[0], arg1) == -1)
			exit(1);
	}

	// Closing val1 that is lowest file descriptor
	if (val1 != 0)
		close(val1);

	// Closing write file descriptor.
	close(fd[1]);

	// Closing read file descriptor
	if (val3 == 1)
		close(fd[0]);

		/* Parent waiting for child commands to execute in sequence*/
		int z;
		for (z = 0; z < no_cmd; ++z)
			wait(NULL);

	return fd[0];
}


/* Splitting the commands based on space in order to separate command from options */
static void tokenize(char* cmdLine)
{
	while (isspace(*cmdLine)) ++cmdLine;

  	// spliting on occurence of space
	char* pipe_pos = strchr(cmdLine, ' ');
	int i = 0;
	while(pipe_pos != NULL) {
		pipe_pos[0] = '\0';
		arg1[i] = cmdLine;
		++i;
		char* new_pipe_pos = pipe_pos + 1;
		while (isspace(*new_pipe_pos)) ++new_pipe_pos;
		cmdLine = new_pipe_pos;
		pipe_pos = strchr(cmdLine, ' ');
	}

	if (cmdLine[0] != '\0') {
		arg1[i] = cmdLine;
		pipe_pos = strchr(cmdLine, '\n');
		pipe_pos[0] = '\0';
		++i;
	}

	arg1[i] = NULL;
}

/* To run each command by calling execute_pipe method */
static int execute1(char* cmdLine, int val1, int val2, int val3)
{
	tokenize(cmdLine);
	if (arg1[0] != NULL) {
		if (strcmp(arg1[0], "exit") == 0)
			exit(0);
		no_cmd += 1;
		return execute_pipe(val1, val2, val3);
	}
	return 0;
}


void pipe_method(char* buf)
{

	int val1 = 0, val2 = 1, val3 = 0;

	char* cmdLine = buf;
		char* pipe_pos = strchr(cmdLine, '|');

		/* to split the command based on ''|'' */
		while (pipe_pos != NULL) {
			*pipe_pos = '\0';
			val1 = execute1(cmdLine, val1, val2, val3);

			cmdLine = pipe_pos + 1;
			pipe_pos = strchr(cmdLine, '|');
			val2 = 0;
		}
		val3 = 1;
		val1 = execute1(cmdLine, val1, val2, val3);

}
