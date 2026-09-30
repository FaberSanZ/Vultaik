using System;
using System.Numerics;

namespace Vultaik;

public abstract class GameBase : IDisposable
{
    private readonly GameWindow _window = new();
    private readonly GameTime _gameTime = new();
    private readonly RenderSystem _renderer = new();
    private readonly CameraSystem _camera = new();
    private readonly PhysicsSystem _physics = new();
    private readonly World _world = new();

    private ImGuiController? _imgui;
    private bool _initialized;
    private bool _disposed;

    protected GameWindow Window => _window;
    protected GameTime Time => _gameTime;
    protected RenderSystem Renderer => _renderer;
    protected CameraSystem Camera => _camera;
    protected World World => _world;
    protected ImGuiController ImGui => _imgui!;

    protected PhysicsSystem Physics => _physics;

    protected virtual string Title => "VoxelCraft";
    protected virtual uint Width => 1640;
    protected virtual uint Height => 820;
    protected virtual bool Resizable => true;

    public void Run()
    {
        if (_initialized) throw new InvalidOperationException("GameBase.Run() can only be called once.");

        InitializeEngine();

        OnInitialize(_world);
        _gameTime.Reset();

        try
        {
            while (_window.IsRunning)
            {
                GameInput.BeginFrame();
                _window.PumpMessages();

                if (!_window.IsRunning) break;
                if (_window.IsMinimized) continue;

                _gameTime.Update();

                float deltaTime = _gameTime.DeltaTime;

                OnUpdate(_world, deltaTime);
                UpdateCamera(_world);
                _physics.Step(_world, deltaTime);

                _imgui!.BeginFrame(_window.ClientWidth, _window.ClientHeight);
                OnDrawImGui(_world);

                _renderer.BeginFrame();
                OnRender(_world);
                _imgui.Render();
                _renderer.Present();
            }

            OnDestroy(_world);
        }
        finally
        {
            Dispose();
        }
    }

    protected Entity CreateEntity(string name = "Entity")
    {
        Entity entity = _world.Create();

        ref NameComponent nameComponent = ref _world.Set<NameComponent>(entity);
        nameComponent.Name = name;

        ref TransformComponent transform = ref _world.Set<TransformComponent>(entity);
        transform.Position = Vector2.Zero;
        transform.Rotation = 0.0f;
        transform.Scale = Vector2.One;

        return entity;
    }
    protected abstract void OnInitialize(World world);
    protected abstract void OnUpdate(World world, float deltaTime);
    protected virtual void OnRender(World world) { }
    protected virtual void OnDrawImGui(World world) { }
    protected virtual void OnDestroy(World world) { }

    private void InitializeEngine()
    {
        _window.Initialize(new GameWindow.Config
        {
            Title = Title,
            Width = Width,
            Height = Height,
            Resizable = Resizable
        });

        _renderer.Initialize(_window.Handle, _window.ClientWidth, _window.ClientHeight);

        GameInput.Initialize(_window.Handle);

        _imgui = new ImGuiController(_renderer.Device, _renderer.DeviceContext);

        _window.MessageReceived += ProcessInput;
        _window.MessageReceived += _imgui.ProcessMessage;

        _initialized = true;
    }

    private static bool ProcessInput(nint hwnd, uint message, nuint wParam, nint lParam)
    {
        GameInput.ProcessMessage(hwnd, message, wParam, lParam);
        return false;
    }

    private void UpdateCamera(World world)
    {
        if (_window.ClientHeight == 0) return;

        float aspectRatio = _window.ClientWidth / (float)_window.ClientHeight;

        if (_camera.TryGetMatrices(world, aspectRatio, out Matrix4x4 view, out Matrix4x4 projection))
            _renderer.SetCamera(view, projection);
    }

    public void Dispose()
    {
        if (_disposed) return;

        _disposed = true;

        if (_imgui is not null)
        {
            _window.MessageReceived -= ProcessInput;
            _window.MessageReceived -= _imgui.ProcessMessage;
        }

        GameInput.Shutdown();

        _imgui?.Dispose();
        _renderer.Dispose();
        _world.Clear();
        _window.Dispose();

        _imgui = null;

        GC.SuppressFinalize(this);
    }
}