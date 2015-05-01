/*******************************
tcp_client.c: the source file of the client in tcp transmission 
********************************/

#include "headsock_tcp4.h"

float str_cli(FILE *fp, int sockfd, long *len);                       //transmission function
void tv_sub(struct  timeval *out, struct timeval *in);	    //calcu the time interval between out and in

int main(int argc, char **argv)
{
	int sockfd, ret;
	float ti, rt;
	long len;
	struct sockaddr_in ser_addr;
	char ** pptr;
	struct hostent *sh;
	struct in_addr **addrs;
	FILE *fp;

	if (argc != 2) {
		printf("Parameters don't match");
	}

	sh = gethostbyname(argv[1]);	                                       //get host's information
	if (sh == NULL) {
		printf("Error: gethostbyname");
		exit(0);
	}

	printf("Canonical Name: %s\n", sh->h_name);					//print the remote host's information
	for (pptr=sh->h_aliases; *pptr != NULL; pptr++)
		printf("The aliases name is: %s\n", *pptr);
	switch(sh->h_addrtype)
	{
		case AF_INET:
			printf("AF_INET\n");
		break;
		default:
			printf("unknown addrtype\n");
		break;
	}
        
	addrs = (struct in_addr **)sh->h_addr_list;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);                           //create the socket
	if (sockfd < 0)
	{
		printf("Error in socket!");
		exit(1);
	}

	ser_addr.sin_family = AF_INET;                                                      
	ser_addr.sin_port = htons(MYTCP_PORT);
	memcpy(&(ser_addr.sin_addr.s_addr), *addrs, sizeof(struct in_addr));
	bzero(&(ser_addr.sin_zero), 8) ;

	ret = connect(sockfd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr));         //connect the socket with the host

	if (ret != 0) {
		printf ("connection failed\n"); 
		close(sockfd); 
		exit(1);
	}
	
	if((fp = fopen (argv[2],"r+t")) == NULL)
	{
		printf("File doesn't exist!\n");
		exit(0);
	}

	ti = str_cli(fp, sockfd, &len);                       //perform the transmission and receiving
	rt = (len/(float)ti);                                 //caculate the average transmission rate
	printf("Time (ms.) : %.3f, Data Sent (bytes): %d\nData Rate: %f (Kbytes/s)\n", ti, (int) len, rt);

	close(sockfd);
	fclose(fp);

	exit(0);
}

float str_cli(FILE *fp, int sockfd, long *len)
{
	char *buf ;
	int c ;
	long lsize, ci ;
	struct pack_so sends ;
	struct ack_so ack;
	int n, slen;
	int count = 0 ;
	int count1 = 0 ;

	//time stuff
	float time_inv = 0.0;
	struct timeval sendt, recvt ;

	ci = 0;
	sends.num = 0 ;

	fseek (fp , 0 , SEEK_END);
	lsize = ftell(fp) ;                //number of bytes in the file

	rewind (fp);


	printf("The packet length is %d bytes\n", DATALEN) ;

	//allocate memory to contain the whole file.
	buf = (char *) malloc (lsize);
	if (buf == NULL)
	 exit (2);

  	//copy the file into the buffer.
	//fread (buf,1,lsize,fp);

	//copy the file into the buffer
	
	while((c = fgetc(fp)) != EOF) {
		buf[count1++] = (char) c ;
	} 

  	/*** the whole file is loaded in the buffer. ***/

	buf[count1] ='\0';									//append the end byte

	//printf("The file is: %s", buf) ;

	printf("The file length is %d bytes\n", (int) strlen(buf));


	gettimeofday(&sendt, NULL);							//get the current time


	//read DATALEN bytes from file till the end of the file in a while loop
	//for each read segment, send a packet
	//wait to receive ACK before sending the next packet

	while (ci < lsize)
	{
		if ((lsize - ci) < DATALEN)
			slen = lsize-ci;
		else 
			slen = DATALEN;

		sends.len = slen ;
		sends.num = count ; 

		memcpy(sends.data, (buf+ci), slen) ;

 		//send the data
		n = send(sockfd, &sends, sizeof(sends), 0) ;

		if(n == -1) {
			printf("Send error!");								
			exit(1);
		}


		printf("Sends is: %s\n", sends.data) ;
		printf("Sends size is: %d\n", sends.len) ;
		printf("Sends num is: %d\n", sends.num) ;

		ci += slen;

		if ((n = recv(sockfd, &ack, 8, 0)) == -1)                                   //receive the ack
		{
			printf("Error: no ACK received!\n");
			
			//resend the same packet
			exit(1) ;
		}

		if (ack.num != sends.num ) {
			printf("Error in transmission!\n") ;
		}

		else
			printf("ACK %d Received! \n", ack.num) ;
		
		count++ ;

	}

	//send terminating packet

	sends.len = 0 ;
	sends.num = count ; 
	strcpy(sends.data, "\0") ;

	n = send(sockfd, &sends, sizeof(sends), 0) ;


	gettimeofday(&recvt, NULL) ;                         //get current time
	
	*len = ci ;  //the filesize in bytes, so you can calc data rate in kbytes/s                                                       
	
	tv_sub(&recvt, &sendt);                                                                 // get the whole trans time
	time_inv += (recvt.tv_sec)*1000.0 + (recvt.tv_usec)/1000.0;
	return(time_inv);
}

void tv_sub(struct  timeval *out, struct timeval *in)
{
	if ((out->tv_usec -= in->tv_usec) <0)
	{
		--out ->tv_sec;
		out ->tv_usec += 1000000;
	}
	out->tv_sec -= in->tv_sec;
}
