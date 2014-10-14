using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Windows.Storage.Streams;
using Windows.Foundation;
using System.Runtime.InteropServices.WindowsRuntime;

namespace UnitTests
{
    class MockStreamSocket : IInputStream, IOutputStream
    {
        BlockingCollection<IBuffer> m_buffers = new BlockingCollection<IBuffer>(new ConcurrentQueue<IBuffer>());

        public IAsyncOperationWithProgress<IBuffer, uint> ReadAsync(IBuffer buffer, uint count, InputStreamOptions options)
        {
            Contract.Requires<ArgumentNullException>(buffer != null);
            Contract.Requires<ArgumentException>(count > 0);
            Contract.Requires<NotImplementedException>(options == InputStreamOptions.Partial);

            return AsyncInfo.Run<IBuffer, uint>((cancellation, progress) =>
            {
                return Task.Run(() =>
                {
                    IBuffer bufferOut = m_buffers.Take(); // Blocks until a buffer is available

                    Contract.Requires<NotImplementedException>(bufferOut.Length <= count);

                    progress.Report(bufferOut.Length);
                    return bufferOut;
                });
            });
        }

        public IAsyncOperationWithProgress<uint, uint> WriteAsync(IBuffer buffer)
        {
            Contract.Requires<ArgumentNullException>(buffer != null);

            m_buffers.Add(buffer);

            return AsyncInfo.Run<uint, uint>((cancellation, progress) => 
            {
                progress.Report(buffer.Length);
                return Task.FromResult(buffer.Length); 
            });
        }

        public IAsyncOperation<bool> FlushAsync()
        {
            // Nothing to do here
            return AsyncInfo.Run<bool>((cancellation) =>
            {
                return Task.FromResult(true);
            });
        }

        public void Add(byte data)
        {
            m_buffers.Add((new byte[] { data }).AsBuffer());
        }

        public void Add(byte[] data)
        {
            m_buffers.Add(data.AsBuffer());
        }

        public int Count { get { return m_buffers.Count; } }

        public void Dispose()
        {
            m_buffers.Dispose();
        }
    }
}
