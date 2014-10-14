#include "pch.h"
#include "Contracts.h"
#include "MessageChannel.h"

using namespace concurrency;
using namespace ProximityMediaStreaming;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;

MessageChannel::MessageChannel(
    IInputStream^ inputStream,
    IOutputStream^ outputStream
    )
    : m_reader(ref new DataReader(inputStream))
    , m_writer(ref new DataWriter(outputStream))
    , m_messageType(MessageType::None)
    , m_messageLength(0)
{
    Contract::Requires<NullReferenceException>((inputStream != nullptr) && (outputStream != nullptr));

    m_writer->ByteOrder = ByteOrder::BigEndian;
    m_writer->UnicodeEncoding = UnicodeEncoding::Utf8;

    m_reader->ByteOrder = ByteOrder::BigEndian;
    m_reader->UnicodeEncoding = UnicodeEncoding::Utf8;
    m_reader->InputStreamOptions = InputStreamOptions::Partial;

    create_task(
        m_reader->LoadAsync(s_maxMessageSize),
        m_loadCancellation.get_token()
        ).then([this](unsigned int)
    { 
        this->OnLoad(); 
    });
}

MessageChannel::~MessageChannel()
{
    auto lock = m_lock.LockExclusive();

    m_reader = nullptr;
    m_writer = nullptr;

    // Cancel any ReadAsync op in flight
    m_loadCancellation.cancel();
}

IAsyncAction^ MessageChannel::SendBufferAsync(IBuffer^ buffer)
{
    auto lock = m_lock.LockExclusive();

    Contract::Requires<NullReferenceException>(buffer != nullptr);
    Contract::Requires<InvalidArgumentException>(buffer->Length + s_messageHeaderSize < s_maxMessageSize);
    
    m_writer->WriteByte((unsigned char)MessageType::Buffer);
    m_writer->WriteUInt32(buffer->Length);
    m_writer->WriteBuffer(buffer);

    return create_async([this]()
    {
        return create_task(m_writer->FlushAsync()).then([](bool)
        {
        });
    });
}

IAsyncAction^ MessageChannel::SendStringAsync(String^ string)
{
    auto lock = m_lock.LockExclusive();

    Contract::Requires<NullReferenceException>(string != nullptr);
    Contract::Requires<InvalidArgumentException>(string->Length() * sizeof(wchar_t) + s_messageHeaderSize < s_maxMessageSize);

    m_writer->WriteByte((unsigned char)MessageType::Buffer);
    m_writer->WriteUInt32(m_writer->MeasureString(string));
    m_writer->WriteString(string);

    return create_async([this]()
    {
        return create_task(m_writer->FlushAsync()).then([](bool)
        {
        });
    });
}

void MessageChannel::OnLoad()
{
    MessageReceivedEventArgs^ args;
    DataReader^ reader;
    cancellation_token_source cancellation;

    {
        auto lock = m_lock.LockExclusive();

        if (IsClosed())
        {
            return;
        }
        reader = m_reader;
        cancellation = m_loadCancellation;

        // Read the message header if not done yet
        if ((m_messageLength == 0) && (m_reader->UnconsumedBufferLength >= s_messageHeaderSize))
        {
            m_messageType = (MessageType)m_reader->ReadByte();
            m_messageLength = (size_t)m_reader->ReadUInt32();
        }

        // Read the message payload if fully loaded
        if ((m_messageLength > 0) && (m_reader->UnconsumedBufferLength >= m_messageLength))
        {
            switch (m_messageType)
            {
            case MessageType::Buffer:
            {
                IBuffer^ buffer = m_reader->ReadBuffer(m_messageLength);
                args = ref new MessageReceivedEventArgs(buffer);
            }
                break;

            case MessageType::String:
            {
                String^ string = m_reader->ReadString(m_messageLength);
                args = ref new MessageReceivedEventArgs(string);
            }
                break;

            default:
                throw ref new InvalidArgumentException();
            }

            m_messageType = MessageType::None;
            m_messageLength = 0;
        }
    }

    if (args != nullptr)
    {
        MessageReceived(this, args);
    }

    if (reader != nullptr)
    {
        create_task(
            reader->LoadAsync(s_maxMessageSize),
            cancellation.get_token()
            ).then([this](unsigned int)
        {
            this->OnLoad();
        });
    }
}
