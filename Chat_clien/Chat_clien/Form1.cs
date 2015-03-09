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

namespace Chat_clien
{
    public partial class Form1 : Form
    {
        String nama;
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
            nama = Pengirim.Text;
            ipServer.Text = "";
            portServer.Text = "";
            Pengirim.Text = "";
        }

        private void button2_Click(object sender, EventArgs e)
        {

        }

        private void button3_Click(object sender, EventArgs e)
        {
            listBox1.Items.Add(nama+ ": " + Pesan.Text);
            Pesan.Text = "";
            Penerima.Text = "";
        }
    }
}
