using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Security.Cryptography;

namespace hash_Form
{
    class toSHA
    {
        private byte[] hashShow;
        public String SHA(String plain)
        {
            SHA512 manager = new SHA512Managed();
            var toByte = Encoding.UTF8.GetBytes(plain);
            hashShow = manager.ComputeHash(toByte);

            String output = BitConverter.ToString(hashShow);
            return output;
        }
    }
}
