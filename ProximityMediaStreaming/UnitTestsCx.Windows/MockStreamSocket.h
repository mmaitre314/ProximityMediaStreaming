#pragma once

ref class MockStreamSocket sealed 
    : public Windows::Storage::Streams::IInputStream
    , public Windows::Storage::Streams::IOutputStream
{
public:

    MockStreamSocket();

    // IClosable
    virtual ~MockStreamSocket();

    // IInputStream
    Windows::Foundation::IAsyncOperationWithProgress<Windows::Storage::Streams::IBuffer^, uint32>^
        ReadAsync(
        Windows::Storage::Streams::IBuffer^ buffer, 
        uint32 count, 
        Windows::Storage::Streams::InputStreamOptions options
        ) override;

    // IOutputStream
    Windows::Foundation::IAsyncOperationWithProgress<uint32, uint32>^ WriteAsync(
        Windows::Storage::Streams::IBuffer^ buffer
        ) override;
    Windows::Foundation::IAsyncOperation<bool>^ FlushAsync() override;

    void Add(byte data);
    void Add(byte[] data);
    
    property int Count { int get(); }

private:

    BlockingCollection<IBuffer> m_buffers = new BlockingCollection<IBuffer>(new ConcurrentQueue<IBuffer>());

};

