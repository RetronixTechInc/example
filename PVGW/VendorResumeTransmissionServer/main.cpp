/* Server code in C */
 
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

int iMainQuitFlag = 0;

void vMain_QuitSignal( int iValue )
{
	printf("vMain_QuitSignal!!!!!!!!!!!!!!!!!!!!\n");
    /* 設定離開程式 */
	iMainQuitFlag = 1 ;
}

void socketServer(int iPort)
{
	struct sockaddr_in stSockAddr;
	int SocketFD = socket(AF_INET, SOCK_STREAM, 0);
	int iReuseaddr=1;
	setsockopt(SocketFD, SOL_SOCKET,SO_REUSEADDR,(const char*)&iReuseaddr,sizeof(int));

	if(-1 == SocketFD)
	{
		perror("can not create socket");
		exit(EXIT_FAILURE);
	}
 
	memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons(iPort);
	stSockAddr.sin_addr.s_addr = INADDR_ANY;
 
	if(-1 == bind(SocketFD,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
	{
		perror("error bind failed");
		close(SocketFD);
		exit(EXIT_FAILURE);
	}
 
	if(-1 == listen(SocketFD, 10))
	{
		perror("error listen failed");
		close(SocketFD);
		exit(EXIT_FAILURE);
	}
 
	while(1)
	{
		int ConnectFD = accept(SocketFD, NULL, NULL);
 
		if(0 > ConnectFD)
		{
			perror("error accept failed");
			close(SocketFD);
			exit(EXIT_FAILURE);
		}

		char buffer[1024];
		int iLen = 0;

		while(1)
		{
			memset(buffer, 0x00, 1024);
			iLen = read( ConnectFD , buffer , 1024 ) ;
			
			if(iLen > 0)
			{
				printf("recv = \n%s\n", buffer);
				{
					char cConfirm[2];

					cConfirm[0] = 0x4F;
					cConfirm[1] = 0x4B;

					send(ConnectFD, cConfirm, 2 ,0);
				}
			}

			if(iMainQuitFlag == 1)
			{
				printf("[%d]iMainQuitFlag = %d\n", __LINE__, iMainQuitFlag);
				break;
			}
		}
 
		/* perform read write operations ... */
		shutdown(ConnectFD, SHUT_RDWR);
		close(ConnectFD);

		if(iMainQuitFlag == 1)
		{
			printf("The End!!!\n");
			break;
		}
	}

	close(SocketFD);
}

int main(int argc, char* argv[])
{
	signal( SIGINT , vMain_QuitSignal ) ;

	int iArgsCount = argc;

	{
		for(int i = 0 ; i < iArgsCount ; i++)
		{
			printf("argv[ %d ] = %s\n", i, argv[i]);
		}
	}

	if(iArgsCount == 2)
	{
		socketServer(atoi(argv[1]));
	}
	else
	{
		printf("\n============================================================\n");
		printf("Need input port\n");
		printf("example : ./SocketServer 10010\n");
		printf("============================================================\n");
	}

    return 0;
}
