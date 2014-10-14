#pragma once

namespace ProximityMediaStreaming
{
    const size_t s_maxMessageSize = 1024*1024; // Max: 1MB
    const size_t s_messageHeaderSize = 1 + 4; // [1B: messageType][4B: payloadSize]

    public enum class MessageType
    {
        None = 0,
        String,
        Buffer
    };

    public ref class MessageReceivedEventArgs sealed
    {
    public:

        property Windows::Storage::Streams::IBuffer^ Buffer
        {
            Windows::Storage::Streams::IBuffer^ get()
            {
                return m_buffer;
            }
        }

        property Platform::String^ String
        {
            Platform::String^ get()
            {
                return m_string;
            }
        }

        property MessageType Type
        {
            MessageType get()
            {
                return m_type;
            }
        }

    internal:

        MessageReceivedEventArgs(Windows::Storage::Streams::IBuffer^ buffer)
            : m_buffer(buffer)
            , m_type(MessageType::Buffer)
        {
        }

        MessageReceivedEventArgs(Platform::String^ string)
            : m_string(string)
            , m_type(MessageType::String)
        {
        }

    private:

        Windows::Storage::Streams::IBuffer^ m_buffer;
        Platform::String^ m_string;
        MessageType m_type;
    };

    /// <summary>
    /// A message-based full-duplex connection akin to WebSocket or WebRTC's RTCDataChannel on WinRT streams
    /// </summary>
    public ref class MessageChannel sealed
    {
    public:

        MessageChannel(
            Windows::Storage::Streams::IInputStream^ inputStream,
            Windows::Storage::Streams::IOutputStream^ outputStream
            );

        // IClosable
        virtual ~MessageChannel();

        Windows::Foundation::IAsyncAction^ SendBufferAsync(Windows::Storage::Streams::IBuffer^ buffer);
        Windows::Foundation::IAsyncAction^ SendStringAsync(Platform::String^ string);

        event Windows::Foundation::TypedEventHandler<Platform::Object^, MessageReceivedEventArgs^>^ MessageReceived;

        // TODO: event Failed

    private:

        void OnLoad();

        bool IsClosed() const
        {
            return (m_reader == nullptr);
        }

        Windows::Storage::Streams::DataReader^ m_reader;
        Windows::Storage::Streams::DataWriter^ m_writer;
        concurrency::cancellation_token_source m_loadCancellation;

        MessageType m_messageType;
        size_t m_messageLength;

        Microsoft::WRL::Wrappers::SRWLock m_lock;
    };
}
