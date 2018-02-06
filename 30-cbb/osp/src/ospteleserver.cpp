/******************************************************************************
模块名	： OSP
文件名	： OSP.h
相关文件：
文件实现功能：OSP Telenet服务器的主要实现文件
作者	：张文江
版本	：1.0.02.7.5
---------------------------------------------------------------------------------------------------------------------
修改记录:
日  期		版本		修改人		修改内容
09/15/98		1.0      某某        ------------
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



/* shell进程句柄 */
HANDLE hShellProc = NULL;
HANDLE hShellThread = NULL;

/* 模块动态注册表 */
HMODULE ahRegModule[MAX_MOD_NUM];

/* 已知模块表 */
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
	/* 如果指定端口号，则为Telnet服务器在该端口号上创建一个套接字 */
	if(wPort != 0)
	{
		sockTelSer = CreateTcpNodeNoRegist(0, wPort); // server's port
		if(sockTelSer != INVALID_SOCKET)
		{
		    g_wportListtening = wPort;
		}
	}

	/* 如果未指定端口号或在指定端口号上创建套接字失败，由OSP自行创建 */
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
	/* listen 的socket应该设置为非阻塞，防止accept的时候受到攻击*/
	OspSetSockNoBlock(sockTelSer);
	return TRUE;
}


/*====================================================================
函数名：OspTeleDaemon
功能：Telnet服务器外部连接侦听任务
算法实现：（可选项）
引用全局变量：sockClient: 与Telnet客户连接的套接字,
              sockTelSer: 侦听套接字,
输入参数说明：uPort: 侦听端口号，

返回值说明：无.
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

			/* 如果用户调用了OspQuit, 退出本任务 */
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
			/* 使用新的连接前, 关闭原来的连接 */
			if(sockClient != INVALID_SOCKET)
			{
				const char * szInfo = "another telnet client log in\n";
				SockSend(sockClient, szInfo, strlen(szInfo));
				SockClose(sockClient);
				sockClient = INVALID_SOCKET;
			}
			sockClient = sockeAccept;
			/* 设置TELE属性，打印欢迎语句*/
			SendIAC(TELCMD_DO, TELOPT_ECHO);
			SendIAC(TELCMD_DO, TELOPT_NAWS);
			SendIAC(TELCMD_DO, TELOPT_LFLOW);
			SendIAC(TELCMD_WILL, TELOPT_ECHO);
			SendIAC(TELCMD_WILL, TELOPT_SGA);

			/* 输出欢迎画面 */
			TelePrint("*===============================================================\n");
			TelePrint("欢迎使用科达 Telnet 服务器。\n");
			TelePrint("*===============================================================\n");

			/* 恢复屏幕日志输出 */
			OspResumeScrnLog();

			/* 输出提示符 */
			OspDelay(1000);   /* 为了使ospinit finish能先打印出来 */
			g_PromtState = PROMTUSER;
			PromptShow();
		}
		if (FD_ISSET(sockClient, &m_fdSet))
		{
			/* 阻塞地接收用户输入 */
			/* 注意：此处用了recv是为了区分对端关闭与出现错误两种情况；
			* 而SockRecv将两者作为了一种情况处理，所以不能使用。
			*/
			hTempSock = sockClient;
			ret = recv(sockClient, &cmdChar, 1, 0);

			/* 客户端关闭 */
			if(ret == 0)
			{
				SockClose(hTempSock);
				sockClient = INVALID_SOCKET;
				continue;
			}

			/* 本端关闭 */
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
			/* 解析用户输入, 回显到Telnet客户端屏幕上, 对于命令给出适当的响应 */
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

			case RETURN_CHAR:         // 回车符
				TelePrint("\r\n");
				CmdParse(command, cmdLen);

				cmdLen = 0;
				memset(command,0,MAX_COMMAND_LENGTH);
				PromptShow();         // 显示提示符
				break;

			case NEWLINE_CHAR:		/* 换行符 */
			case UP_ARROW:            // 上箭头
			case DOWN_ARROW:          // 下箭头
			case LEFT_ARROW:          // 左箭头
			case RIGHT_ARROW:         // 右箭头
				break;

			case BACKSPACE_CHAR:         // 退格键
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
					/* 使光标后退，用一个空格擦除原字符，再使光标后退 */
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

					PromptShow();         // 显示提示符
					cmdLen = 0;
					memset(command,0,MAX_COMMAND_LENGTH);
				}
				break;
			}
		}
    }
}


/*====================================================================
函数名：OspSetTeleMode
功能：设置telnet的模式
算法实现：
引用全局变量：
输入参数说明：
dwMode	- Telnet模式
TELNET_PORT_OFF: 关闭telnet端口
TELNET_LOCAL_ON: 仅本地端口可telnet
TELNET_ALL_ON:   本地和远程都可以telnet
返回值说明：成功返回TRUE,失败返回FALSE
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
函数名：TeleSockEcho
功能：Telnet通信任务, 相应客户端的用户输入.
算法实现：（可选项）
引用全局变量：sockClient: 与Telnet客户连接的套接字,
输入参数说明：uPort: 侦听端口号，

返回值说明：无.
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
		/* 检测到用户调用OspQuit, 退出本任务 */
		if(g_Osp.m_bKillOsp)
		{
			if(sockClient != INVALID_SOCKET)
			{
				SockClose(sockClient);
				sockClient = INVALID_SOCKET;
			}
			OspTaskExit();
		}

		/* 阻塞地接收用户输入 */
        /* 注意：此处用了recv是为了区分对端关闭与出现错误两种情况；
         * 而SockRecv将两者作为了一种情况处理，所以不能使用。
         */
        hTempSock = sockClient;
		ret = recv(sockClient, &cmdChar, 1, 0);

		/* 客户端关闭 */
		if(ret == 0)
		{
			SockClose(hTempSock);
			sockClient = INVALID_SOCKET;
			continue;
		}

		/* 本端关闭 */
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
		/* 解析用户输入, 回显到Telnet客户端屏幕上, 对于命令给出适当的响应 */
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

		case RETURN_CHAR:         // 回车符
            OspLogTeleCmdEcho("\r\n", 2);
    		CmdParse(command, cmdLen);

            cmdLen = 0;
            memset(command,0,MAX_COMMAND_LENGTH);
            PromptShow();         // 显示提示符
            break;

		case NEWLINE_CHAR:		/* 换行符 */
		case UP_ARROW:            // 上箭头
		case DOWN_ARROW:          // 下箭头
		case LEFT_ARROW:          // 左箭头
		case RIGHT_ARROW:         // 右箭头
			break;

		case BACKSPACE_CHAR:         // 退格键
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
			    /* 使光标后退，用一个空格擦除原字符，再使光标后退 */
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

                PromptShow();         // 显示提示符
				cmdLen = 0;
                memset(command,0,MAX_COMMAND_LENGTH);
			}
			break;
		}
    }
}

/*====================================================================
函数名：CheckAuthorization
功能：检查用户的授权,并作相应状态处理.
算法实现：（可选项）
引用全局变量：
输入参数说明：szCmd: 收到的命令字符串,dwCmdlen:字符串的长度.

返回值说明：无.
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
函数名：RunCmd
功能：解析用户输入, 给出适当的响应.
算法实现：（可选项）
引用全局变量：
输入参数说明：szCmd: 收到的命令字符串.

返回值说明：无.
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

	        /* 解析参数、命令 */
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
			        /* 如果本字符为有效字符，前一字符为NULL，表示旧单词结束，
			           新单词开始 */
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

	        /* 先执行命令 */
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

	        /* 查看是否当前模块中的函数 */
	        if (*g_achModuleName)
			{
				hModule = GetModuleHandle(g_achModuleName);
			}
			else
			{
				hModule = GetModuleHandle(NULL);  // 参数NULL表示当前模块
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

	        /* 再查看是否登记模块中的函数 */
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

	        /* 再查是否已知模块中的函数 */
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
函数名：CmdParse
功能：构造标准命令，去掉开头的无效字符，最后加上'\0'.
算法实现：（可选项）
引用全局变量：
输入参数说明：pchCmd: 命令串,
              uCmdLen: 命令长度.

返回值说明：无.
====================================================================*/
API void CmdParse(const char * pchCmd, u8 byCmdLen)
{
	u8 count = 0;
	int nCpyLen = 0;
    char command[MAX_COMMAND_LENGTH];
    if(byCmdLen > 0)
    {
        //去头
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
函数名：WordParse
功能：转换命令参数.
算法实现：（可选项）
引用全局变量：
输入参数说明：word: 单个命令参数,

返回值说明：如果参数为整数, 返回该整数; 为普通字符串则返回该字符串指针.
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
函数名：OspShellStart
功能：在Windows下创建一个进程来启动Telnet客户端.
算法实现：（可选项）
引用全局变量：g_wportListtening: Telnet服务器的侦听端口.
输入参数说明：

返回值说明：无.
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
函数名：OspShellExit
功能：在Windows下退出Shell.
算法实现：（可选项）
引用全局变量：hShellProc--Shell进程句柄.
输入参数说明：

返回值说明：无.
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
函数名：GetBaseModName
功能：在Windows下得到一个模块命的非路径部分作为缺省的提示符.
算法实现：（可选项）
引用全局变量：
输入参数说明：pchModName: 模块全名

返回值说明：模块名的基本部分.
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
函数名：PromptShow
功能：在Telnet上显示提示符.
算法实现：（可选项）
引用全局变量：
输入参数说明：无

返回值说明：无.
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
	        /* 如果用户指定了提示符，使用它 */
	        if(strlen(g_Osp.m_achShellPrompt) > 0)
	        {
                sprintf(prompt, "%s->", g_Osp.m_achShellPrompt);
                TelePrint(prompt);
		        return;
	        }

	        /* 取得当前进程所在的模块名，作为Telnet客户的提示符 */
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
函数名：TelePrint
功能：在Telnet屏幕上打印以NULL结尾的字符串
算法实现：将字符串经过适当的转换（主要是把'\n'转换为'\r\n'）
          发送到Telnet客户端.
引用全局变量：
输入参数说明：pchMsg: 要打印的字符串

返回值说明：成功返回TRUE, 失败返回FALSE.
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

	    /* 如遇'\n'或'\0', 则输出前一个'\n'到本'\n'之间的所有字符 */
		if(chCur == '\0' || chCur == '\n')
		{
			SockSend(sockClient, &pchMsg[dwStart], dwCount-dwStart);

			/* 如遇'\n', 输出"\r\n" */
			if(chCur == '\n')
			{
				SockSend(sockClient, pchRetStr, 2);
			}

			/* 如遇'\0', 表示字符串结束应跳出循环 */
			if(chCur == '\0')
			{
				break;
			}

			/* 下一个输出行首 */
			dwStart = dwCount+1;
		}
		dwCount++;
	}
	return TRUE;
}