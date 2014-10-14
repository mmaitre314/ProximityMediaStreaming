#include "pch.h"
#include "MockStreamSocket.h"

using namespace Windows::Storage::Streams;
using namespace Windows::Foundation;

IAsyncOperationWithProgress<IBuffer, uint> MockStreamSocket::ReadAsync(IBuffer buffer, uint count, InputStreamOptions options)
{
    Contract.Requires<ArgumentNullException>(buffer != null);
    Contract.Requires<ArgumentException>(count > 0);
    Contract.Requires<NotImplementedException>(options == InputStreamOptions.Partial);

    return AsyncInfo.Run<IBuffer, uint>((cancellation, progress) = >
    {
        return Task.Run(() = >
        {
            IBuffer bufferOut = m_buffers.Take(); // Blocks until a buffer is available

            Contract.Requires<NotImplementedException>(bufferOut.Length <= count);

            progress.Report(bufferOut.Length);
            return bufferOut;
        });
    });
}

IAsyncOperationWithProgress<uint, uint> MockStreamSocket::WriteAsync(IBuffer buffer)
{
    Contract.Requires<ArgumentNullException>(buffer != null);

    m_buffers.Add(buffer);

    return AsyncInfo.Run<uint, uint>((cancellation, progress) = >
    {
        progress.Report(buffer.Length);
        return Task.FromResult(buffer.Length);
    });
}

IAsyncOperation<bool> MockStreamSocket::FlushAsync()
{
    // Nothing to do here
    return AsyncInfo.Run<bool>((cancellation) = >
    {
        return Task.FromResult(true);
    });
}

void MockStreamSocket::Add(byte data)
{
    m_buffers.Add((new byte[] { data }).AsBuffer());
}

void MockStreamSocket::Add(byte[] data)
{
    m_buffers.Add(data.AsBuffer());
}

int MockStreamSocket::Count
{ 
    get
    { 
        return m_buffers.Count; 
    } 
}

void MockStreamSocket::Dispose()
{
    m_buffers.Dispose();
}
