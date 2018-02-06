using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace Demo
{
    public partial class ucVideoPanel : UserControl
    {
        static int mnCol = 4;
        static int mnRow = 4;
        private SingerScreen[,] mScreens = new SingerScreen[mnCol, mnRow];


        public SingerScreen GetFreeScreen()
        {

            foreach (SingerScreen ss in mScreens)
            {
                if (ss.RealHandle == -1 && ss.Status == 0)
                {
                    return ss;
                }
            }

            return null;
        }

        public void SetFreeScreen(int nRealHandle)
        {
            foreach (SingerScreen ss in mScreens)
            {
                if (ss.RealHandle == nRealHandle)
                {
                    ss.ID = 0;
                    ss.Status = 0;
                    ss.ResourceID = null;
                    ss.RealHandle = -1;
                    break;
                }
            }
        }
        public ucVideoPanel()
        {
            InitializeComponent();
            InitScreen();
        }

        private void InitScreen()
        {
            int nIndex = 0;
            for (int i = 0; i < mnCol; i++)
            {
                for(int j = 0;j < mnRow; j ++)
                {
                    SingerScreen ss = new SingerScreen();
                    PictureBox pb = new PictureBox();

                    pb.BackColor = Color.Black;
                    pb.BorderStyle = BorderStyle.FixedSingle;
                    pb.DoubleClick += Pb_DoubleClick;
                    pb.Paint += Pb_Paint;
                    this.Controls.Add(pb);
                    ss.Screen = pb;
                    ss.ID = nIndex;
                    ss.Status = 0;
                    ss.RealHandle = -1;
                    mScreens[i,j] = ss;
                    nIndex++;
                }
            }
        }

        private void Pb_Paint(object sender, PaintEventArgs e)
        {
            bool bShowLogo = true;

            if(bShowLogo)
            {
                e.Graphics.DrawString("无视频信号", new Font("宋体", 9, FontStyle.Bold), Brushes.White, new PointF(10, 10));
            }
        }

        private void Pb_DoubleClick(object sender, EventArgs e)
        {
            PictureBox pb = sender as PictureBox;
            foreach (SingerScreen ss in mScreens)
            {
                if(pb == ss.Screen)
                {
                    Form1 form = this.Tag as Form1;
                    form.StopPlay(ss);
                    break;
                }
            }
        }

        private void ResizeScreen()
        {
            // 计算单个画面的长和宽
            int nBaseWidth = this.Width / mnCol;
            int nBaseHeight = this.Height / mnRow;

            for (int i = 0; i < mnCol; i++)
            {
                for (int j = 0; j < mnRow; j++)
                {
                    PictureBox pb = mScreens[i, j].Screen;
                    pb.Size = new Size(nBaseWidth, nBaseHeight);
                    // 计算其位置
                    pb.Location = new Point(j * nBaseWidth, i * nBaseHeight);
                }
            }
        }

        private void ucVideoPanel_Resize(object sender, EventArgs e)
        {
            ResizeScreen();
        }
    }

    public delegate void fSingerScreenDbClick(object sender, EventArgs e);

    public class SingerScreen
    {
        public int ID{ get; set; }
        public PictureBox Screen { get; set; }
        public int Status { get; set; }
        public int RealHandle { get; set; }
        public string ResourceID { get; set; }
    }
}
