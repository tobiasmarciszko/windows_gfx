
// From https://docs.microsoft.com/en-us/windows/win32/direct2d/direct2d-quickstart
//
// And https://docs.microsoft.com/en-us/windows/win32/gdi/drawing-at-timed-intervals

// Investigate how to draw pixels:
// ID2D1RenderTarget::CreateBitmap() and ID2D1RenderTarget::DrawBitmap()

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

#include <array>
#include <vector>

struct Color {
    Color(UINT8 r_, UINT8 g_, UINT8 b_):  b{b_}, g{g_}, r{r_} {}
    UINT8 b;
    UINT8 g;
    UINT8 r;
    UINT8 a{255};
};

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

// #ifndef Assert
// #if defined( DEBUG ) || defined( _DEBUG )
// #define Assert(b) do {if (!(b)) {OutputDebugStringA("Assert: " #b "\n");}} while(0)
// #else
// #define Assert(b)
// #endif //DEBUG || _DEBUG
// #endif

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
// #define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)

// avoiding C-style cast in C++
inline HINSTANCE getHInstance() {
    return reinterpret_cast<HINSTANCE>(&__ImageBase);
};

#endif

class DemoApp
{
public:
    DemoApp() = default;
    ~DemoApp();

    // Register the window class and call methods for instantiating drawing resources
    HRESULT Initialize();

    // Process and dispatch messages
    void RunMessageLoop();

    inline void spreadFire(const size_t& src) {
        
        const auto pixel = m_firePixels[src];
        if (pixel == 0) {
            m_firePixels[src - m_width] = 0;
        } else {
            const auto randIdx = rand() % 3;
            const auto dst = src - static_cast<size_t>(randIdx) + 1;
            m_firePixels[dst - m_width] = pixel - (randIdx & 1);
        }   
    }

    inline void doFire() {
        for (size_t x = 0 ; x < m_width; x++) {
            for (size_t y = 1; y < m_height; y++) {
                spreadFire(y * m_width + x);
            }
        }
    }

    inline void initFire() {

        m_firePixels.resize(m_width*m_height, 0);
        m_pixelData.resize(m_width*m_height, Color{0, 0, 0});
        
        // Black background
        for (size_t i = 0; i < m_width*m_height; i++) {
            m_firePixels.at(i) = 0;
        }   

        // Bottom line is white (where the fire starts from)
        for(size_t i = 0; i < m_width; i++) {
            m_firePixels.at((m_height-1)*m_width + i) = 36;
        }
    }

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
    HWND m_hwnd{nullptr};
    ID2D1Factory* m_pDirect2dFactory{nullptr};
    ID2D1HwndRenderTarget* m_pRenderTarget{nullptr};
    ID2D1SolidColorBrush* m_pLightSlateGrayBrush{nullptr};
    ID2D1SolidColorBrush* m_pCornflowerBlueBrush{nullptr};

    std::array<Color, 37> m_palette{
        Color(0x07,0x07,0x07),
        Color(0x1F,0x07,0x07),
        Color(0x2F,0x0F,0x07),
        Color(0x47,0x0F,0x07),
        Color(0x57,0x17,0x07),
        Color(0x67,0x1F,0x07),
        Color(0x77,0x1F,0x07),
        Color(0x8F,0x27,0x07),
        Color(0x9F,0x2F,0x07),
        Color(0xAF,0x3F,0x07),
        Color(0xBF,0x47,0x07),
        Color(0xC7,0x47,0x07),
        Color(0xDF,0x4F,0x07),
        Color(0xDF,0x57,0x07),
        Color(0xDF,0x57,0x07),
        Color(0xD7,0x5F,0x07),
        Color(0xD7,0x5F,0x07),
        Color(0xD7,0x67,0x0F),
        Color(0xCF,0x6F,0x0F),
        Color(0xCF,0x77,0x0F),
        Color(0xCF,0x7F,0x0F),
        Color(0xCF,0x87,0x17),
        Color(0xC7,0x87,0x17),
        Color(0xC7,0x8F,0x17),
        Color(0xC7,0x97,0x1F),
        Color(0xBF,0x9F,0x1F),
        Color(0xBF,0x9F,0x1F),
        Color(0xBF,0xA7,0x27),
        Color(0xBF,0xA7,0x27),
        Color(0xBF,0xAF,0x2F),
        Color(0xB7,0xAF,0x2F),
        Color(0xB7,0xB7,0x2F),
        Color(0xB7,0xB7,0x37),
        Color(0xCF,0xCF,0x6F),
        Color(0xDF,0xDF,0x9F),
        Color(0xEF,0xEF,0xC7),
        Color(0xFF,0xFF,0xFF)
    };

    UINT32 m_width{640};
    UINT32 m_height{480};
    // static constexpr int FIRE_WIDTH{640};
    // static constexpr int FIRE_HEIGHT{480};
    std::vector<char> m_firePixels{0};
    std::vector<Color> m_pixelData{Color{0, 0, 0}};
    ID2D1Bitmap *m_bmp;
};

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

        m_width = 640;
        m_height = 480;

        initFire();

        // Create the window.
        m_hwnd = CreateWindow(
            L"D2DDemoApp",
            L"Direct2D Demo App",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            static_cast<INT>(ceil(640.f * dpiX / 96.f)),
            static_cast<INT>(ceil(480.f * dpiY / 96.f)),
            nullptr,
            nullptr,
            getHInstance(),
            this);

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

// static UINT8* Data;

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

            m_width = size.width;
            m_height = size.height;

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

        hr = m_pRenderTarget->CreateBitmap(
            D2D1::SizeU(m_width, m_height),
            D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
            &m_bmp);

        initFire();
    }

    return hr;
}

void DemoApp::DiscardDeviceResources()
{
    SafeRelease(&m_pRenderTarget);
    SafeRelease(&m_pLightSlateGrayBrush);
    SafeRelease(&m_pCornflowerBlueBrush);
    SafeRelease(&m_bmp);
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

        SetTimer(hwnd, 1, 60/1000, nullptr);

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

            // D2D1_SIZE_F rtSize = m_pRenderTarget->GetSize();
            // Draw a grid background.
            // int width = static_cast<int>(rtSize.width);
            // int height = static_cast<int>(rtSize.height);

            // for (int x = 0; x < width; x += 10)
            // {
            //     m_pRenderTarget->DrawLine(
            //         D2D1::Point2F(static_cast<FLOAT>(x), 0.0f),
            //         D2D1::Point2F(static_cast<FLOAT>(x), rtSize.height),
            //         m_pLightSlateGrayBrush,
            //         0.5f
            //         );
            // }

            // for (int y = 0; y < height; y += 10)
            // {
            //     m_pRenderTarget->DrawLine(
            //         D2D1::Point2F(0.0f, static_cast<FLOAT>(y)),
            //         D2D1::Point2F(rtSize.width, static_cast<FLOAT>(y)),
            //         m_pLightSlateGrayBrush,
            //         0.5f
            //         );
            // }

            // rectangle1 = D2D1::RectF(
            //     rtSize.width/2 - 50.0f, // left
            //     rtSize.height/2 - 50.0f + 100 * cosf((elapsed % 360) * 0.0174533F), // top
            //     rtSize.width/2 + 50.0f, // right
            //     rtSize.height/2 + 50.0f + 100 * cosf((elapsed % 360) * 0.0174533F) // bottom
            // );


            doFire();

            // Convert the palette data to "real" colors
            size_t index{0};
            for (auto&& pixelData: m_firePixels) {
                m_pixelData.at(index++) = m_palette.at(static_cast<size_t>(pixelData));
            }

            // Copy over the colors (pixel data) to the bitmap
            m_bmp->CopyFromMemory(nullptr, m_pixelData.data(), m_width * 4);
            m_pRenderTarget->DrawBitmap(m_bmp);
            // m_pRenderTarget->FillRectangle(&rectangle1, m_pCornflowerBlueBrush);

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
        m_width = width;
        m_height = height;

        SafeRelease(&m_bmp);
        (void)m_pRenderTarget->CreateBitmap(
            D2D1::SizeU(m_width, m_height),
            D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE)),
            &m_bmp);

        initFire();
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
    HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);

    if (SUCCEEDED(CoInitialize(nullptr)))
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
