/**********************************
tcp_ser.c: the source file of the server in tcp transmission 
***********************************/


#include "headsock_tcp4.h"


#define BACKLOG 10

char toPrint[BUFSIZE] = {"\0"} ;

void str_ser(int sockfd);                                                        // transmitting and receiving function

int main(void)
{
	//socket file descriptors
	int sockfd, con_fd, ret;

	//internet socket structures
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;


	int sin_size;

	pid_t pid;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);          //create stream socket running over IPv4
	if (sockfd < 0)
	{
		printf("Error in socket!");
		exit(1);
	}
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYTCP_PORT);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);//inet_addr("172.0.0.1");

	//returns Void
	//places 8 null bytes in the first argument, which is a string

	bzero(&(my_addr.sin_zero), 8);

	ret = bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr));                //bind socket

	if (ret < 0)
	{
		printf("Error in binding!");
		exit(1);
	}
	
	ret = listen(sockfd, BACKLOG);                              //listen

	if (ret < 0) {
		printf("Error in listening!");
		exit(1);
	}

	
		printf("Waiting for data...\n");

		sin_size = sizeof (struct sockaddr_in);

		//this is the other socket fd which will accept, while the other, sockfd, continues to listen

		con_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);            //accept the packet
		
		if (con_fd < 0)
		{
			printf("Error in accept!\n");
			exit(1);
		}

	
//child process pid = 0	
		if ((pid = fork())==0)                                         // create acception process
		{
			close(sockfd);                                            //close this socket fd so that only one connection to this port can be made
			str_ser(con_fd);                     
		}
		else close(con_fd);                                         //parent process
		close(sockfd);
		exit(0);
}		



void str_ser(int sockfd)
{
	struct pack_so recvs ; 
	struct ack_so ack ;	
	int	   n = 0;
	ssize_t	x = 0;
	char buf[BUFSIZE] = {"\0"} ;
	char rbuf[32];
	FILE * fp ;
	int i = 0;
	
	while (1) {
		ack.len = 0 ;
		n = recv(sockfd, &recvs, PACKLEN, MSG_WAITALL) ;	 //receive the packet
		if(n == -1) {      
			printf("Error receiving!\n") ;
			break;
		}

		ack.num = recvs.num ;

		printf("i = %d, N = %d\n", i++, n) ;
		if (n == 0) break;
		printf("Received the packet %d %d!\n", recvs.num, recvs.len) ;
		printf(recvs.data) ;

		//getting stuck here in memcpy! coz n - headlen becomes negative!! 2-8 = -6
		memcpy(buf, recvs.data, (n-HEADLEN)) ;
		strcat(toPrint, buf) ;
		x = send(sockfd, &ack, 8, 0);
		if(( x == -1)) {
			printf("Error in sending ACK\n") ;
			exit(1) ;
		}    
	    
		int ackNum = ack.num ;    
		printf("ACK %d sent\n", ackNum) ;
	 
	}

	if((fp = fopen("New.txt", "wt")) == NULL) {
		printf("File doesn't exist.") ;
		exit(0) ;
	}

	fwrite(toPrint, 1, strlen(toPrint), fp) ;
	fclose(fp) ;
	printf("A file has been successfully received!\nThe total data received is %d bytes\n", (int)strlen(toPrint)); 	
	close(sockfd);
	exit(0) ;  
}


