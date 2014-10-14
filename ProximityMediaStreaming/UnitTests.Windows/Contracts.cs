using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace UnitTests
{
    /// <summary>
    /// Pre-condition validation akin to System.Diagnostics.Contracts but without the need to install an MSIL rewriter pack
    /// </summary>
    class Contract
    {
        public static void Requires<TException>(bool condition) where TException : Exception, new()
        {
            if (!condition)
            {
                throw new TException();
            }
        }
    }
}
