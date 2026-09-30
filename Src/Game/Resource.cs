using Vortice.Direct3D11;

namespace Vultaik;

public sealed class Resource : IDisposable
{
    public ID3D11Buffer? Buffer;
    public ID3D11ShaderResourceView? View;

    public uint Stride;
    public uint Count;

    public void Dispose()
    {
        View?.Dispose();
        Buffer?.Dispose();

        View = null;
        Buffer = null;
    }
}