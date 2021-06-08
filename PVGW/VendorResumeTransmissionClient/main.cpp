#include <iostream>
#include <string>
#include <thread>

/*********ADD*********/
#include <sstream>
#include <string>
#include <cstdlib>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <limits.h> 

#include <vector>
#include <cstring>
#include <algorithm>

#include <chrono>
#include <fstream>
#include <time.h>
#include "common.h"

///////////////////////////////////////
#define DEF_IP		"0.0.0.0"
//////////////////////////////////////

int iMainClientSocketID = -1 ;
int iMainQuitFlag = 0;

char cIp[128] = {0};
char recvBuf[1024];


void vMain_QuitSignal( int iValue )
{
	iMainQuitFlag = 1 ;
}

///////////////////////////////////////////////////////

void getHost(struct hostent** ppHost)
{
	hostent *pHost;
	char CONNECT_IP[32];
	memset(CONNECT_IP, 0, 32);

	if(strlen(cIp) > 0)
		memcpy(CONNECT_IP, cIp, strlen(cIp));
	else
		memcpy(CONNECT_IP, DEF_IP, strlen(DEF_IP));

	pHost = gethostbyname(CONNECT_IP);
	*ppHost = pHost;
}

int runnable_SocketClose()
{
	close(iMainClientSocketID);
	iMainClientSocketID =0;
}

int runnable_SocketConnect(int iPort)
{
	struct sockaddr_in serverAddr;
	struct hostent *pHost;

	getHost(&pHost);

	if ((iMainClientSocketID = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("Create Socket Fail!!!\n");
		return -1;
	}

	int iReuseaddr=1;
	setsockopt(iMainClientSocketID, SOL_SOCKET,SO_REUSEADDR,(const char*)&iReuseaddr,sizeof(int));

	printf("Connect IP = %s ; Port = %d\n", cIp, iPort);

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(iPort);
	serverAddr.sin_addr = *((struct in_addr *)pHost->h_addr);
	bzero(&(serverAddr.sin_zero),8);
	if (connect(iMainClientSocketID, (struct sockaddr *)&serverAddr,sizeof(struct sockaddr)) == -1)
	{
		printf("Connect Fail!!!\n");
		return -1;
	}

	printf("TCP Client Connect port %d\n", iPort);
	return 0;
}

int runnable_SendData(char* cData, int iLen)
{
	int iNum = 0;

	iNum = send(iMainClientSocketID, cData, iLen ,0);
	if(iNum == -1)
	{
		printf("Send the message Fail\n");
	}
	else
	{
		//printf("Send the message Success\n");
	}

	return iNum;
}

int runnable_Recv()
{
	fd_set all_fd_set;
	int iRet = 0;
	struct timeval timeout;
	int error = 0;
	socklen_t len = sizeof (error);
	int iNum = 0;

	memset(recvBuf, 0x00, 1024);

	timeout.tv_sec=0;
	timeout.tv_usec=1000000;

	FD_ZERO(&all_fd_set);
	if(iMainClientSocketID > 0)
	{
		FD_SET(iMainClientSocketID, &all_fd_set);
		//~ dbgprintf("FD_SET sock=%d\n", sock);
	}

	iRet = getsockopt (iMainClientSocketID, SOL_SOCKET, SO_ERROR, &error, &len);

	if(iRet != 0)
	{
		printf("getsockopt Fail ! iRet = %d\n", iRet);
		return iRet;
	}
	if(error != 0)
	{
		printf("getsockopt error = %d\n", error);
		return error;
	}

	iRet = select(iMainClientSocketID + 1, &all_fd_set, NULL, NULL, &timeout);


	if(iRet < 0)		//Error
	{
		printf("select Error ; iRet = %d\n", iRet);
		return iRet;
	}
	else if(iRet == 0)	//Timeout
	{
		//printf("select Timeout!!");
		return 0;
	}
	else 				//recv data
	{
		if(FD_ISSET(iMainClientSocketID, &all_fd_set))
		{
			iNum = recv(iMainClientSocketID,recvBuf,sizeof(recvBuf),0);
			if((iNum == -1) ||(iNum == 0))
			{
				printf("sock.%d Receive the message Fail or Unexpected Server disconnect\n", iMainClientSocketID);
				return -1;
			}
			else
			{
				//printf("sock.%d Receive the message Success\n", iMainClientSocketID);
				printf("iNum = %d ; recvBuf = \n%s\n", iNum, recvBuf);
				
				char cConfirm[2];

				cConfirm[0] = 0x4F;
				cConfirm[1] = 0x4B;

				if(runnable_SendData(cConfirm, 2) >= 0)
				{
					//printf("Send OK Success!!!\n");
				}
				else
				{
					printf("Send OK Fail!!!\n");
				}
			}
		}
		else
		{
			printf("FD_ISSET Fail!!!\n");
		}
	}

	return iNum;
}


/////////////////////////////////////////////////////////
bool bRunnable()
{
	bool bResult = true;

	while(iMainQuitFlag == 0)
	{

		int irecv_ret = -1;

		{
			char cRead[4];

			cRead[0] = 0x52;
			cRead[1] = 0x45;
			cRead[2] = 0x41;
			cRead[3] = 0x44;

			if(runnable_SendData(cRead, 4) >= 0)
			{
				//printf("Send READ Success!!!\n");

				irecv_ret = runnable_Recv();
				if(irecv_ret < 0)
				{
					printf("error!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					bResult = false;
					//break;
				}
				else
				if(irecv_ret == 0)
				{
					//printf("re-recv!!!!!!!!!!!!!!!!!!!!!!!!!\n");
					usleep(10000);
				}
				else
				{
					//printf("Success!!!\n");
					//break;
				}
			}
			else
			{
				printf("Send READ Fai;!!!\n");
			}
		}

		usleep(100000);
	}
	
	usleep(1000000);
	return bResult;
}

/////////////////////////////////////////////////////////



int main(int argc, char* argv[])
{
	signal( SIGINT , vMain_QuitSignal ) ;

	int iCount = 0;
	bool bConnect = false;

	int iArgsCount = argc;

	{
		for(int i = 0 ; i < iArgsCount ; i++)
		{
			printf("argv[ %d ] = %s\n", i, argv[i]);
		}
	}

	if(iArgsCount == 3)
	{
		strcpy(cIp, argv[1]);
		if(runnable_SocketConnect(atoi(argv[2])) == 0)
		{
			bConnect = true;
			bRunnable();
		}
		runnable_SocketClose();
	}
	else
	{
		printf("\n============================================================\n");
		printf("Need input ip and port\n");
		printf("example : ./SocketClient 10.69.99.135 10011\n");
		printf("============================================================\n");
		return 0;
	}
}

