using System;
using System.Numerics;
using System.Runtime.CompilerServices;
using Vortice.D3DCompiler;
using Vortice.Direct3D;
using Vortice.Direct3D11;
using Vortice.DXGI;
using Vortice.Mathematics;
using static Vortice.Direct3D11.D3D11;
using static Vortice.DXGI.DXGI;

namespace Vultaik;

public sealed class RenderSystem : IDisposable
{
    private ID3D11Device _device = null!;
    private ID3D11DeviceContext _deviceContext = null!;
    private IDXGIFactory2 _factory = null!;
    private IDXGISwapChain1 _swapChain = null!;
    private ID3D11Texture2D _backBuffer = null!;
    private ID3D11RenderTargetView _renderTargetView = null!;
    private ID3D11Texture2D _depthStencilBuffer = null!;
    private ID3D11DepthStencilView _depthStencilView = null!;
    private ID3D11DepthStencilState _depthStencilState = null!;
    private ID3D11RasterizerState _rasterizerState = null!;
    private ID3D11VertexShader _vertexShader = null!;
    private ID3D11PixelShader _pixelShader = null!;
    private ID3D11InputLayout _inputLayout = null!;
    private ID3D11Buffer _cameraBuffer = null!;
    private ID3D11Buffer _objectBuffer = null!;

    private CameraBuffer _cameraData;
    private ObjectBuffer _objectData;
    private bool _initialized;
    private bool _disposed;

    public uint Width { get; private set; }
    public uint Height { get; private set; }
    public uint FrameCount => 2;
    public uint DrawCalls { get; private set; }
    public ulong RenderedTriangles { get; private set; }

    public ID3D11Device Device => _device;
    public ID3D11DeviceContext DeviceContext => _deviceContext;

    private struct CameraBuffer
    {
        public Matrix4x4 View;
        public Matrix4x4 Projection;
    }

    private struct ObjectBuffer
    {
        public Matrix4x4 World;
    }

    public void Initialize(nint hwnd, uint width = 1640, uint height = 820)
    {
        if (_initialized) return;

        Width = width;
        Height = height;

        FeatureLevel[] featureLevels = [FeatureLevel.Level_11_1, FeatureLevel.Level_11_0];
        D3D11CreateDevice(IntPtr.Zero, DriverType.Hardware, DeviceCreationFlags.None, featureLevels, out _device, out _, out _deviceContext);

        _factory = CreateDXGIFactory1<IDXGIFactory2>();

        SwapChainDescription1 swapChainDescription = new()
        {
            Width = Width,
            Height = Height,
            Format = Format.R8G8B8A8_UNorm,
            BufferCount = FrameCount,
            BufferUsage = Usage.RenderTargetOutput,
            SampleDescription = SampleDescription.Default,
            Scaling = Scaling.Stretch,
            SwapEffect = SwapEffect.FlipDiscard,
            AlphaMode = AlphaMode.Ignore,
            Flags = SwapChainFlags.AllowTearing
        };

        _swapChain = _factory.CreateSwapChainForHwnd(_device, hwnd, swapChainDescription, new SwapChainFullscreenDescription { Windowed = true });
        _factory.MakeWindowAssociation(hwnd, WindowAssociationFlags.IgnoreAltEnter);

        CreateBackBuffer();
        CreateDepthResources();
        CreateRasterizer();
        CreateShaders();
        CreateCameraBuffer();
        CreateObjectBuffer();

        _initialized = true;
    }

    public void SetCamera(Matrix4x4 view, Matrix4x4 projection)
    {
        _cameraData.View = Matrix4x4.Transpose(view);
        _cameraData.Projection = Matrix4x4.Transpose(projection);
    }

    public void BeginFrame()
    {
        DrawCalls = 0;
        RenderedTriangles = 0;

        _deviceContext.UpdateSubresource(_cameraData, _cameraBuffer);
        _deviceContext.ClearRenderTargetView(_renderTargetView, new Color4(0.0f, 0.2f, 0.4f, 1.0f));
        _deviceContext.ClearDepthStencilView(_depthStencilView, DepthStencilClearFlags.Depth | DepthStencilClearFlags.Stencil, 1.0f, 0);

        _deviceContext.OMSetRenderTargets(_renderTargetView, _depthStencilView);
        _deviceContext.OMSetDepthStencilState(_depthStencilState, 1);
        _deviceContext.RSSetState(_rasterizerState);
        _deviceContext.RSSetViewport(new Viewport(Width, Height));

        _deviceContext.IASetPrimitiveTopology(PrimitiveTopology.TriangleList);
        _deviceContext.IASetInputLayout(_inputLayout);

        _deviceContext.VSSetConstantBuffer(0, _cameraBuffer);
        _deviceContext.VSSetConstantBuffer(1, _objectBuffer);
        _deviceContext.VSSetShader(_vertexShader);
        _deviceContext.PSSetShader(_pixelShader);
    }

    public void Draw(Mesh2D mesh, in TransformComponent transform)
    {
        Matrix4x4 world =
            Matrix4x4.CreateScale(transform.Scale.X, transform.Scale.Y, 1.0f) *
            Matrix4x4.CreateRotationZ(transform.Rotation) *
            Matrix4x4.CreateTranslation(transform.Position.X, transform.Position.Y, 0.0f);

        _objectData.World = Matrix4x4.Transpose(world);
        _deviceContext.UpdateSubresource(_objectData, _objectBuffer);

        _deviceContext.IASetVertexBuffer(0, mesh.VertexBuffer, mesh.VertexStride);
        _deviceContext.IASetIndexBuffer(mesh.IndexBuffer, Format.R32_UInt, 0);
        _deviceContext.DrawIndexed(mesh.IndexCount, 0, 0);

        DrawCalls++;
        RenderedTriangles += mesh.IndexCount / 3;
    }

    public void Present() => _swapChain.Present(0, PresentFlags.AllowTearing);

    private void CreateBackBuffer()
    {
        _backBuffer = _swapChain.GetBuffer<ID3D11Texture2D>(0);
        _renderTargetView = _device.CreateRenderTargetView(_backBuffer);
    }

    private void CreateDepthResources()
    {
        _depthStencilState = _device.CreateDepthStencilState(new DepthStencilDescription(true, DepthWriteMask.All, ComparisonFunction.Less));
        _depthStencilBuffer = _device.CreateTexture2D(Format.D24_UNorm_S8_UInt, Width, Height, mipLevels: 1, bindFlags: BindFlags.DepthStencil);
        _depthStencilView = _device.CreateDepthStencilView(_depthStencilBuffer);
    }

    private void CreateRasterizer()
    {
        _rasterizerState = _device.CreateRasterizerState(RasterizerDescription.CullNone);
    }

    private void CreateShaders()
    {
        ReadOnlyMemory<byte> vertexShaderByteCode = Compiler.CompileFromFile("Vertex.hlsl", "VS", "vs_5_0");
        ReadOnlyMemory<byte> pixelShaderByteCode = Compiler.CompileFromFile("Pixel.hlsl", "PS", "ps_5_0");

        _vertexShader = _device.CreateVertexShader(vertexShaderByteCode.Span);
        _pixelShader = _device.CreatePixelShader(pixelShaderByteCode.Span);

        InputElementDescription[] inputElements =
        [
            new InputElementDescription("POSITION", 0, Format.R32G32_Float, 0, 0),
            new InputElementDescription("COLOR", 0, Format.R32G32B32A32_Float, 8, 0)
        ];

        _inputLayout = _device.CreateInputLayout(inputElements, vertexShaderByteCode.Span);
    }

    private void CreateCameraBuffer()
    {
        BufferDescription description = new()
        {
            ByteWidth = (uint)Unsafe.SizeOf<CameraBuffer>(),
            Usage = ResourceUsage.Default,
            BindFlags = BindFlags.ConstantBuffer,
            CPUAccessFlags = CpuAccessFlags.None
        };

        _cameraBuffer = _device.CreateBuffer(_cameraData, description);
    }

    private void CreateObjectBuffer()
    {
        BufferDescription description = new()
        {
            ByteWidth = (uint)Unsafe.SizeOf<ObjectBuffer>(),
            Usage = ResourceUsage.Default,
            BindFlags = BindFlags.ConstantBuffer,
            CPUAccessFlags = CpuAccessFlags.None
        };

        _objectBuffer = _device.CreateBuffer(_objectData, description);
    }

    public void Dispose()
    {
        if (_disposed) return;
        _disposed = true;

        if (_initialized)
        {
            _deviceContext.ClearState();
            _deviceContext.Flush();

            _objectBuffer.Dispose();
            _cameraBuffer.Dispose();
            _inputLayout.Dispose();
            _pixelShader.Dispose();
            _vertexShader.Dispose();
            _rasterizerState.Dispose();
            _depthStencilState.Dispose();
            _depthStencilView.Dispose();
            _depthStencilBuffer.Dispose();
            _renderTargetView.Dispose();
            _backBuffer.Dispose();
            _swapChain.Dispose();
            _factory.Dispose();
            _deviceContext.Dispose();
            _device.Dispose();
        }

        _initialized = false;
        GC.SuppressFinalize(this);
    }
}
