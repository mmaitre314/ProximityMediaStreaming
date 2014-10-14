using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.TestPlatform.UnitTestFramework;
using System.Threading.Tasks;
using System.Runtime.InteropServices.WindowsRuntime;
using ProximityMediaStreaming;
using Windows.Storage.Streams;
using System.Threading;

namespace UnitTests
{
    [TestClass]
    public class MessageChannelTests
    {
        [TestMethod]
        public void CS_W_Basic()
        {
            var socket = new MockStreamSocket();
            socket.Add((byte)MessageType.Buffer); 
            socket.Add(0);
            socket.Add(0);
            socket.Add(0);
            socket.Add(1);
            socket.Add(0xFF);
            Assert.AreEqual(6, socket.Count);

            using (var channel = new MessageChannel(socket, socket))
            {
                AutoResetEvent e = new AutoResetEvent(false);
                IBuffer messageBuffer = null;
                MessageType messageType = MessageType.None;

                channel.MessageReceived += (object sender, MessageReceivedEventArgs args) =>
                    {
                        messageType = args.Type;
                        messageBuffer = args.Buffer;
                        e.Set();
                    };

                e.WaitOne();

                Assert.AreEqual(MessageType.Buffer, messageType);
                Assert.AreEqual(1, messageBuffer.Length);
                Assert.AreEqual(0xFF, messageBuffer.GetByte(0));
            }
        }

        // TODO: test receive string
        // TODO: test receive 2 events in a row (while loop)
        // TODO: test send buffer/string (events out on same channel)
    }
}
