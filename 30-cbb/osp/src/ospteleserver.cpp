/******************************************************************************
ģ����	�� OSP
�ļ���	�� OSP.h
����ļ���
�ļ�ʵ�ֹ��ܣ�OSP Telenet����������Ҫʵ���ļ�
����	�����Ľ�
�汾	��1.0.02.7.5
---------------------------------------------------------------------------------------------------------------------
�޸ļ�¼:
��  ��		�汾		�޸���		�޸�����
09/15/98		1.0      ĳĳ        ------------
******************************************************************************/

#include "../include/ospSch.h"
#include "../include/ospLog.h"
#include "../include/OspTeleServer.h"

#include "string.h"
#include "stdio.h"
#include "io.h"
#include <windows.h>
#include <process.h>
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>

#include <conio.h>

#define LOCAL_ADDR_UL inet_addr("127.0.0.1")
extern char g_achModuleName[MAX_MODULE_NAME_LENGTH+1];

SOCKHANDLE sockClient = INVALID_SOCKET;
SOCKHANDLE sockTelSer = INVALID_SOCKET;
u16 g_wportListtening = 0;
u32 g_PromtState = PROMTUSER;
s8 g_TelnetUsername[AUTHORIZATION_NAME_SIZE]= {0};
s8 g_TelnetPasswd[AUTHORIZATION_NAME_SIZE] = {0};
BOOL32 g_UsernamePass = FALSE;

BOOL32 g_bIstelnetLocalIP = TRUE;
SEMHANDLE g_TelnetModeSem = NULL;
u16 g_wUserTelnetPort = 0;



/* shell���̾�� */
HANDLE hShellProc = NULL;
HANDLE hShellThread = NULL;

/* ģ�鶯̬ע��� */
HMODULE ahRegModule[MAX_MOD_NUM];

/* ��֪ģ��� */
char *pachModTable[MAX_MOD_NUM]={"OspDll.dll"};

TASKID g_dwTeletTaskID;

API void stdoutFlush();
API void OspTelePipe(u32 queHandle);
API void TeleSockEcho();
static void SendIAC(s8 cmd, s8 opt)
{
    u8 buf[5];
    buf[0] = TELCMD_IAC;
    buf[1] = cmd;
    buf[2] = opt;
/*	telSend((s8*)buf, 3);*/
    OspLogTeleCmdEcho((s8*)buf, 3);
}

enum tel_state
{
    tel_normal = 0,
    tel_nego = 1,
    tel_sub = 2
};
static s32 seen_iac = 0;
static enum tel_state state;
static s32 count_after_sb = 0;
static s8 remove_iac(u8 c)
{
    s8 ret = 0;
    if ((c == 255) && !seen_iac)    /* IAC*/
    {
        seen_iac = 1;
        return ret;
    }

    if (seen_iac)
    {
        switch(c)
        {
        case 251:
        case 252:
        case 253:
        case 254:
            if (state != tel_normal) {
                printf(" illegal negotiation.\n");
            }
            state = tel_nego;
            break;
        case 250:
            if (state != tel_normal){
                printf(" illegal sub negotiation.\n");
            }
            state = tel_sub;
            count_after_sb = 0;
            break;
        case 240:
            if (state != tel_sub) {
                printf(" illegal sub end.\n");
            }
            state = tel_normal;
            break;
        default:
            if (!((c > 240) && (c < 250) && (state == tel_normal)))
            {
                printf("illegal command.\n");
            }
            state = tel_normal;
        }
        seen_iac = 0;
        return 0;
    }

    switch (state)
    {
    case tel_nego:
        state = tel_normal;
        break;
    case tel_sub:
        count_after_sb++; /* set maximize sub negotiation length*/
        if (count_after_sb >= 100) state = tel_normal;
        break;
    default:
        ret = c;
    }
    return ret;
}

static void CreatSocketOnTelnetPort(u32 dwAddr,u16 wPort)
{
    u16 port = 0 ;
    if (INVALID_SOCKET != sockTelSer)
	{
		SockClose(sockTelSer);
		sockTelSer = INVALID_SOCKET;
	}
	/* ���ָ���˿ںţ���ΪTelnet�������ڸö˿ں��ϴ���һ���׽��� */
	if(wPort != 0)
	{
		sockTelSer = CreateTcpNodeNoRegist(0, wPort); // server's port
		if(sockTelSer != INVALID_SOCKET)
		{
		    g_wportListtening = wPort;
		}
	}

	/* ���δָ���˿ںŻ���ָ���˿ں��ϴ����׽���ʧ�ܣ���OSP���д��� */
	if(sockTelSer == INVALID_SOCKET)
	{
		for(port=MIN_TELSVR_PORT; port<MAX_TELSVR_PORT; port++)
		{
			sockTelSer = CreateTcpNodeNoRegist(0, port); // server's port
			if(sockTelSer != INVALID_SOCKET)
			{
				g_wportListtening = port;
				break;
			}
		}
	}
}

static void OspSetSockNoBlock(SOCKHANDLE hSock)
{
	unsigned long dwOn = TRUE;

	ioctl(hSock, FIONBIO, &dwOn);

	return;
}

API BOOL32 OspTelInit(u16 dwPort)
{
	g_wportListtening = 0;
	BOOL32 bRet =FALSE;

	g_wUserTelnetPort = dwPort;

	bRet = OspSemBCreate(&g_TelnetModeSem);
	if(!bRet)
	{
		return FALSE;
	}
	if(INVALID_SOCKET != sockTelSer)
	{
		SockClose(sockTelSer);
	}

	CreatSocketOnTelnetPort(0,g_wUserTelnetPort);
	if(sockTelSer == INVALID_SOCKET)
	{
		// OspTaskSuspend was not implemented under Linux
		OspSemDelete(g_TelnetModeSem);
		return FALSE;
	}
	/* listen ��socketӦ������Ϊ����������ֹaccept��ʱ���ܵ�����*/
	OspSetSockNoBlock(sockTelSer);
	return TRUE;
}


/*====================================================================
��������OspTeleDaemon
���ܣ�Telnet�������ⲿ������������
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����sockClient: ��Telnet�ͻ����ӵ��׽���,
              sockTelSer: �����׽���,
�������˵����uPort: �����˿ںţ�

����ֵ˵������.
====================================================================*/
API void OspTeleDaemon(u16 wPort)
{
	TASKHANDLE hTele;
	SOCKHANDLE sockeAccept;
    sockaddr_in addrClient;
    int addrLenIn = sizeof(addrClient);
	u32 dwTaskID = 0;
	char cmdChar;
	char command[MAX_COMMAND_LENGTH];
	SOCKHANDLE hTempSock = INVALID_SOCKET;
	u8 cmdLen = 0;

	struct timeval timeout;

	g_wUserTelnetPort = wPort;
	g_bIstelnetLocalIP = TRUE;

    sockClient = INVALID_SOCKET;

    while(TRUE)
    {
		int ret;
		int retLen = 0;
		fd_set m_fdSet;
		SOCKHANDLE maxSocket;

		OspSemTake(g_TelnetModeSem);
		FD_ZERO(&m_fdSet);
		if(INVALID_SOCKET != sockTelSer)
		{
			FD_SET(sockTelSer, &m_fdSet);
		}
		if (sockClient != INVALID_SOCKET)
		{
			FD_SET(sockClient, &m_fdSet);
		}

		if (sockTelSer > sockClient)
		{
			maxSocket = sockTelSer + 1;
		}
		else
		{
			maxSocket = sockClient +1;
		}
		OspSemGive(g_TelnetModeSem);
		timeout.tv_sec = 0;
		timeout.tv_usec = 300000;
		ret = select(maxSocket, &m_fdSet, NULL, NULL, &timeout);
		if (ret == SOCKET_ERROR || 0 == ret)
		{
			if (ret == SOCKET_ERROR)
			{
				printf("TeleDaemon : Telnet Server Select Error %d!!! \r\n",WSAGetLastError());
				OspDelay(300);
			}
			continue;
		}
		if (FD_ISSET(sockTelSer, &m_fdSet))
		{
			sockeAccept = INVALID_SOCKET;
			sockeAccept = accept(sockTelSer, (sockaddr *)&addrClient, &addrLenIn);

			/* ����û�������OspQuit, �˳������� */
			if(g_Osp.m_bKillOsp)
			{
				SockClose(sockClient);
				SockClose(sockeAccept);
				sockClient = INVALID_SOCKET;
				sockeAccept = INVALID_SOCKET;
				OspSemDelete(g_TelnetModeSem);
				g_Osp.DelTask(OspTaskSelfID());
				OspTaskExit();;
			}
			if(INVALID_SOCKET == sockeAccept)
			{
				OspDelay(500);
				continue;
			}
			if(g_bIstelnetLocalIP)
			{
				if(LOCAL_ADDR_UL !=addrClient.sin_addr.S_un.S_addr)
				{
					const char * szInfo = "only localhost can telnet,closed by telnetSvr\n";
					SockSend(sockeAccept, szInfo, strlen(szInfo));
					SockClose(sockeAccept);
					sockeAccept = INVALID_SOCKET;
					continue;
				}
			}
			/* ʹ���µ�����ǰ, �ر�ԭ�������� */
			if(sockClient != INVALID_SOCKET)
			{
				const char * szInfo = "another telnet client log in\n";
				SockSend(sockClient, szInfo, strlen(szInfo));
				SockClose(sockClient);
				sockClient = INVALID_SOCKET;
			}
			sockClient = sockeAccept;
			/* ����TELE���ԣ���ӡ��ӭ���*/
			SendIAC(TELCMD_DO, TELOPT_ECHO);
			SendIAC(TELCMD_DO, TELOPT_NAWS);
			SendIAC(TELCMD_DO, TELOPT_LFLOW);
			SendIAC(TELCMD_WILL, TELOPT_ECHO);
			SendIAC(TELCMD_WILL, TELOPT_SGA);

			/* �����ӭ���� */
			TelePrint("*===============================================================\n");
			TelePrint("��ӭʹ�ÿƴ� Telnet ��������\n");
			TelePrint("*===============================================================\n");

			/* �ָ���Ļ��־��� */
			OspResumeScrnLog();

			/* �����ʾ�� */
			OspDelay(1000);   /* Ϊ��ʹospinit finish���ȴ�ӡ���� */
			g_PromtState = PROMTUSER;
			PromptShow();
		}
		if (FD_ISSET(sockClient, &m_fdSet))
		{
			/* �����ؽ����û����� */
			/* ע�⣺�˴�����recv��Ϊ�����ֶԶ˹ر�����ִ������������
			* ��SockRecv��������Ϊ��һ������������Բ���ʹ�á�
			*/
			hTempSock = sockClient;
			ret = recv(sockClient, &cmdChar, 1, 0);

			/* �ͻ��˹ر� */
			if(ret == 0)
			{
				SockClose(hTempSock);
				sockClient = INVALID_SOCKET;
				continue;
			}

			/* ���˹ر� */
			if(ret == SOCKET_ERROR)
			{
				if (INVALID_SOCKET != hTempSock)
				{
					s32 nError = WSAGetLastError();
					if (nError == 10054)
					{
						SockClose(hTempSock);
						sockClient = INVALID_SOCKET;
					}
				}
				OspTaskDelay(500);
				continue;
			}
			cmdChar = remove_iac(cmdChar);
			/* �����û�����, ���Ե�Telnet�ͻ�����Ļ��, ������������ʵ�����Ӧ */
			switch(cmdChar)
			{
			case CTRL_S:
				OspStopScrnLog();
				TelePrint("\nScreen log disabled temporarily. Use ^R to resume it if necessary.\n\n");
				PromptShow();
				break;

			case CTRL_R:
				OspResumeScrnLog();
				TelePrint("\nScreen log resumed again.\n");
				PromptShow();
				break;

			case RETURN_CHAR:         // �س���
				TelePrint("\r\n");
				CmdParse(command, cmdLen);

				cmdLen = 0;
				memset(command,0,MAX_COMMAND_LENGTH);
				PromptShow();         // ��ʾ��ʾ��
				break;

			case NEWLINE_CHAR:		/* ���з� */
			case UP_ARROW:            // �ϼ�ͷ
			case DOWN_ARROW:          // �¼�ͷ
			case LEFT_ARROW:          // ���ͷ
			case RIGHT_ARROW:         // �Ҽ�ͷ
				break;

			case BACKSPACE_CHAR:         // �˸��
				if(cmdLen <= 0)
				{
					continue;
				}

				cmdLen--;
				if(cmdLen >= 0 && cmdLen < MAX_COMMAND_LENGTH )
				{
					command[cmdLen] = '\0';
				}
				if(g_PromtState != PROMTPWD)
				{
					/* ʹ�����ˣ���һ���ո����ԭ�ַ�����ʹ������ */
					char tmpChar[4] = {0};

					tmpChar[0] = BACKSPACE_CHAR;
					tmpChar[1] = BLANK_CHAR;
					tmpChar[2] = BACKSPACE_CHAR;
					TelePrint(tmpChar);
				}
				break;

			default:
				/* add to command string */
				if(g_PromtState != PROMTPWD)
				{
					char tmpChar[2] = {0};
					tmpChar[0]=cmdChar;
					TelePrint(tmpChar);
				}
				if(cmdLen < MAX_COMMAND_LENGTH)
				{
					command[cmdLen++] = cmdChar;
				}
				else
				{
					TelePrint("\n");
					CmdParse(command, cmdLen);

					PromptShow();         // ��ʾ��ʾ��
					cmdLen = 0;
					memset(command,0,MAX_COMMAND_LENGTH);
				}
				break;
			}
		}
    }
}


/*====================================================================
��������OspSetTeleMode
���ܣ�����telnet��ģʽ
�㷨ʵ�֣�
����ȫ�ֱ�����
�������˵����
dwMode	- Telnetģʽ
TELNET_PORT_OFF: �ر�telnet�˿�
TELNET_LOCAL_ON: �����ض˿ڿ�telnet
TELNET_ALL_ON:   ���غ�Զ�̶�����telnet
����ֵ˵�����ɹ�����TRUE,ʧ�ܷ���FALSE
====================================================================*/
API BOOL32 OspSetTeleMode(u32 dwMode)
{
    BOOL32 bRet = TRUE;

    if(!IsOspInitd())
    {
        return FALSE;
    }
    OspSemTake(g_TelnetModeSem);
    switch(dwMode)
    {
    case TELNET_PORT_OFF:
        {
            if(INVALID_SOCKET != sockClient)
            {
				SockClose(sockClient);
				sockClient = INVALID_SOCKET;
            }
            if(INVALID_SOCKET != sockTelSer)
            {
				SockClose(sockTelSer);
				sockTelSer = INVALID_SOCKET;
            }
        }
        break;
    case TELNET_LOCAL_ONLY:
        {
			if(FALSE == g_bIstelnetLocalIP)
            {
				if(INVALID_SOCKET != sockClient)
				{
					SockClose(sockClient);
					sockClient = INVALID_SOCKET;
				}
            }
			g_bIstelnetLocalIP = TRUE;
            if(INVALID_SOCKET == sockTelSer)
			{
                CreatSocketOnTelnetPort(0,g_wUserTelnetPort);
                if(INVALID_SOCKET == sockTelSer)
                {
                    OspSemGive(g_TelnetModeSem);
                    return FALSE;
                }
            }
        }
        break;
    case TELNET_ALL_ON:
        {
			g_bIstelnetLocalIP = FALSE;
            if(INVALID_SOCKET == sockTelSer)
			{
                CreatSocketOnTelnetPort(0,g_wUserTelnetPort);
                if(INVALID_SOCKET == sockTelSer)
                {
                    OspSemGive(g_TelnetModeSem);
                    return FALSE;
                }
            }
        }
        break;
    default:
        {
            printf("Unknow mode :%u\n",dwMode);
            bRet = FALSE;
        }
        break;
    }
    OspSemGive(g_TelnetModeSem);
    return bRet;
}

/*====================================================================
��������TeleSockEcho
���ܣ�Telnetͨ������, ��Ӧ�ͻ��˵��û�����.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����sockClient: ��Telnet�ͻ����ӵ��׽���,
�������˵����uPort: �����˿ںţ�

����ֵ˵������.
====================================================================*/
API void TeleSockEcho()
{
	int ret;
	char cmdChar;
    char command[MAX_COMMAND_LENGTH];
    SOCKHANDLE hTempSock = INVALID_SOCKET;
    u8 cmdLen = 0;

    while(TRUE)
    {
		/* ��⵽�û�����OspQuit, �˳������� */
		if(g_Osp.m_bKillOsp)
		{
			if(sockClient != INVALID_SOCKET)
			{
				SockClose(sockClient);
				sockClient = INVALID_SOCKET;
			}
			OspTaskExit();
		}

		/* �����ؽ����û����� */
        /* ע�⣺�˴�����recv��Ϊ�����ֶԶ˹ر�����ִ������������
         * ��SockRecv��������Ϊ��һ������������Բ���ʹ�á�
         */
        hTempSock = sockClient;
		ret = recv(sockClient, &cmdChar, 1, 0);

		/* �ͻ��˹ر� */
		if(ret == 0)
		{
			SockClose(hTempSock);
			sockClient = INVALID_SOCKET;
			continue;
		}

		/* ���˹ر� */
		if(ret == SOCKET_ERROR)
		{
            if (INVALID_SOCKET != hTempSock)
            {
                s32 nError = WSAGetLastError();
                if (nError == 10054)
                {
                    SockClose(hTempSock);
					sockClient = INVALID_SOCKET;
                }
            }
			OspTaskDelay(500);
			continue;
		}
        cmdChar = remove_iac(cmdChar);
		/* �����û�����, ���Ե�Telnet�ͻ�����Ļ��, ������������ʵ�����Ӧ */
		switch(cmdChar)
		{
		case CTRL_S:
			OspStopScrnLog();
			OspPrintf(TRUE, FALSE, "\nScreen log disabled temporarily. Use ^R to resume it if necessary.\n\n");
			PromptShow();
			break;

		case CTRL_R:
			OspResumeScrnLog();
			OspPrintf(TRUE, FALSE, "\nScreen log resumed again.\n");
			PromptShow();
			break;

		case RETURN_CHAR:         // �س���
            OspLogTeleCmdEcho("\r\n", 2);
    		CmdParse(command, cmdLen);

            cmdLen = 0;
            memset(command,0,MAX_COMMAND_LENGTH);
            PromptShow();         // ��ʾ��ʾ��
            break;

		case NEWLINE_CHAR:		/* ���з� */
		case UP_ARROW:            // �ϼ�ͷ
		case DOWN_ARROW:          // �¼�ͷ
		case LEFT_ARROW:          // ���ͷ
		case RIGHT_ARROW:         // �Ҽ�ͷ
			break;

		case BACKSPACE_CHAR:         // �˸��
			if(cmdLen <= 0)
			{
				continue;
			}

			cmdLen--;
            if(cmdLen >= 0 && cmdLen < MAX_COMMAND_LENGTH )
            {
                command[cmdLen] = '\0';
            }
            if(g_PromtState != PROMTPWD)
            {
			    /* ʹ�����ˣ���һ���ո����ԭ�ַ�����ʹ������ */
			    char tmpChar[3];

                tmpChar[0] = BACKSPACE_CHAR;
			    tmpChar[1] = BLANK_CHAR;
			    tmpChar[2] = BACKSPACE_CHAR;
                OspLogTeleCmdEcho(tmpChar, 3);
            }
			break;

		default:
			/* add to command string */
            if(g_PromtState != PROMTPWD)
            {
			    OspLogTeleCmdEcho(&cmdChar, 1);
            }
			if(cmdLen < MAX_COMMAND_LENGTH)
			{
				command[cmdLen++] = cmdChar;
			}
			else
			{
				OspPrintf(TRUE, FALSE, "\n");
                CmdParse(command, cmdLen);

                PromptShow();         // ��ʾ��ʾ��
				cmdLen = 0;
                memset(command,0,MAX_COMMAND_LENGTH);
			}
			break;
		}
    }
}

/*====================================================================
��������CheckAuthorization
���ܣ�����û�����Ȩ,������Ӧ״̬����.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����szCmd: �յ��������ַ���,dwCmdlen:�ַ����ĳ���.

����ֵ˵������.
====================================================================*/
API void CheckAuthorization(char *szCmd,u32 dwCmdlen)
{
    char szInput[AUTHORIZATION_NAME_SIZE];
    if((dwCmdlen >=AUTHORIZATION_NAME_SIZE)&&(g_PromtState != PROMTAUTHORIZED))
    {
        OspPrintf(TRUE,FALSE,"Osp:CMD NAME is too long! dwCmdlen=%d\n",dwCmdlen);
        return;
    }
    if(dwCmdlen>0)
    {
        switch(g_PromtState)
        {
        case PROMTUSER:
            {
                strncpy(szInput,szCmd,dwCmdlen);
                szInput[dwCmdlen]='\0';
                if(strcmp(g_TelnetUsername,szInput) == 0)
                {
                      g_UsernamePass = TRUE;
                }
                else
                {
                    g_UsernamePass = FALSE;
                }
                g_PromtState = PROMTPWD;
                break;
            }
        case PROMTPWD:
            {
                strncpy(szInput,szCmd,dwCmdlen);
                szInput[dwCmdlen]='\0';
                if(strcmp(g_TelnetPasswd,szInput) == 0)
                {
                    if(g_UsernamePass == TRUE)
                    {
                        g_PromtState = PROMTAUTHORIZED;
                    }
                    else
                    {
                        g_PromtState = PROMTUSER;
                    }
                }
                else
                {
                    g_UsernamePass = FALSE;
                    g_PromtState = PROMTUSER;
                }
                break;
            }
        default:
            break;
        }
    }
    else
    {
        switch(g_PromtState)
        {
        case PROMTUSER:
            {
                if(strcmp(g_TelnetUsername,"") == 0)
                {
                      g_UsernamePass = TRUE;
                }
                else
                {
                    g_UsernamePass = FALSE;
                }
                g_PromtState = PROMTPWD;
                break;
            }
        case PROMTPWD:
            {
                if(strcmp(g_TelnetPasswd,"") == 0)
                {
                    if(g_UsernamePass == TRUE)
                    {
                        g_PromtState = PROMTAUTHORIZED;
                    }
                    else
                    {
                        g_PromtState = PROMTUSER;
                    }
                }
                else
                {
                    g_UsernamePass = FALSE;
                    g_PromtState = PROMTUSER;
                }
                break;
            }
        default:
            break;
        }
    }

}


/*====================================================================
��������RunCmd
���ܣ������û�����, �����ʵ�����Ӧ.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����szCmd: �յ��������ַ���.

����ֵ˵������.
====================================================================*/
typedef struct
{
	char *paraStr;
	BOOL32 bInQuote;
	BOOL32 bIsChar;
}TRawPara;

API void RunCmd(char *szCmd)
{
	int ret = 0;
    int i;
	s32 (* cmdFunc)(void* para1, void* para2, void* para3, void* para4, void* para5, void* para6, void* para7, void* para8, void* para9, void* para10);
	char *cmd = szCmd;
	void* para[10];
	TRawPara atRawPara[10];
	int paraNum = 0;
	u8 count = 0;
	u8 chStartCnt = 0;
	BOOL32 bStrStart = FALSE;
	BOOL32 bCharStart = FALSE;
	HMODULE hModule;
	u32 cmdLen = strlen(szCmd)+1;

	memset(para, 0, sizeof(para));
	memset(atRawPara, 0, sizeof(TRawPara)*10);
    switch(g_PromtState)
    {
    case PROMTUSER:
    case PROMTPWD:
        {
            CheckAuthorization(szCmd,strlen(szCmd));
            break;
        }
    case PROMTAUTHORIZED:
        {

	        /* �������������� */
	        while( count < cmdLen )
	        {
		        switch(szCmd[count])
		        {
		        case '\'':
			        szCmd[count] = '\0';
			        if(!bCharStart)
			        {
				        chStartCnt = count;
			        }
			        else
			        {
				        if(count > chStartCnt+2)
				        {
					        OspPrintf(TRUE, FALSE, "input error.\n");
					        return;
				        }
			        }
			        bCharStart = !bCharStart;
			        break;

		        case '\"':
			        szCmd[count] = '\0';
			        bStrStart = !bStrStart;
 			        break;

		        case ',':
		        case ' ':
		        case '\t':
		        case '\n':
		        case '(':
		        case ')':
                    if( ! bStrStart )
			        {
				        szCmd[count] = '\0';
			        }
			        break;

		        default:
			        /* ������ַ�Ϊ��Ч�ַ���ǰһ�ַ�ΪNULL����ʾ�ɵ��ʽ�����
			           �µ��ʿ�ʼ */
			        if(count > 0 && szCmd[count-1] == '\0')
			        {
				        atRawPara[paraNum].paraStr = &szCmd[count];
				        if(bStrStart)
				        {
					        atRawPara[paraNum].bInQuote = TRUE;
				        }
				        if(bCharStart)
				        {
					        atRawPara[paraNum].bIsChar = TRUE;
				        }
				        if(++paraNum >= 10)
					        break;
			        }
		        }
		        count++;
	        }

	        if(bStrStart || bCharStart)
	        {
		        OspPrintf(TRUE, FALSE, "input error.\n");
		        return;
	        }

	        for(count=0; count<10; count++)
	        {
		        if(atRawPara[count].paraStr == NULL)
		        {
			        para[count] = 0;
			        continue;
		        }

		        if(atRawPara[count].bInQuote)
		        {
			        para[count] = (void*)atRawPara[count].paraStr;
			        continue;
		        }

		        if(atRawPara[count].bIsChar)
		        {
			        para[count] = (void*)atRawPara[count].paraStr[0];
			        continue;
		        }

		        para[count] = (void*)WordParse(atRawPara[count].paraStr);
	        }

	        /* ��ִ������ */
            if ( strcmp("bye", cmd) == 0 )
            {
                OspPrintf(TRUE, FALSE, "\n  bye......\n");
                OspTaskDelay(500);            // not reliable,
                SockClose(sockClient);
                sockClient = INVALID_SOCKET;
                g_PromtState = PROMTUSER;
		        return;
            }

            if (strcmp("osphelp", cmd) == 0)
            {
		        osphelp();
		        return;
            }

	        /* �鿴�Ƿ�ǰģ���еĺ��� */
	        if (*g_achModuleName)
			{
				hModule = GetModuleHandle(g_achModuleName);
			}
			else
			{
				hModule = GetModuleHandle(NULL);  // ����NULL��ʾ��ǰģ��
			}

            if(hModule != NULL)
            {
		        cmdFunc = (s32 (* )(void*,void*,void*,void*,void*,void*,void*,void*,void*,void*)) GetProcAddress(hModule, cmd);
		        if(cmdFunc != NULL)
		        {
			        ret = cmdFunc(para[0], para[1], para[2], para[3], para[4], para[5], para[6], para[7], para[8], para[9]);
			        OspPrintf(TRUE, FALSE, "\nvalue=%d\n", ret);
			        return;
		        }
            }

	        /* �ٲ鿴�Ƿ�Ǽ�ģ���еĺ��� */
	        for(i=0; i<MAX_MOD_NUM; i++)
	        {
		        if( (hModule = ahRegModule[i]) != NULL &&
			        (cmdFunc = (int (* )(void*,void*,void*,void*,void*,void*,void*,void*,void*,void*))
			                             GetProcAddress(hModule, cmd)) != NULL )
		        {
                    ret = cmdFunc(para[0], para[1], para[2], para[3], para[4], para[5], para[6], para[7], para[8], para[9]);
			        OspPrintf(TRUE, FALSE, "\nvalue=%d\n", ret);
                    return;
		        }
	        }

	        /* �ٲ��Ƿ���֪ģ���еĺ��� */
	        for(i=0; i<MAX_MOD_NUM; i++)
	        {
		        if( pachModTable[i] != NULL &&
			        (hModule = GetModuleHandle(pachModTable[i])) != NULL )
		        {
			        if( (cmdFunc = (int (* )(void*,void*,void*,void*,void*,void*,void*,void*,void*,void*))
				                   GetProcAddress(hModule, cmd)) != NULL )
			        {
				        ret = cmdFunc(para[0], para[1], para[2], para[3], para[4], para[5], para[6], para[7], para[8], para[9]);
				        OspPrintf(TRUE, FALSE, "\nvalue=%d\n", ret);
				        return;
			        }
		        }
	        }

            OspPrintf(TRUE, FALSE, "function '%s' doesn't exist!\n", szCmd);
            break;
        }
    }
    return;
}

/*====================================================================
��������CmdParse
���ܣ������׼���ȥ����ͷ����Ч�ַ���������'\0'.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����pchCmd: ���,
              uCmdLen: �����.

����ֵ˵������.
====================================================================*/
API void CmdParse(const char * pchCmd, u8 byCmdLen)
{
	u8 count = 0;
	int nCpyLen = 0;
    char command[MAX_COMMAND_LENGTH];
    if(byCmdLen > 0)
    {
        //ȥͷ
        for(count=0; count<byCmdLen; count++)
        {
            char chTmp;

            chTmp = pchCmd[count];
            if(IsCharAlphaNumeric(chTmp)|| IsCharLower(chTmp) || IsCharUpper(chTmp))
            {
                break;
            }
        }

	    nCpyLen = byCmdLen-count;
    }
	if(nCpyLen <= 0)
	{
        CheckAuthorization(command,0);
		return;
	}

	memcpy(command, pchCmd+count, nCpyLen);
	if(byCmdLen < MAX_COMMAND_LENGTH)
	{
		command[nCpyLen] = '\0';
	}
	else
	{
		command[MAX_COMMAND_LENGTH-1] = '\0';
	}

    RunCmd(command);
}

/*====================================================================
��������WordParse
���ܣ�ת���������.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����word: �����������,

����ֵ˵�����������Ϊ����, ���ظ�����; Ϊ��ͨ�ַ����򷵻ظ��ַ���ָ��.
====================================================================*/
#ifdef WIN64
API s64 WordParse(const char * szWord)
{
    u64 qwTemp;
    if (NULL == szWord)
    {
        return 0;
    }
    qwTemp = _strtoui64(szWord, NULL, 10);
    if ((0 == qwTemp) && ('0' != szWord[0]))
    {
        return (s64)szWord;
    }
    return (s64)qwTemp;
}
#else
API int WordParse(const char * word)
{
	int tmp;

	if(word == NULL) return 0;

	tmp = atoi(word);
	if(tmp == 0 && word[0] != '0')
	{
		return (int)word;
	}
    return tmp;
}
#endif

/*====================================================================
��������OspShellStart
���ܣ���Windows�´���һ������������Telnet�ͻ���.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����g_wportListtening: Telnet�������������˿�.
�������˵����

����ֵ˵������.
====================================================================*/
API void OspShellStart()
{
    char command[100];
	PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;

    sprintf((char*)command, "telnet.exe localhost %d", g_wportListtening);

    memset(&siStartInfo, 0, sizeof(STARTUPINFO));
	memset(&piProcInfo, 0, sizeof(PROCESS_INFORMATION));

	siStartInfo.cb = sizeof(STARTUPINFO);
	if( !CreateProcess(NULL, command, NULL, NULL, FALSE,
		               CREATE_NEW_CONSOLE, NULL, NULL, &siStartInfo, &piProcInfo) )
	{
//		OspPrintf(TRUE, TRUE, "Osp: create process for shell failed.\n");
		OspLog(1, "Osp: create process for shell failed %u.try sysnative \n", WSAGetLastError());
		printf("Osp: create process for shell failed %u.try sysnative \n", WSAGetLastError());
		sprintf((char*)command, "c:\\windows\\sysnative\\telnet.exe localhost %d", g_wportListtening);
		if( !CreateProcess(NULL, command, NULL, NULL, FALSE,
			CREATE_NEW_CONSOLE, NULL, NULL, &siStartInfo, &piProcInfo) )
		{
			OspLog(1, "Osp: create process for shell failed %u. \n", WSAGetLastError());
			printf("Osp: create process for shell failed %u.\n", WSAGetLastError());
			return;
		}
	}

	hShellProc = piProcInfo.hProcess;
	hShellThread = piProcInfo.hThread;
}

/*====================================================================
��������OspShellExit
���ܣ���Windows���˳�Shell.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����hShellProc--Shell���̾��.
�������˵����

����ֵ˵������.
====================================================================*/
API void OspShellExit()
{
	u32 exitCode;

	if(hShellProc != NULL)
	{
		GetExitCodeProcess(hShellProc, (unsigned long *)&exitCode);
		if(exitCode == STILL_ACTIVE)
		{
			TerminateProcess(hShellProc, 0);
		}
		CloseHandle(hShellProc);
		hShellProc = NULL;
	}

	if(hShellThread != NULL)
	{
		CloseHandle(hShellThread);
		hShellThread = NULL;
	}
}

/*====================================================================
��������GetBaseModName
���ܣ���Windows�µõ�һ��ģ�����ķ�·��������Ϊȱʡ����ʾ��.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵����pchModName: ģ��ȫ��

����ֵ˵����ģ�����Ļ�������.
====================================================================*/
API char *GetBaseModName(char *pchModName)
{
	char *sep1 = "\\";
	char *sep2 = ".";
	char *pchBaseModName=NULL;
	char *token=NULL;

    token = strtok(pchModName, sep1);
    while( token != NULL )
    {
		pchBaseModName = token;
		token = strtok(NULL, sep1);
	}

    pchBaseModName = strtok( pchBaseModName, sep2);
    return pchBaseModName;
}

/*====================================================================
��������PromptShow
���ܣ���Telnet����ʾ��ʾ��.
�㷨ʵ�֣�����ѡ�
����ȫ�ֱ�����
�������˵������

����ֵ˵������.
====================================================================*/
API void PromptShow(void)
{
	char achModName[MAX_MODNAME_LEN];
	char *pchBaseModName=NULL;
	char prompt[MAX_PROMPT_LEN];
    switch(g_PromtState)
    {
    case PROMTUSER:
        {
            TelePrint("Username:");
            break;
        }
    case PROMTPWD:
        {
            TelePrint("Password:");
            break;
        }
    case PROMTAUTHORIZED:
        {
	        /* ����û�ָ������ʾ����ʹ���� */
	        if(strlen(g_Osp.m_achShellPrompt) > 0)
	        {
                sprintf(prompt, "%s->", g_Osp.m_achShellPrompt);
                TelePrint(prompt);
		        return;
	        }

	        /* ȡ�õ�ǰ�������ڵ�ģ��������ΪTelnet�ͻ�����ʾ�� */
	        int ret = GetModuleFileName(NULL, achModName, MAX_MODNAME_LEN);
	        if(ret != 0)
	        {
		        achModName[MAX_MODNAME_LEN-1] = '\0';
		        pchBaseModName = GetBaseModName(achModName);
	        }

	        if(pchBaseModName != NULL)
	        {
		        sprintf(prompt, "%s->", pchBaseModName);
	        }
	        else
	        {
		        sprintf(prompt, "\n");
	        }
            TelePrint(prompt);
            break;
        }
    default:
        {
            OspLog(1,"Osp Telnet Prompt State error!\n");
            break;
        }
    }

    return;
}

/*====================================================================
��������TelePrint
���ܣ���Telnet��Ļ�ϴ�ӡ��NULL��β���ַ���
�㷨ʵ�֣����ַ��������ʵ���ת������Ҫ�ǰ�'\n'ת��Ϊ'\r\n'��
          ���͵�Telnet�ͻ���.
����ȫ�ֱ�����
�������˵����pchMsg: Ҫ��ӡ���ַ���

����ֵ˵�����ɹ�����TRUE, ʧ�ܷ���FALSE.
====================================================================*/
API BOOL32 TelePrint(const char * pchMsg)
{
	char chCur;
	u32 dwStart = 0;
	u32 dwCount = 0;
	char *pchRetStr = "\n\r";

	if( (pchMsg == NULL) || (sockClient == INVALID_SOCKET) )
	{
		return FALSE;
	}

	while(TRUE)
	{
		chCur = pchMsg[dwCount];

	    /* ����'\n'��'\0', �����ǰһ��'\n'����'\n'֮��������ַ� */
		if(chCur == '\0' || chCur == '\n')
		{
			SockSend(sockClient, &pchMsg[dwStart], dwCount-dwStart);

			/* ����'\n', ���"\r\n" */
			if(chCur == '\n')
			{
				SockSend(sockClient, pchRetStr, 2);
			}

			/* ����'\0', ��ʾ�ַ�������Ӧ����ѭ�� */
			if(chCur == '\0')
			{
				break;
			}

			/* ��һ��������� */
			dwStart = dwCount+1;
		}
		dwCount++;
	}
	return TRUE;
}