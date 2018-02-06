using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Runtime.InteropServices;

namespace Demo
{
    public partial class frmRecord : Form
    {
        public NIMediaNet.MEDIA_NET_CATALOG_ITEM_INFO CatalogInfo{ get; set; }
        public Int32 UserID { get; set; }

        public frmRecord()
        {
            InitializeComponent();
        }

        private void frmRecord_Load(object sender, EventArgs e)
        {
            this.Text = "录像回放[" + CatalogInfo.sName + "_" + CatalogInfo.sResourceID + "]";
            InitDateTime();
        }

        private void InitDateTime()
        {
            this.dtpBegin.Value = DateTime.Now.AddDays(-1);
            this.dtpEnd.Value = DateTime.Now;
        }

        private void btnQueryRecord_Click(object sender, EventArgs e)
        {
            QueryRecord(CatalogInfo.sResourceID,this.dtpBegin.Value,this.dtpEnd.Value);
        }

        private void QueryRecord(string sResourceID,DateTime dtBegin,DateTime dtEnd)
        {
            NIMediaNet.MEDIA_NET_VIDEOFILECOND param = new NIMediaNet.MEDIA_NET_VIDEOFILECOND();
            param.sResourceID = CatalogInfo.sResourceID;

            param.struStartTime.uYear = (UInt32)dtBegin.Year;
            param.struStartTime.uMonth = (UInt32)dtBegin.Month;
            param.struStartTime.uDay = (UInt32)dtBegin.Day;
            param.struStartTime.uHour = (UInt32)dtBegin.Hour;
            param.struStartTime.uMinute = (UInt32)dtBegin.Minute;
            param.struStartTime.uSecond = (UInt32)dtBegin.Second;

            param.struStopTime.uYear = (UInt32)dtEnd.Year;
            param.struStopTime.uMonth = (UInt32)dtEnd.Month;
            param.struStopTime.uDay = (UInt32)dtEnd.Day;
            param.struStopTime.uHour = (UInt32)dtEnd.Hour;
            param.struStopTime.uMinute = (UInt32)dtEnd.Minute;
            param.struStopTime.uSecond = (UInt32)dtEnd.Second;

            NIMediaNet.Media_Net_QueryVideoRecord(UserID,ref param);
        }

        delegate void SafeAddRecordTreeNode(NIMediaNet.MEDIA_NET_VIDEORECORD_ITEM item);
        public void AddRecordTreeNode(NIMediaNet.MEDIA_NET_VIDEORECORD_ITEM item)
        {
            if (listView1.InvokeRequired)
            {
                SafeAddRecordTreeNode objSet = new SafeAddRecordTreeNode(AddRecordTreeNode);
                listView1.Invoke(objSet, new object[] { item });
            }
            else
            {
                listView1.Items.Clear();
                ListViewItem lvitem = new ListViewItem();
                lvitem.Tag = item;
                lvitem.Text = item.sFilePath;
                listView1.Items.Add(lvitem);

                IntPtr ptNext = item.ItemNext;
                while (ptNext != IntPtr.Zero)
                {
                    NIMediaNet.MEDIA_NET_VIDEORECORD_ITEM item2 = (NIMediaNet.MEDIA_NET_VIDEORECORD_ITEM)Marshal.PtrToStructure(ptNext, typeof(NIMediaNet.MEDIA_NET_VIDEORECORD_ITEM));
                    lvitem = new ListViewItem();
                    lvitem.Tag = item2;
                    lvitem.Text = item2.sFilePath;
                    ptNext = item2.ItemNext;
                    listView1.Items.Add(lvitem);
                }
            }
        }

        private void btnReset_Click(object sender, EventArgs e)
        {
            listView1.Items.Clear();
        }
    }
}
