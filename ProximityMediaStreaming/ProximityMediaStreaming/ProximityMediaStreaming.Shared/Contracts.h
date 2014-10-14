/// <summary>
/// Pre-condition validation akin to System.Diagnostics.Contracts but without the need to install an MSIL rewriter pack
/// </summary>
class Contract
{
public:

    template<typename TException>
    static void Requires(bool condition)
    {
        if (!condition)
        {
            throw ref new TException();
        }
    }

};
