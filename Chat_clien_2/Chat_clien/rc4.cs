using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Chat_clien
{
    class rc4
    {
        //public static string encrypt_rc4(string input, string key)
        public static byte[] encrypt_rc4(byte[] input, string key)
        {
            //StringBuilder result = new StringBuilder();
            byte[] result = new byte[input.Length];
            //int x, y, j = 0;
            byte x, y, j = 0;
            //int[] box = new int[256];
            byte[] box = new byte[256];
            //for (int i = 0; i < 256; i++)
            for (int i = 0; i < 256; i++ )
            {
                box[i] = (byte)i;
            }

            //for (int i = 0; i < 256; i++) 
            for (int i = 0; i < 256; i++)
            {
                //j = (key[i % key.Length] + box[i] + j) % 256;
                j = (byte)((int)(key[i % key.Length] + box[i] + j) % 256);
                x = box[i];
                box[i] = box[j];
                box[j] = x;
            }

            for (int i = 0; i < input.Length; i++)
            {
                //y = i % 256;
                y = (byte)((int)i % 256);
                j = (byte)((int)(box[y] + j) % 256);
                x = box[y];
                box[y] = box[j];
                box[j] = x;
                //result.Append((char)(input[i] ^ box[(box[y] + box[j]) % 256]));
                result[i] = (byte)(input[i] ^ box[(int)(box[y] + box[j]) % 256]);
            }
            //return result.ToString;
            return result;
        }
    }
}
