using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Security.Cryptography;

namespace Chat_clien
{
    class toSHA
    {
        private static byte[] hashShow;
        public static String SHA(String plain)
        {
            SHA512 manager = new SHA512Managed();
            var toByte = Encoding.ASCII.GetBytes(plain);
            hashShow = manager.ComputeHash(toByte);

            String output = System.Convert.ToBase64String(hashShow);
            return output;
        }
    }
}
