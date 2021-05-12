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
#include "pvgw_socket.h"

///////////////////////////////////////
int SOCKET_PORT = 10000;
char CONNECT_IP[32] = "";
//////////////////////////////////////

int iMainClientSocketID = -1 ;
int iMainQuitFlag = 0;

#pragma pack(1)
typedef struct __REQ_CMD__ {
	uint32_t  ucmd      ;
	char	  sTag[64]	;	/**< Read/Write sTag */
	int32_t   iVal   	;	/**< uval for QUEUE number and Write value */
	uint16_t  crc       ;
} REQ_CMD , *PTR_REQ_CMD ;
#pragma pack()
#define REQ_CMD_LEN	sizeof(REQ_CMD)

typedef struct __WRITE_CMD__ {
	char		sTag[64]	;
	int32_t		iVal		;
} WRITE_CMD, *PTR_WRITE_CMD;

char recvBuf[1024];
typedef std::list<WRITE_CMD*> LIST_WRITE_CMD;
LIST_WRITE_CMD listWriteCmd;


enum __CMD_REQUEST__ {
	DEF_REQ_CMD_READ           ,  // AI
	DEF_REQ_CMD_WRITE          ,  // AO
};

void vMain_QuitSignal( int iValue )
{
	iMainQuitFlag = 1 ;
}

///////////////////////////////////////////////////////
/**< CRC lookup table */
const uint8_t CRCtableHi[256]=
{
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
	0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
	0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
	0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
	0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,
	0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40,0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,
	0x00,0xC1,0x81,0x40,0x01,0xC0,0x80,0x41,0x01,0xC0,0x80,0x41,0x00,0xC1,0x81,0x40
};
const uint8_t CRCtableLo[256]=
{
	0x00,0xC0,0xC1,0x01,0xC3,0x03,0x02,0xC2,0xC6,0x06,0x07,0xC7,0x05,0xC5,0xC4,0x04,
	0xCC,0x0C,0x0D,0xCD,0x0F,0xCF,0xCE,0x0E,0x0A,0xCA,0xCB,0x0B,0xC9,0x09,0x08,0xC8,
	0xD8,0x18,0x19,0xD9,0x1B,0xDB,0xDA,0x1A,0x1E,0xDE,0xDF,0x1F,0xDD,0x1D,0x1C,0xDC,
	0x14,0xD4,0xD5,0x15,0xD7,0x17,0x16,0xD6,0xD2,0x12,0x13,0xD3,0x11,0xD1,0xD0,0x10,
	0xF0,0x30,0x31,0xF1,0x33,0xF3,0xF2,0x32,0x36,0xF6,0xF7,0x37,0xF5,0x35,0x34,0xF4,
	0x3C,0xFC,0xFD,0x3D,0xFF,0x3F,0x3E,0xFE,0xFA,0x3A,0x3B,0xFB,0x39,0xF9,0xF8,0x38,
	0x28,0xE8,0xE9,0x29,0xEB,0x2B,0x2A,0xEA,0xEE,0x2E,0x2F,0xEF,0x2D,0xED,0xEC,0x2C,
	0xE4,0x24,0x25,0xE5,0x27,0xE7,0xE6,0x26,0x22,0xE2,0xE3,0x23,0xE1,0x21,0x20,0xE0,
	0xA0,0x60,0x61,0xA1,0x63,0xA3,0xA2,0x62,0x66,0xA6,0xA7,0x67,0xA5,0x65,0x64,0xA4,
	0x6C,0xAC,0xAD,0x6D,0xAF,0x6F,0x6E,0xAE,0xAA,0x6A,0x6B,0xAB,0x69,0xA9,0xA8,0x68,
	0x78,0xB8,0xB9,0x79,0xBB,0x7B,0x7A,0xBA,0xBE,0x7E,0x7F,0xBF,0x7D,0xBD,0xBC,0x7C,
	0xB4,0x74,0x75,0xB5,0x77,0xB7,0xB6,0x76,0x72,0xB2,0xB3,0x73,0xB1,0x71,0x70,0xB0,
	0x50,0x90,0x91,0x51,0x93,0x53,0x52,0x92,0x96,0x56,0x57,0x97,0x55,0x95,0x94,0x54,
	0x9C,0x5C,0x5D,0x9D,0x5F,0x9F,0x9E,0x5E,0x5A,0x9A,0x9B,0x5B,0x99,0x59,0x58,0x98,
	0x88,0x48,0x49,0x89,0x4B,0x8B,0x8A,0x4A,0x4E,0x8E,0x8F,0x4F,0x8D,0x4D,0x4C,0x8C,
	0x44,0x84,0x85,0x45,0x87,0x47,0x46,0x86,0x82,0x42,0x43,0x83,0x41,0x81,0x80,0x40
};

/**
 * @breif
 * Calcalate CRC
 * ------------
 * @param [in] *pData : Calcalate Data
 * @param [in] len Calcalate length
 * @return
 * crc value.
 * @note
 * None
 * @remarks
 *
 */
uint16_t u16_calculate_CRC( uint8_t *pData , uint32_t len)
{
    uint16_t crc;
    uint8_t index;
    uint8_t m, l;
    crc=0xffff;
    while(len--)
    {
        m=(unsigned char)((crc>>8)&0x00ff);
        l=crc&0x00ff;
        index=m ^ *pData++;
        m=l ^ CRCtableHi[index];
        l=CRCtableLo[index];
        crc=(((uint16_t)m)<<8)+((uint16_t)l);
    }
    return(crc);
}

void getHost(struct hostent** ppHost)
{
	hostent *pHost;

	pHost = gethostbyname(CONNECT_IP);
	*ppHost = pHost;
}

void getCmd(REQ_CMD* pReqCmd, int iCtl, char* cTag, int32_t iVal, char* pCmd)
{
	pReqCmd->ucmd = iCtl; //DEF_REQ_CMD_READ or DEF_REQ_CMD_WRITE
	memcpy(pReqCmd->sTag, cTag, strlen(cTag));
	pReqCmd->iVal = iVal;
	pReqCmd->crc = u16_calculate_CRC((uint8_t*)pCmd, REQ_CMD_LEN-2);
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

int runnable_Recv()
{
	uint16_t crc, crc_cal;
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
		printf("getsockopt iRet = %d\n", iRet);
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
		printf("select iRet = %d\n", iRet);
		return iRet;
	}
	else if(iRet == 0)	//Timeout
	{
		printf("select Timeout!!");
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
				//printf("iNum = %d\n", iNum);
				if ( iNum >= 3)
				{
					crc_cal = u16_calculate_CRC((uint8_t*)recvBuf, iNum-2);
					crc = recvBuf[iNum - 2];
					crc <<= 8;
					crc += recvBuf[iNum - 1];

					//if(crc_cal == crc)
					{
						int iCount = iNum/sizeof(double);
						printf("iNum = %d ; iCount = %d\n", iNum, iCount);

						for(int iLoop = 0; iLoop < iCount; iLoop++)
						{
							double dRecvVal = -1;
							memcpy(&dRecvVal, recvBuf + (sizeof(double) * iLoop), sizeof(double));
							printf("[%d] dRecvVal = %lf\n", iLoop, dRecvVal);
						}
					}
//					else
//					{
//						printf("Recv crc Fail !!!\n");
//					}
				}
				else
				{
					printf("Recv iNum < 3 (%d)!!!\n", iNum);
					for(int iLoop = 0; iLoop < iNum; iLoop++)
					{
						double dRecvVal = -1;
						memcpy(&dRecvVal, recvBuf + (sizeof(double) * iLoop), sizeof(double));
						printf("[%d] recvBuf = 0x%02X\n", iLoop, recvBuf[iLoop]);
					}
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

int runnable_RequestWriteData(char* cCmd, int iCmdValue)
{
	bool bResult = false;
	{
		time_t now; //例項化time_t結構
		struct tm *timenow; //例項化tm結構指標
		time(&now);
		//time函式讀取現在的時間(國際標準時間非北京時間)，然後傳值給now
		timenow = localtime(&now);

		printf("%s !!!!!! runnable_RequestWriteData !!!!!!\n", asctime(timenow));
	}

	REQ_CMD req_cmd;
	char *sendBuf;
	uint16_t crc, crc_cal;
	double *dval;
	int iNum = 0;
	char sTag[64];
	int32_t iVal = 0;

	memset(sTag, 0x00, 64);
	memset(&req_cmd, 0, REQ_CMD_LEN);
	sendBuf = (char*)&req_cmd;
	int iCtl = DEF_REQ_CMD_WRITE;

	{
		iCtl = DEF_REQ_CMD_WRITE;
		strcpy(sTag, cCmd);	//Weite Tag
		iVal = iCmdValue;	//Write Value

		printf("[ Write ] Tag = %s\n", sTag);
		printf("[ Write ] Val = %d\n", iVal);
	}

	getCmd(&req_cmd, iCtl, sTag, iVal, sendBuf);
	iNum = send(iMainClientSocketID, sendBuf, REQ_CMD_LEN ,0);
	if(iNum == -1)
	{
		printf("Send the message Fail\n");
	}
	else
	{
		//printf("Send the message Success\n");
	}

	for(int i = 0 ; i < 5 ; i++)
	{
		iNum = runnable_Recv();
		if(iNum < 0)
		{
			printf("error!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			bResult = false;
			//break;
		}
		else
		if(iNum == 0)
		{
			printf("re-recv!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			usleep(10000);
		}
		else
		{
			printf("Success!!!(%d)\n", iNum);
			break;
		}
	}

	return iNum;
}

int runnable_RequestReadData(char* cTag)
{
	REQ_CMD req_cmd;
	char *sendBuf;
	uint16_t crc, crc_cal;
	double *dval;
	int iNum = 0;
	char sTag[64];
	int32_t iVal = 0;

	memset(sTag, 0x00, 64);
	memset(&req_cmd, 0, REQ_CMD_LEN);
	sendBuf = (char*)&req_cmd;
	int iCtl = DEF_REQ_CMD_READ;

	{
		iCtl = DEF_REQ_CMD_READ;
		strcpy(sTag, cTag);
		iVal = 0;
	}

	getCmd(&req_cmd, iCtl, sTag, iVal, sendBuf);
	iNum = send(iMainClientSocketID, sendBuf, REQ_CMD_LEN ,0);
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

/////////////////////////////////////////////////////////
bool bReadSocketData(char* cTag)
{
	bool bResult = true;
	char cMergeTag[512];
	
	memset(cMergeTag, 0x00, 512);
	sprintf(cMergeTag, "%s", cTag);
	//printf("cMergeTag = %s\n", cMergeTag);
	printf("============Resq %s data==================\n", cMergeTag);
	if(runnable_RequestReadData(cMergeTag) >= 0)
	{
		int irecv_ret = -1;
		//for(int i = 0 ; i < 5 ; i++)
		{
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
				printf("re-recv!!!!!!!!!!!!!!!!!!!!!!!!!\n");
				usleep(10000);
			}
			else
			{
				//printf("Success!!!\n");
				//break;
			}
		}
		//usleep(10000);
		
	}
	else
	{
		printf("runnable_RequestReadData(cTag) Fail!!!!\n");
	}
	
	usleep(1000000);
	return bResult;
}

/////////////////////////////////////////////////////////



int main(int argc, char* argv[])
{
	signal( SIGINT , vMain_QuitSignal ) ;

	for(int i = 0 ; i < argc ; i++)
	{
		printf("argv[ %d ] = %s\n", i, argv[i]);
	}

	memset(CONNECT_IP, 0, 32);
	if(argc > 1)
	{
		memcpy(CONNECT_IP, argv[1], strlen(argv[1]));
	}
	
	if(argc == 3)
	{
		if(runnable_SocketConnect(SOCKET_PORT) == 0)
		{
			bReadSocketData(argv[2]);
		}
		runnable_SocketClose();
	}
	else if(argc == 4)
	{
		if(runnable_SocketConnect(SOCKET_PORT) == 0)
		{
			runnable_RequestWriteData(argv[2], atoi(argv[3]));
		}
		runnable_SocketClose();
	}
	else
	{
		printf("usage : %s server-ip tag [val]\n", argv[0]);
		printf("Example:\n");
		printf("\tRead : %s  server-ip read-tag\n", argv[0]);
		printf("\tWrite : %s  server-ip write-tag 33\n", argv[0]);
		printf("\n");
		
		return 0;
	}

//	while(1)
//	{
//		usleep(1000);
//	}
}

