/**
* @file         dataswitch.h
* @brief        data switch
* @details      ����ת��
* @author       ������
* @date         2016/06/20
* @version      2016/06/20
* @par Copyright (c):
*    kedacom
* @par History:
*   version:
*/

#ifndef DATASWITCH_H        ///< ͷ�ļ���,��ֹ�ذ���
#define DATASWITCH_H        ///< ͷ�ļ���,��ֹ�ذ���

#define DSVERSION       "Dataswitch_64bit v1.1.0 2008.4.24" ///< �汾

///////////////////////////////////////////////////////////////////
//    ����ϵͳ�޹غ궨��
///////////////////////////////////////////////////////////////////
// for Microsoft c++
#ifdef _MSC_VER

    //������
    #ifndef _EQUATOR_
        #pragma pack(push)
        #pragma pack(1)
    #endif

    #ifdef __cplusplus
    #define DS_DS_API extern "C" __declspec(dllexport)
    #else
    #define DS_DS_API __declspec(dllexport)
    #endif

// for gcc
#else
    #ifdef __cplusplus
    #define DS_API extern "C"
    #else
    #define DS_API
    #endif

#endif    // _MSC_VER


/// #include "osp.h"
#include "oscbb.h"

/** DataSwitch ��� */
#define DSID    uint32_t

/** DataSwitch ����ֵ*/
#define DSOK    1
#define DSERROR 0

#define INVALID_DSID  0xffffffff   //<��Ч�ľ��ֵ


/**
 * ��ַ���ͽṹ
 * dwIP : IP��ַ,������
 * wPort : �˿�,������
 */
typedef struct
{
	uint32_t dwIP;
	uint16_t wPort;
}TDSNetAddr;

/**
 * UDP ���Ľ��ջص���������
 * pPackBuf : ���Ļ����ַ
 * wPackLen : ���ĳ���
 * ptDstAddr : ���ؽ��յ�ַ
 * ptSrcAddr : ����Դ��ַ
 * qwTimeStamp : ʱ���
 * dwContext : �û��ص�������
 */
typedef void* (*DSUDPPACKCALLBACK)(uint8_t* pPackBuf, uint16_t wPackLen, TDSNetAddr* ptDstAddr,
                                    TDSNetAddr* ptSrcAddr, uint64_t qwTimeStamp, void* pvContext);


/**
 *@brief ���ý��ձ��Ļص�
 *@param[in]   const TDSNetAddr* ptRcvAddr ���յ�ַ
 *@param[in]   DSUDPPACKCALLBACK pUdpCallBack �ص�����
 *@param[in]   void* pvContext �ص�������
 *@return      DSOK:�ɹ� ������:ʧ��
 *@ref
 *@note
 */
DS_API uint32_t dsRegRcvChannel(const TDSNetAddr* ptRcvAddr, DSUDPPACKCALLBACK pUdpCallBack, void* pvContext);


 /**
 *@brief �Ƴ����ձ��Ļص�
 *@param[in]   const TDSNetAddr* ptRcvAddr ���յ�ַ
 *@param[in]   DSUDPPACKCALLBACK pUdpCallBack �ص�����
 *@param[in]   void* pvContext �ص�������
 *@return      DSOK:�ɹ� ������:ʧ��
 *@ref
 *@note
 */
DS_API uint32_t dsUnRegRcvChannel(const TDSNetAddr* ptRcvAddr, DSUDPPACKCALLBACK pUdpCallBack, void* pvContext);


/**
 * @func FilterFunc
 * @brief ���չ��˺���
 *
 * ÿ�����ս����һ�����˺��������˽����յ�UDP���ݰ�ʱִ�д˺�����
 * �����ݺ����ķ���ֵ������̬�ؾ����Ƿ�Դ����ݰ�ת����
 *
 * @param ptRecvAddr        - ���յ�ַ
 * @param ptSrcAddr         - Դ��ַ
 * @param pData          - [in, out]���ݰ��������޸ģ�
 * @param uLen           - [in, out]���ݰ����ȣ��޸ĺ�ĳ��Ȳ��ɳ������ֵ
 * @return DSOK, �����ݰ���Ч�� ����ֵ�������ݰ���Ч��
 */
typedef uint32_t (*FilterFunc)(uint32_t dwRecvIP, uint16_t wRecvPort, uint32_t dwSrcIP, uint16_t wSrcPort, uint8_t* pData, uint32_t uLen);


#define SENDMEM_MAX_MODLEN        (uint8_t)32        ///<����ʱ���������޸ĳ���


/** �����鶨��*/
typedef struct tagNetSndMember
{
	TDSNetAddr  tDstAddr;               //<ת��Ŀ�ĵ�ַ
    int64_t         lIdx;                   //<�ӿ�����
    uint32_t         errNum;                 //<�������
    int64_t         errNo;                  //<�����
    void *      lpuser;                 //<user info
    uint16_t         wOffset;                //<�޸Ĳ��ֵ���ʼλ��; Ŀǰδʹ��
    uint16_t         wLen;		            //<�޸Ĳ��ֵĳ��ȣ�����Ϊ4�ı�����Ϊ���ʾ���޸�
    uint8_t     pNewData[SENDMEM_MAX_MODLEN]; //<�û��Զ�������
    void *      pAppData;                     //<�û��Զ���Ļص������Ĳ���
}TNetSndMember;


/**
 *@brief  �������ݻص�
 *@param[in]  uint32_t dwRecvIP  ������ip
 *@param[in]  uint16_t wRecvPort ������˿�
 *@param[in]  uint32_t dwSrcIP   Դip
 *@param[in]  uint16_t wSrcPort  Դ�˿�
 *@param[in]  TNetSndMember *ptSends    ת��Ŀ���б�
 *@param[in]  u16* pwSendNum            ת��Ŀ�����
 *@param[in]  u8* pUdpData      ���ݱ���
 *@param[in]  uint32_t dwUdpLen      ���ĳ���
 *@return
 *@ref
 *@see
 *@note
 */
typedef void (*SendCallback)(uint32_t dwRecvIP, uint16_t wRecvPort, uint32_t dwSrcIP, uint16_t wSrcPort, TNetSndMember *ptSends,
                             uint16_t* pwSendNum,
                             uint8_t* pUdpData, uint32_t dwUdpLen);


/**
 *@brief ���ýӿڳ�ʱʱ��
 *@param[in]   int timeout : ʱ��,��λ:ms
 *@return
 *@ref
 *@note
 */
DS_API void dsSetApiTimeOut(int32_t timeout);


/**
 *@brief ����ds������
 *@param[in]   uint32_t dwMaxRcvGrp  ����������Ŀ
 *@param[in]   uint32_t dwMaxSndMmbPerRcvGrp  ����������������õ�ת������Ŀ
 *@return      DSOK:�ɹ� ������:ʧ�ܣ�
 *@ref
 *@see
 *@note
 */
DS_API uint32_t dsSetCapacity( uint32_t dwMaxRcvGrp , uint32_t dwMaxSndMmbPerRcvGrp );


/**
 *@brief ����DataSwitchģ��
 *@param[in]   uint32_t num      �ӿ�IP�������
 *@param[in]   uint32_t adwIP[]  �ӿ�IP����
 *@param[in]   const char* pszSendIf  ָ���ķ����豸����
 *@return      DSID:�û�id,ʧ�ܷ���INVALID_DSID
 *@ref
 *@see
 *@note
 */
DS_API DSID dsCreate( uint32_t num, uint32_t adwIP[], const char* pszSendIf = NULL);


/**
 *@brief ����DataSwitchģ��
 *@param[in]   dsId  - �û���ʶ
 *@return
 *@ref
 *@see
 *@note
 */
DS_API void dsDestroy( DSID dsId );


/**
 *@brief ����ת��Ŀ��Ψһ��ת������
 *@param[in]   dsId         �û���ʶ
 *@param[in]   tRecvAddr    ���յ�ַ�����������ݰ���Ŀ��IP
 *@param[in]   dwInLocalIP  �������ݰ�������ӿ�
 *@param[in]   tSendToAddr  ת��Ŀ�ĵ�ַ
 *@param[in]   dwOutLocalIP ת�����ݰ����ñ���IP
 *@return       DSOK: ��ʾ�ɹ� ; DSERROR: ��ʾʧ��
 *@ref
 *@see
 *@note ����Ѿ����ڵĽ���������ת��Ŀ���뵱ǰ�������ͬ������ɾ����Щ����
 */
DS_API uint32_t dsAdd(DSID dsId, TDSNetAddr tRecvAddr, uint32_t dwInLocalIP, TDSNetAddr tSendToAddr, uint32_t dwOutLocalIP = 0);


/**
 *@brief ɾ��ת��Ŀ��Ϊָ����ַ��ת������
 *@param[in]   dsId         �û���ʶ
 *@param[in]   tSendToAddr  ת��Ŀ�ĵ�ַ
 *@return       DSOK: ��ʾ�ɹ� ; DSERROR: ��ʾʧ��
 *@ref
 *@see
 *@note
 */
DS_API uint32_t dsRemove(DSID dsId, TDSNetAddr tSendToAddr);


/**
 *@brief ����Dump����
 *@param[in]   dsId          �û���ʶ
 *@param[in]   tRecvAddr     ���յ�ַ�����������ݰ���Ŀ��IP��Port
 *@param[in]   dwInLocalIP   �������ݰ�������ӿ�
 *@return       DSOK: ��ʾ�ɹ� ; DSERROR: ��ʾʧ��
 *@ref
 *@see
 *@note ��ָ����ַ���յ����ݰ�����ת�������һ�����յ�ַֻ��DUMP��������յ����ݰ�����ת����
 *      �����������ת������������������ת����
 */
DS_API uint32_t dsAddDump(DSID dsId, TDSNetAddr tRecvAddr, uint32_t dwInLocalIP);


/**
 *@brief ɾ��Dump����
 *@param[in]   dsId          �û���ʶ
 *@param[in]   tRecvAddr     ���յ�ַ�����������ݰ���Ŀ��IP��Port
 *@return       DSOK: ��ʾ�ɹ� ; DSERROR: ��ʾʧ��
 *@ref
 *@see
 *@note
 */
DS_API uint32_t dsRemoveDump(DSID dsId, TDSNetAddr tRecvAddr);


/**
 *@brief ���Ӷ��һ��ת������
 *@param[in]    dsId          �û���ʶ
 *@param[in]    tRecvAddr     ���յ�ַ�����������ݰ���Ŀ��IP��Port
 *@param[in]    dwInLocalIP   �������ݰ�������ӿ�
 *@param[in]    tSendToAddr   ת��Ŀ��IP��Port
 *@param[in]    dwOutLocalIP  ת�����ݰ����ñ���IP
 *@return       DSOK: ��ʾ�ɹ� ; DSERROR: ��ʾʧ��
 *@ref
 *@see
 *@note
 */
DS_API uint32_t dsAddManyToOne(DSID dsId , TDSNetAddr tRecvAddr, uint32_t  dwInLocalIP, TDSNetAddr tSendToAddr, uint32_t  dwOutLocalIP = 0);


/**
 *@brief ɾ������ת��Ŀ��Ϊָ����ַ�Ķ��һ����
 *@param[in]    dsId          �û���ʶ
 *@param[in]    tSendToAddr   ת��Ŀ��IP��Port
 *@return       DSOK: ��ʾ�ɹ� ; DSERROR: ��ʾʧ��
 *@ref
 *@see
 *@note
 */
DS_API uint32_t dsRemoveAllManyToOne(DSID dsId , TDSNetAddr tSendToAddr);


/**
 *@brief ɾ��ָ���Ķ��һ����
 *@param[in]    dsId          �û���ʶ
 *@param[in]    tRecvAddr     ���յ�ַ�����������ݰ���Ŀ��IP��Port
 *@param[in]    tSendToAddr   ת��Ŀ��IP��Port
 *@return       DSOK: ��ʾ�ɹ� ; DSERROR: ��ʾʧ��
 *@ref
 *@see
 *@note
 */
DS_API uint32_t dsRemoveManyToOne(DSID dsId , TDSNetAddr tRecvAddr, TDSNetAddr tSendToAddr);


/**
 *@brief ���Ӱ�Դת���Ĺ���
 *@param[in]    dsId          �û���ʶ
 *@param[in]    tRecvAddr     ���յ�ַ�����������ݰ���Ŀ��IP��Port
 *@param[in]    dwInLocalIP   �������ݰ�������ӿ�
 *@param[in]    tSrcAddr      �������ݰ���ԴIP��Port
 *@param[in]    tSendToAddr   ת��Ŀ��IP��Port
 *@param[in]    dwOutLocalIP  ת�����ݰ����ñ���IP
 *@return       DSOK: ��ʾ�ɹ� ; DSERROR: ��ʾʧ��
 *@ref
 *@see
 *@note ʹ�ô˹���ʱ���������ݰ���Դ��ַ����������е�Դ
 *     ��ַ��ȡ�ÿ��ת��ʱ����Ҫ�ȸ������ݰ��е�Դ��ַ����ת����
 * ������Ҳ�������ʹ��Ĭ��Դ��ַ����0@0����ת������
 * ע�⣺�ýӿ�֧�ֶ�㵽һ�㡣���dwSrcIP��wSrcPort��Ϊ�㣬
 * ������ȫ��ͬ��dsAddManyToOne��
 */
DS_API uint32_t dsAddSrcSwitch(DSID dsId , TDSNetAddr tRecvAddr, uint32_t  dwInLocalIP, TDSNetAddr tSrcAddr, TDSNetAddr tSendToAddr, uint32_t  dwOutLocalIP = 0);


/**
 *@brief ɾ������ָ���İ�Դת������
 *@param[in]    dsId          �û���ʶ
 *@param[in]    tRecvAddr     ���յ�ַ�����������ݰ���Ŀ��IP��Port
 *@param[in]    tSrcAddr      �������ݰ���ԴIP��Port
 *@return       DSOK: ��ʾ�ɹ� ; DSERROR: ��ʾʧ��
 *@ref
 *@see
 *@note
 */
DS_API uint32_t dsRemoveAllSrcSwitch(DSID dsId, TDSNetAddr tRecvAddr, TDSNetAddr tSrcAddr);


/**
 *@brief ɾ��ָ���İ�Դת���Ĺ���
 *@param[in]    dsId          �û���ʶ
 *@param[in]    tRecvAddr     ���յ�ַ�����������ݰ���Ŀ��IP��Port
 *@param[in]    tSrcAddr      �������ݰ���ԴIP��Port
 *@param[in]    tSendToAddr   ת��Ŀ��IP��Port
 *@return       DSOK: ��ʾ�ɹ� ; DSERROR: ��ʾʧ��
 *@ref
 *@see
 *@note
 */
DS_API uint32_t dsRemoveSrcSwitch(DSID dsId, TDSNetAddr tRecvAddr,  TDSNetAddr tSrcAddr,  TDSNetAddr tSendToAddr);


/**
 *@brief �����û�����
 *@param[in]    dsId          �û���ʶ
 *@param[in]    tLocalAddr    ���յ�ַ�����������ݰ���Ŀ��IP��Port
 *@param[in]    dwInLocalIP    �������ݰ�������ӿ�
 *@param[in]    tSrcAddr      �������ݰ���ԴIP��Port
 *@param[in]    tDstAddr      Ŀ��IP��Port
 *@param[in]    dwOutLocalIP  ת�����ݰ����ñ���IP
 *@param[in]    bSend       ���Ƿ���Ҫ����
 *@param[in]    pbyUserData   �û�����buffer
 *@param[in]    nUserLen      �û����ݳ���
 *@return       DSOK: ��ʾ�ɹ� ; DSERROR: ��ʾʧ��
 *@ref
 *@see
 *@note
 */
DS_API uint32_t dsSetUserData(DSID dsId,
					  TDSNetAddr tLocalAddr,
					  uint32_t        dwLocalIfIp,
					  TDSNetAddr tSrcAddr,
					  TDSNetAddr tDstAddr,
					  uint32_t        dwDstOutIfIP,
					  bool          bSend,
                      uint8_t* pbyUserData,
                      int32_t nUserLen);


/**
 *@brief ���ù��˺���
 *@param[in]    dsId          �û���ʶ
 *@param[in]    tRecvAddr     ���յ�ַ�����������ݰ���Ŀ��IP��Port
 *@param[in]    ptFunc        ���˺���ָ��
 *@return       DSOK: ��ʾ�ɹ� ; DSERROR: ��ʾʧ��
 *@ref
 *@see
 *@note ����ָ�����Ϊ�գ���ʾ���ԭ�е����ã����⣬����
 * DataSwitch����������߳��е��ô˺����ģ�Ҫ���õĺ�����
 * ����ȫ�ֺ��������õĲ�����Ҳ������ȫ����Ч�ġ�
 */
DS_API uint32_t dsSetFilterFunc(DSID dsId, TDSNetAddr tRecvAddr, FilterFunc ptFunc);


/**
 *@brief ɾ�����е�ת������
 *@param[in]    dsId          �û���ʶ
 *@param[in]    bKeepDump     �Ƿ������е�dump����Ĭ�ϲ�����
 *@return       DSOK: ��ʾ�ɹ� ; DSERROR: ��ʾʧ��
 *@ref
 *@see
 *@note
 */
DS_API uint32_t dsRemoveAll( DSID dsId, bool bKeepDump = false);


/**
 *@brief Ϊָ�����յ�ַ����ת�����ݰ�������Դ��ַ(���ڽ�����αװת��Դ)
 *@param[in]    dsId          �û���ʶ
 *@param[in]    tOrigAddr     ԭʼIP��Port
 *@param[in]    tMappedAddr   ӳ��IP��Port
 *@return       DSOK: ��ʾ�ɹ� ; DSERROR: ��ʾʧ��
 *@ref
 *@see
 *@note
 */
DS_API uint32_t dsSpecifyFwdSrc(DSID dsId, TDSNetAddr tOrigAddr, TDSNetAddr tMappedAddr);


/**
 *@brief �ָ�ָ����ַת�����ݰ���Դ��ַ
 *@param[in]    dsId         �û���ʶ
 *@param[in]    tOrigAddr    ԭʼIP��Port
 *@return       DSOK: ��ʾ�ɹ� ; DSERROR: ��ʾʧ��
 *@ref
 *@see
 *@note
 */
DS_API uint32_t dsResetFwdSrc(DSID dsId, TDSNetAddr tOrigAddr);


/**
 *@brief �������ݱ���Ŀ�ĵ�ַ�ͽ��յ�ַ���ת����Դ��ַ(����ĳһת��ͨ��αװת��Դ)
 *@param[in]    dsId         �û���ʶ
 *@param[in]    tRcvAddr    ���յ�IP��Port
 *@param[in]    tDstAddr    Ŀ��IP��Port
 *@param[in]    tRawAddr    ӳ���IP��Port
 *@return       DSOK: ��ʾ�ɹ� ; DSERROR: ��ʾʧ��
 *@ref
 *@see
 *@note
 */
DS_API uint32_t dsSetSrcAddrByDst(DSID dsId, TDSNetAddr tRcvAddr, TDSNetAddr tDstAddr, TDSNetAddr tRawAddr);

/**
 *@brief �ָ����ڽ��յ�ַ��Ŀ�ĵ�ַ�����ݵ�Դ��ַαװ
 *@param[in]    dsId         �û���ʶ
 *@param[in]    tRcvAddr    ���յ�IP��Port
 *@param[in]    tDstAddr    Ŀ��IP��Port
 *@return       DSOK: ��ʾ�ɹ� ; DSERROR: ��ʾʧ��
 *@ref
 *@see
 *@note
 */
DS_API uint32_t dsResetSrcAddrByDst(DSID dsId, TDSNetAddr tRcvAddr, TDSNetAddr tDstAddr);


/**
 *@brief ���÷��ͻص�����(���ڽ�����)
 *@param[in]    dsId         �û���ʶ
 *@param[in]    tRcvAddr    ���յ�IP��Port
 *@param[in]    tSrcAddr    ԴĿ��IP��Port
 *@param[in]    pfCallback  �ص�����
 *@return       DSOK: ��ʾ�ɹ� ; DSERROR: ��ʾʧ��
 *@ref
 *@see
 *@note
 */
DS_API uint32_t dsSetSendCallback( DSID dsId, TDSNetAddr tRcvAddr, TDSNetAddr tSrcAddr, SendCallback pfCallback);


/**
 *@brief Ϊ����Ŀ������һ���Զ����ָ��
 *@param[in]    dsId         �û���ʶ
 *@param[in]    tRcvAddr    ���յ�IP��Port
 *@param[in]    tSrcAddr    ԴĿ��IP��Port
 *@param[in]    tDstAddr    ת��Ŀ��IP��ַ��Port
 *@param[in]    pAppData    �Զ���ָ��
 *@return       DSOK: ��ʾ�ɹ� ; DSERROR: ��ʾʧ��
 *@ref
 *@see
 *@note
 */
DS_API uint32_t dsSetAppDataForSend( DSID dsId, TDSNetAddr tRcvAddr, TDSNetAddr tSrcAddr, TDSNetAddr tDstAddr, void * pAppData);


/**
 *@brief ��ѯ�����ܰ���
 *@param[in]    dsId            �û���ʶ
 *@param[in]    tRcvAddr        ���յ�IP��Port
 *@param[in]    tSrcAddr        ԴĿ��IP��Port
 *@param[in]    dwRecvPktCount  �����ܰ���
 *@return       DSOK: ��ʾ�ɹ� ; DSERROR: ��ʾʧ��
 *@ref
 *@see
 *@note
 */
DS_API uint32_t dsGetRecvPktCount( DSID dsId , TDSNetAddr tRcvAddr, TDSNetAddr tSrcAddr, uint32_t &dwRecvPktCount );


/**
 *@brief  ��ѯ�����ܰ���
 *@param[in]    dsId            �û���ʶ
 *@param[in]    tRcvAddr        ���յ�IP��Port
 *@param[in]    tSendToAddr     ת��Ŀ��IP��ַ��Port
 *@param[in]    dwSendPktCount  �����ܰ���
 *@return       DSOK: ��ʾ�ɹ� ; DSERROR: ��ʾʧ��
 *@ref
 *@see
 *@note
 */
DS_API uint32_t dsGetSendPktCount( DSID dsId, TDSNetAddr tRcvAddr, TDSNetAddr tSrcAddr, TDSNetAddr tSendToAddr, uint32_t &dwSendPktCount);


/**
 *@brief  ��ѯ�������ֽ���
 *@return       ���ؽ������ֽ���
 *@ref
 *@see
 *@note
 */
DS_API int64_t dsGetRecvBytesCount( );


/**
 *@brief  ��ѯ�������ֽ���
 *@return       ���ط������ֽ���
 *@ref
 *@see
 *@note
 */
DS_API int64_t dsGetSendBytesCount( );


/**
 *@brief  ��ʾ���е�ת�����򣬼��������ڼ����Ķ˿�
 *@return
 *@ref
 *@see
 *@note
 */
DS_API void dsinfo();


/**
 *@brief  ��ʾDataswitch�İ汾��Ϣ
 *@return
 *@ref
 *@see
 *@note
 */
DS_API void dsver();


/**
 *@brief  ��ʾDataswitch���ṩ������İ�����Ϣ
 *@return
 *@ref
 *@see
 *@note
 */
DS_API void dshelp();


/**
 *@brief        ��/�رյ�����Ϣ
 *@param[IN]    op    ָ�������û��������set, clear
 *@return
 *@ref
 *@see
 *@note
 */
DS_API void dsdebug(const char* op = NULL);


/**
 *@brief        ��/�رո���һ��������Ϣ
 *@return
 *@ref
 *@see
 *@note �������ô˺�������Ϊ������Ϣ��Ӱ����������
 */
DS_API void dsdebug2();


/**
 *@brief  ����ĳһ���������
 *@param[in]    dsId            �û���ʶ
 *@param[in]    pszLocalIP      ���յ�IP
 *@param[in]    wLocalPort      ���յ�port
 *@param[in]    dwTraceNum      ���е��ȴ���
 *@return
 *@ref
 *@see
 *@note
 */
DS_API void dstrace(DSID dsId, char* pszLocalIP, uint16_t wLocalPort, uint32_t dwTraceNum);

#endif
