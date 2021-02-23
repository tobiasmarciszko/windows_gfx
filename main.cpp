
// From https://docs.microsoft.com/en-us/windows/win32/direct2d/direct2d-quickstart
//
// And https://docs.microsoft.com/en-us/windows/win32/gdi/drawing-at-timed-intervals

// Windows Header Files:
#include <Windows.h>

// C RunTime Header Files:
//#include <stdlib.h>
//#include <malloc.h>
//#include <memory.h>
//#include <wchar.h>
#include <cmath>

#include <d2d1.h>
#pragma comment(lib,"d2d1.lib")
#include <d2d1helper.h>
//#include <dwrite.h>
//#include <wincodec.h>

template<class Interface>
inline void SafeRelease(
    Interface **ppInterfaceToRelease
    )
{
    if (*ppInterfaceToRelease != nullptr)
    {
        (*ppInterfaceToRelease)->Release();

        (*ppInterfaceToRelease) = nullptr;
    }
}


#ifndef Assert
#if defined( DEBUG ) || defined( _DEBUG )
#define Assert(b) do {if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}} while(0)
#else
#define Assert(b)
#endif //DEBUG || _DEBUG
#endif


#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

// avoiding C-style cast in C++
inline HINSTANCE getHInstance() {
    return reinterpret_cast<HINSTANCE>(&__ImageBase);
};

#endif

//#ifndef HINST_THISCOMPONENT
//EXTERN_C IMAGE_DOS_HEADER __ImageBase;
//#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
//#endif

class DemoApp
{
public:
    DemoApp();
    ~DemoApp();

    // Register the window class and call methods for instantiating drawing resources
    HRESULT Initialize();

    // Process and dispatch messages
    void RunMessageLoop();

private:
    // Initialize device-independent resources.
    HRESULT CreateDeviceIndependentResources();

    // Initialize device-dependent resources.
    HRESULT CreateDeviceResources();

    // Release device-dependent resource.
    void DiscardDeviceResources();

    // Draw content.
    HRESULT OnRender();

    // Resize the render target.
    void OnResize(
        UINT width,
        UINT height
        );

    // The windows procedure.
    static LRESULT CALLBACK WndProc(
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
        );

private:
    HWND m_hwnd;
    ID2D1Factory* m_pDirect2dFactory;
    ID2D1HwndRenderTarget* m_pRenderTarget;
    ID2D1SolidColorBrush* m_pLightSlateGrayBrush;
    ID2D1SolidColorBrush* m_pCornflowerBlueBrush;
};

DemoApp::DemoApp() :
    m_hwnd(nullptr),
    m_pDirect2dFactory(nullptr),
    m_pRenderTarget(nullptr),
    m_pLightSlateGrayBrush(nullptr),
    m_pCornflowerBlueBrush(nullptr)
{
}


DemoApp::~DemoApp()
{
    SafeRelease(&m_pDirect2dFactory);
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pLightSlateGrayBrush);
    SafeRelease(&m_pCornflowerBlueBrush);

}

void DemoApp::RunMessageLoop()
{
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

HRESULT DemoApp::Initialize()
{
    HRESULT hr;

    // Initialize device-indpendent resources, such
    // as the Direct2D factory.
    hr = CreateDeviceIndependentResources();

    if (SUCCEEDED(hr))
    {
        // Register the window class.
//        WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
//        wcex.style         = CS_HREDRAW | CS_VREDRAW;
//        wcex.lpfnWndProc   = DemoApp::WndProc;
//        wcex.cbClsExtra    = 0;
//        wcex.cbWndExtra    = sizeof(LONG_PTR);
//        wcex.hInstance     = HINST_THISCOMPONENT;
//        wcex.hbrBackground = nullptr;
//        wcex.lpszMenuName  = nullptr;
//        wcex.hCursor       = LoadCursor(nullptr, IDI_APPLICATION);
//        wcex.lpszClassName = L"D2DDemoApp";


//        UINT        cbSize;
//        /* Win 3.x */
//        UINT        style;
//        WNDPROC     lpfnWndProc;
//        int         cbClsExtra;
//        int         cbWndExtra;
//        HINSTANCE   hInstance;
//        HICON       hIcon;
//        HCURSOR     hCursor;
//        HBRUSH      hbrBackground;
//        LPCWSTR     lpszMenuName;
//        LPCWSTR     lpszClassName;
//        /* Win 4.0 */
//        HICON       hIconSm;

        WNDCLASSEX wcex = {
            sizeof(WNDCLASSEX),
            CS_HREDRAW | CS_VREDRAW,
            DemoApp::WndProc,
            0,
            sizeof(LONG_PTR),
            getHInstance(),
            nullptr,
            LoadCursor(nullptr, IDI_APPLICATION),
            nullptr,
            L"MenuName",
            L"D2DDemoApp",
            nullptr
        };

        RegisterClassEx(&wcex);


        // Because the CreateWindow function takes its size in pixels,
        // obtain the system DPI and use it to scale the window size.
        FLOAT dpiX, dpiY;

        // The factory returns the current system DPI. This is also the value it will use
        // to create its own windows.
        m_pDirect2dFactory->GetDesktopDpi(&dpiX, &dpiY);


        // Create the window.
        m_hwnd = CreateWindow(
            L"D2DDemoApp",
            L"Direct2D Demo App",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            static_cast<UINT>(ceil(640.f * dpiX / 96.f)),
            static_cast<UINT>(ceil(480.f * dpiY / 96.f)),
            NULL,
            NULL,
            getHInstance(),
            this
            );
        hr = m_hwnd ? S_OK : E_FAIL;
        if (SUCCEEDED(hr))
        {
            ShowWindow(m_hwnd, SW_SHOWNORMAL);
            UpdateWindow(m_hwnd);
        }
    }

    return hr;
}

HRESULT DemoApp::CreateDeviceIndependentResources()
{
    HRESULT hr = S_OK;

    // Create a Direct2D factory.
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pDirect2dFactory);

    return hr;
}

HRESULT DemoApp::CreateDeviceResources()
{
    HRESULT hr = S_OK;

    if (!m_pRenderTarget)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(
            static_cast<UINT32>(rc.right - rc.left),
            static_cast<UINT32>(rc.bottom - rc.top)
            );

        // Create a Direct2D render target.
        hr = m_pDirect2dFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &m_pRenderTarget
            );


        if (SUCCEEDED(hr))
        {
            // Create a gray brush.
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::LightSlateGray),
                &m_pLightSlateGrayBrush
                );
        }
        if (SUCCEEDED(hr))
        {
            // Create a blue brush.
            hr = m_pRenderTarget->CreateSolidColorBrush(
                D2D1::ColorF(D2D1::ColorF::CornflowerBlue),
                &m_pCornflowerBlueBrush
                );
        }
    }

    return hr;
}

void DemoApp::DiscardDeviceResources()
{
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pLightSlateGrayBrush);
    SafeRelease(&m_pCornflowerBlueBrush);
}

static unsigned int elapsed = 0;
static D2D1_RECT_F rectangle1;

LRESULT CALLBACK DemoApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        DemoApp *pDemoApp = reinterpret_cast<DemoApp *>(pcs->lpCreateParams);

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(pDemoApp)
            );


        SetTimer(hwnd, 1, 10, nullptr);

        result = 1;
    }
    else
    {
        DemoApp *pDemoApp = reinterpret_cast<DemoApp *>(static_cast<LONG_PTR>(
            ::GetWindowLongPtrW(
                hwnd,
                GWLP_USERDATA
                )));

        bool wasHandled = false;

        if (pDemoApp)
        {
            switch (message)
            {
            case WM_SIZE:
                {
                    UINT width = LOWORD(lParam);
                    UINT height = HIWORD(lParam);
                    pDemoApp->OnResize(width, height);
                    rectangle1.top +=1;
                    rectangle1.bottom +=1;
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_DISPLAYCHANGE:
                {
                    InvalidateRect(hwnd, nullptr, FALSE);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_PAINT:
                {
                    pDemoApp->OnRender();
                    ValidateRect(hwnd, nullptr);
                }
                result = 0;
                wasHandled = true;
                break;

            case WM_DESTROY:
                {
                    PostQuitMessage(0);
                }
                result = 1;
                wasHandled = true;
                break;
            case WM_TIMER:
                {
                    elapsed++;
                    pDemoApp->OnRender();
                    ValidateRect(hwnd, nullptr);
                }
                result = 0;
                wasHandled = true;
                break;
            }

        }

        if (!wasHandled)
        {
            result = DefWindowProc(hwnd, message, wParam, lParam);
        }
    }

    return result;
}

HRESULT DemoApp::OnRender()
    {
        HRESULT hr = S_OK;

        hr = CreateDeviceResources();

        if (SUCCEEDED(hr))
        {
            m_pRenderTarget->BeginDraw();
            m_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
            m_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));

            D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();
            // Draw a grid background.
            int width = static_cast<int>(rtSize.width);
            int height = static_cast<int>(rtSize.height);

            for (int x = 0; x < width; x += 10)
            {
                m_pRenderTarget->DrawLine(
                    D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
                    D2D1::Point2F(static_cast<FLOAT>(x), rtSize.height),
                    m_pLightSlateGrayBrush,
                    0.5f
                    );
            }

            for (int y = 0; y < height; y += 10)
            {
                m_pRenderTarget->DrawLine(
                    D2D1::Point2F(0.0f, static_cast<FLOAT>(y)),
                    D2D1::Point2F(rtSize.width, static_cast<FLOAT>(y)),
                    m_pLightSlateGrayBrush,
                    0.5f
                    );
            }

            rectangle1 = D2D1::RectF(
                rtSize.width/2 - 50.0f, // left
                rtSize.height/2 - 50.0f + 100 * cosf((elapsed % 360) * 0.0174533F), // top
                rtSize.width/2 + 50.0f, // right
                rtSize.height/2 + 50.0f + 100 * cosf((elapsed % 360) * 0.0174533F) // bottom
            );

            // Draw a filled rectangle.
            m_pRenderTarget->FillRectangle(&rectangle1, m_pCornflowerBlueBrush);

            hr = m_pRenderTarget->EndDraw();
        }

        if (hr == D2DERR_RECREATE_TARGET)
        {
            hr = S_OK;
            DiscardDeviceResources();
        }

        return hr;
    }

void DemoApp::OnResize(UINT width, UINT height)
{
    if (m_pRenderTarget)
    {
        // Note: This method can fail, but it's okay to ignore the
        // error here, because the error will be returned again
        // the next time EndDraw is called.
        m_pRenderTarget->Resize(D2D1::SizeU(width, height));
    }
}

int WINAPI WinMain(
        HINSTANCE /* hInstance */,
        HINSTANCE /* hPrevInstance */,
        LPSTR /* lpCmdLine */,
        int /* nCmdShow */
        )
{
    // Use HeapSetInformation to specify that the process should
    // terminate if the heap manager detects an error in any heap used
    // by the process.
    // The return value is ignored, because we want to continue running in the
    // unlikely event that HeapSetInformation fails.
    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    if (SUCCEEDED(CoInitialize(NULL)))
    {
        {
            DemoApp app;

            if (SUCCEEDED(app.Initialize()))
            {
                app.RunMessageLoop();
            }
        }
        CoUninitialize();
    }

    return 0;
}
