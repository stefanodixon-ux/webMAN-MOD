/****************************************************                                                 
 *                                                  *
 * Written by Starman.				    *
 *                                                  *
 * FUNCTIONS:                                       *
 *                                                  *
 * Call any syscall with upto 8 parameters in C#!   *
 *                                                  *
 *                                                  *
 ****************************************************/


using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Net;
using System.Net.Sockets;
using System.IO;

namespace SystemCalls
{
    public  class Webman
    {
        // Definitions
        private  Socket sock = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        
        /// <summary>
        /// The IP Address of the target, use this variable if you want to implement this in your CCAPI or PS3MAPI Tool.
        /// </summary>
        public  IPAddress _ipAddress;

        // PS3MAPI port.
        private const int PS3MAPI_PORT = 7887;

        // Sends a web request to the target.
        public string HandleRequest(string cmd)
        {
            WebRequest req = WebRequest.Create("http://" + _ipAddress + "/" + cmd);
            WebResponse resp = req.GetResponse();

            string data;

            using (Stream dataStream = resp.GetResponseStream())
            {
                StreamReader rd = new StreamReader(dataStream);
                data = rd.ReadToEnd();
            }

            resp.Close();
            resp.Dispose();

            return data;
        }
        
        /// <summary>
        /// Connects to the specified target.
        /// </summary>
        /// <param name="ipAddress"></param>
        /// <returns></returns>
        public  bool Connect(string ipAddress)
        {
            if(IPAddress.TryParse(ipAddress, out _ipAddress))
            {
                sock.Connect(_ipAddress, PS3MAPI_PORT);
                if (sock.Connected)
                    return true;
                return false;
            }
            return false;
        }

        /// <summary>
        /// Disconnects from the target. Set parameter to true if you want to erase the IP Address data, the data will be rewritten with the next connection.
        /// </summary>
        /// <param name="EraseIP"></param>
        public  void Disconnect(bool EraseIP)
        {
            if (sock.Connected)
            {
                if (EraseIP)
                    _ipAddress = null;
                sock.Disconnect(false);
            }
        }

        public  bool IsConnected()
        {
            return sock.Connected;
        }
        /// <summary>
        /// Provides functions to execute any syscall with upto 8 parameters. This class is compatible with Custom Firmware (CFW) and Homebrew Enabler (HEN)
        /// </summary>

        /// <summary>
        /// Calls a syscall, this function can only call syscalls that have a void as a parameter (no parameters)
        /// </summary>
        /// <param name="num"></param>
        public void System_Call_0(int num)
        {
            if (sock.Connected)
                HandleRequest("syscall.ps3?" + num.ToString());
        }

        /// <summary>
        /// Calls a syscall with 1 parameter.
        /// </summary>
        /// <param name="num"></param>
        /// <param name="param1"></param>
        public void System_Call_1(int num, dynamic param1)
        {
            if (sock.Connected)
                HandleRequest("syscall.ps3?" + num.ToString() + "|" + param1);
        }
        /// <summary>
        /// Calls a syscall with 2 parameter.
        /// </summary>
        /// <param name="num"></param>
        /// <param name="param1"></param>
        /// <param name="param2"></param>
        public void System_Call_2(int num, dynamic param1, dynamic param2)
        {
            if (sock.Connected)
                HandleRequest("syscall.ps3?" + num.ToString() + "|" + param1 + "|" + param2);
        }

        /// <summary>
        /// Calls a syscall with 3 parameters.
        /// </summary>
        /// <param name="num"></param>
        /// <param name="param1"></param>
        /// <param name="param2"></param>
        /// <param name="param3"></param>
        public void System_Call_3(int num, dynamic param1, dynamic param2, dynamic param3)
        {
            if (sock.Connected)
                HandleRequest("syscall.ps3?" + num.ToString() + "|" + param1 + "|" + param2 + "|" + param3);
        }

        /// <summary>
        /// Calls a syscall with 4 parameters.
        /// </summary>
        /// <param name="num"></param>
        /// <param name="param1"></param>
        /// <param name="param2"></param>
        /// <param name="param3"></param>
        /// <param name="param4"></param>
        public void System_Call_4(int num, dynamic param1, dynamic param2, dynamic param3, dynamic param4)
        {
            if (sock.Connected)
                HandleRequest("syscall.ps3?" + num.ToString() + "|" + param1 + "|" + param2 + "|" + param3 + "|" + param4);
        }

        /// <summary>
        /// Calls a syscall with 5 parameters.
        /// </summary>
        /// <param name="num"></param>
        /// <param name="param1"></param>
        /// <param name="param2"></param>
        /// <param name="param3"></param>
        /// <param name="param4"></param>
        /// <param name="param5"></param>
        public void System_Call_5(int num, dynamic param1, dynamic param2, dynamic param3, dynamic param4, dynamic param5)
        {
            if (sock.Connected)
                HandleRequest("syscall.ps3?" + num.ToString() + "|" + param1 + "|" + param2 + "|" + param3 + "|" + param4 + "|" + param5);
        }

        /// <summary>
        /// Calls a syscall with 6 parameters.
        /// </summary>
        /// <param name="num"></param>
        /// <param name="param1"></param>
        /// <param name="param2"></param>
        /// <param name="param3"></param>
        /// <param name="param4"></param>
        /// <param name="param5"></param>
        /// <param name="param6"></param>
        public void System_Call_6(int num, dynamic param1, dynamic param2, dynamic param3, dynamic param4, dynamic param5, dynamic param6)
        {
            if (sock.Connected)
                HandleRequest("syscall.ps3?" + num.ToString() + "|" + param1 + "|" + param2 + "|" + param3 + "|" + param4 + "|" + param5 + "|" + param6);
        }

        /// <summary>
        /// Calls a syscall with 7 parameters.
        /// </summary>
        /// <param name="num"></param>
        /// <param name="param1"></param>
        /// <param name="param2"></param>
        /// <param name="param3"></param>
        /// <param name="param4"></param>
        /// <param name="param5"></param>
        /// <param name="param6"></param>
        /// <param name="param7"></param>
        public void System_Call_7(int num, dynamic param1, dynamic param2, dynamic param3, dynamic param4, dynamic param5, dynamic param6, dynamic param7)
        {
            if (sock.Connected)
                HandleRequest("syscall.ps3? " + num.ToString() + "|" + param1 + "|" + param2 + "|" + param3 + "|" + param4 + "|" + param5 + "|" + param6 + "|" + param7);
        }

        /// <summary>
        /// Calls a syscall with 8 parameters.
        /// </summary>
        /// <param name="num"></param>
        /// <param name="param1"></param>
        /// <param name="param2"></param>
        /// <param name="param3"></param>
        /// <param name="param4"></param>
        /// <param name="param5"></param>
        /// <param name="param6"></param>
        /// <param name="param7"></param>
        /// <param name="param8"></param>
        public void System_Call_8(int num, dynamic param1, dynamic param2, dynamic param3, dynamic param4, dynamic param5, dynamic param6, dynamic param7, dynamic param8)
        {
            if (sock.Connected)
                HandleRequest("syscall.ps3? " + num.ToString() + "|" + param1 + "|" + param2 + "|" + param3 + "|" + param4 + "|" + param5 + "|" + param6 + "|" + param7 + "|" + param8);
        }
    }
}
