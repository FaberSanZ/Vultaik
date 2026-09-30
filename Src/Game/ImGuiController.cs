using ImGuiNET;
using System.Diagnostics;
using System.Numerics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Vortice;
using Vortice.D3DCompiler;
using Vortice.Direct3D;
using Vortice.Direct3D11;
using Vortice.DXGI;
using Vortice.Mathematics;

namespace Vultaik;

public sealed unsafe class ImGuiController : IDisposable
{
    private readonly ID3D11Device _device;
    private readonly ID3D11DeviceContext _deviceContext;
    private readonly Stopwatch _timer = Stopwatch.StartNew();
    private readonly Dictionary<nint, ID3D11ShaderResourceView> _textures = [];

    private nint _context;
    private nint _fontTextureId;
    private nint _nextTextureId = 1;
    private double _lastTime;
    private int _mouseButtonsDown;

    private ID3D11Buffer? _vertexBuffer;
    private ID3D11Buffer? _indexBuffer;
    private ID3D11Buffer? _constantBuffer;
    private ID3D11VertexShader? _vertexShader;
    private ID3D11PixelShader? _pixelShader;
    private ID3D11InputLayout? _inputLayout;
    private ID3D11ShaderResourceView? _fontTextureView;
    private ID3D11SamplerState? _fontSampler;
    private ID3D11BlendState? _blendState;
    private ID3D11RasterizerState? _rasterizerState;
    private ID3D11DepthStencilState? _depthStencilState;

    private int _vertexBufferSize = 5000;
    private int _indexBufferSize = 10000;

    private struct TransformBuffer
    {
        public Vector2 Scale;
        public Vector2 Translate;
    }

    public ImGuiController(ID3D11Device device, ID3D11DeviceContext deviceContext)
    {
        _device = device;
        _deviceContext = deviceContext;

        _context = ImGui.CreateContext();
        ImGui.SetCurrentContext(_context);

        ImGuiIOPtr io = ImGui.GetIO();
        io.ConfigFlags |= ImGuiConfigFlags.NavEnableKeyboard | ImGuiConfigFlags.DockingEnable;
        io.BackendFlags |= ImGuiBackendFlags.RendererHasVtxOffset;

        ImGui.StyleColorsDark();

        CreateDeviceResources();
        CreateFontTexture();
    }

    public void BeginFrame(uint width, uint height)
    {


        ImGui.SetCurrentContext(_context);

        ImGuiIOPtr io = ImGui.GetIO();
        io.DisplaySize = new Vector2(width, height);
        io.DisplayFramebufferScale = Vector2.One;

        double currentTime = _timer.Elapsed.TotalSeconds;
        io.DeltaTime = _lastTime > 0.0 ? (float)(currentTime - _lastTime) : 1.0f / 60.0f;
        _lastTime = currentTime;

        ImGui.NewFrame();
    }

    public void Render()
    {
        ImGui.SetCurrentContext(_context);
        ImGui.Render();
        RenderDrawData(ImGui.GetDrawData());
    }

    public nint BindTexture(ID3D11ShaderResourceView texture)
    {
        nint id = _nextTextureId++;
        _textures[id] = texture;
        return id;
    }

    public void UnbindTexture(nint textureId)
    {
        if (textureId == _fontTextureId)
            return;

        _textures.Remove(textureId);
    }

    public bool ProcessMessage(nint hwnd, uint message, nuint wParam, nint lParam)
    {
        if (_context == 0)
            return false;

        ImGui.SetCurrentContext(_context);
        ImGuiIOPtr io = ImGui.GetIO();

        switch (message)
        {
            case WM_MOUSEMOVE:
                io.AddMousePosEvent(GetSignedLowWord(lParam), GetSignedHighWord(lParam));
                return io.WantCaptureMouse;

            case WM_LBUTTONDOWN:
                SetMouseButton(hwnd, io, 0, true);
                return io.WantCaptureMouse;

            case WM_LBUTTONUP:
                SetMouseButton(hwnd, io, 0, false);
                return io.WantCaptureMouse;

            case WM_RBUTTONDOWN:
                SetMouseButton(hwnd, io, 1, true);
                return io.WantCaptureMouse;

            case WM_RBUTTONUP:
                SetMouseButton(hwnd, io, 1, false);
                return io.WantCaptureMouse;

            case WM_MBUTTONDOWN:
                SetMouseButton(hwnd, io, 2, true);
                return io.WantCaptureMouse;

            case WM_MBUTTONUP:
                SetMouseButton(hwnd, io, 2, false);
                return io.WantCaptureMouse;

            case WM_XBUTTONDOWN:
                {
                    int button = GetHighWord(wParam) == XBUTTON1 ? 3 : 4;
                    SetMouseButton(hwnd, io, button, true);
                    return io.WantCaptureMouse;
                }

            case WM_XBUTTONUP:
                {
                    int button = GetHighWord(wParam) == XBUTTON1 ? 3 : 4;
                    SetMouseButton(hwnd, io, button, false);
                    return io.WantCaptureMouse;
                }

            case WM_MOUSEWHEEL:
                io.AddMouseWheelEvent(0.0f, GetWheelDelta(wParam) / (float)WHEEL_DELTA);
                return io.WantCaptureMouse;

            case WM_MOUSEHWHEEL:
                io.AddMouseWheelEvent(-GetWheelDelta(wParam) / (float)WHEEL_DELTA, 0.0f);
                return io.WantCaptureMouse;

            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
                UpdateKeyModifiers(io);
                AddKeyEvent(io, wParam, lParam, true);
                return io.WantCaptureKeyboard;

            case WM_KEYUP:
            case WM_SYSKEYUP:
                UpdateKeyModifiers(io);
                AddKeyEvent(io, wParam, lParam, false);
                return io.WantCaptureKeyboard;

            case WM_CHAR:
                if (wParam > 0 && wParam < 0x10000)
                    io.AddInputCharacter((uint)wParam);
                return io.WantTextInput;

            case WM_SETFOCUS:
                io.AddFocusEvent(true);
                return false;

            case WM_KILLFOCUS:
                io.AddFocusEvent(false);
                return false;
        }

        return false;
    }

    private void CreateDeviceResources()
    {
        ReadOnlyMemory<byte> vertexShaderByteCode = Compiler.Compile(VertexShaderSource, "VSMain", "ImGuiVertex.hlsl", "vs_5_0");
        ReadOnlyMemory<byte> pixelShaderByteCode = Compiler.Compile(PixelShaderSource, "PSMain", "ImGuiPixel.hlsl", "ps_5_0");

        _vertexShader = _device.CreateVertexShader(vertexShaderByteCode.Span);
        _pixelShader = _device.CreatePixelShader(pixelShaderByteCode.Span);

        InputElementDescription[] inputElements =
        [
            new("POSITION", 0, Format.R32G32_Float, 0, 0),
            new("TEXCOORD", 0, Format.R32G32_Float, 8, 0),
            new("COLOR", 0, Format.R8G8B8A8_UNorm, 16, 0)
        ];

        _inputLayout = _device.CreateInputLayout(inputElements, vertexShaderByteCode.Span);
        _vertexBuffer = CreateDynamicBuffer((uint)(_vertexBufferSize * Unsafe.SizeOf<ImDrawVert>()), BindFlags.VertexBuffer);
        _indexBuffer = CreateDynamicBuffer((uint)(_indexBufferSize * sizeof(ushort)), BindFlags.IndexBuffer);
        _constantBuffer = _device.CreateBuffer((uint)Unsafe.SizeOf<TransformBuffer>(), BindFlags.ConstantBuffer);
        _fontSampler = _device.CreateSamplerState(SamplerDescription.LinearClamp);

        BlendDescription blendDescription = new(Blend.SourceAlpha, Blend.InverseSourceAlpha, Blend.One, Blend.InverseSourceAlpha);
        blendDescription.RenderTarget[0].BlendEnable = true;
        _blendState = _device.CreateBlendState(blendDescription);

        RasterizerDescription rasterizerDescription = new(CullMode.None, FillMode.Solid)
        {
            ScissorEnable = true,
            DepthClipEnable = true
        };

        _rasterizerState = _device.CreateRasterizerState(rasterizerDescription);
        _depthStencilState = _device.CreateDepthStencilState(DepthStencilDescription.None);
    }

    private ID3D11Buffer CreateDynamicBuffer(uint size, BindFlags bindFlags)
    {
        return _device.CreateBuffer(size, bindFlags, ResourceUsage.Dynamic, CpuAccessFlags.Write);
    }

    private void CreateFontTexture()
    {
        ImGuiIOPtr io = ImGui.GetIO();
        io.Fonts.GetTexDataAsRGBA32(out nint pixels, out int width, out int height, out int bytesPerPixel);

        ReadOnlySpan<byte> pixelData = new((void*)pixels, width * height * bytesPerPixel);

        using ID3D11Texture2D texture = _device.CreateTexture2D(
            pixelData,
            Format.R8G8B8A8_UNorm,
            (uint)width,
            (uint)height,
            mipLevels: 1,
            bindFlags: BindFlags.ShaderResource
        );

        _fontTextureView = _device.CreateShaderResourceView(texture);
        _fontTextureId = BindTexture(_fontTextureView);

        io.Fonts.SetTexID(_fontTextureId);
        io.Fonts.ClearTexData();
    }

    private void RenderDrawData(ImDrawDataPtr drawData)
    {
        int framebufferWidth = (int)(drawData.DisplaySize.X * drawData.FramebufferScale.X);
        int framebufferHeight = (int)(drawData.DisplaySize.Y * drawData.FramebufferScale.Y);

        if (framebufferWidth <= 0 || framebufferHeight <= 0 || drawData.CmdListsCount == 0)
            return;

        EnsureBufferCapacity(drawData);
        UploadDrawData(drawData);

        TransformBuffer transform = new()
        {
            Scale = new Vector2(2.0f / drawData.DisplaySize.X, -2.0f / drawData.DisplaySize.Y),
            Translate = new Vector2(
                -1.0f - drawData.DisplayPos.X * (2.0f / drawData.DisplaySize.X),
                1.0f + drawData.DisplayPos.Y * (2.0f / drawData.DisplaySize.Y)
            )
        };

        _deviceContext.UpdateSubresource(transform, _constantBuffer!);
        SetupRenderState(framebufferWidth, framebufferHeight);

        Vector2 clipOffset = drawData.DisplayPos;
        Vector2 clipScale = drawData.FramebufferScale;
        uint globalIndexOffset = 0;
        uint globalVertexOffset = 0;

        for (int listIndex = 0; listIndex < drawData.CmdListsCount; listIndex++)
        {
            ImDrawListPtr commandList = drawData.CmdLists[listIndex];

            for (int commandIndex = 0; commandIndex < commandList.CmdBuffer.Size; commandIndex++)
            {
                ImDrawCmdPtr command = commandList.CmdBuffer[commandIndex];

                if (command.UserCallback != IntPtr.Zero)
                    continue;

                Vector4 clipRect = command.ClipRect;
                float clipMinX = (clipRect.X - clipOffset.X) * clipScale.X;
                float clipMinY = (clipRect.Y - clipOffset.Y) * clipScale.Y;
                float clipMaxX = (clipRect.Z - clipOffset.X) * clipScale.X;
                float clipMaxY = (clipRect.W - clipOffset.Y) * clipScale.Y;

                if (clipMaxX <= clipMinX || clipMaxY <= clipMinY)
                    continue;

                int left = Math.Max((int)clipMinX, 0);
                int top = Math.Max((int)clipMinY, 0);
                int right = Math.Min((int)clipMaxX, framebufferWidth);
                int bottom = Math.Min((int)clipMaxY, framebufferHeight);

                if (right <= left || bottom <= top)
                    continue;

                _deviceContext.RSSetScissorRect(new RawRect(left, top, right, bottom));

                if (!_textures.TryGetValue(command.TextureId, out ID3D11ShaderResourceView? texture))
                    continue;

                _deviceContext.PSSetShaderResource(0, texture);
                _deviceContext.DrawIndexed(command.ElemCount, command.IdxOffset + globalIndexOffset, (int)(command.VtxOffset + globalVertexOffset));
            }

            globalIndexOffset += (uint)commandList.IdxBuffer.Size;
            globalVertexOffset += (uint)commandList.VtxBuffer.Size;
        }
    }

    private void EnsureBufferCapacity(ImDrawDataPtr drawData)
    {
        if (drawData.TotalVtxCount > _vertexBufferSize)
        {
            _vertexBufferSize = drawData.TotalVtxCount + 5000;
            _vertexBuffer?.Dispose();
            _vertexBuffer = CreateDynamicBuffer((uint)(_vertexBufferSize * Unsafe.SizeOf<ImDrawVert>()), BindFlags.VertexBuffer);
        }

        if (drawData.TotalIdxCount > _indexBufferSize)
        {
            _indexBufferSize = drawData.TotalIdxCount + 10000;
            _indexBuffer?.Dispose();
            _indexBuffer = CreateDynamicBuffer((uint)(_indexBufferSize * sizeof(ushort)), BindFlags.IndexBuffer);
        }
    }

    private void UploadDrawData(ImDrawDataPtr drawData)
    {
        MappedSubresource vertexMapped = _deviceContext.Map(_vertexBuffer!, MapMode.WriteDiscard);
        MappedSubresource indexMapped = _deviceContext.Map(_indexBuffer!, MapMode.WriteDiscard);

        byte* vertexDestination = (byte*)vertexMapped.DataPointer;
        byte* indexDestination = (byte*)indexMapped.DataPointer;

        for (int listIndex = 0; listIndex < drawData.CmdListsCount; listIndex++)
        {
            ImDrawListPtr commandList = drawData.CmdLists[listIndex];
            int vertexBytes = commandList.VtxBuffer.Size * Unsafe.SizeOf<ImDrawVert>();
            int indexBytes = commandList.IdxBuffer.Size * sizeof(ushort);

            Buffer.MemoryCopy((void*)commandList.VtxBuffer.Data, vertexDestination, vertexBytes, vertexBytes);
            Buffer.MemoryCopy((void*)commandList.IdxBuffer.Data, indexDestination, indexBytes, indexBytes);

            vertexDestination += vertexBytes;
            indexDestination += indexBytes;
        }

        _deviceContext.Unmap(_vertexBuffer!);
        _deviceContext.Unmap(_indexBuffer!);
    }

    private void SetupRenderState(int width, int height)
    {
        _deviceContext.RSSetViewport(new Viewport(width, height));
        _deviceContext.IASetInputLayout(_inputLayout);
        _deviceContext.IASetVertexBuffer(0, _vertexBuffer!, (uint)Unsafe.SizeOf<ImDrawVert>());
        _deviceContext.IASetIndexBuffer(_indexBuffer!, Format.R16_UInt, 0);
        _deviceContext.IASetPrimitiveTopology(PrimitiveTopology.TriangleList);

        _deviceContext.VSSetShader(_vertexShader);
        _deviceContext.VSSetConstantBuffer(0, _constantBuffer);
        _deviceContext.PSSetShader(_pixelShader);
        _deviceContext.PSSetSampler(0, _fontSampler);

        _deviceContext.OMSetBlendState(_blendState);
        _deviceContext.OMSetDepthStencilState(_depthStencilState, 0);
        _deviceContext.RSSetState(_rasterizerState);
    }

    private void SetMouseButton(nint hwnd, ImGuiIOPtr io, int button, bool down)
    {
        int mask = 1 << button;

        if (down)
        {
            if (_mouseButtonsDown == 0)
                SetCapture(hwnd);

            _mouseButtonsDown |= mask;
        }
        else
        {
            _mouseButtonsDown &= ~mask;

            if (_mouseButtonsDown == 0 && GetCapture() == hwnd)
                ReleaseCapture();
        }

        io.AddMouseButtonEvent(button, down);
    }

    private static void AddKeyEvent(ImGuiIOPtr io, nuint wParam, nint lParam, bool down)
    {
        ImGuiKey key = MapKey((int)wParam, lParam);

        if (key != ImGuiKey.None)
            io.AddKeyEvent(key, down);
    }

    private static void UpdateKeyModifiers(ImGuiIOPtr io)
    {
        io.AddKeyEvent(ImGuiKey.ModCtrl, IsKeyDown(VK_CONTROL));
        io.AddKeyEvent(ImGuiKey.ModShift, IsKeyDown(VK_SHIFT));
        io.AddKeyEvent(ImGuiKey.ModAlt, IsKeyDown(VK_MENU));
        io.AddKeyEvent(ImGuiKey.ModSuper, IsKeyDown(VK_LWIN) || IsKeyDown(VK_RWIN));
    }

    private static ImGuiKey MapKey(int key, nint lParam)
    {
        bool extended = (lParam.ToInt64() & (1L << 24)) != 0;

        if (key >= VK_0 && key <= VK_9)
            return (ImGuiKey)((int)ImGuiKey._0 + key - VK_0);

        if (key >= VK_A && key <= VK_Z)
            return (ImGuiKey)((int)ImGuiKey.A + key - VK_A);

        if (key >= VK_F1 && key <= VK_F12)
            return (ImGuiKey)((int)ImGuiKey.F1 + key - VK_F1);

        if (key >= VK_NUMPAD0 && key <= VK_NUMPAD9)
            return (ImGuiKey)((int)ImGuiKey.Keypad0 + key - VK_NUMPAD0);

        return key switch
        {
            VK_TAB => ImGuiKey.Tab,
            VK_LEFT => ImGuiKey.LeftArrow,
            VK_RIGHT => ImGuiKey.RightArrow,
            VK_UP => ImGuiKey.UpArrow,
            VK_DOWN => ImGuiKey.DownArrow,
            VK_PRIOR => ImGuiKey.PageUp,
            VK_NEXT => ImGuiKey.PageDown,
            VK_HOME => ImGuiKey.Home,
            VK_END => ImGuiKey.End,
            VK_INSERT => ImGuiKey.Insert,
            VK_DELETE => ImGuiKey.Delete,
            VK_BACK => ImGuiKey.Backspace,
            VK_SPACE => ImGuiKey.Space,
            VK_RETURN => extended ? ImGuiKey.KeypadEnter : ImGuiKey.Enter,
            VK_ESCAPE => ImGuiKey.Escape,
            VK_LCONTROL => ImGuiKey.LeftCtrl,
            VK_RCONTROL => ImGuiKey.RightCtrl,
            VK_CONTROL => extended ? ImGuiKey.RightCtrl : ImGuiKey.LeftCtrl,
            VK_LSHIFT => ImGuiKey.LeftShift,
            VK_RSHIFT => ImGuiKey.RightShift,
            VK_SHIFT => ImGuiKey.LeftShift,
            VK_LMENU => ImGuiKey.LeftAlt,
            VK_RMENU => ImGuiKey.RightAlt,
            VK_MENU => extended ? ImGuiKey.RightAlt : ImGuiKey.LeftAlt,
            VK_LWIN => ImGuiKey.LeftSuper,
            VK_RWIN => ImGuiKey.RightSuper,
            VK_APPS => ImGuiKey.Menu,
            VK_OEM_7 => ImGuiKey.Apostrophe,
            VK_OEM_COMMA => ImGuiKey.Comma,
            VK_OEM_MINUS => ImGuiKey.Minus,
            VK_OEM_PERIOD => ImGuiKey.Period,
            VK_OEM_2 => ImGuiKey.Slash,
            VK_OEM_1 => ImGuiKey.Semicolon,
            VK_OEM_PLUS => ImGuiKey.Equal,
            VK_OEM_4 => ImGuiKey.LeftBracket,
            VK_OEM_5 => ImGuiKey.Backslash,
            VK_OEM_6 => ImGuiKey.RightBracket,
            VK_OEM_3 => ImGuiKey.GraveAccent,
            VK_CAPITAL => ImGuiKey.CapsLock,
            VK_SCROLL => ImGuiKey.ScrollLock,
            VK_NUMLOCK => ImGuiKey.NumLock,
            VK_SNAPSHOT => ImGuiKey.PrintScreen,
            VK_PAUSE => ImGuiKey.Pause,
            VK_DECIMAL => ImGuiKey.KeypadDecimal,
            VK_DIVIDE => ImGuiKey.KeypadDivide,
            VK_MULTIPLY => ImGuiKey.KeypadMultiply,
            VK_SUBTRACT => ImGuiKey.KeypadSubtract,
            VK_ADD => ImGuiKey.KeypadAdd,
            _ => ImGuiKey.None
        };
    }

    private static bool IsKeyDown(int key)
    {
        return (GetKeyState(key) & 0x8000) != 0;
    }

    private static float GetSignedLowWord(nint value)
    {
        return (short)(value.ToInt64() & 0xFFFF);
    }

    private static float GetSignedHighWord(nint value)
    {
        return (short)((value.ToInt64() >> 16) & 0xFFFF);
    }

    private static ushort GetHighWord(nuint value)
    {
        return (ushort)((value >> 16) & 0xFFFF);
    }

    private static short GetWheelDelta(nuint value)
    {
        return (short)((value >> 16) & 0xFFFF);
    }

    public void Dispose()
    {
        if (_context == 0)
            return;

        ImGui.SetCurrentContext(_context);
        ImGui.GetIO().Fonts.SetTexID(IntPtr.Zero);

        _textures.Clear();

        _depthStencilState?.Dispose();
        _rasterizerState?.Dispose();
        _blendState?.Dispose();
        _fontSampler?.Dispose();
        _fontTextureView?.Dispose();
        _inputLayout?.Dispose();
        _pixelShader?.Dispose();
        _vertexShader?.Dispose();
        _constantBuffer?.Dispose();
        _indexBuffer?.Dispose();
        _vertexBuffer?.Dispose();

        ImGui.DestroyContext(_context);
        _context = 0;
    }

    private const uint WM_SETFOCUS = 0x0007;
    private const uint WM_KILLFOCUS = 0x0008;
    private const uint WM_KEYDOWN = 0x0100;
    private const uint WM_KEYUP = 0x0101;
    private const uint WM_CHAR = 0x0102;
    private const uint WM_SYSKEYDOWN = 0x0104;
    private const uint WM_SYSKEYUP = 0x0105;
    private const uint WM_MOUSEMOVE = 0x0200;
    private const uint WM_LBUTTONDOWN = 0x0201;
    private const uint WM_LBUTTONUP = 0x0202;
    private const uint WM_RBUTTONDOWN = 0x0204;
    private const uint WM_RBUTTONUP = 0x0205;
    private const uint WM_MBUTTONDOWN = 0x0207;
    private const uint WM_MBUTTONUP = 0x0208;
    private const uint WM_MOUSEWHEEL = 0x020A;
    private const uint WM_XBUTTONDOWN = 0x020B;
    private const uint WM_XBUTTONUP = 0x020C;
    private const uint WM_MOUSEHWHEEL = 0x020E;

    private const int WHEEL_DELTA = 120;
    private const ushort XBUTTON1 = 0x0001;

    private const int VK_BACK = 0x08;
    private const int VK_TAB = 0x09;
    private const int VK_RETURN = 0x0D;
    private const int VK_SHIFT = 0x10;
    private const int VK_CONTROL = 0x11;
    private const int VK_MENU = 0x12;
    private const int VK_PAUSE = 0x13;
    private const int VK_CAPITAL = 0x14;
    private const int VK_ESCAPE = 0x1B;
    private const int VK_SPACE = 0x20;
    private const int VK_PRIOR = 0x21;
    private const int VK_NEXT = 0x22;
    private const int VK_END = 0x23;
    private const int VK_HOME = 0x24;
    private const int VK_LEFT = 0x25;
    private const int VK_UP = 0x26;
    private const int VK_RIGHT = 0x27;
    private const int VK_DOWN = 0x28;
    private const int VK_SNAPSHOT = 0x2C;
    private const int VK_INSERT = 0x2D;
    private const int VK_DELETE = 0x2E;
    private const int VK_0 = 0x30;
    private const int VK_9 = 0x39;
    private const int VK_A = 0x41;
    private const int VK_Z = 0x5A;
    private const int VK_LWIN = 0x5B;
    private const int VK_RWIN = 0x5C;
    private const int VK_APPS = 0x5D;
    private const int VK_NUMPAD0 = 0x60;
    private const int VK_NUMPAD9 = 0x69;
    private const int VK_MULTIPLY = 0x6A;
    private const int VK_ADD = 0x6B;
    private const int VK_SUBTRACT = 0x6D;
    private const int VK_DECIMAL = 0x6E;
    private const int VK_DIVIDE = 0x6F;
    private const int VK_F1 = 0x70;
    private const int VK_F12 = 0x7B;
    private const int VK_NUMLOCK = 0x90;
    private const int VK_SCROLL = 0x91;
    private const int VK_LSHIFT = 0xA0;
    private const int VK_RSHIFT = 0xA1;
    private const int VK_LCONTROL = 0xA2;
    private const int VK_RCONTROL = 0xA3;
    private const int VK_LMENU = 0xA4;
    private const int VK_RMENU = 0xA5;
    private const int VK_OEM_1 = 0xBA;
    private const int VK_OEM_PLUS = 0xBB;
    private const int VK_OEM_COMMA = 0xBC;
    private const int VK_OEM_MINUS = 0xBD;
    private const int VK_OEM_PERIOD = 0xBE;
    private const int VK_OEM_2 = 0xBF;
    private const int VK_OEM_3 = 0xC0;
    private const int VK_OEM_4 = 0xDB;
    private const int VK_OEM_5 = 0xDC;
    private const int VK_OEM_6 = 0xDD;
    private const int VK_OEM_7 = 0xDE;

    [DllImport("user32.dll")]
    private static extern nint SetCapture(nint hwnd);

    [DllImport("user32.dll")]
    private static extern nint GetCapture();

    [DllImport("user32.dll")]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool ReleaseCapture();

    [DllImport("user32.dll")]
    private static extern short GetKeyState(int key);

    private const string VertexShaderSource = """
cbuffer ImGuiTransform : register(b0)
{
    float2 Scale;
    float2 Translate;
};

struct VSInput
{
    float2 Position : POSITION;
    float2 UV : TEXCOORD0;
    float4 Color : COLOR0;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
    float4 Color : COLOR0;
};

PSInput VSMain(VSInput input)
{
    PSInput output;
    output.Position = float4(input.Position * Scale + Translate, 0.0f, 1.0f);
    output.UV = input.UV;
    output.Color = input.Color;
    return output;
}
""";

    private const string PixelShaderSource = """
Texture2D Texture : register(t0);
SamplerState TextureSampler : register(s0);

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
    float4 Color : COLOR0;
};

float4 PSMain(PSInput input) : SV_Target
{
    return input.Color * Texture.Sample(TextureSampler, input.UV);
}
""";
}
