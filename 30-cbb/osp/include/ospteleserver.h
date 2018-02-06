/*************************************************************
ģ����	�� OSP
�ļ���	�� OSP.h
����ļ���
�ļ�ʵ�ֹ��ܣ�OSP Win32 telenet server ����Ҫ����ͷ�ļ� 
����	�����Ľ�
�汾	��1.0.02.7.5
---------------------------------------------------------------------------------------------------------------------
�޸ļ�¼:
��  ��		�汾		�޸���		�޸�����
09/15/98		1.0      ĳĳ        ------------
******************************************************************************/

#ifndef OSP_TELE_INC
#define OSP_TELE_INC

/* Telnet server's port range */
const u16 MIN_TELSVR_PORT = 2500;
const u16 MAX_TELSVR_PORT = 8000;/* 2800; for debug mulitnode */

const u16 MAX_MODNAME_LEN = MAX_PATH;
const u16 MAX_MOD_NUM = 20;

const u8 MAX_COMMAND_LENGTH = 100;
const char NEWLINE_CHAR = '\n';
const char BACKSPACE_CHAR = 8;
const char BLANK_CHAR = ' ';
const char RETURN_CHAR = 13;
const char TAB_CHAR = 9;
const char DEL_CHAR = 127;
const char CTRL_S = 19;
const char CTRL_R = 18;
const char UP_ARROW = 27;
const char DOWN_ARROW = 28;
const char LEFT_ARROW = 29;
const char RIGHT_ARROW = 30;

typedef enum PROMTSTATE
{
	// �û���״̬
	PROMTUSER	 = 1,
	// ����״̬
	PROMTPWD	 = 2,
	// ��׼��ʾ״̬
	PROMTAUTHORIZED	 = 3,
}PROMTSTATE; 

#define TELCMD_WILL    (u8)251
#define TELCMD_WONT    (u8)252
#define TELCMD_DO      (u8)253
#define TELCMD_DONT    (u8)254
#define TELCMD_IAC     (u8)255

#define AUTHORIZATION_NAME_SIZE 20

#define TELOPT_ECHO     (u8)1
#define TELOPT_SGA      (u8)3
#define TELOPT_LFLOW    (u8)33
#define TELOPT_NAWS     (u8)34

API BOOL32 OspTelInit(u16 dwPort);
API void OspTeleDaemon(u16 wPort);
API void OspConsole(void);
API void PromptShow(void);
API void CmdParse(const char *pchCmd, u8 byCmdLen);
API char *GetBaseModName(char *pchModName);
API void RunCmd(char * szCmd);
#ifdef WIN64
API s64 WordParse(const char *word);
#else
API int WordParse(const char * word);
#endif
API BOOL32 TelePrint(const char *pchMsg);
API void CheckAuthorization(char *szCmd,u32 dwCmdlen);

#endif //OSP_TELE_INC

