//client tcp process
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

#define BUF_SIZE     60000


int msg_validation(int fd, char* buffer)
{
	int n,msg_len=0;

	do {
		n = recv(fd,&buffer[msg_len],BUF_SIZE-msg_len-1,0);
		msg_len += n;
		if(msg_len >= BUF_SIZE-1) {
			printf("Buffer overflow.\n");
			exit(1);
		}
	} while(n > 0);

	return msg_len;
}


int main(int argc, char* argv[])
{
	int simpleSocket, serLen;
	struct sockaddr_in serverAddress;
	struct sockaddr* serverAddrPtr;
	struct hostent* servername;

	char com[256], buffer[BUF_SIZE];

	int res;
	unsigned long inetAddress;  /* 32-bit IP address */

	/* Avoiding zombies */
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s <server> <port>\n", argv[0]);
		exit(1);
	}


	serverAddrPtr = (struct sockaddr*) &serverAddress;
	serLen = sizeof(serverAddress);
	bzero((char *) &serverAddress, serLen);
	serverAddress.sin_family=AF_INET;
	serverAddress.sin_port=htons(atoi(argv[2]));

	inetAddress = inet_addr(argv[1]);
	serverAddress.sin_addr.s_addr = inetAddress;


	/* create a streaming socket */
	simpleSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (simpleSocket < 0)
	{
		fprintf(stderr, "Could not create a socket!\n");
		exit(1);
	}
	else
	{
		fprintf(stderr, "Socket created!\n");
 	}

	/* Repeatatively wait to make the connection with Server until the 60 sec */
	int nosec=1;
	do {
		res = connect(simpleSocket, serverAddrPtr, serLen);
		if(res == -1) { sleep(1); nosec++; }
	} while(res == -1 && nosec <= 60);

	if(nosec > 60)
	{
		 perror("ERROR **** Connection timeout Error ****\n");
		 exit(1);
	}

	struct timeval tmv;
	tmv.tv_sec = 5;
	setsockopt(simpleSocket, SOL_SOCKET, SO_RCVTIMEO,
	(struct timeval*) &tmv, sizeof(struct timeval));


	printf("***** Client Process  *****\n***** Press 'exit' to quit from Client Process *****\n");


	while(1) {
		printf("Executing Unix Commands. \nThis takes upto 3 pipe connected commands.\nPlease provide space before and after pipe. \n");
		printf("%s ", "$");
		bzero(com, sizeof(com));
		fgets(com, 256, stdin);

		bzero(buffer, sizeof(buffer));

		/* Sending the command to server */
		if(send(simpleSocket,com,sizeof(com),0) < 0)
		{
			perror("ERROR **** Unable to send the message\n");
			exit(1);
		}

		/* To exit on typing 'exit' */
		if (strcmp(com, "exit\n") == 0) break;

		/* Receiving the output from server */
		int msg_len = msg_validation(simpleSocket, buffer);

		if(msg_len>0){
			buffer[msg_len]='\0';
			printf("Output of command line is :\n %s\n\n\n", buffer);
		}
	}

	close (simpleSocket);

	return 0;
}
