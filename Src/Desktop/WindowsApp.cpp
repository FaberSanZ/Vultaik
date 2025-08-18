#include "WindowsApp.h"



namespace Desktop
{
    WindowsApp::WindowsApp(uint32_t width, uint32_t height, const wchar_t* title)
        : m_Width(width), m_Height(height), m_Title(title)
    {
    }


    bool WindowsApp::Initialize(HINSTANCE hInstance)
    {
        m_hInstance = hInstance;

        WNDCLASS wc = {};
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = L"DX12WindowClass";
        RegisterClass(&wc);

        RECT rect = { 0, 0, static_cast<LONG>(m_Width), static_cast<LONG>(m_Height) };
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

        m_hWnd = CreateWindowEx(
            0, wc.lpszClassName, m_Title,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            rect.right - rect.left, rect.bottom - rect.top,
            nullptr, nullptr, hInstance, this
        );

        if (!m_hWnd)
            return false;

        ShowWindow(m_hWnd, SW_SHOW);
        return true;
    }

    int WindowsApp::Run()
    {
        MSG msg = {};
        while (msg.message != WM_QUIT)
        {
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                if (onUpdate) onUpdate();
                if (onRender) onRender();
            }
        }

        return static_cast<int>(msg.wParam);
    }

    void WindowsApp::SetOnUpdate(std::function<void()> func)
    {
        onUpdate = func;
    }

    void WindowsApp::SetOnRender(std::function<void()> func)
    {
        onRender = func;
    }

    void WindowsApp::SetOnResize(std::function<void(UINT, UINT)> func)
    {
        onResize = func;
    }


    LRESULT CALLBACK WindowsApp::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        
		// TODO: using ImGui for Windows message handling
        //if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        //    return true;

        // Capturar el puntero a la clase al iniciar
        if (msg == WM_NCCREATE)
        {
            CREATESTRUCT* createStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
            WindowsApp* window = reinterpret_cast<WindowsApp*>(createStruct->lpCreateParams);
            SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
            return DefWindowProc(hWnd, msg, wParam, lParam);
        }

        WindowsApp* window = reinterpret_cast<WindowsApp*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        if (!window)
            return DefWindowProc(hWnd, msg, wParam, lParam);

        //     if (window->onImgui)
                 //window->onImgui(hWnd, msg, wParam, lParam);


        switch (msg)
        {
        case WM_SIZE:
        {
            UINT w = LOWORD(lParam);
            UINT h = HIWORD(lParam);
            if (window->onResize)
                window->onResize(w, h);
            break;
        }

        case WM_CLOSE:
            PostQuitMessage(0);
            break;
        }

        return DefWindowProc(hWnd, msg, wParam, lParam);
    }



    void WindowsApp::HandleResize(UINT width, UINT height)
    {
        m_Width = width;
        m_Height = height;
        if (onResize)
            onResize(width, height);
    }
}