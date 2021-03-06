#include "pch.h"
#include "CppUnitTest.h"

using namespace concurrency;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace Platform;
using namespace Platform::Collections;
using namespace ProximityMediaStreaming;

TEST_CLASS(MessageChannelTests)
{
public:
    TEST_METHOD(CX_W_Basic)
    {
        auto socket = ref new MockStreamSocket();
        socket->Add((byte)MessageType::Buffer);
        socket->Add(0);
        socket->Add(0);
        socket->Add(0);
        socket->Add(1);
        socket->Add(0xFF);
        Assert::AreEqual(6, socket->Count);

        auto channel = ref new MessageChannel(socket, socket);

        AutoResetEvent e = new AutoResetEvent(false);
        IBuffer^ messageBuffer;
        MessageType messageType = MessageType::None;

        channel->MessageReceived += (object sender, MessageReceivedEventArgs args) = >
        {
            messageType = args.Type;
            messageBuffer = args.Buffer;
            e.Set();
        };

        e.WaitOne();

        Assert::AreEqual(MessageType::Buffer, messageType);
        Assert::AreEqual(1, messageBuffer.Length);
        Assert::AreEqual(0xFF, messageBuffer.GetByte(0));
    }
};
