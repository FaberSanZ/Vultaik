using System.Runtime.InteropServices;

namespace Vultaik;

public sealed class GameWindow : IDisposable
{
    public sealed class Config
    {
        public string Title { get; init; } = "Game";
        public uint Width { get; init; } = 1280;
        public uint Height { get; init; } = 720;
        public bool Resizable { get; init; } = true;
    }

    public delegate bool WindowMessageHandler(nint hwnd, uint message, nuint wParam, nint lParam);

    public event WindowMessageHandler? MessageReceived;

    public nint Handle => _handle;
    public uint ClientWidth => _clientWidth;
    public uint ClientHeight => _clientHeight;
    public bool IsRunning => _isRunning;
    public bool IsFocused => _isFocused;
    public bool IsMinimized => _isMinimized;
    public bool WasResized => _wasResized;

    private const string WindowClassName = "Vultaik.GameWindow";

    private nint _instance;
    private nint _handle;
    private uint _style;
    private uint _clientWidth;
    private uint _clientHeight;

    private bool _isRunning;
    private bool _isFocused;
    private bool _isMinimized;
    private bool _wasResized;
    private bool _classRegistered;

    private static GameWindow? _currentWindow;
    private static readonly WindowProcDelegate WindowProcCallback = WindowProc;

    public void Initialize(Config? config = null)
    {
        if (_handle != 0) return;

        config ??= new Config();

        EnableDpiAwareness();

        _instance = GetModuleHandleW(null);
        _currentWindow = this;

        WndClass windowClass = new()
        {
            Style = CS_HREDRAW | CS_VREDRAW,
            WindowProc = Marshal.GetFunctionPointerForDelegate(WindowProcCallback),
            Instance = _instance,
            Cursor = LoadCursorW(0, (nint)IDC_ARROW),
            ClassName = WindowClassName
        };

        ushort atom = RegisterClassW(ref windowClass);

        if (atom == 0)
        {
            if (Marshal.GetLastWin32Error() != ERROR_CLASS_ALREADY_EXISTS) throw new InvalidOperationException("Failed to register window class.");
        }
        else
        {
            _classRegistered = true;
        }

        _style = WS_OVERLAPPEDWINDOW;

        if (!config.Resizable)
        {
            _style &= ~WS_THICKFRAME;
            _style &= ~WS_MAXIMIZEBOX;
        }

        Rect windowRect = new() { Right = (int)config.Width, Bottom = (int)config.Height };

        AdjustWindowRectExForDpi(ref windowRect, _style, false, 0, GetDpiForSystem());

        int width = windowRect.Right - windowRect.Left;
        int height = windowRect.Bottom - windowRect.Top;

        _handle = CreateWindowExW(0, WindowClassName, config.Title, _style, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, _instance, 0);

        if (_handle == 0) throw new InvalidOperationException("Failed to create game window.");

        ShowWindow(_handle, SW_SHOW);
        UpdateWindow(_handle);

        UpdateClientSize();

        GameInput.Initialize(_handle);

        _isFocused = GetFocus() == _handle;
        _isRunning = true;
    }

    public void PumpMessages()
    {
        GameInput.BeginFrame();

        while (PeekMessageW(out Message message, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(ref message);
            DispatchMessageW(ref message);

            if (message.Id == WM_QUIT) _isRunning = false;
        }
    }

    public void ClearResizeFlag() => _wasResized = false;

    public void SetTitle(string title)
    {
        if (_handle != 0) SetWindowTextW(_handle, title);
    }

    public void SetClientSize(uint width, uint height)
    {
        if (_handle == 0) return;

        Rect rect = new() { Right = (int)width, Bottom = (int)height };

        AdjustWindowRectExForDpi(ref rect, _style, false, 0, GetDpiForWindow(_handle));
        SetWindowPos(_handle, 0, 0, 0, rect.Right - rect.Left, rect.Bottom - rect.Top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }

    private nint HandleMessage(uint message, nuint wParam, nint lParam)
    {
        nint inputResult = GameInput.ProcessMessage(_handle, message, wParam, lParam);
        bool imguiHandled = MessageReceived?.Invoke(_handle, message, wParam, lParam) == true;

        switch (message)
        {
            case WM_SETFOCUS:
                _isFocused = true;
                break;

            case WM_KILLFOCUS:
                _isFocused = false;
                break;

            case WM_SIZE:
                if (wParam == SIZE_MINIMIZED)
                {
                    _isMinimized = true;
                    break;
                }

                _isMinimized = false;

                uint width = LowWord(lParam);
                uint height = HighWord(lParam);

                if (width > 0 && height > 0 && (width != _clientWidth || height != _clientHeight))
                {
                    _clientWidth = width;
                    _clientHeight = height;
                    _wasResized = true;
                }

                break;

            case WM_CLOSE:
                DestroyWindow(_handle);
                return 0;

            case WM_DESTROY:
                _isRunning = false;
                PostQuitMessage(0);
                return 0;

            case WM_ERASEBKGND:
                return 1;
        }

        if (imguiHandled) return 1;
        if (inputResult != 0) return inputResult;

        return DefWindowProcW(_handle, message, wParam, lParam);
    }

    private static nint WindowProc(nint handle, uint message, nuint wParam, nint lParam)
    {
        if (_currentWindow is null) return DefWindowProcW(handle, message, wParam, lParam);

        if (_currentWindow._handle == 0) _currentWindow._handle = handle;

        return _currentWindow.HandleMessage(message, wParam, lParam);
    }

    private void UpdateClientSize()
    {
        if (_handle == 0 || !GetClientRect(_handle, out Rect rect)) return;

        _clientWidth = (uint)(rect.Right - rect.Left);
        _clientHeight = (uint)(rect.Bottom - rect.Top);
    }

    public void Dispose()
    {
        GameInput.Shutdown();

        if (_handle != 0)
        {
            DestroyWindow(_handle);
            _handle = 0;
        }

        if (_classRegistered)
        {
            UnregisterClassW(WindowClassName, _instance);
            _classRegistered = false;
        }

        _isRunning = false;
        _currentWindow = null;
    }

    private static void EnableDpiAwareness() => SetProcessDpiAwarenessContext((nint)(-4));
    private static uint LowWord(nint value) => (uint)(value.ToInt64() & 0xFFFF);
    private static uint HighWord(nint value) => (uint)((value.ToInt64() >> 16) & 0xFFFF);

    private delegate nint WindowProcDelegate(nint hwnd, uint message, nuint wParam, nint lParam);

    private const uint CS_VREDRAW = 0x0001;
    private const uint CS_HREDRAW = 0x0002;
    private const uint WS_OVERLAPPEDWINDOW = 0x00CF0000;
    private const uint WS_THICKFRAME = 0x00040000;
    private const uint WS_MAXIMIZEBOX = 0x00010000;
    private const uint WM_DESTROY = 0x0002;
    private const uint WM_SIZE = 0x0005;
    private const uint WM_SETFOCUS = 0x0007;
    private const uint WM_KILLFOCUS = 0x0008;
    private const uint WM_CLOSE = 0x0010;
    private const uint WM_QUIT = 0x0012;
    private const uint WM_ERASEBKGND = 0x0014;
    private const nuint SIZE_MINIMIZED = 1;
    private const uint PM_REMOVE = 0x0001;
    private const int SW_SHOW = 5;
    private const int CW_USEDEFAULT = unchecked((int)0x80000000);
    private const int IDC_ARROW = 32512;
    private const uint SWP_NOMOVE = 0x0002;
    private const uint SWP_NOZORDER = 0x0004;
    private const uint SWP_NOACTIVATE = 0x0010;
    private const int ERROR_CLASS_ALREADY_EXISTS = 1410;

    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
    private struct WndClass
    {
        public uint Style;
        public nint WindowProc;
        public int ClassExtra;
        public int WindowExtra;
        public nint Instance;
        public nint Icon;
        public nint Cursor;
        public nint Background;

        [MarshalAs(UnmanagedType.LPWStr)]
        public string? MenuName;

        [MarshalAs(UnmanagedType.LPWStr)]
        public string ClassName;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct Rect
    {
        public int Left;
        public int Top;
        public int Right;
        public int Bottom;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct Point
    {
        public int X;
        public int Y;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct Message
    {
        public nint Handle;
        public uint Id;
        public nuint WParam;
        public nint LParam;
        public uint Time;
        public Point Position;
        public uint Private;
    }

    [DllImport("kernel32.dll", CharSet = CharSet.Unicode)]
    private static extern nint GetModuleHandleW(string? moduleName);

    [DllImport("user32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    private static extern ushort RegisterClassW(ref WndClass windowClass);

    [DllImport("user32.dll", CharSet = CharSet.Unicode)]
    private static extern bool UnregisterClassW(string className, nint instance);

    [DllImport("user32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
    private static extern nint CreateWindowExW(uint exStyle, string className, string title, uint style, int x, int y, int width, int height, nint parent, nint menu, nint instance, nint param);

    [DllImport("user32.dll")]
    private static extern bool DestroyWindow(nint hwnd);

    [DllImport("user32.dll")]
    private static extern bool ShowWindow(nint hwnd, int command);

    [DllImport("user32.dll")]
    private static extern bool UpdateWindow(nint hwnd);

    [DllImport("user32.dll")]
    private static extern nint GetFocus();

    [DllImport("user32.dll")]
    private static extern bool GetClientRect(nint hwnd, out Rect rect);

    [DllImport("user32.dll")]
    private static extern bool PeekMessageW(out Message message, nint hwnd, uint min, uint max, uint remove);

    [DllImport("user32.dll")]
    private static extern bool TranslateMessage(ref Message message);

    [DllImport("user32.dll")]
    private static extern nint DispatchMessageW(ref Message message);

    [DllImport("user32.dll")]
    private static extern nint DefWindowProcW(nint hwnd, uint message, nuint wParam, nint lParam);

    [DllImport("user32.dll")]
    private static extern void PostQuitMessage(int exitCode);

    [DllImport("user32.dll", CharSet = CharSet.Unicode)]
    private static extern bool SetWindowTextW(nint hwnd, string text);

    [DllImport("user32.dll")]
    private static extern bool SetWindowPos(nint hwnd, nint insertAfter, int x, int y, int width, int height, uint flags);

    [DllImport("user32.dll")]
    private static extern bool AdjustWindowRectExForDpi(ref Rect rect, uint style, bool menu, uint exStyle, uint dpi);

    [DllImport("user32.dll")]
    private static extern uint GetDpiForWindow(nint hwnd);

    [DllImport("user32.dll")]
    private static extern uint GetDpiForSystem();

    [DllImport("user32.dll")]
    private static extern bool SetProcessDpiAwarenessContext(nint context);

    [DllImport("user32.dll")]
    private static extern nint LoadCursorW(nint instance, nint cursor);
}
