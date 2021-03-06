﻿using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Security.Cryptography;

using System.Net;
using System.Net.Sockets;
using System.Threading;
using System.IO;
using System.Xml.Serialization;

namespace Chat_clien
{
    public partial class Form1 : Form
    {
        int stat = 0;
        Dictionary<string, bool> isWaitingKey = new Dictionary<string, bool>();
        Dictionary<string,string> KeySim = new Dictionary<string,string>();
        Dictionary<string, RSAParameters> KeyPub = new Dictionary<string, RSAParameters>();
        RSACryptoServiceProvider rsaProvider = null,rsa2=new RSACryptoServiceProvider();
        RSAParameters parameters,parameters2;
        string publicKey = "";
        string privateKey = "",publicKeyB; 
        UnicodeEncoding ByteConverter = new UnicodeEncoding();
        byte[] plaintext;
        byte[] encryptedtext; 
        String nama;
        System.Net.Sockets.TcpClient clientSocket = new System.Net.Sockets.TcpClient();
        NetworkStream serverStream;
        Thread th;
        //TcpListener listensocket = new TcpListener(8888);
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
            Keys();
            rsaProvider = new RSACryptoServiceProvider();
            rsaProvider.ImportParameters(parameters);
            /*
            byte[] something = rsaProvider.SignData(Encoding.ASCII.GetBytes("halo"), new SHA1CryptoServiceProvider());
            string somethingb64 = System.Convert.ToBase64String(something);
            MessageBox.Show(somethingb64);
            var rsaProvider2 = new RSACryptoServiceProvider();
            rsaProvider2.ImportParameters(parameters2);
            var verified = rsaProvider2.VerifyData(Encoding.ASCII.GetBytes("halo"), new SHA1CryptoServiceProvider(), something);
            MessageBox.Show(verified ? "Verified" : "Not Verified");
            */
            /*
            byte[] enkrip = rsaProvider.Encrypt(System.Text.Encoding.ASCII.GetBytes("123456789"), false);
            string enkrips = System.Text.Encoding.ASCII.GetString(enkrip);
            rsaProvider.ImportParameters(parameters); 
            byte[] dekrip = rsaProvider.Decrypt(enkrip, false);
            string dekrips = System.Text.Encoding.ASCII.GetString(dekrip);
            MessageBox.Show(enkrips + "\n" + dekrips);
            */
        }

        private void button2_Click(object sender, EventArgs e)
        {
            th.Abort();
            clientSocket.Close();
        }

        private void button3_Click(object sender, EventArgs e)
        {
            //rc4 pesanx = new rc4();
            byte[] bytePesan = Encoding.ASCII.GetBytes(Pesan.Text);
            string pesany = System.Convert.ToBase64String(rc4.encrypt_rc4(bytePesan, KeySim[Penerima.Text]));

            pesany = Penerima.Text + ": " + pesany;
            rsaProvider = new RSACryptoServiceProvider();
            rsaProvider.ImportParameters(parameters);
            String signature = System.Convert.ToBase64String(rsaProvider.SignData(bytePesan, new SHA1CryptoServiceProvider()));

            pesany = pesany + " " + signature;

            Kirim(pesany);

            listBox1.Items.Add(nama + ":" + Pesan.Text);
            Pesan.Text = "";

        }

        private void Kirim(string pesany)
        {
             serverStream = clientSocket.GetStream();
            
            byte[] outStream = System.Text.Encoding.ASCII.GetBytes(pesany);

            serverStream.Write(outStream, 0, outStream.Length);  //memberikan tulisan ke server

            serverStream.Flush();
        }

        private void getmsg()   //mengambil message dari server
        {
            while (true)
            {
                serverStream = clientSocket.GetStream();

                byte[] inStream = new byte[65537];

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
                output.Items.Add(mesg);
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
                    KeySim.Remove(word[1]);
                    KeyPub.Remove(word[1]);
                }
                else if (String.Compare(word[0], "OK") == 0)
                {
                    if (stat == 0)
                    {
                        Kirim(nama + " " + publicKey);
                        stat = 1;
                    }
                    else
                    {
                        listBox2.Enabled = true;
                        Penerima.Enabled = true;
                        Pesan.Enabled = true;
                    }
                }   
                else if(String.Compare(word[0],"!KEYPU")==0)
                {
                    publicKeyB = word[2];
                    //RSA
                    rsaProvider = new RSACryptoServiceProvider();
                    rsaProvider.FromXmlString(word[2]);
                    KeyPub.Add(word[1], rsaProvider.ExportParameters(false));
                    if (isWaitingKey.ContainsKey(word[1]))
                    {
                        Random rnd = new Random();
                        string keysi = Convert.ToString(rnd.Next(1, 255));
                        KeySim.Add(word[1], keysi);
                        byte[] enkrip = rsaProvider.Encrypt(System.Text.Encoding.ASCII.GetBytes(keysi), false);
                        string enkrips = System.Convert.ToBase64String(enkrip);
                        Kirim("!KEYSI " + word[1] + " " + enkrips);
                        isWaitingKey.Remove(word[1]);
                    }
                    
                }
                else if(String.Compare(word[0],"!KEYSI")==0)
                {
                    //decrypt RSA
                    rsaProvider=new RSACryptoServiceProvider();
                    rsaProvider.ImportParameters(parameters);
                    //byte[] dekrip = rsaProvider.Decrypt(System.Text.Encoding.ASCII.GetBytes(word[2]), false);
                    byte[] dekrip = rsaProvider.Decrypt(System.Convert.FromBase64String(word[2]), false);
                    string dekrips = System.Text.Encoding.ASCII.GetString(dekrip);
                    KeySim.Add(word[1],dekrips);
                }
                else
                {
                    string nama = word[0].Substring(0, word[0].Length - 1);
                    byte[] signature = System.Convert.FromBase64String(word[2]);
                    string pesany = Encoding.ASCII.GetString(rc4.encrypt_rc4(System.Convert.FromBase64String(word[1]), KeySim[nama]));
                    var verifier = new RSACryptoServiceProvider();
                    verifier.ImportParameters(KeyPub[nama]);
                    if (verifier.VerifyData(Encoding.ASCII.GetBytes(pesany), new SHA1CryptoServiceProvider(), signature))
                    {
                        listBox1.Items.Add(word[0] + " " + pesany);
                    }
                }
                    
            }
        }

        private void listBox2_SelectedIndexChanged(object sender, EventArgs e)
        {
            string SiB= Convert.ToString(listBox2.SelectedItem);
            Penerima.Text = SiB;
            if (!KeySim.ContainsKey(SiB))
            {
                Kirim("!GET " + SiB);
                isWaitingKey.Add(Penerima.Text, true);
            }
            /*
            else
            {
                MessageBox.Show("punya");
            }
             */
        }

        private void listBox1_SelectedIndexChanged(object sender, EventArgs e)
        {
            Penerima.Text = Convert.ToString(listBox1.SelectedItem);
        }

        void Keys()
        {
            CspParameters cspParams = null;
            StreamWriter publicKeyFile = null;
            try
            {
                cspParams = new CspParameters();
                cspParams.ProviderType = 1; // PROV_RSA_FULL 
                //cspParams.ProviderName; // CSP name
                cspParams.Flags = CspProviderFlags.UseArchivableKey;
                cspParams.KeyNumber = (int)KeyNumber.Exchange;
                rsaProvider = new RSACryptoServiceProvider(cspParams);
                
                parameters = rsaProvider.ExportParameters(true);
                parameters2 = rsaProvider.ExportParameters(false);

                publicKey = rsaProvider.ToXmlString(false);
            }
            catch (Exception ex)
            {
                // Any errors? Show them
                Console.WriteLine("Exception generating a new key pair! More info:");
                Console.WriteLine(ex.Message);
            }
           
        }
    }
}
