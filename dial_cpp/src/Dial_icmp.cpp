#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "Dial_common.h"

//#define ERROR -1

#define E_FAILD_FD -1
#define ICMP_DATA_LEN 20  /* ICMP负载长度 */
#define ICMP_BUFF_LEN 128  /* ICMP负载长度 */
#define ICMP_ECHO_MAX 4  /* ECHO-REQUEST报文发送次数 */
#define ICMP_REQUEST_TIMEOUT 2  /* 超时时间间隔 */

/* ICMP报文发送与接收缓存 */

static unsigned char aucSendBuf[1024 * 1024] = {0};
static unsigned char aucRecvBuf[1024 * 1024] = {0};

/* 结构体定义 */

typedef struct tagIcmpStatic
{
	unsigned int uiSendPktNum;
	unsigned int uiRcvPktNum;
	float fMinTime;
	float fMaxTime;
	float fArgTime;
}ICMP_STATIC_S;

/* 全局数据结构 */

ICMP_STATIC_S g_stPktStatic;      /* ICMP报文统计 */
struct timeval stSendTime = {0,0};  /* ECHO-REQUEST报文发送时间 */
struct timeval stRcvTime ={0,0};    /* ECHO-REPLY报文接收时间 */
/* 输出报文统计信息 */

void showStatic(ICMP_STATIC_S *pstStInfo)
{

unsigned int uiSend, uiRecv;

uiSend = pstStInfo->uiSendPktNum;

uiRecv = pstStInfo->uiRcvPktNum;

debug_printf(LOG_LEVEL_DEBUG," ***PING Statistics***");

debug_printf(LOG_LEVEL_DEBUG," Packets:Send = %u,Recveived = %u,Lost = %u", uiSend, uiRecv, uiSend - uiRecv);

debug_printf(LOG_LEVEL_DEBUG," Time:Minimum = %.1fms,Maximum = %.1fms,Average=%.2fms ", pstStInfo->fMinTime, pstStInfo->fMaxTime, pstStInfo->fArgTime);

}

/* 计算时间差,返回时间以毫秒为单位 */

unsigned int timeSub(const struct timeval *pstOut, const struct timeval *pstIn)
{

	unsigned int uiSec = 0;

	int iUsec = 0;

	uiSec = pstOut->tv_sec - pstIn->tv_sec;

	iUsec = pstOut->tv_usec - pstIn->tv_usec;

	if (0 > iUsec)

	{

		iUsec += 1000000;

		uiSec--;

	}

	return uiSec * 1000 + (unsigned int)(iUsec / 1000);
}

/* 校验和计算 */

unsigned short calc_icmp_chksum(const void *pPacket, int iPktLen)
{

	unsigned short usChkSum = 0;
	unsigned short *pusOffset = NULL;
	pusOffset = (unsigned short *)pPacket;

	while(1 < iPktLen)
	{
		usChkSum += *pusOffset++;
		iPktLen -= sizeof(unsigned short);
	}

	if (1 == iPktLen)
	{
		usChkSum += *((char *)pusOffset);
	}
	
	usChkSum = (usChkSum >> 16) + (usChkSum & 0xffff);
	usChkSum += (usChkSum >>16);

	return ~usChkSum;

}

/* ICMP报文填充 */

int new_icmp_echo(const int iPacketNum, unsigned char *aucSendBuf,const int iDataLen)
{

	struct icmp *pstIcmp = NULL;
	pstIcmp = (struct icmp *)aucSendBuf;
	pstIcmp->icmp_type = ICMP_ECHO;
	pstIcmp->icmp_code = 0;
	pstIcmp->icmp_seq = htons((unsigned short)iPacketNum);
	pstIcmp->icmp_id = htons((unsigned short)pthread_self());
	pstIcmp->icmp_cksum = 0;
	pstIcmp->icmp_cksum = calc_icmp_chksum(pstIcmp, iDataLen + 8);

	return iDataLen + 8;

}

/* 解析ECHO-REPLY响应报文 */

int parseIcmp(const struct sockaddr_in *pstFromAddr, char *pRecvBuf, const int iLen)
{

	int iIpHeadLen = 0;
	int iIcmpLen = 0;
	struct ip *pstIp = NULL;
	struct icmp *pstIcmpReply = NULL;
	pstIp = (struct ip *)pRecvBuf;
	iIpHeadLen = pstIp->ip_hl << 2;

	pstIcmpReply = (struct icmp *)(pRecvBuf + iIpHeadLen);

	/* 报文长度非法 */

	iIcmpLen = iLen - iIpHeadLen;

	if (8 > iIcmpLen)
	{
		debug_printf(LOG_LEVEL_ERROR,"[Error]Bad ICMP Echo-reply\n");
		return -1;
	}

	/* 报文类型非法 */
	
	if (pstIcmpReply->icmp_type == ICMP_ECHO)
	{
		return 3;
	}

	if (pstIcmpReply->icmp_type != ICMP_ECHOREPLY)
	{
		return -1;
	}
	
	if (pstIcmpReply->icmp_id != htons((unsigned short)pthread_self()))
	{
		return 2;
	}


	//debug_printf(LOG_LEVEL_DEBUG,"%d bytes reply from %s: icmp_seq=%u Time=%dms TTL=%d \n", iIcmpLen, inet_ntoa(pstFromAddr->sin_addr), ntohs(pstIcmpReply->icmp_seq), timeSub(&stRcvTime, &stSendTime), pstIp->ip_ttl);

	return 1;
}

/* Echo响应报文接收 */

int recvIcmp(const int fd,struct sockaddr_in *pstDestAddr)

{

	int iRet = 0;
	int iRecvLen = 0;
	unsigned int uiInterval = 0;
	socklen_t fromLen = sizeof(struct sockaddr_in);
	struct sockaddr_in stFromAddr;
	unsigned char *aucRecvBuf = (unsigned char *)malloc(ICMP_BUFF_LEN);
	int count =0;

	/* 清空接收缓存，并准备接收响应报文 */

REDO:
	memset(aucRecvBuf, 0, ICMP_BUFF_LEN);
	iRecvLen = recvfrom(fd, (void *)aucRecvBuf,ICMP_BUFF_LEN, 0, (struct sockaddr *)&stFromAddr,&fromLen);
	//iRecvLen = recv(fd, (void *)aucRecvBuf,ICMP_BUFF_LEN, 0);
	
	//gettimeofday(&stRcvTime, NULL);
	debug_printf(LOG_LEVEL_DEBUG,"recvIcmp:receive buffer length=%d\n",iRecvLen);
	if (0 > iRecvLen)
	{
		if (EAGAIN == errno)
		{
			/* 请求超时 */
			debug_printf(LOG_LEVEL_ERROR,"Request time out.\n");
		}
		else
		{

		/* 接收数据包出错 */
			debug_printf(LOG_LEVEL_ERROR,"[Error]ICMP Receive");
		}
		free(aucRecvBuf);
		return -1;
	}
	
	if(pstDestAddr->sin_addr.s_addr != stFromAddr.sin_addr.s_addr)
		goto REDO;

	/* 获取统计参数 */
	/*g_stPktStatic.uiRcvPktNum++;

	uiInterval = timeSub(&stRcvTime, &stSendTime);

	g_stPktStatic.fArgTime = (g_stPktStatic.fArgTime * (g_stPktStatic.uiSendPktNum - 1) + uiInterval)/g_stPktStatic.uiSendPktNum;

	if (uiInterval < g_stPktStatic.fMinTime)
	{
		g_stPktStatic.fMinTime = uiInterval;
	}
	
	if (uiInterval > g_stPktStatic.fMaxTime)
	{
		g_stPktStatic.fMaxTime = uiInterval;
	}
	*/
	/* 解析ICMP响应报文 */
	iRet = parseIcmp(&stFromAddr, (char *)aucRecvBuf, iRecvLen);
	if (0 > iRet)
	{
		free(aucRecvBuf);
		return -1;
	} else if(2 == iRet) {
		debug_printf(LOG_LEVEL_ERROR,"id=%d,redo %d times\n",pthread_self(),count++);
		goto REDO;
	}else if(3 == iRet) {
		debug_printf(LOG_LEVEL_ERROR,"ping self,id=%d,redo %d times\n",pthread_self(),count++);
		goto REDO;
	}
	
	
	free(aucRecvBuf);
	
	return iRet;

}

/* 发送ICMP报文 */

int
sendIcmp(const int fd, const char *dip)
{

	unsigned char ucEchoNum = 1;
	int iPktLen = 0;
	int iRet = 0;
	struct sockaddr_in pstDestAddr;
	
	unsigned char *aucSendBuf = (unsigned char *)malloc(ICMP_BUFF_LEN);


	memset(&pstDestAddr, 0, sizeof(pstDestAddr));	
	memset(aucSendBuf, 0,ICMP_BUFF_LEN);	
	inet_pton(AF_INET, dip, &pstDestAddr.sin_addr);

	/*if(-1 ==connect(fd,(struct sockaddr *)&pstDestAddr,sizeof(struct sockaddr))){
                debug_printf(LOG_LEVEL_ERROR,"sendIcmp:build_tcp_connection:connect  failed,dip=%s\n",dip);
                return ERROR;
        }*/


	iPktLen = new_icmp_echo(ucEchoNum,aucSendBuf,ICMP_DATA_LEN);

	/* 发送ICMP-ECHO报文 */
	pstDestAddr.sin_family = AF_INET;
	iRet = sendto(fd, aucSendBuf, iPktLen, 0, (struct sockaddr *)&pstDestAddr, sizeof(struct sockaddr_in));
	//iRet = send(fd, aucSendBuf, iPktLen, 0);
	if(0 > iRet)
	{
		debug_printf(LOG_LEVEL_ERROR,"Send ICMP Error,iRet = %d,errno=%d\n",iRet,errno);
		free(aucSendBuf);
		return -1;
	}

	/* 等待接收响应报文 */
	iRet = recvIcmp(fd,&pstDestAddr);

	free(aucSendBuf);
	return iRet;

}

int 
create_client_raw_socket()
{
	int rtn = 0;
	int fd;
	struct timeval timeout;
	int fdbuflen;
	
	
	fd = socket(PF_INET, SOCK_RAW,IPPROTO_ICMP);
	if(fd < 0) {

		debug_printf(LOG_LEVEL_ERROR,"create_client_raw_socket: socket failed,errno=%d\n",errno);
		return ERROR;
	}
	
	   
	/*fdbuflen = 1024*1024;
	rtn = setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &fdbuflen, sizeof(int));
	if(rtn < 0){
		debug_printf(LOG_LEVEL_ERROR,"setsockopt SO_RCVBUF failed \n");
		return ERROR;
	}*/
	
	fdbuflen = 1024*1024*256;//1
	rtn = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &fdbuflen, sizeof(int));
	if(rtn < 0){
		debug_printf(LOG_LEVEL_ERROR,"setsockopt SO_SNDBUF failed,errno=%d\n",errno);
		close(fd);
		return ERROR;
	}

	timeout.tv_sec=0;
	timeout.tv_usec = 1000*50;
	rtn = setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,(char *)&timeout,sizeof(struct timeval));
	if(rtn < 0){
		debug_printf(LOG_LEVEL_ERROR,"create_client_raw_socket:setsockopt SO_RCVTIMEO failed,errno=%d\n",errno);
		close(fd);
		return ERROR;
    }


	timeout.tv_sec=0;
	timeout.tv_usec = 1000;
	rtn = setsockopt(fd,SOL_SOCKET,SO_SNDTIMEO,(char *)&timeout,sizeof(struct timeval));
	if(rtn < 0){
		debug_printf(LOG_LEVEL_ERROR,"create_client_raw_socket:setsockopt SO_SNDTIMEO failed,errno=%d\n",errno);
		close(fd);
		return ERROR;
	}

	
	return fd;
}

