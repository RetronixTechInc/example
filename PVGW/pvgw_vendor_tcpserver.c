#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "message_config.h"

static int sock;
int SOCKET_PORT = 10001;
char CONNECT_IP[32] = "";

SOCKET_ITEM socket_arr[socket_max];
CLIENT_ITEM *client_current = NULL;

static int Socket_Init()
{
	struct sockaddr_in server_addr;
	BOOL bReuseaddr=TRUE;
	BOOL bDontLinger = FALSE;
	int nNetTimeout=1000;//1秒
	int nRecvBuf=32*1024;//Recive buffer 32K
	int nSendBuf=32*1024;//Send buffer 32K
	int iLoop;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		dbgprintf("Socket\n");
		return -1;
	}

	//~ https://blog.csdn.net/leitianjun/article/details/7783609
	//~ need TIME_WAIT to close socket
	if (setsockopt(sock, SOL_SOCKET,SO_REUSEADDR,(const char*)&bReuseaddr,sizeof(BOOL)) == -1) {
		dbgprintf("Setsockopt set socket reuse fail!\n");
		return -1;
	}
	
	//~ didn't need TIME_WAIT to close socket
	//~ if (setsockopt(sock, SOL_SOCKET,SO_DONTLINGER,(const char*)&bDontLinger,sizeof(BOOL)) == -1) {
		//~ dbgprintf("Setsockopt set socket direct close fail!\n");
		//~ return -1;
	//~ }
	
	//~ if (setsockopt(sock,SOL_SOCKET,SO_SNDTIMEO,(char *)&nNetTimeout,sizeof(int)) == -1) {
		//~ dbgprintf("Setsockopt set send timeout fail!\n");
		//~ return -1;
	//~ }
	
	//~ if (setsockopt(sock,SOL_SOCKET,SO_RCVTIMEO,(char *)&nNetTimeout,sizeof(int)) == -1) {
		//~ dbgprintf("Setsockopt set recive timeout fail!\n");
		//~ return -1;
	//~ }
	
	//~ if (setsockopt(sock,SOL_SOCKET,SO_RCVBUF,(const char*)&nRecvBuf,sizeof(int)) == -1) {
		//~ dbgprintf("Setsockopt set recive buffer fail!\n");
		//~ return -1;
	//~ }
	
	//~ if (setsockopt(sock,SOL_SOCKET,SO_SNDBUF,(const char*)&nSendBuf,sizeof(int)) == -1) {
		//~ dbgprintf("Setsockopt set send buffer fail!\n");
		//~ return -1;
	//~ }

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SOCKET_PORT);
	if(strlen(CONNECT_IP) >= 7)
	{
		server_addr.sin_addr.s_addr = inet_addr(CONNECT_IP);
		printf("Server IP=%s\n", CONNECT_IP);
	}
	else
	{
		server_addr.sin_addr.s_addr = INADDR_ANY;
		printf("Server IP=0.0.0.0\n");
	}
	bzero(&(server_addr.sin_zero),8);

	if (bind(sock, (struct sockaddr *)&server_addr,sizeof(struct sockaddr))==-1) {
		dbgprintf("Unable to bind\n");
		return -1;
	}

	//~ Wait accept num.
	if (listen(sock, 3) == -1) {
		dbgprintf("Set Listen num fail!\n");
		return -1;
	}

	dbgprintf("TCP Server Waiting for client on port %d\n", SOCKET_PORT);

	for(iLoop = 0; iLoop < socket_max; iLoop++)
	{
		socket_arr[iLoop].socket_fd = 0;
	}
	
	return 0;

}

static void Socket_Loop()
{
	fd_set all_fd_set;
	int max_fd = sock;
	int new_fd=0;
	struct timeval timeout = {3, 0};

	int connected = 0;
	struct sockaddr_in client_addr;
	char recive_message[256] = {'\0'};
	char send_message[256] = "none";
	socklen_t sin_size = sizeof(client_addr);

	int ret;
	int numbytes;
	int iLoop;

	while(1){
		timeout.tv_sec=3;
		timeout.tv_usec=1000;

		FD_ZERO(&all_fd_set);
		
		if(sock > 0){
			FD_SET(sock, &all_fd_set);
			//dbgprintf("FD_SET sock=%d\n", sock);
		}
		
		//~ if(connected != 0){
			//~ FD_SET(connected, &all_fd_set);
			//~ dbgprintf("FD_SET connected=%d\n", connected);
		//~ }
		
		for(iLoop = 0; iLoop < socket_max; iLoop++)
		{
			if( socket_arr[iLoop].socket_fd > 0 )
			{
				FD_SET(socket_arr[iLoop].socket_fd, &all_fd_set);
				//~ dbgprintf("FD_SET socket_arr[%d]=%d\n", iLoop, socket_arr[iLoop].socket_fd);
			}
		}

		ret = select(max_fd + 1, &all_fd_set, NULL, NULL, &timeout);

		if(ret < 0){
			dbgprintf("select error**************\n");
			break;
		}else if(ret == 0){
		    //dbgprintf("select time out\n");
			continue;
		}

		//~ dbgprintf("select change!\n");

		for(iLoop = 0; iLoop < socket_max; iLoop++)
		{
			if((socket_arr[iLoop].socket_fd > 0) && FD_ISSET(socket_arr[iLoop].socket_fd, &all_fd_set)){
				memset(recive_message, 0, 256);
				numbytes = recv(socket_arr[iLoop].socket_fd,recive_message,sizeof(recive_message), 0);

				if((numbytes == -1) ||(numbytes == 0)){
					dbgprintf("socket_arr[%d].%d Receive the message Fail or Unexpected Server disconnect\n", iLoop, socket_arr[iLoop].socket_fd );
					close(socket_arr[iLoop].socket_fd );
					socket_arr[iLoop].socket_fd = 0;
				}else{
					dbgprintf("socket_arr[%d].%d Receive the message :\n%s\n", iLoop, socket_arr[iLoop].socket_fd, recive_message);
					if((numbytes = send(socket_arr[iLoop].socket_fd, send_message, strlen(send_message) ,0))== -1){
						printf("socket_arr[%d].%d Send the message Fail\n", iLoop, socket_arr[iLoop].socket_fd);
					}
				}
			}
		}

		if(FD_ISSET(sock,&all_fd_set)){
			connected = accept(sock, (struct sockaddr*)&client_addr, &sin_size);

			if(connected <=0){
				printf("accept error*********************\n");
				continue;
			}

			if(connected > max_fd){
				max_fd = connected;
			}
			
			for(iLoop = 0; iLoop < socket_max; iLoop++)
			{
				if( socket_arr[iLoop].socket_fd <= 0 )
				{
					socket_arr[iLoop].socket_fd = connected;
					FD_CLR(connected,&all_fd_set);
					connected = 0;
					if((numbytes = send(socket_arr[iLoop].socket_fd, send_message, strlen(send_message) ,0))== -1){
						printf("socket_arr[%d].%d Send the message Fail\n", iLoop, socket_arr[iLoop].socket_fd);
					}
					break;
				}
			}
			
			if( connected != 0 )
			{
				close( connected );
			}

//			if((numbytes = send(new_fd,"connect", strlen("connect") ,0))== -1){
//				dbgprintf("Send the message Fail\n");
//			}
		}
    }

    //close server client serial
	if(sock != 0){
		close(sock);
		sock =0;
	}
	for(iLoop = 0; iLoop < socket_max; iLoop++)
	{
		if(socket_arr[iLoop].socket_fd > 0){
			close(socket_arr[iLoop].socket_fd);
			socket_arr[iLoop].socket_fd = 0;
		}
	}
	
	return;
}

int main( int argc, char ** argv ){
	int ret;
	dbgprintf("Socket_server main \n");

	memset(CONNECT_IP, 0, 32);
		
	if(argc > 1)
	{
		SOCKET_PORT = strtol(argv[1], NULL, 0);
		if(argc > 2)
		{
			memcpy(CONNECT_IP, argv[2], strlen(argv[2]));
		}
		ret = Socket_Init();
		dbgprintf("Socket_Init : %d \n", ret);

		if(ret < 0){
			return ret;
		}

		Socket_Loop();	
		dbgprintf("Socket_server main end \n");
	}
	else
	{
		printf("usage : %s listen-port [listen-ip]\n", argv[0]);
		printf("Example:\n");
		printf("\t%s 3333\n", argv[0]);
		printf("\t%s 3333 192.168.0.55\n", argv[0]);
		printf("\n");
	}
	
	return 0;

}

