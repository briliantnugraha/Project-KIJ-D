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
        Thread th;
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
            th =  new Thread(getmsg);
            th.Start(); 
            nama = Pengirim.Text;
            ipServer.Text = "";
            portServer.Text = "";
            Pengirim.Text = "";
        }

        private void button2_Click(object sender, EventArgs e)
        {
            th.Abort();
            clientSocket.Close();
        }

        private void button3_Click(object sender, EventArgs e)
        {
            serverStream = clientSocket.GetStream();
            
            byte[] outStream = System.Text.Encoding.ASCII.GetBytes(Penerima.Text+":"+Pesan.Text + "\r\n\0");

            serverStream.Write(outStream, 0, outStream.Length);  //memberikan tulisan ke server

            serverStream.Flush();

            listBox1.Items.Add(nama + ":" + Pesan.Text);
            Pesan.Text = "";

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
                char[] sep=new char[4];
                sep[0]=' ';
                sep[1] = '\r';
                sep[2] = '\n';
                sep[3] = '\0';
                var word = mesg.Split(sep);
                if (String.Compare(word[0], "!CONNECT") == 0)
                    listBox2.Items.Add(word[1]);
                else if (String.Compare(word[0], "NOPE\r\n") == 0)
                {
                    MessageBox.Show("Nama sudah digunakan silahkan connect ulang dengan nama yang berbeda");
                    clientSocket.Close();
                }
                else if (String.Compare(word[0], "!DISCONNECT") == 0)
                {
                    listBox2.Items.Remove(word[1]);
                }
                else if (String.Compare(word[0], "OK") == 0)
                {
                    serverStream = clientSocket.GetStream();

                    byte[] outStream = System.Text.Encoding.ASCII.GetBytes(nama + "\r\n\0");

                    serverStream.Write(outStream, 0, outStream.Length);  //memberikan tulisan ke server

                    serverStream.Flush();

                }
                else
                    listBox1.Items.Add(mesg);
            }
        }

        private void listBox2_SelectedIndexChanged(object sender, EventArgs e)
        {
            Penerima.Text = Convert.ToString(listBox2.SelectedItem);
        }

        private void listBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            Penerima.Text = Convert.ToString(listBox1.SelectedItem);
        }
    }
}
