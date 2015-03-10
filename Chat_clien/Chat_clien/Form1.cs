using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

using System.Net;
using System.Net.Sockets;
using System.Threading;

namespace Chat_clien
{
    public partial class Form1 : Form
    {
        String nama;
        System.Net.Sockets.TcpClient clientSocket = new System.Net.Sockets.TcpClient();
        NetworkStream serverStream;
        TcpListener listensocket = new TcpListener(8888);
        public Form1()
        {
            InitializeComponent();
        }

        private void label2_Click(object sender, EventArgs e)
        {

        }

        private void textBox2_TextChanged(object sender, EventArgs e)
        {

        }

        private void button1_Click(object sender, EventArgs e)
        {
            String IP = ipServer.Text;
            String port = portServer.Text;
            clientSocket.Connect(IP, Convert.ToInt32(port));
            serverStream = clientSocket.GetStream();
            Thread th = new Thread(getmsg);
            th.Start(); 
            nama = Pengirim.Text;
            ipServer.Text = "";
            portServer.Text = "";
            Pengirim.Text = "";
        }

        private void button2_Click(object sender, EventArgs e)
        {
            clientSocket.Close();
        }

        private void button3_Click(object sender, EventArgs e)
        {
            serverStream = clientSocket.GetStream();
            
            byte[] outStream = System.Text.Encoding.ASCII.GetBytes(nama+": "+Pesan.Text + "$");

            serverStream.Write(outStream, 0, outStream.Length);  //memberikan tulisan ke server

            serverStream.Flush();

            msg(nama + ": " + Pesan.Text);
            Pesan.Text = "";
            Penerima.Text = "";
        }

        private void getmsg()   //mengambil message dari server
        {
            while (true)
            {
                serverStream = clientSocket.GetStream();

                byte[] inStream = new byte[10023];

                int Buff = clientSocket.ReceiveBufferSize;

                serverStream.Read(inStream, 0, Buff);

                string returndata = System.Text.Encoding.ASCII.GetString(inStream);

                msg(returndata);
            }
        }

        public void msg(string mesg)
        {
            if (this.InvokeRequired)    //mengambil message secara mutex
                this.Invoke(new MethodInvoker(() => msg(mesg)));
            else
            {
                char[] sep=new char[1];
                sep[0]=' ';
                var word = mesg.Split(sep);
                if (String.Compare(word[0], "!CONNECT") == 0)
                    listBox2.Items.Add(word[1]);
                else if (String.Compare(word[0], "NOPE") == 0)
                    clientSocket.Close();
                else if (String.Compare(word[0], "!DISCONNECT") == 0)
                {
                    listBox2.Items.Remove(word[1]);
                }
                else
                    listBox1.Items.Add(mesg);
            }
        }
    }
}
