using System;
using System.Runtime.InteropServices;

namespace Vultaik;

public enum KeyState : byte
{
    Up,
    Down,
    Pressed,
    Released
}

public enum KeyCode : ushort
{
    A = 'A', B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Num0 = '0', Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    Space = 0x20,
    Enter = 0x0D,
    Escape = 0x1B,
    Shift = 0x10,
    Control = 0x11,
    Alt = 0x12,
    Tab = 0x09,
    Backspace = 0x08,
    Left = 0x25,
    Up = 0x26,
    Right = 0x27,
    Down = 0x28,
    F1 = 0x70, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12
}

public enum MouseButton : byte
{
    Left,
    Right,
    Middle,
    Button4,
    Button5
}

public enum MouseMode : byte
{
    Absolute,
    Relative
}

public static class GameInput
{
    private const int KeyCount = 256;
    private const int MouseButtonCount = 5;

    private static readonly bool[] _currentKeys = new bool[KeyCount];
    private static readonly bool[] _previousKeys = new bool[KeyCount];
    private static readonly bool[] _currentMouseButtons = new bool[MouseButtonCount];
    private static readonly bool[] _previousMouseButtons = new bool[MouseButtonCount];

    private static int _mouseX;
    private static int _mouseY;
    private static int _mouseDeltaX;
    private static int _mouseDeltaY;
    private static float _scrollDelta;

    private static MouseMode _mouseMode = MouseMode.Absolute;
    private static nint _window;

    private static bool _initialized;
    private static bool _hasFocus;
    private static bool _hasMousePosition;
    private static bool _cursorHidden;

    public static int MouseX => _mouseX;
    public static int MouseY => _mouseY;
    public static int MouseDeltaX => _mouseDeltaX;
    public static int MouseDeltaY => _mouseDeltaY;
    public static float ScrollDelta => _scrollDelta;
    public static MouseMode Mode => _mouseMode;
    public static bool IsFocused => _hasFocus;

    public static void Initialize(nint window)
    {
        if (_initialized) return;

        _window = window;

        Array.Clear(_currentKeys);
        Array.Clear(_previousKeys);
        Array.Clear(_currentMouseButtons);
        Array.Clear(_previousMouseButtons);

        _mouseX = 0;
        _mouseY = 0;
        _mouseDeltaX = 0;
        _mouseDeltaY = 0;
        _scrollDelta = 0.0f;

        _hasMousePosition = false;
        _hasFocus = window == 0 || GetFocus() == window;

        if (_window != 0) RegisterRawMouse();

        _initialized = true;
        ApplyMouseModeForFocus();
    }

    public static void Shutdown()
    {
        if (!_initialized) return;

        ReleaseCursorClip();
        SetCursorHidden(false);

        Array.Clear(_currentKeys);
        Array.Clear(_previousKeys);
        Array.Clear(_currentMouseButtons);
        Array.Clear(_previousMouseButtons);

        _mouseDeltaX = 0;
        _mouseDeltaY = 0;
        _scrollDelta = 0.0f;

        _window = 0;
        _initialized = false;
        _hasFocus = false;
        _hasMousePosition = false;
        _mouseMode = MouseMode.Absolute;
    }

    public static void BeginFrame()
    {
        if (!_initialized) return;

        Array.Copy(_currentKeys, _previousKeys, KeyCount);
        Array.Copy(_currentMouseButtons, _previousMouseButtons, MouseButtonCount);

        _mouseDeltaX = 0;
        _mouseDeltaY = 0;
        _scrollDelta = 0.0f;
    }

    public static nint ProcessMessage(nint hwnd, uint message, nuint wParam, nint lParam)
    {
        if (!_initialized) return 0;

        switch (message)
        {
            case WM_SETFOCUS:
                _hasFocus = true;
                _hasMousePosition = false;
                ApplyMouseModeForFocus();
                break;

            case WM_KILLFOCUS:
                _hasFocus = false;
                ClearHeldInput();
                ReleaseCursorClip();
                SetCursorHidden(false);
                break;

            case WM_ACTIVATEAPP:
                if (wParam != 0)
                {
                    _hasFocus = true;
                    _hasMousePosition = false;
                    ApplyMouseModeForFocus();
                }
                else
                {
                    _hasFocus = false;
                    ClearHeldInput();
                    ReleaseCursorClip();
                    SetCursorHidden(false);
                }
                break;

            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
                if (wParam < KeyCount) _currentKeys[(int)wParam] = true;
                break;

            case WM_KEYUP:
            case WM_SYSKEYUP:
                if (wParam < KeyCount) _currentKeys[(int)wParam] = false;
                break;

            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_XBUTTONDOWN:
                {
                    MouseButton button = MessageToMouseButton(message, wParam);
                    SetMouseButton(button, true, lParam);
                    if (_window != 0) SetCapture(_window);
                    return message == WM_XBUTTONDOWN ? 1 : 0;
                }

            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
            case WM_MBUTTONUP:
            case WM_XBUTTONUP:
                {
                    MouseButton button = MessageToMouseButton(message, wParam);
                    SetMouseButton(button, false, lParam);
                    if (!AnyMouseButtonDown() && GetCapture() == _window) ReleaseCapture();
                    return message == WM_XBUTTONUP ? 1 : 0;
                }

            case WM_MOUSEMOVE:
                {
                    int newX = GetSignedLowWord(lParam);
                    int newY = GetSignedHighWord(lParam);

                    if (_mouseMode == MouseMode.Absolute)
                    {
                        if (_hasMousePosition)
                        {
                            _mouseDeltaX += newX - _mouseX;
                            _mouseDeltaY += newY - _mouseY;
                        }

                        _mouseX = newX;
                        _mouseY = newY;
                        _hasMousePosition = true;
                    }
                    else
                    {
                        _mouseX = newX;
                        _mouseY = newY;
                    }

                    break;
                }

            case WM_INPUT:
                if (_mouseMode == MouseMode.Relative && _hasFocus) ProcessRawMouse(lParam);
                break;

            case WM_MOUSEWHEEL:
                _scrollDelta += GetWheelDelta(wParam) / 120.0f;
                break;

            case WM_SIZE:
                if (_mouseMode == MouseMode.Relative && _hasFocus) UpdateCursorClip();
                break;
        }

        return 0;
    }

    public static bool IsKeyDown(KeyCode key) => _currentKeys[(int)key];
    public static bool IsKeyUp(KeyCode key) => !_currentKeys[(int)key];
    public static bool IsKeyPressed(KeyCode key) => _currentKeys[(int)key] && !_previousKeys[(int)key];
    public static bool IsKeyReleased(KeyCode key) => !_currentKeys[(int)key] && _previousKeys[(int)key];

    public static bool IsMouseButtonDown(MouseButton button) => _currentMouseButtons[(int)button];
    public static bool IsMouseButtonUp(MouseButton button) => !_currentMouseButtons[(int)button];
    public static bool IsMouseButtonPressed(MouseButton button) => _currentMouseButtons[(int)button] && !_previousMouseButtons[(int)button];
    public static bool IsMouseButtonReleased(MouseButton button) => !_currentMouseButtons[(int)button] && _previousMouseButtons[(int)button];

    public static void SetMouseMode(MouseMode mode)
    {
        if (_mouseMode == mode) return;

        _mouseMode = mode;
        _mouseDeltaX = 0;
        _mouseDeltaY = 0;
        _hasMousePosition = false;

        ApplyMouseModeForFocus();
    }

    private static void SetMouseButton(MouseButton button, bool down, nint lParam)
    {
        _currentMouseButtons[(int)button] = down;
        _mouseX = GetSignedLowWord(lParam);
        _mouseY = GetSignedHighWord(lParam);
        _hasMousePosition = true;
    }

    private static MouseButton MessageToMouseButton(uint message, nuint wParam)
    {
        return message switch
        {
            WM_LBUTTONDOWN or WM_LBUTTONUP => MouseButton.Left,
            WM_RBUTTONDOWN or WM_RBUTTONUP => MouseButton.Right,
            WM_MBUTTONDOWN or WM_MBUTTONUP => MouseButton.Middle,
            WM_XBUTTONDOWN or WM_XBUTTONUP => GetHighWord(wParam) == XBUTTON1 ? MouseButton.Button4 : MouseButton.Button5,
            _ => MouseButton.Left
        };
    }

    private static bool AnyMouseButtonDown()
    {
        for (int i = 0; i < _currentMouseButtons.Length; i++) if (_currentMouseButtons[i]) return true;
        return false;
    }

    private static void ClearHeldInput()
    {
        Array.Clear(_currentKeys);
        Array.Clear(_currentMouseButtons);

        _mouseDeltaX = 0;
        _mouseDeltaY = 0;
        _scrollDelta = 0.0f;
        _hasMousePosition = false;

        if (GetCapture() == _window) ReleaseCapture();
    }

    private static void RegisterRawMouse()
    {
        RawInputDevice device = new() { UsagePage = 0x01, Usage = 0x02, Flags = 0, Target = _window };

        if (!RegisterRawInputDevices([device], 1, (uint)Marshal.SizeOf<RawInputDevice>())) throw new InvalidOperationException("Failed to register Raw Input mouse.");
    }

    private static void ProcessRawMouse(nint lParam)
    {
        uint size = (uint)Marshal.SizeOf<RawInput>();
        uint result = GetRawInputData(lParam, RID_INPUT, out RawInput raw, ref size, (uint)Marshal.SizeOf<RawInputHeader>());

        if (result == uint.MaxValue || raw.Header.Type != RIM_TYPEMOUSE) return;
        if ((raw.Mouse.Flags & MOUSE_MOVE_ABSOLUTE) != 0) return;

        _mouseDeltaX += raw.Mouse.LastX;
        _mouseDeltaY += raw.Mouse.LastY;
    }

    private static void ApplyMouseModeForFocus()
    {
        if (!_initialized) return;

        if (_mouseMode == MouseMode.Relative && _hasFocus)
        {
            SetCursorHidden(true);
            UpdateCursorClip();
        }
        else
        {
            ReleaseCursorClip();
            SetCursorHidden(false);
        }
    }

    private static void SetCursorHidden(bool hidden)
    {
        if (hidden == _cursorHidden) return;

        if (hidden)
        {
            while (ShowCursor(false) >= 0) { }
            _cursorHidden = true;
        }
        else
        {
            while (ShowCursor(true) < 0) { }
            _cursorHidden = false;
        }
    }

    private static void UpdateCursorClip()
    {
        if (_window == 0 || !GetClientRect(_window, out Rect client)) return;

        Point topLeft = new() { X = client.Left, Y = client.Top };
        Point bottomRight = new() { X = client.Right, Y = client.Bottom };

        ClientToScreen(_window, ref topLeft);
        ClientToScreen(_window, ref bottomRight);

        Rect screen = new() { Left = topLeft.X, Top = topLeft.Y, Right = bottomRight.X, Bottom = bottomRight.Y };
        ClipCursor(ref screen);
    }

    private static void ReleaseCursorClip() => ClipCursor(nint.Zero);

    private static int GetSignedLowWord(nint value) => unchecked((short)(value.ToInt64() & 0xFFFF));
    private static int GetSignedHighWord(nint value) => unchecked((short)((value.ToInt64() >> 16) & 0xFFFF));
    private static ushort GetHighWord(nuint value) => (ushort)((value.ToUInt64() >> 16) & 0xFFFF);
    private static short GetWheelDelta(nuint value) => unchecked((short)((value.ToUInt64() >> 16) & 0xFFFF));

    private const uint WM_SIZE = 0x0005;
    private const uint WM_SETFOCUS = 0x0007;
    private const uint WM_KILLFOCUS = 0x0008;
    private const uint WM_ACTIVATEAPP = 0x001C;
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
    private const uint WM_INPUT = 0x00FF;

    private const ushort XBUTTON1 = 0x0001;
    private const uint RID_INPUT = 0x10000003;
    private const uint RIM_TYPEMOUSE = 0;
    private const ushort MOUSE_MOVE_ABSOLUTE = 0x0001;

    [StructLayout(LayoutKind.Sequential)]
    private struct RawInputDevice
    {
        public ushort UsagePage;
        public ushort Usage;
        public uint Flags;
        public nint Target;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct RawInputHeader
    {
        public uint Type;
        public uint Size;
        public nint Device;
        public nuint WParam;
    }

    [StructLayout(LayoutKind.Explicit)]
    private struct RawMouseButtons
    {
        [FieldOffset(0)] public uint Buttons;
        [FieldOffset(0)] public ushort ButtonFlags;
        [FieldOffset(2)] public ushort ButtonData;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct RawMouse
    {
        public ushort Flags;
        public RawMouseButtons Buttons;
        public uint RawButtons;
        public int LastX;
        public int LastY;
        public uint ExtraInformation;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct RawInput
    {
        public RawInputHeader Header;
        public RawMouse Mouse;
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

    [DllImport("user32.dll", SetLastError = true)]
    private static extern bool RegisterRawInputDevices([In] RawInputDevice[] devices, uint deviceCount, uint size);

    [DllImport("user32.dll", SetLastError = true)]
    private static extern uint GetRawInputData(nint rawInput, uint command, out RawInput data, ref uint size, uint headerSize);

    [DllImport("user32.dll")]
    private static extern nint GetFocus();

    [DllImport("user32.dll")]
    private static extern nint SetCapture(nint hwnd);

    [DllImport("user32.dll")]
    private static extern nint GetCapture();

    [DllImport("user32.dll")]
    private static extern bool ReleaseCapture();

    [DllImport("user32.dll")]
    private static extern int ShowCursor(bool show);

    [DllImport("user32.dll")]
    private static extern bool GetClientRect(nint hwnd, out Rect rect);

    [DllImport("user32.dll")]
    private static extern bool ClientToScreen(nint hwnd, ref Point point);

    [DllImport("user32.dll")]
    private static extern bool ClipCursor(ref Rect rect);

    [DllImport("user32.dll", EntryPoint = "ClipCursor")]
    private static extern bool ClipCursor(nint rect);
}
