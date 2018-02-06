using System;
using System.Runtime.InteropServices;

namespace Demo
{
    public class NIMediaNet
    {
        public const UInt32 MEDIA_NET_NOERROR                                   = 0;    /* 没有错误 */
        public const UInt32 MEDIA_NET_PASSWORD_ERROR                            = 1;    /* 用户名或密码错误 */
        public const UInt32 MEDIA_NET_NOUGHPRI                                  = 2;    /* 权限不足 */
        public const UInt32 MEDIA_NET_NOINIT                                    = 3;    /* 没有初始化 */
        public const UInt32 MEDIA_NET_NETWORK_FAIL_CONNECT                      = 4;    /* 连接服务器失败 */
        public const UInt32 MEDIA_NET_MEMORY_NOT_ENOUGH                         = 5;    /* 内存不足 */
        public const UInt32 MEDIA_NET_LOCAL_INIT_ERROR                          = 6;    /* 本地初始化错误 */

        public const UInt32 MEDIA_NET_GET_LOGIN_RESULT                          = 100;  /* 登录是否成功 */
        public const UInt32 MEDIA_NET_GET_RESOURCE_RESULT                       = 101;  /* 查询资源结果 */
        public const UInt32 MEDIA_NET_GET_VIDEORECORD_RESULT                    = 102;  /* 查询录像结果 */

        public delegate int fExcuteResultCallBack(UInt32 lUserID,UInt32 uMsgID, IntPtr uResult,IntPtr pUser);
        public delegate void fRealDataCallBack(UInt32 lRealHandle, UInt32 uDataType, IntPtr pBuffer, UInt32 dwBufSize, IntPtr pUser);

        [StructLayoutAttribute(LayoutKind.Sequential)]
        public struct MEDIA_NET_LOGIN_INFO
        {
            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 16)]
            public string sServerAddress;

            public ushort wServerPort;

            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string sServerPassword;

            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 21)]
            public string sServerSipID;

            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 21)]
            public string sServerDomainID;

            public UInt32 uServerKeepInterval;

            public UInt32 uServerKeepMaxCnt;

            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 16)]
            public string sClientAddress;

            public ushort wClientPort;

            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 21)]
            public string sClientSipID;

            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 21)]
            public string sClientDomainID;

            public UInt32 uSessionLingerTime;

            [MarshalAs(UnmanagedType.FunctionPtr)]
            public fExcuteResultCallBack cbExcuteResult;

            public IntPtr pUser;
        }

        [StructLayoutAttribute(LayoutKind.Sequential)]
        public struct MEDIA_NET_PREVIEW_INFO
        {
            public IntPtr hPlayWnd;

            public UInt32 uShowMode;

            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 21)]
            public string sResourceID;

            [MarshalAs(UnmanagedType.FunctionPtr)]
            public fRealDataCallBack cbExcuteResult;

            public UInt32 uMediaType;

            public IntPtr pUser;
        }

        [StructLayoutAttribute(LayoutKind.Sequential)]
        public struct MEDIA_NET_CATALOG_ITEM_INFO
        {
            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 21)]
            public string sResourceID;

            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string sParentDevId;

            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 64)]
            public string sName;

            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 64)]
            public string sManufacturer;

            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 64)]
            public string sModel;

            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 64)]
            public string sOwner;

            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 64)]
            public string sCivilCode;

            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 64)]
            public string sAddress;

            int nParental;

            int nSafeWay;

            int nRegisterWay;

            int nSecrecy;

            int nStatus;
        }

        [StructLayoutAttribute(LayoutKind.Sequential)]
        public struct MEDIA_NET_TIME
        {
            public UInt32 uYear;
            public UInt32 uMonth;
            public UInt32 uDay;
            public UInt32 uHour;
            public UInt32 uMinute;
            public UInt32 uSecond;
        }

        [StructLayoutAttribute(LayoutKind.Sequential)]
        public struct MEDIA_NET_VIDEOFILECOND
        {
            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 21)]
            public string sResourceID;

            public MEDIA_NET_TIME struStartTime;
            public MEDIA_NET_TIME struStopTime;
        }

        [StructLayoutAttribute(LayoutKind.Sequential)]
        public struct MEDIA_NET_VIDEORECORD_ITEM
        {
            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string sParentDevID;

            public int nSn;

            public UInt32 nSumNum;

            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string sParentName;

            public IntPtr UserCtx;

            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string sResourceID;

            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string sName;

            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string sFilePath;

            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string sAddress;

            public UInt64 uStartTime;

            public UInt64 uEndTime;

            public int nSecrecy;

            public int nType;

            [MarshalAsAttribute(UnmanagedType.ByValTStr, SizeConst = 32)]
            public string sRecordID;

            public int nIndistinct;

            public IntPtr ItemNext;
        }

        [DllImport(@"NIMediaNet.dll")]
        public static extern UInt32 Media_Net_Init();

        [DllImport(@"NIMediaNet.dll")]
        public static extern UInt32 Media_Net_Cleanup();

        [DllImport(@"NIMediaNet.dll")]
        public static extern UInt32 Media_Net_Login(ref MEDIA_NET_LOGIN_INFO LoginInfo, out Int32 lUserID);

        [DllImport(@"NIMediaNet.dll")]
        public static extern UInt32 Media_Net_Logout(Int32 lUserID);

        [DllImport(@"NIMediaNet.dll")]
        public static extern UInt32 Media_Net_RealPlay(Int32 lUserID, ref MEDIA_NET_PREVIEW_INFO PreviewInfo, out Int32 lRealHandle);

        [DllImport(@"NIMediaNet.dll")]
        public static extern UInt32 Media_Net_StopRealPlay(Int32 lRealHandle);

        [DllImport(@"NIMediaNet.dll")]
        public static extern UInt32 Media_Net_QueryVideoRecord(Int32 lUserID,ref MEDIA_NET_VIDEOFILECOND VideoFileCond);
    }
}
