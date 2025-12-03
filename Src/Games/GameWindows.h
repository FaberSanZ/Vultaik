
#pragma once
#include <windows.h>
#include <array>
#include <functional>
#include <vector>
#include <iostream>

class GameInput
{
public:

    enum class KeyState
    {
        Up = 0,      
        Down,        
        Pressed,     
        Released     
    };

    enum class KeyCode
    {

        A = 'A', B, C, D, E, F, G, H, I, J, K, L, M,
        N, O, P, Q, R, S, T, U, V, W, X, Y, Z,


        Num0 = '0', Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,

        // Teclas especiales
        Space = VK_SPACE,
        Enter = VK_RETURN,
        Escape = VK_ESCAPE,
        Shift = VK_SHIFT,
        Control = VK_CONTROL,
        Alt = VK_MENU,
        Tab = VK_TAB,
        Backspace = VK_BACK,


        Left = VK_LEFT,
        Right = VK_RIGHT,
        Up = VK_UP,
        Down = VK_DOWN,

        F1 = VK_F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12
    };

    enum class MouseButton
    {
        Left = 0,
        Right,
        Middle,
        Button4,
        Button5
    };

    // Callback types
    using KeyCallback = std::function<void(KeyCode, KeyState)>;
    using MouseCallback = std::function<void(int, int, MouseButton, KeyState)>;
    using MouseMoveCallback = std::function<void(int, int)>;
    using ScrollCallback = std::function<void(float)>;

    static void Initialize();
    static void Shutdown();


    static void Update();


    static LRESULT ProcessMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);


    static bool IsKeyDown(KeyCode key);
    static bool IsKeyUp(KeyCode key);
    static bool IsKeyPressed(KeyCode key);
    static bool IsKeyReleased(KeyCode key);

    
    static bool IsMouseButtonDown(MouseButton button);
    static bool IsMouseButtonUp(MouseButton button);
    static bool IsMouseButtonPressed(MouseButton button);
    static bool IsMouseButtonReleased(MouseButton button);


    static void GetMousePosition(int& x, int& y);
    static int GetMouseX();
    static int GetMouseY();


    static void GetMouseDelta(int& deltaX, int& deltaY);
    static int GetMouseDeltaX();
    static int GetMouseDeltaY();


    static float GetScrollDelta();


    static void AddKeyCallback(const KeyCallback& callback);
    static void AddMouseCallback(const MouseCallback& callback);
    static void AddMouseMoveCallback(const MouseMoveCallback& callback);
    static void AddScrollCallback(const ScrollCallback& callback);

private:

    static std::array<bool, 256> currentKeys;
    static std::array<bool, 5> currentMouseButtons;


    static std::array<bool, 256> previousKeys;
    static std::array<bool, 5> previousMouseButtons;

    static int mouseX;
    static int mouseY;
    static int prevMouseX;
    static int prevMouseY;
    static float scrollDelta;

    static std::vector<KeyCallback> keyCallbacks;
    static std::vector<MouseCallback> mouseCallbacks;
    static std::vector<MouseMoveCallback> mouseMoveCallbacks;
    static std::vector<ScrollCallback> scrollCallbacks;

    static bool initialized;

    static KeyCode VirtualKeyToKeyCode(WPARAM wParam);
    static MouseButton MessageToMouseButton(UINT msg);
};


inline std::array<bool, 256> GameInput::currentKeys;
inline std::array<bool, 256> GameInput::previousKeys;
inline std::array<bool, 5> GameInput::currentMouseButtons;
inline std::array<bool, 5> GameInput::previousMouseButtons;

inline int GameInput::mouseX = 0;
inline int GameInput::mouseY = 0;
inline int GameInput::prevMouseX = 0;
inline int GameInput::prevMouseY = 0;
inline float GameInput::scrollDelta = 0.0f;

inline std::vector<GameInput::KeyCallback> GameInput::keyCallbacks;
inline std::vector<GameInput::MouseCallback> GameInput::mouseCallbacks;
inline std::vector<GameInput::MouseMoveCallback> GameInput::mouseMoveCallbacks;
inline std::vector<GameInput::ScrollCallback> GameInput::scrollCallbacks;

inline bool GameInput::initialized = false;


inline void GameInput::Initialize()
{
    currentKeys.fill(false);
    previousKeys.fill(false);
    currentMouseButtons.fill(false);
    previousMouseButtons.fill(false);

    mouseX = 0;
    mouseY = 0;
    prevMouseX = 0;
    prevMouseY = 0;
    scrollDelta = 0.0f;

    initialized = true;

    std::cout << "GameInput initialized successfully\n";
}

inline void GameInput::Shutdown()
{
    keyCallbacks.clear();
    mouseCallbacks.clear();
    mouseMoveCallbacks.clear();
    scrollCallbacks.clear();
    initialized = false;
}

inline void GameInput::Update()
{
    previousKeys = currentKeys;
    previousMouseButtons = currentMouseButtons;
    prevMouseX = mouseX;
    prevMouseY = mouseY;

    scrollDelta = 0.0f;
}

inline LRESULT GameInput::ProcessMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (!initialized) return 1;

    switch (msg)
    {
    case WM_KEYDOWN:
    {

        if ((lParam & 0x40000000) == 0)
        {
            KeyCode key = VirtualKeyToKeyCode(wParam);
            currentKeys[static_cast<int>(key)] = true;


            for (const auto& callback : keyCallbacks)
            {
                callback(key, KeyState::Pressed);
            }
        }
        break;
    }

    case WM_KEYUP:
    {
        KeyCode key = VirtualKeyToKeyCode(wParam);
        currentKeys[static_cast<int>(key)] = false;


        for (const auto& callback : keyCallbacks)
        {
            callback(key, KeyState::Released);
        }


        break;
    }

    case WM_LBUTTONDOWN:
    {
        currentMouseButtons[static_cast<int>(MouseButton::Left)] = true;
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        for (const auto& callback : mouseCallbacks)
        {
            callback(x, y, MouseButton::Left, KeyState::Pressed);
        }

        break;
    }

    case WM_RBUTTONDOWN:
    {
        currentMouseButtons[static_cast<int>(MouseButton::Right)] = true;
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        for (const auto& callback : mouseCallbacks)
        {
            callback(x, y, MouseButton::Right, KeyState::Pressed);
        }
        break;
    }

    case WM_MBUTTONDOWN:
    {
        currentMouseButtons[static_cast<int>(MouseButton::Middle)] = true;
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        for (const auto& callback : mouseCallbacks)
        {
            callback(x, y, MouseButton::Middle, KeyState::Pressed);
        }
        break;
    }

    case WM_LBUTTONUP:
    {
        currentMouseButtons[static_cast<int>(MouseButton::Left)] = false;
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        for (const auto& callback : mouseCallbacks)
        {
            callback(x, y, MouseButton::Left, KeyState::Released);
        }

        break;
    }

    case WM_RBUTTONUP:
    {
        currentMouseButtons[static_cast<int>(MouseButton::Right)] = false;
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        for (const auto& callback : mouseCallbacks)
        {
            callback(x, y, MouseButton::Right, KeyState::Released);
        }
        break;
    }

    case WM_MBUTTONUP:
    {
        currentMouseButtons[static_cast<int>(MouseButton::Middle)] = false;
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        for (const auto& callback : mouseCallbacks)
        {
            callback(x, y, MouseButton::Middle, KeyState::Released);
        }
        break;
    }

    case WM_MOUSEMOVE:
    {
        prevMouseX = mouseX;
        prevMouseY = mouseY;
        mouseX = LOWORD(lParam);
        mouseY = HIWORD(lParam);

        for (const auto& callback : mouseMoveCallbacks)
        {
            callback(mouseX, mouseY);
        }
        break;
    }

    case WM_MOUSEWHEEL:
    {
        scrollDelta = static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam)) / WHEEL_DELTA;

        for (const auto& callback : scrollCallbacks)
        {
            callback(scrollDelta);
        }
        break;
    }
    }

    return 0;
}


inline bool GameInput::IsKeyDown(KeyCode key)
{
    return currentKeys[static_cast<int>(key)];
}

inline bool GameInput::IsKeyUp(KeyCode key)
{
    return !currentKeys[static_cast<int>(key)];
}

inline bool GameInput::IsKeyPressed(KeyCode key)
{
    return currentKeys[static_cast<int>(key)] && !previousKeys[static_cast<int>(key)];
}

inline bool GameInput::IsKeyReleased(KeyCode key)
{
    return !currentKeys[static_cast<int>(key)] && previousKeys[static_cast<int>(key)];
}


inline bool GameInput::IsMouseButtonDown(MouseButton button)
{
    return currentMouseButtons[static_cast<int>(button)];
}

inline bool GameInput::IsMouseButtonUp(MouseButton button)
{
    return !currentMouseButtons[static_cast<int>(button)];
}

inline bool GameInput::IsMouseButtonPressed(MouseButton button)
{
    return currentMouseButtons[static_cast<int>(button)] && !previousMouseButtons[static_cast<int>(button)];
}

inline bool GameInput::IsMouseButtonReleased(MouseButton button)
{
    return !currentMouseButtons[static_cast<int>(button)] && previousMouseButtons[static_cast<int>(button)];
}


inline void GameInput::GetMousePosition(int& x, int& y)
{
    x = mouseX;
    y = mouseY;
}

inline int GameInput::GetMouseX()
{
    return mouseX;
}

inline int GameInput::GetMouseY()
{
    return mouseY;
}


inline void GameInput::GetMouseDelta(int& deltaX, int& deltaY)
{
    deltaX = mouseX - prevMouseX;
    deltaY = mouseY - prevMouseY;
}

inline int GameInput::GetMouseDeltaX()
{
    return mouseX - prevMouseX;
}

inline int GameInput::GetMouseDeltaY()
{
    return mouseY - prevMouseY;
}


inline float GameInput::GetScrollDelta()
{
    return scrollDelta;
}


inline void GameInput::AddKeyCallback(const KeyCallback& callback)
{
    keyCallbacks.push_back(callback);
}

inline void GameInput::AddMouseCallback(const MouseCallback& callback)
{
    mouseCallbacks.push_back(callback);
}

inline void GameInput::AddMouseMoveCallback(const MouseMoveCallback& callback)
{
    mouseMoveCallbacks.push_back(callback);
}

inline void GameInput::AddScrollCallback(const ScrollCallback& callback)
{
    scrollCallbacks.push_back(callback);
}


inline GameInput::KeyCode GameInput::VirtualKeyToKeyCode(WPARAM wParam)
{
    return static_cast<KeyCode>(wParam);
}

inline GameInput::MouseButton GameInput::MessageToMouseButton(UINT msg)
{
    switch (msg)
    {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
        return MouseButton::Left;
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
        return MouseButton::Right;
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
        return MouseButton::Middle;
    default:
        return MouseButton::Left;
    }
}




class GameWindows
{
public:
    GameWindows() = default;
    bool IsRunning() const { return isRunning; }
    HWND GetHandle() const { return hwnd; }
    uint32_t GetClientWidth() const { return clientWidth; }
    uint32_t GetClientHeight() const { return clientHeight; }

    void OnInitialize()
    {
        WNDCLASS wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = L"My game";
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

        RegisterClass(&wc);

        hwnd = CreateWindow(wc.lpszClassName, L"My game", WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 1200, 820,
            nullptr, nullptr, wc.hInstance, nullptr);

        ShowWindow(hwnd, SW_SHOW);



        RECT clientRect;
        GetClientRect(hwnd, &clientRect);

        clientHeight = clientRect.bottom - clientRect.top;
        clientWidth = clientRect.right - clientRect.left;

        isRunning = true;

        GameInput::Initialize();
    }

    void PumpMessages()
    {
        GameInput::Update();


        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
                isRunning = false;
        }
    }

    void OnShutdown()
    {
        GameInput::Shutdown();

        if (hwnd)
        {
            DestroyWindow(hwnd);
            hwnd = nullptr;
        }
        UnregisterClass(L"My game", GetModuleHandle(nullptr));
        isRunning = false;
    }

private:
    HWND hwnd = nullptr;
    bool isRunning = false;
    uint32_t clientWidth = 0;
    uint32_t clientHeight = 0;

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        GameInput::ProcessMessage(hwnd, uMsg, wParam, lParam);

        switch (uMsg)
        {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }

        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
};
