#ifndef __DATA_SWITCH_COMMON_H__
#define __DATA_SWITCH_COMMON_H__

#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <sys/resource.h>

#include "debug.h"

#include "dataswitch.h"

enum E_DS_LOG_LEVEL
{
    DS_LOG_NONE, 
    DS_LOG_ERROR, 
    DS_LOG_DEBUG, 
    DS_LOG_RECV, 
    DS_LOG_SEND, 

    DS_LOG_ALL = (uint32_t)(~0),
};

void dslog(uint32_t level, const char * fmt, ...);


#define IS_MCIP( ip ) (((long)(htonl(ip)) & 0xf0000000) == 0xe0000000)
#define IS_BCIP( ip ) (((long)(htonl(ip)) & 0x000000ff) == 0x000000ff)

#define INVALID_IFIDX   (-1)

#define MINPORT_MCBIND   20300
#define MAXPORT_MCBIND   20399

#define MINPORT_SRVLISTEN  20200
#define MAXPORT_SRVLISTEN  20299

#define DS_MAXNUM_HANLE    4

#define SET_HANDLE(handle ,x )   ( x |=(1<<handle))  /*���þ��λ*/
#define CLR_HANDLE(handle ,x )   ( x &=~(1<<handle)) /*������λ*/
#define IS_HANLDE(handle ,x )    ( x &(1<<handle))   /*�Ƿ�ʹ��*/


#define IsZeroIP( ip )  (ip == 0 )
#define IsSetIP(ip) (ip== 0xffffffff )
#define IsValidIP(ip) (ip!=0 && ip!=0xffffffff)


#define IsZeroPort( port )  (port == 0 )
#define IsSetPort( port) (port == 0xffff )
#define IsValidPort(port) (port!=0 && port!=0xffff)


#define SETSOCKADDR( tSockAddr ,dwIP,wPort )\
{\
	memset(&tSockAddr, 0, sizeof(tSockAddr));\
    tSockAddr.sin_family      = AF_INET; \
    tSockAddr.sin_port        = htons (wPort);\
    tSockAddr.sin_addr.s_addr = dwIP;\
}




#define MAXNUM_DSINTERFACE    4    /* �ӿ�������*/
#define MAXNUM_DSMCEACHIF     32   /* ÿ���ӿ�֧�ֵ�������鲥�����*/
#define MAXNUM_SPY            2    /* ��������˿ں� */

#define DS_MAX_USER_LEN       16   /* ����û����ݳ��� */


#if defined(_LINUX_) && !defined(_EQUATOR_)
#define USE_EPOLL
#endif



//<<  data switch inter API >>
char*    ipString(uint32_t dwIP );
int      getSockErrno();
SOCKHANDLE   createUDPSocket( uint32_t dwIP ,uint16_t wPort, uint32_t dwBufSize  );
SOCKHANDLE   createRawSocket();
uint32_t      joinMulticast( SOCKHANDLE sock ,uint32_t dwMcIP ,uint32_t dwIfIP );
uint32_t      dropMulticast( SOCKHANDLE sock ,uint32_t dwMcIP ,uint32_t dwIfIP );

void     dsInitIDPool();
DSID     dsAllocDSID();
void     dsFreeDSID(DSID dsId );
int      GetDSID( DSID dsId[] ,int nArraySize);
bool     IsExistDSID(DSID dsId);

class CDSSocket
{
	typedef struct tagMulitcastMmb
	{
		uint32_t dwMcIP; /* �鲥���ַ */
		uint32_t dwIfIP;
		uint32_t dwRef;  /* ���鲥�����ô��� */
		struct tagMulitcastMmb *ptNext;
	}TMcMmb;
protected:

	static TMcMmb      m_atMc[256]; /* �ýӿڼ����鲥��״��  */
	static TMcMmb*     m_ptMcFree;
	static bool        m_bMcPoolInit;
	static bool        Init();
	static TMcMmb*     AllocMcMmb();
	static void        FreeMcMmb( TMcMmb* ptMmb);

protected:
	SOCKHANDLE  m_sock;           /* Socket            */
	bool    m_bBC;            /* �Ƿ�֧�ֹ㲥����  */
	uint32_t     m_dwIP;           /* ����IP            */
	uint16_t     m_wPort;          /* ���ն˿ں�        */
	uint32_t     m_dwMapIP;        /* ӳ��IP            */
	uint16_t     m_wMapPort;       /* ӳ��˿ں�        */
	uint32_t     m_dwIfIP;         /* �ӿ�IP��ַ        */
	uint32_t     m_dwRcvPkt;       /* ���հ���          */
	uint32_t     m_dwRcvBytes;     /* �����ֽ���        */
	TMcMmb* m_ptMcMmb;        /* �����鲥������    */

	void    InitInterVal();   /* ��ʼ���ڲ���Ա */
public:
	CDSSocket();
	~CDSSocket(){};
	bool Create( uint32_t dwIP ,uint16_t wPort ,uint32_t dwIfIP, uint32_t dwBufSize );
	bool Create();
	void Destroy();
	bool JoinMc( uint32_t dwMcIP ,uint32_t dwIfIP );
	bool DropMc( uint32_t dwMcIP ,uint32_t dwIfIP );
	
	bool SetBC();
	void Show();
	void ShowMc(uint32_t dwIfIP );

	SOCKHANDLE GetSock(){ return m_sock; };
	uint32_t GetIP(){ return m_dwIP; }
	uint16_t GetPort(){ return m_wPort;  }
	uint32_t GetIfIP(){ return m_dwIfIP; }
	uint32_t GetRcvPkt(){ return m_dwRcvPkt; }

	uint32_t GetMapIP(){ return m_dwMapIP; }
	uint16_t GetMapPort(){ return m_wMapPort;  }
	void SetMapIP( uint32_t dwMapIP )
	{
		m_dwMapIP = dwMapIP;
	}
	void SetMapPort( uint16_t wMapPort )
	{
		m_wMapPort = wMapPort;
	}	
	
	bool IsEqual(uint32_t dwIP ,uint16_t wPort )//�ж��Ƿ�Ϊ��Socket
	{
		return dwIP == m_dwIP && wPort == m_wPort;
	}

	int RecvFrom(char* buf,int len,unsigned long &dwFromIP ,unsigned short &wFromPort )
	{
		SOCKADDR_IN from;
		int fromlen = sizeof(from);
		int recvlen;
	#ifdef _LINUX_
		recvlen = recvfrom( m_sock ,buf,len,0,(SOCKADDR*)&from,(socklen_t*)&fromlen );
	#else
		recvlen = recvfrom( m_sock ,buf,len,0,(SOCKADDR*)&from,&fromlen );
	#endif
		if( recvlen > 0)
		{
			m_dwRcvPkt++;
			m_dwRcvBytes +=recvlen;
			dwFromIP   = from.sin_addr.s_addr;
			wFromPort  = from.sin_port;
            wFromPort  = ntohs(wFromPort);
		}
        else if (recvlen < 0)
        {
		    dslog(DS_LOG_RECV, "udp socket recvfrom fail : %d(%s)\n", errno, strerror(errno));
        }
		return recvlen;
	}



	int SendTo(char* buf,int len,uint32_t dwToIP ,uint16_t wToPort )
	{
	    SOCKADDR_IN to;
		SETSOCKADDR( to ,dwToIP,wToPort);
		int nSndLen = sendto(m_sock, buf, len, 0, (SOCKADDR*)&to, sizeof(to));
	    if ( nSndLen < 0 )
	    {
		    dslog(DS_LOG_SEND, "udp socket send fail : %d(%s)\n", errno, strerror(errno));
	    }
	    else if (nSndLen < len)
	    {
		    dslog(DS_LOG_SEND, "udp socket send less : %d/%d\n", nSndLen, len);
	    }

        return nSndLen;
	}

};

/**
 * CDSInterface��������ģ������������б���IP��
 * ��Ϊת������Դ��ַ��Ŀ�ĵ�ַ���п��ܲ���ͬһ�����Σ�����DS����Ҫ�ж��IP��
 * ���ڶ�����μ�ת����
 * 
 */
class CDSInterface
{
	typedef struct tagTIPInterface
	{
		long lIfIdx;			/* ����?? */
		uint32_t  dwIP;			/* ����IP */
		uint32_t  dwRef;			/* ʹ�ô�IP�Ľ���ģ������� */
		bool dsId[DS_MAXNUM_HANLE+1];   /* ��������ģ���Ƿ�ʹ�ø�IP */
	}TIPIf;
protected:
	TIPIf m_atIPIf[MAXNUM_DSINTERFACE];
	uint32_t  dwIfNum;				/* ʵ��IP���� */
public:
	CDSInterface(){memset( this ,0 ,sizeof(CDSInterface));}
    
	bool Add( uint32_t dwIP ,DSID dsId );
	void Remove( DSID dsId );
	int  GetIP( DSID dsId ,uint32_t adwIP[] ,uint32_t dwArrySize);
	int  GetIP( uint32_t adwIP[] ,uint32_t dwArrySize);
	long GetIfIdx(uint32_t dwIP );		/* ͨ��IP ��ȡ�ӿ����� */
};
///////////////////////////////////////
#define  DSCMD_CREATE       1
#define  DSCMD_DESTORY      2
#define  DSCMD_ADD          3
#define  DSCMD_REMOVE       4
#define  DSCMD_ADD_M2ONE     5
#define  DSCMD_RM_M2ONE      6
#define  DSCMD_RM_ALLM2ONE   7
#define  DSCMD_ADD_SRC_M2ONE     8
#define  DSCMD_RM_SRC_M2ONE      9
#define  DSCMD_RM_SRC_ALLM2ONE   10
#define  DSCMD_ADD_DUMP          11
#define  DSCMD_RM_DUMP           12
#define  DSCMD_ADD_SPY           13
#define  DSCMD_RM_SPY           14
#define  DSCMD_RM_ALL           15

#define  DSCMD_SET_CALLBACK       16
#define  DSCMD_SET_APPDATA       17

#define  DSCMD_SET_MAP_ADDR       18

#define  DSCMD_SET_USERDATA    19
#define  DSCMD_SET_SRC_ADDR_BY_DST	22
#define  DSCMD_RM_ALL_EXCEPT_DUMP_FILTER 23
#define  DSCMD_RM_ALL_EXCEPT_DUMP        25

#define		DS_CMD_MAGIC1		0x23
#define		DS_CMD_MAGIC2		0xCD
#define		DS_CMD_MAGIC3		0x52
#define		DS_CMD_MAGIC4		0xFE

typedef struct 
{
	uint8_t	m_Magic1;
	uint8_t	m_Magic2;
	uint8_t	m_Magic3;
	uint8_t	m_Magic4;
	uint32_t    m_dwMsgLen;
	uint32_t    m_sn ;
	uint32_t    m_time;
} TCmdHead;

typedef struct 
{
    bool bSend;
    int nUserDataLen;
    uint8_t achUserData[DS_MAX_USER_LEN];
} TUserData;

typedef struct 
{
    int nAttchedDataLen;
    union
    {
        TUserData tUserData;
    } uAttchedData;
} TDSCommand_AttchedData;

struct TDSCommand
{
	uint32_t dwCmd;
	
	DSID dsId;
	uint32_t dwLocalIP;
	uint16_t wLocalPort;
	uint32_t dwLocalIfIP;

	uint32_t dwSrcIP;
	uint16_t wSrcPort;

	uint32_t dwDstIP;
	uint16_t wDstPort;
	uint32_t dwDstIfIP;

	uint16_t wSpyDataLen;
	uint16_t wAttatchEvent;
	uint32_t wAttatchInstId;
	uint64_t llContext;
};

DS_API void ddlevel(uint32_t level);

#endif/*!__DATA_SWITCH_COMMON_H__*/
