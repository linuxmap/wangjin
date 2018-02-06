/*****************************************************************************
   ģ����      : 
   �ļ���      : sockswitch.h
   ����ʱ��    : 2004�� 3�� 18��
   ʵ�ֹ���    : Windows�汾��DataSwitchʵ��ģ��
   ����        : ������
   �汾        : 3.0

-----------------------------------------------------------------------------
   �޸ļ�¼:
   ��  ��      �汾        �޸���      �޸�����
   04/12       3.0         zmy         Create


******************************************************************************/
#ifndef  _SOCKET_DATA_SWITCH_H_
#define  _SOCKET_DATA_SWITCH_H_

#include "dscommon.h"


#define			DATASWITCH_PORT_START		21050
#define			DATASWITCH_PORT_END			21099


struct TSndMmb
{
	TSndMmb()
	{
		wRawPort = 0;
		dwRawIp = 0;
	}


	uint32_t dwDstIP;   //ת��Ŀ��IP
	uint16_t wDstPort;  //ת��Ŀ��Port
	uint32_t dwDstOutIfIP;//ת�����ڽӿ�IP
	uint32_t dwSrcIP; //ת������ԴIP�����ڸ���Դ��ַת���ı��ĸ�ֵ����������ֵΪ��
	uint16_t wSrcPort;//ת������ԴPort�����ڸ���Դ��ַת���ı��ĸ�ֵ����������ֵΪ��
	uint32_t dwRawIp;
	uint16_t wRawPort;
	uint32_t dwSndPkt;
	DSID dsId;   //�ó�Ա����ģ��
	void* pAppData;
    
    int32_t nUserDataLen;
    uint8_t achUserData[16];

    TSndMmb* ptHashNext;
    
    struct TRcvGrp* ptRcv;

	TSndMmb* ptNextMmb;

};

struct TRcvGrp
{
	bool    bUsing;
	uint32_t     dwDump;
	uint32_t     dwSpying;
	CDSSocket cSock;
	TSndMmb *ptSndMmb;
	uint32_t     dwInstId;//���ĸñ��ĵ�Ӧ��ʵ��ID
    FilterFunc pfFilter;        // ���˺���ָ��
	SendCallback pfCallback;
    int32_t nUserDataLen;
    uint8_t achUserData[DS_MAX_USER_LEN];
    
    uint32_t dwTraceNum; //�Ը�·����trace�İ���
    struct TRcvGrp* ptHashNext;

};

//
// Define the IP header. Make the version and length field one
// character since we can't declare two 4 bit fields without
// the compiler aligning them on at least a 1 byte boundary.
//
typedef struct tagIPHdr
{
    uint8_t  byIPVerLen;        // IP version & IHL
    uint8_t  byIPTos;           // IP type of service
    uint16_t wIPTotalLen;       // Total length
    uint16_t wIPID;             // Unique identifier 
    uint16_t wIPOffset;         // Fragment offset field
    uint8_t  byIPTtl;           // Time to live
    uint8_t  byIPProtocol;      // Protocol(TCP,UDP etc)
    uint16_t wIPCheckSum;       // IP checksum
    uint32_t dwSrcAddr;         // Source address
    uint32_t dwDstAddr;         // Destination address
} TDSIPHdr, *PTDSIPHdr;
//
// Define the UDP header 
//
typedef struct tagUDPHdr
{
    uint16_t wSrcPort;          // Source port number
    uint16_t wDstPort;          // Destination port number
    uint16_t wUDPLen;           // UDP packet length
    uint16_t wUDPCheckSum;      // UDP checksum (optional)
} TDSUDPHdr, *PTDSUDPHdr;

typedef struct tagUDPPsdHdr //����TCPα�ײ�
{
    uint32_t dwSrcAddr;         // Source address
    uint32_t dwDstAddr;         // Destination address
	uint8_t  byMbz;
	uint8_t  byIPProtocol;      // Protocol(UDP==17)
	uint16_t wUDPLen;           // UDP  packet length
} TDSUDPPsdHdr;

/////////////////////////////////
class CSockSwitch
{
public:
	CSockSwitch();
	~CSockSwitch();
	TASKHANDLE  m_hThread;
	static void* DataSwitchTaskProc( void* pParam );
	void* DataSwitchTaskProck();
protected:
	SOCKHANDLE       m_sktInterServ;
	SOCKHANDLE       m_sktInterClient;
	
	bool         m_bInit;
	CDSInterface m_cDSIf;

	uint32_t m_dsSpyId;
	uint32_t m_dwSpyInstId;
	int m_nSpyUDPDataLen;
	uint16_t m_wSpyEvent;
	fd_set m_fdSockSet;

#ifdef USE_EPOLL
	epoll_event* m_ptEpollEvent;
#endif

	SOCKHANDLE m_hDomainSocket;
	SOCKHANDLE m_sktRawSend;
	SOCKADDR_IN m_tAddrIn;
	TDSIPHdr m_tIPHdr;
    TDSUDPHdr m_tUDPHdr;
	char m_szRawPackBuf[64*1024];
	
    //��TODO��20130124 huangzhichun:����TransData�й��˳���Ҫ�ַ�����ʱMnb�б�,�����������е����࣬�������Կ��Ǻϲ���
    TSndMmb** m_aptMmbSameSrc;
    TSndMmb** m_aptMmbDefaultSrc;
	
private:

#ifdef _LINUX_
	int		DomainSocketSend(char* buf,int buflen ,
		uint32_t dwSrcIP ,uint16_t wSrcPort , uint32_t dwDstIP , uint16_t wDstPort);
#endif

	bool        AllocMemory();
	void		FreeMemory();
	void        InitGrpMmbPool();//��ʼ����ͳ�Ա��
	TSndMmb*    AllocMmb( uint32_t  dwDstIP,uint16_t  wDstPort, uint32_t  dwDstOutIfIP );
	void        FreeMmb( TSndMmb* ptSndMmb );

	TRcvGrp*    AllocGrp(uint32_t dwLocalIP ,uint16_t wLocalPort ,uint32_t dwLoalIfIP, uint32_t dsBufSize);
	void        FreeGrp(TRcvGrp* ptGrp );

	//  << ͨ�ſ��ƽӿ�  >>

	bool        CreateInterSock();
	bool        StartupInterServer();//�����ڲ�����

	uint32_t  AddSpy(    DSID dsId , uint32_t dwLocalIP ,uint16_t wLocalPort );
    uint32_t  RemvoeSpy( DSID dsId , uint32_t dwLocalIP ,uint16_t wLocalPort );


	uint32_t Add(DSID dsId ,
		uint32_t  dwLocalIP,
		uint16_t  wLocalPort,
		uint32_t  dwLoalIfIP,
		uint32_t  dwSrcIP ,
		uint16_t  wSrcPort,
		uint32_t  dwDstIP,
		uint16_t  wDstPort,
		uint32_t  dwDstOutIfIP, 
		uint32_t dwBufSize = 2097152);

	uint32_t SetUserData(DSID dsId ,
        uint32_t  dwLocalIP,
        uint16_t  wLocalPort,
        uint32_t  dwLoalIfIP,
        uint32_t  dwSrcIP ,
        uint16_t  wSrcPort,
        uint32_t  dwDstIP,
        uint16_t  wDstPort,
        uint32_t  dwDstOutIfIP,
        bool    bSend,
        uint8_t  *pchUserData,
        int32_t nUserDataLen);
	
	uint32_t Remove( DSID dsId,
		        uint32_t  dwLocalIP,
				uint16_t  wLocalPort,
				uint32_t  dwSrcIP,
				uint16_t  wSrcPort,
				uint32_t  dwDstIP,
				uint16_t  wDstPort);

	uint32_t RemoveAllMmb(DSID dsId,	
		             uint32_t  dwSrcIP,
					 uint16_t  wSrcPort,
					 uint32_t  dwDstIP,
					 uint16_t  wDstPort);

	uint32_t RemoveAll( DSID dsId, bool bRemoveDump = true);//add by jh

	TRcvGrp*   GetRcvGrp(uint32_t dwIP ,uint16_t wPort );
	TSndMmb*   GetSndMmb( TRcvGrp* ptGrp ,uint32_t dwIP ,uint16_t wPort );
	TSndMmb*   GetSndMmb( TRcvGrp* ptGrp ,uint32_t dwSrcIP, uint16_t wSrcPort, uint32_t dwDstIP ,uint16_t wDstPort );
	bool ProssInterMsg();
	bool GetCmdData(uint8_t * pBuf);
	bool TransData( TRcvGrp* ptRcvGrp ,char* buf,int buflen );

	uint16_t CalcChecksum(uint16_t *wBuffer, int32_t nSize);
	int RawSockSend( char* buf,int buflen ,
		uint32_t dwSrcIP ,uint16_t wSrcPort , uint32_t dwDstIP , uint16_t wDstPort );

public://����API�ӿ�
	bool Init();
	uint32_t  SetCmd(TDSCommand * ptCmd, TDSCommand_AttchedData * ptAttached = NULL);
	DSID Create( uint32_t num ,uint32_t adwIP[]);

	void ShowInfo(DSID dsId );
    
	uint32_t SetTrace(DSID dsId, uint32_t dwLocalIP, uint16_t wLocalPort, uint32_t dwTraceNum);

	void ShowInfo();
	void ShowIFInfo();
	void ShowSocket();

	uint32_t GetRecvPktCount( DSID dsId , uint32_t  dwLocalIP , uint16_t wLocalPort ,
		uint32_t dwSrcIP , uint16_t wSrcPort , uint32_t &dwRecvPktCount );
	uint32_t GetSendPktCount( DSID dsId , uint32_t  dwLocalIP , uint16_t wLocalPort ,
		uint32_t dwSrcIP , uint16_t wSrcPort ,
		uint32_t dwDstIP, uint16_t  wDstPort, uint32_t &dwSendPktCount );

	uint32_t  RegSpy( DSID dsId , uint32_t dwInstId , uint16_t wEvent,uint16_t wUDPDataLen);
	uint32_t  UnRegSpy( DSID dsId );
	void Destroy(DSID dsId);

};
#endif/*!_SOCKET_DATA_SWITCH_H_*/
