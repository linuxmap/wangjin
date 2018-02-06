using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Configuration;
using System.Runtime.InteropServices;

namespace Demo
{
    public partial class Form1 : Form
    {
        private Int32 mUserID = -1;
        private frmRecord mRecord = null;

        public Form1()
        {
            InitializeComponent();
            ucVideoPanel1.Tag = this;
            treeView1.Nodes.Add("--所有视频--");
        }

        public int ExcuteResultCallBack(UInt32 lUserID, UInt32 uMsgID, IntPtr uResult, IntPtr pUser)
        {
            switch (uMsgID)
            {
                case NIMediaNet.MEDIA_NET_GET_LOGIN_RESULT:
                    {
                        //MessageBox.Show("登录成功！");
                    }                    
                    break;
                case NIMediaNet.MEDIA_NET_GET_RESOURCE_RESULT:
                    {
                        NIMediaNet.MEDIA_NET_CATALOG_ITEM_INFO info = (NIMediaNet.MEDIA_NET_CATALOG_ITEM_INFO)Marshal.PtrToStructure(uResult, typeof(NIMediaNet.MEDIA_NET_CATALOG_ITEM_INFO));
                        AddTreeNode(info);
                    }
                    break;
                case NIMediaNet.MEDIA_NET_GET_VIDEORECORD_RESULT:
                    {
                        NIMediaNet.MEDIA_NET_VIDEORECORD_ITEM item = (NIMediaNet.MEDIA_NET_VIDEORECORD_ITEM)Marshal.PtrToStructure(uResult, typeof(NIMediaNet.MEDIA_NET_VIDEORECORD_ITEM));
                        //while(item != null)
                        //{
                        //    //AddRecordTreeNode(item);
                        //    item = (NIMediaNet.MEDIA_NET_VIDEORECORD_ITEM)Marshal.PtrToStructure(item->ItemNext, typeof(NIMediaNet.MEDIA_NET_VIDEORECORD_ITEM));
                        //}
                        if(mRecord != null)
                        {
                            mRecord.AddRecordTreeNode(item);
                        }

                    }
                    break;
            }
            return 0;
        }

        delegate void SafeAddTreeNode(NIMediaNet.MEDIA_NET_CATALOG_ITEM_INFO info);
        private void AddTreeNode(NIMediaNet.MEDIA_NET_CATALOG_ITEM_INFO info)
        {
            if (treeView1.InvokeRequired)
            {
                SafeAddTreeNode objSet = new SafeAddTreeNode(AddTreeNode);
                treeView1.Invoke(objSet, new object[] { info });
            }
            else
            {
                TreeNode tn = new TreeNode();
                tn.Tag = info;
                tn.Text = info.sName;
                tn.ToolTipText = info.sResourceID;
                tn.ContextMenuStrip = contextMenuStrip1;
                treeView1.Nodes[0].Nodes.Add(tn);
            }
        }

        private void btnLogin_Click(object sender, EventArgs e)
        {
            UInt32 nRet = NIMediaNet.Media_Net_Init();
            if(NIMediaNet.MEDIA_NET_NOERROR != nRet)
            {
                string sErrText = string.Format("Err:{0}",nRet);
                MessageBox.Show(sErrText);
                return;
            }

            NIMediaNet.MEDIA_NET_LOGIN_INFO info = new NIMediaNet.MEDIA_NET_LOGIN_INFO();
            info.sClientAddress = ConfigurationManager.AppSettings["LocalParam.ip"];
            info.sClientDomainID = ConfigurationManager.AppSettings["LocalParam.domain_id"];
            info.sClientSipID = ConfigurationManager.AppSettings["LocalParam.sip_id"];
            info.wClientPort = Convert.ToUInt16(ConfigurationManager.AppSettings["LocalParam.port"]);
            info.uSessionLingerTime = Convert.ToUInt32(ConfigurationManager.AppSettings["LocalParam.session_linger_time"]);
            info.wServerPort = Convert.ToUInt16(ConfigurationManager.AppSettings["ServParam.server_port"]);
            info.sServerAddress = ConfigurationManager.AppSettings["ServParam.server_ip"];
            info.sServerDomainID = ConfigurationManager.AppSettings["ServParam.server_sip_id"]; 
            info.sServerPassword = ConfigurationManager.AppSettings["ServParam.passwd"];
            info.sServerSipID = ConfigurationManager.AppSettings["ServParam.server_sip_id"];
            info.uServerKeepInterval = Convert.ToUInt32(ConfigurationManager.AppSettings["ServParam.keep_interval"]);
            info.uServerKeepMaxCnt = Convert.ToUInt32(ConfigurationManager.AppSettings["ServParam.keep_max_cnt"]);
            info.cbExcuteResult = ExcuteResultCallBack;
            info.pUser = IntPtr.Zero;

            nRet = NIMediaNet.Media_Net_Login(ref info,out mUserID);

            if (NIMediaNet.MEDIA_NET_NOERROR != nRet)
            {
                string sErrText = string.Format("Err:{0}", nRet);
                MessageBox.Show(sErrText);
                return;
            }
        }

        private void treeView1_DoubleClick(object sender, EventArgs e)
        {
            TreeNode tn = treeView1.SelectedNode;
            if(tn != null)
            {
                NIMediaNet.MEDIA_NET_CATALOG_ITEM_INFO cam = (NIMediaNet.MEDIA_NET_CATALOG_ITEM_INFO)tn.Tag;
                StartPlay(cam.sResourceID);
            }
        }

        private void StartPlay(string sID)
        {
            SingerScreen ss = ucVideoPanel1.GetFreeScreen();
            if (ss == null)
            {
                MessageBox.Show("没有多余的窗口显示！");
                return;
            }

            NIMediaNet.MEDIA_NET_PREVIEW_INFO PreviewInfo = new NIMediaNet.MEDIA_NET_PREVIEW_INFO();
            PreviewInfo.sResourceID = sID;
            PreviewInfo.hPlayWnd = ss.Screen.Handle;
            PreviewInfo.pUser = IntPtr.Zero;
            PreviewInfo.uShowMode = 0;

            Int32 ReadHandle = -1;
            if(NIMediaNet.MEDIA_NET_NOERROR != NIMediaNet.Media_Net_RealPlay(mUserID, ref PreviewInfo,out ReadHandle))
            {
                MessageBox.Show("打开视频失败！");
            }
            ss.RealHandle = ReadHandle;
            ss.Status = 1;
            ss.ResourceID = sID;
        }

        public void StopPlay(SingerScreen ss)
        {
            if (NIMediaNet.MEDIA_NET_NOERROR != NIMediaNet.Media_Net_StopRealPlay(ss.RealHandle))
            {
                MessageBox.Show("关闭视频失败！");
            }
            ucVideoPanel1.SetFreeScreen(ss.RealHandle);
        }

        private void tsmRecord_Click(object sender, EventArgs e)
        {
            TreeNode tn = this.treeView1.SelectedNode;
            if (tn == null) return;
            NIMediaNet.MEDIA_NET_CATALOG_ITEM_INFO info = (NIMediaNet.MEDIA_NET_CATALOG_ITEM_INFO)tn.Tag;

            mRecord = new frmRecord();
            mRecord.CatalogInfo = info;
            mRecord.UserID = mUserID;
            DialogResult dr = mRecord.ShowDialog(this);
            mRecord.Dispose();
            mRecord = null;
        }



        private void treeView1_MouseClick(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                TreeView tree = sender as TreeView;
                Point pt = tree.PointToClient(Cursor.Position);
                TreeViewHitTestInfo info = tree.HitTest(pt);
                TreeNode tn = info.Node;

                if(tn != null)
                {
                    tree.SelectedNode = tn;
                }
            }
        }
    }
}
