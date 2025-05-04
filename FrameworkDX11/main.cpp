//--------------------------------------------------------------------------------------
// File: main.cpp
//
// This application demonstrates animation using matrix transformations
//
// http://msdn.microsoft.com/en-us/library/windows/apps/ff729722.aspx
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------
#define _XM_NO_INTRINSICS_

#include "main.h"
#include "constants.h"
#include "Camera.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"

Camera* g_pCamera;


//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT		InitWindow(HINSTANCE hInstance, int nCmdShow);
HRESULT		InitDevice();
HRESULT		InitRunTimeParameters();
HRESULT		InitWorld(int width, int height);
void		CleanupDevice();
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void		Render();



//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE               g_hInst = nullptr;
HWND                    g_hWnd = nullptr;
D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
ID3D11Device*           g_pd3dDevice = nullptr;
ID3D11Device1*          g_pd3dDevice1 = nullptr;
ID3D11DeviceContext*    g_pImmediateContext = nullptr;
ID3D11DeviceContext1*   g_pImmediateContext1 = nullptr;
IDXGISwapChain*         g_pSwapChain = nullptr;
IDXGISwapChain1*        g_pSwapChain1 = nullptr;
ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
ID3D11Texture2D*        g_pDepthStencil = nullptr;
ID3D11Texture2D*        g_pDepthStencil2 = nullptr;
ID3D11DepthStencilView* g_pDepthStencilView = nullptr;
ID3D11DepthStencilView* g_pDepthStencilView2 = nullptr;
ID3D11VertexShader*     g_pVertexShader = nullptr;

ID3D11PixelShader*      g_pPixelShader = nullptr;

ID3D11InputLayout*      g_pVertexLayout = nullptr;

ID3D11Buffer*           g_pConstantBuffer = nullptr;

ID3D11Buffer*           g_pLightConstantBuffer = nullptr;

ID3D11Buffer* g_pPostProcessingBuffer = nullptr;

ID3D11Texture2D* g_pRTTRenderTargetTexture = nullptr;
ID3D11Texture2D* g_pRTT2RenderTargetTexture = nullptr;
ID3D11Texture2D* g_pRTT3RenderTargetTexture = nullptr;

ID3D11RenderTargetView* g_pRTTRenderTargetView;
ID3D11RenderTargetView* g_pRTT2RenderTargetView;
ID3D11RenderTargetView* g_pRTT3RenderTargetView;

ID3D11ShaderResourceView* g_pRTTShaderResourceView;
ID3D11ShaderResourceView* g_pRTT2ShaderResourceView;
ID3D11ShaderResourceView* g_pRTT3ShaderResourceView;

ID3D11ShaderResourceView* g_pDepthShaderResourceView;

XMMATRIX                g_Projection;

int						g_viewWidth;
int						g_viewHeight;

DrawableGameObject		g_GameObject;
DrawableGameObject		g_GameObject2;
DrawableGameObject		g_GameObject3;
DrawableGameObject		g_GameObject4;
DrawableGameObject		g_GameObject5;
DrawableGameObject		g_GameObject6;

XMFLOAT3 Object1Pos = XMFLOAT3(0,0,0);
XMINT3 Object1Rot = XMINT3(0, 0, 0);
XMFLOAT3 Object2Pos = XMFLOAT3(5,0,2);
XMINT3 Object2Rot = XMINT3(0, 0, 0);
XMFLOAT3 Object3Pos = XMFLOAT3(-3, 0, -1);
XMINT3 Object3Rot = XMINT3(0, 0, 0);
XMFLOAT3 Object4Pos;

XMFLOAT3 Light1Pos  = XMFLOAT3(2, 0, -2);
XMFLOAT3 Light2Pos = XMFLOAT3(0, 0, -2);
XMINT2 Light2Rot = XMINT2(0, 1);

ImVec4                  light_colour = ImVec4(0.9f, 0.7f, 0.03f, 0.2f);
XMMATRIX                obj_rotation = XMMatrixRotationX(0);

struct SCREEN_VERTEX
{
    XMFLOAT3 pos;
    XMFLOAT2 tex;
};

ID3D11InputLayout* p_gQuadLayout = nullptr;
ID3D11VertexShader* g_pQuadVS = nullptr;
ID3D11PixelShader* g_pQuadPS = nullptr;

ID3D11VertexShader* g_pGBUFFERVS = nullptr;
ID3D11PixelShader* g_pGBUFFERPS = nullptr;

ID3D11VertexShader* g_pLightingVS = nullptr;
ID3D11PixelShader* g_pLightingPS = nullptr;

ID3D11VertexShader* g_pFinalVS = nullptr;
ID3D11PixelShader* g_pFinalPS = nullptr;

ID3D11Texture2D* g_pAlbedoTexture;
ID3D11RenderTargetView* g_pAlbedoRTV;
ID3D11ShaderResourceView* g_pAlbedoSRV;

ID3D11Texture2D* g_pSpecularTexture;
ID3D11RenderTargetView* g_pSpecularRTV;
ID3D11ShaderResourceView* g_pSpecularSRV;

ID3D11Texture2D* g_pNormalTexture;
ID3D11RenderTargetView* g_pNormalRTV;
ID3D11ShaderResourceView* g_pNormalSRV;

ID3D11Texture2D* g_pPositionTexture;
ID3D11RenderTargetView* g_pPositionRTV;
ID3D11ShaderResourceView* g_pPositionSRV;

ID3D11Texture2D* g_pLightingTexture;
ID3D11RenderTargetView* g_pLightingRTV;
ID3D11ShaderResourceView* g_pLightingSRV;

ID3D11Texture2D* g_pFinalTexture;
ID3D11RenderTargetView* g_pFinalRTV;
ID3D11ShaderResourceView* g_pFinalSRV;

SCREEN_VERTEX svQuad[4];    

ID3D11Buffer* m_pQuadVertexBuffer;
ID3D11Buffer* m_pQuadIndexBuffer;

ID3D11RenderTargetView* nullRTV[1] = { nullptr };
ID3D11ShaderResourceView* nullSRV[1] = { nullptr };

bool Rendering = false;
bool RenderToCube = false;
bool GrayScale = false;
bool Cold = false;
float ColdAmount = 1.0f;
bool Warm = false;
float WarmAmount = 1.0f;
bool DepthOfField = false;
bool Bloom = false;
bool GausianBlur = false;
//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );

    if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
        return 0;

    if( FAILED( InitDevice() ) )
    {
        CleanupDevice();
        return 0;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(g_hWnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pImmediateContext);

    // Main message loop
    MSG msg = {0};
    while( WM_QUIT != msg.message )
    {
        if( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
        else
        {
            Render();
        }
    }

    CleanupDevice();

    return ( int )msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon( hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    wcex.hCursor = LoadCursor( nullptr, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"lWindowClass";
    wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    if( !RegisterClassEx( &wcex ) )
        return E_FAIL;

    // Create window
    g_hInst = hInstance;
    RECT rc = { 0, 0, 1920, 1080};

	g_viewWidth = SCREEN_WIDTH;
	g_viewHeight = SCREEN_HEIGHT;

    AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
    g_hWnd = CreateWindow( L"lWindowClass", L"DirectX 11",
                           WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                           CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
                           nullptr );
    if( !g_hWnd )
        return E_FAIL;

    ShowWindow( g_hWnd, nCmdShow );

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Helper for compiling shaders with D3DCompile
//
// With VS 11, we could load up prebuilt .cso files instead...
//--------------------------------------------------------------------------------------
HRESULT CompileShaderFromFile( const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut )
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;

    // Disable optimizations to further improve shader debugging
    dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* pErrorBlob = nullptr;
    hr = D3DCompileFromFile( szFileName, nullptr, nullptr, szEntryPoint, szShaderModel, 
        dwShaderFlags, 0, ppBlobOut, &pErrorBlob );
    if( FAILED(hr) )
    {
        if( pErrorBlob )
        {
            OutputDebugStringA( reinterpret_cast<const char*>( pErrorBlob->GetBufferPointer() ) );
            pErrorBlob->Release();
        }
        return hr;
    }
    if( pErrorBlob ) pErrorBlob->Release();

    return S_OK;
}


//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT InitDevice()
{
    HRESULT hr = S_OK;

    RECT rc;
    GetClientRect(g_hWnd, &rc);
    UINT width = rc.right - rc.left;
    UINT height = rc.bottom - rc.top;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_DRIVER_TYPE driverTypes[] =
    {
        D3D_DRIVER_TYPE_HARDWARE,
        D3D_DRIVER_TYPE_WARP,
        D3D_DRIVER_TYPE_REFERENCE,
    };
    UINT numDriverTypes = ARRAYSIZE(driverTypes);

    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
    {
        g_driverType = driverTypes[driverTypeIndex];
        hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
            D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);

        if (hr == E_INVALIDARG)
        {
            // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
            hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
                D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
        }

        if (SUCCEEDED(hr))
            break;
    }
    if (FAILED(hr))
        return hr;

    // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
    IDXGIFactory1* dxgiFactory = nullptr;
    {
        IDXGIDevice* dxgiDevice = nullptr;
        hr = g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
        if (SUCCEEDED(hr))
        {
            IDXGIAdapter* adapter = nullptr;
            hr = dxgiDevice->GetAdapter(&adapter);
            if (SUCCEEDED(hr))
            {
                hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
                adapter->Release();
            }
            dxgiDevice->Release();
        }
    }
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Failed to create device.", L"Error", MB_OK);
        return hr;
    }

    // Create swap chain
    IDXGIFactory2* dxgiFactory2 = nullptr;
    hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
    if (dxgiFactory2)
    {
        // DirectX 11.1 or later
        hr = g_pd3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&g_pd3dDevice1));
        if (SUCCEEDED(hr))
        {
            (void)g_pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&g_pImmediateContext1));
        }

        DXGI_SWAP_CHAIN_DESC1 sd = {};
        sd.Width = width;
        sd.Height = height;
        sd.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;//  DXGI_FORMAT_R16G16B16A16_FLOAT;////DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.BufferCount = 1;

        hr = dxgiFactory2->CreateSwapChainForHwnd(g_pd3dDevice, g_hWnd, &sd, nullptr, nullptr, &g_pSwapChain1);
        if (SUCCEEDED(hr))
        {
            hr = g_pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&g_pSwapChain));
        }

        dxgiFactory2->Release();
    }
    else
    {
        // DirectX 11.0 systems
        DXGI_SWAP_CHAIN_DESC sd = {};
        sd.BufferCount = 1;
        sd.BufferDesc.Width = width;
        sd.BufferDesc.Height = height;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = g_hWnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;

        hr = dxgiFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain);
    }

    // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
    dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);

    dxgiFactory->Release();

    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Failed to create swapchain.", L"Error", MB_OK);
        return hr;
    }

    // Create a render target view
    ID3D11Texture2D* pBackBuffer = nullptr;
    hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Failed to create a back buffer.", L"Error", MB_OK);
        return hr;
    }

    hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView);
    pBackBuffer->Release();
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Failed to create a render target.", L"Error", MB_OK);
        return hr;
    }

    ID3D11DepthStencilState* g_pDepthStencilState = nullptr;

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = TRUE;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; 
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

    /*depthStencilDesc.StencilEnable = TRUE;*/

    hr = g_pd3dDevice->CreateDepthStencilState(&depthStencilDesc, &g_pDepthStencilState);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Failed to create a depth stencil", L"Error", MB_OK);
        return hr;
    }

    g_pImmediateContext->OMSetDepthStencilState(g_pDepthStencilState, 0);

    // Create depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth = {};
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    hr = g_pd3dDevice->CreateTexture2D(&descDepth, nullptr, &g_pDepthStencil);

    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Failed to create a depth / stencil texture.", L"Error", MB_OK);
        return hr;
    }

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    hr = g_pd3dDevice->CreateDepthStencilView( g_pDepthStencil, &descDSV, &g_pDepthStencilView );

    descDepth.Format = DXGI_FORMAT_R24G8_TYPELESS;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

    descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; 
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

    hr = g_pd3dDevice->CreateTexture2D(&descDepth, nullptr, &g_pDepthStencil2);
    hr = g_pd3dDevice->CreateDepthStencilView( g_pDepthStencil2, &descDSV, &g_pDepthStencilView2 );
    if( FAILED( hr ) )
    {
        MessageBox(nullptr,
            L"Failed to create a depth / stencil view.", L"Error", MB_OK);
        return hr;
    }

    g_pImmediateContext->OMSetRenderTargets( 1, &g_pRenderTargetView, g_pDepthStencilView );

    // Create the texture
    D3D11_TEXTURE2D_DESC textureDesc;
    ZeroMemory(&textureDesc, sizeof(textureDesc));

    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    g_pd3dDevice->CreateTexture2D(&textureDesc, NULL, &g_pRTTRenderTargetTexture);
    g_pd3dDevice->CreateTexture2D(&textureDesc, NULL, &g_pRTT2RenderTargetTexture);
    g_pd3dDevice->CreateTexture2D(&textureDesc, NULL, &g_pRTT3RenderTargetTexture);
    g_pd3dDevice->CreateTexture2D(&textureDesc, NULL, &g_pAlbedoTexture);
    g_pd3dDevice->CreateTexture2D(&textureDesc, NULL, &g_pSpecularTexture);
    g_pd3dDevice->CreateTexture2D(&textureDesc, NULL, &g_pNormalTexture);
    g_pd3dDevice->CreateTexture2D(&textureDesc, NULL, &g_pLightingTexture);
    g_pd3dDevice->CreateTexture2D(&textureDesc, NULL, &g_pPositionTexture);
    g_pd3dDevice->CreateTexture2D(&textureDesc, NULL, &g_pFinalTexture);

    // Format the texture as a Render Target (see second param in CreateRenderTargetView for type)
    D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
    renderTargetViewDesc.Format = textureDesc.Format;
    renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    renderTargetViewDesc.Texture2D.MipSlice = 0;

    // Create the render target view – essentially a wrapper around the texture  (used when we call 'set render target')
    hr = g_pd3dDevice->CreateRenderTargetView(g_pRTTRenderTargetTexture, &renderTargetViewDesc, &g_pRTTRenderTargetView);
    hr = g_pd3dDevice->CreateRenderTargetView(g_pRTT2RenderTargetTexture, &renderTargetViewDesc, &g_pRTT2RenderTargetView);
    hr = g_pd3dDevice->CreateRenderTargetView(g_pRTT3RenderTargetTexture, &renderTargetViewDesc, &g_pRTT3RenderTargetView);
    hr = g_pd3dDevice->CreateRenderTargetView(g_pAlbedoTexture, &renderTargetViewDesc, &g_pAlbedoRTV);
    hr = g_pd3dDevice->CreateRenderTargetView(g_pSpecularTexture, &renderTargetViewDesc, &g_pSpecularRTV);
    hr = g_pd3dDevice->CreateRenderTargetView(g_pNormalTexture, &renderTargetViewDesc, &g_pNormalRTV);
    hr = g_pd3dDevice->CreateRenderTargetView(g_pLightingTexture, &renderTargetViewDesc, &g_pLightingRTV);
    hr = g_pd3dDevice->CreateRenderTargetView(g_pPositionTexture, &renderTargetViewDesc, &g_pPositionRTV);
    hr = g_pd3dDevice->CreateRenderTargetView(g_pFinalTexture, &renderTargetViewDesc, &g_pFinalRTV);

    if (FAILED(hr))
    {
        return hr;
    }

    // Create a Shader Resource View – a view to contain a texture for the pixel shader (used when we need to use the texture in a pixel shader)
    // First, a D3D11_SHADER_RESOURCE_VIEW_DESC  is needed
    D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
    shaderResourceViewDesc.Format = textureDesc.Format;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
    shaderResourceViewDesc.Texture2D.MipLevels = 1;

    // Create the shader resource view.
    hr = g_pd3dDevice->CreateShaderResourceView(g_pRTTRenderTargetTexture, &shaderResourceViewDesc, &g_pRTTShaderResourceView);
    hr = g_pd3dDevice->CreateShaderResourceView(g_pRTT2RenderTargetTexture, &shaderResourceViewDesc, &g_pRTT2ShaderResourceView);
    hr = g_pd3dDevice->CreateShaderResourceView(g_pRTT3RenderTargetTexture, &shaderResourceViewDesc, &g_pRTT3ShaderResourceView);
    hr = g_pd3dDevice->CreateShaderResourceView(g_pAlbedoTexture, &shaderResourceViewDesc, &g_pAlbedoSRV);
    hr = g_pd3dDevice->CreateShaderResourceView(g_pSpecularTexture, &shaderResourceViewDesc, &g_pSpecularSRV);
    hr = g_pd3dDevice->CreateShaderResourceView(g_pNormalTexture, &shaderResourceViewDesc, &g_pNormalSRV);
    hr = g_pd3dDevice->CreateShaderResourceView(g_pLightingTexture, &shaderResourceViewDesc, &g_pLightingSRV);
    hr = g_pd3dDevice->CreateShaderResourceView(g_pPositionTexture, &shaderResourceViewDesc, &g_pPositionSRV);
    hr = g_pd3dDevice->CreateShaderResourceView(g_pFinalTexture, &shaderResourceViewDesc, &g_pFinalSRV);

    shaderResourceViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

    hr = g_pd3dDevice->CreateShaderResourceView(g_pDepthStencil2, &shaderResourceViewDesc, &g_pDepthShaderResourceView);

    if (FAILED(hr))
    {
        return hr;
    }


    // Setup the viewport
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)width;
    vp.Height = (FLOAT)height;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    g_pImmediateContext->RSSetViewports( 1, &vp );

	hr = InitRunTimeParameters();
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Failed to initialise mesh.", L"Error", MB_OK);
		return hr;
	}

	hr = InitWorld(width, height);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"Failed to initialise world.", L"Error", MB_OK);
		return hr;
	}

	hr = g_GameObject.initMesh(g_pd3dDevice, g_pImmediateContext);
	if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Failed to init mesh in game object.", L"Error", MB_OK);
        return hr;
    }

    
    hr = g_GameObject2.initMesh(g_pd3dDevice, g_pImmediateContext);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Failed to init mesh in game object.", L"Error", MB_OK);
        return hr;
    }
    hr = g_GameObject3.initMesh(g_pd3dDevice, g_pImmediateContext);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Failed to init mesh in game object.", L"Error", MB_OK);
        return hr;
    }
    g_GameObject4.setObjectNumber(1);
    g_GameObject4.setUseNormalTexture(false);
    hr = g_GameObject4.initMesh(g_pd3dDevice, g_pImmediateContext);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Failed to init mesh in game object.", L"Error", MB_OK);
        return hr;
    }
    g_GameObject5.setObjectNumber(2);
    g_GameObject5.setPosition(Light1Pos);
    g_GameObject5.setUseNormalTexture(false);
    hr = g_GameObject5.initMesh(g_pd3dDevice, g_pImmediateContext);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Failed to init mesh in game object.", L"Error", MB_OK);
        return hr;
    }
    g_GameObject6.setObjectNumber(2);
    g_GameObject6.setPosition(Light2Pos);
    g_GameObject6.setUseNormalTexture(false);
    hr = g_GameObject6.initMesh(g_pd3dDevice, g_pImmediateContext);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"Failed to init mesh in game object.", L"Error", MB_OK);
        return hr;
    }

    return S_OK;
}

// ***************************************************************************************
// InitMesh
// ***************************************************************************************

HRESULT		InitRunTimeParameters()
{

	// Compile the vertex shader
	ID3DBlob* pVSBlob = nullptr;
	HRESULT hr = CompileShaderFromFile(L"shader.fx", "VS", "vs_4_0", &pVSBlob);
	if (FAILED(hr))
	{
		MessageBox(nullptr,
			L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
		return hr;
	}

	// Create the vertex shader
	hr = g_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
	if (FAILED(hr))
	{
		pVSBlob->Release();
		return hr;
	}

    // Compile the pixel shader
    ID3DBlob* pPSBlob = nullptr;
    hr = CompileShaderFromFile(L"shader.fx", "PS", "ps_4_0", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);
    pPSBlob->Release();
    if (FAILED(hr))
        return hr;

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT" , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0},
        { "BITANGENT" , 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT , D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
    hr = g_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
        pVSBlob->GetBufferSize(), &g_pVertexLayout);
    pVSBlob->Release();
    if (FAILED(hr))
        return hr;

    // Compile the vertex shader
    ID3DBlob* pQVSBlob = nullptr;
   hr = CompileShaderFromFile(L"shader.fx", "QuadVS", "vs_4_0", &pQVSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the vertex shader
    hr = g_pd3dDevice->CreateVertexShader(pQVSBlob->GetBufferPointer(), pQVSBlob->GetBufferSize(), nullptr, &g_pQuadVS);
    if (FAILED(hr))
    {
        pQVSBlob->Release();
        return hr;
    }

    // Compile the pixel shader
    ID3DBlob* pQPSBlob = nullptr;
    hr = CompileShaderFromFile(L"shader.fx", "QuadPS", "ps_4_0", &pQPSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader(pQPSBlob->GetBufferPointer(), pQPSBlob->GetBufferSize(), nullptr, &g_pQuadPS);
    pQPSBlob->Release();
    if (FAILED(hr))
        return hr;

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC Qlayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    numElements = ARRAYSIZE(Qlayout);

    // Create the input layout
    hr = g_pd3dDevice->CreateInputLayout(Qlayout, numElements, pQVSBlob->GetBufferPointer(),
        pQVSBlob->GetBufferSize(), &p_gQuadLayout);
    pQVSBlob->Release();
    if (FAILED(hr))
        return hr;

    // Compile the vertex shader
    ID3DBlob* pGBVSBlob = nullptr;
    hr = CompileShaderFromFile(L"shader.fx", "G_BUFFER_VS", "vs_4_0", &pGBVSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the vertex shader
    hr = g_pd3dDevice->CreateVertexShader(pGBVSBlob->GetBufferPointer(), pGBVSBlob->GetBufferSize(), nullptr, &g_pGBUFFERVS);
    if (FAILED(hr))
    {
        pGBVSBlob->Release();
        return hr;
    }

    // Compile the pixel shader
    ID3DBlob* pGBPSBlob = nullptr;
    hr = CompileShaderFromFile(L"shader.fx", "G_BUFFER_PS", "ps_4_0", &pGBPSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader(pGBPSBlob->GetBufferPointer(), pGBPSBlob->GetBufferSize(), nullptr, &g_pGBUFFERPS);
    pGBPSBlob->Release();
    if (FAILED(hr))
        return hr;

    // Compile the vertex shader
    ID3DBlob* pLVSBlob = nullptr;
    hr = CompileShaderFromFile(L"shader.fx", "LIGHTING_VS", "vs_4_0", &pLVSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the vertex shader
    hr = g_pd3dDevice->CreateVertexShader(pLVSBlob->GetBufferPointer(), pLVSBlob->GetBufferSize(), nullptr, &g_pLightingVS);
    if (FAILED(hr))
    {
        pLVSBlob->Release();
        return hr;
    }

    // Compile the pixel shader
    ID3DBlob* pLPSBlob = nullptr;
    hr = CompileShaderFromFile(L"shader.fx", "LIGHTING_PS", "ps_4_0", &pLPSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader(pLPSBlob->GetBufferPointer(), pLPSBlob->GetBufferSize(), nullptr, &g_pLightingPS);
    pLPSBlob->Release();
    if (FAILED(hr))
        return hr;

    // Compile the vertex shader
    ID3DBlob* pFVSBlob = nullptr;
    hr = CompileShaderFromFile(L"shader.fx", "FINAL_VS", "vs_4_0", &pFVSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the vertex shader
    hr = g_pd3dDevice->CreateVertexShader(pFVSBlob->GetBufferPointer(), pFVSBlob->GetBufferSize(), nullptr, &g_pFinalVS);
    if (FAILED(hr))
    {
        pFVSBlob->Release();
        return hr;
    }

    // Compile the pixel shader
    ID3DBlob* pFPSBlob = nullptr;
    hr = CompileShaderFromFile(L"shader.fx", "FINAL_PS", "ps_4_0", &pFPSBlob);
    if (FAILED(hr))
    {
        MessageBox(nullptr,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the pixel shader
    hr = g_pd3dDevice->CreatePixelShader(pFPSBlob->GetBufferPointer(), pFPSBlob->GetBufferSize(), nullptr, &g_pFinalPS);
    pFPSBlob->Release();
    if (FAILED(hr))
        return hr;

    WORD indices[] = {

        // Front Face

        0, 1, 2,

        1,3,2

    };

    svQuad[0].pos = XMFLOAT3(-1.0f, 1.0f, 0.0f);  // Top-left
    svQuad[0].tex = XMFLOAT2(0.0f, 0.0f);

    svQuad[1].pos = XMFLOAT3(1.0f, 1.0f, 0.0f);   // Top-right
    svQuad[1].tex = XMFLOAT2(1.0f, 0.0f);

    svQuad[2].pos = XMFLOAT3(-1.0f, -1.0f, 0.0f);  // Bottom-left
    svQuad[2].tex = XMFLOAT2(0.0f, 1.0f);

    svQuad[3].pos = XMFLOAT3(1.0f, -1.0f, 0.0f);   // Bottom-right
    svQuad[3].tex = XMFLOAT2(1.0f, 1.0f);

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SCREEN_VERTEX) * 4;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = svQuad;
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &m_pQuadVertexBuffer);
    if (FAILED(hr)) { return hr; }

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 6;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    hr = g_pd3dDevice->CreateBuffer(&bd, &InitData, &m_pQuadIndexBuffer);


	// Set the input layout
	g_pImmediateContext->IASetInputLayout(g_pVertexLayout);


	// Create the constant buffer
    ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
    bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pConstantBuffer);
	if (FAILED(hr))
		return hr;

    // Create the light constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(LightPropertiesConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pLightConstantBuffer);
	if (FAILED(hr))
		return hr;

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(PostProcessingBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    hr = g_pd3dDevice->CreateBuffer(&bd, nullptr, &g_pPostProcessingBuffer);
    if (FAILED(hr))
        return hr;
	return hr;
}

// ***************************************************************************************
// InitWorld
// ***************************************************************************************
HRESULT		InitWorld(int width, int height)
{
    g_pCamera = new Camera(XMFLOAT3(0.0f, 0, -3), XMFLOAT3(0, 0, 1), XMFLOAT3(0.0f, 1.0f, 0.0f));

	// Initialize the projection matrix
    constexpr float fovAngleY = XMConvertToRadians(60.0f);
	g_Projection = XMMatrixPerspectiveFovLH(fovAngleY, width / (FLOAT)height, 1.0f, 100.0f);

	return S_OK;
}


//--------------------------------------------------------------------------------------
// Clean up the objects we've created
//--------------------------------------------------------------------------------------
void CleanupDevice()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    g_GameObject.cleanup();
    g_GameObject2.cleanup();
    g_GameObject3.cleanup();

    // Remove any bound render target or depth/stencil buffer
    ID3D11RenderTargetView* nullViews[] = { nullptr };
    g_pImmediateContext->OMSetRenderTargets(_countof(nullViews), nullViews, nullptr);

    if( g_pImmediateContext ) g_pImmediateContext->ClearState();
    // Flush the immediate context to force cleanup
    if (g_pImmediateContext1) g_pImmediateContext1->Flush();
    g_pImmediateContext->Flush();

    if (g_pLightConstantBuffer)
        g_pLightConstantBuffer->Release();
    if (g_pVertexLayout) g_pVertexLayout->Release();
    if( g_pConstantBuffer ) g_pConstantBuffer->Release();
    if( g_pVertexShader ) g_pVertexShader->Release();
    if( g_pPixelShader ) g_pPixelShader->Release();
    if( g_pDepthStencil ) g_pDepthStencil->Release();
    if( g_pDepthStencilView ) g_pDepthStencilView->Release();
    if( g_pRenderTargetView ) g_pRenderTargetView->Release();
    if( g_pSwapChain1 ) g_pSwapChain1->Release();
    if( g_pSwapChain ) g_pSwapChain->Release();
    if( g_pImmediateContext1 ) g_pImmediateContext1->Release();
    if( g_pImmediateContext ) g_pImmediateContext->Release();


    ID3D11Debug* debugDevice = nullptr;
    g_pd3dDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&debugDevice));

    if (g_pd3dDevice1) g_pd3dDevice1->Release();
    if (g_pd3dDevice) g_pd3dDevice->Release();

    // handy for finding dx memory leaks
    debugDevice->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

    if (debugDevice)
        debugDevice->Release();
}

// Function to center the mouse in the window
void CenterMouseInWindow(HWND hWnd)
{
    // Get the dimensions of the window
    RECT rect;
    GetClientRect(hWnd, &rect);

    // Calculate the center position
    POINT center;
    center.x = (rect.right - rect.left) / 2;
    center.y = (rect.bottom - rect.top) / 2;

    // Convert the client area point to screen coordinates
    ClientToScreen(hWnd, &center);

    // Move the cursor to the center of the screen
    SetCursorPos(center.x, center.y);
}

//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    float movement = 0.2f;
    static bool mouseDown = false;

    extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    switch( message )
    {

    case WM_KEYDOWN:
        switch (wParam)
        {
        case 27:
            PostQuitMessage(0);
            break;
        case 'W':
            g_pCamera->MoveForward(movement);  // Adjust distance as needed
            break;
        case 'A':
            g_pCamera->StrafeLeft(movement);  // Adjust distance as needed
            break;
        case 'S':
            g_pCamera->MoveBackward(movement);  // Adjust distance as needed
            break;
        case 'D':
            g_pCamera->StrafeRight(movement);  // Adjust distance as needed
            break;
        }
        break;

    case WM_RBUTTONDOWN:
        mouseDown = true;
        break;
    case WM_RBUTTONUP:
        mouseDown = false;
        break;
    case WM_MOUSEMOVE:
    {
        if (!mouseDown)
        {
            break;
        }
        // Get the dimensions of the window
        RECT rect;
        GetClientRect(hWnd, &rect);

        // Calculate the center position of the window
        POINT windowCenter;
        windowCenter.x = (rect.right - rect.left) / 2;
        windowCenter.y = (rect.bottom - rect.top) / 2;

        // Convert the client area point to screen coordinates
        ClientToScreen(hWnd, &windowCenter);

        // Get the current cursor position
        POINTS mousePos = MAKEPOINTS(lParam);
        POINT cursorPos = { mousePos.x, mousePos.y };
        ClientToScreen(hWnd, &cursorPos);

        // Calculate the delta from the window center
        POINT delta;
        delta.x = cursorPos.x - windowCenter.x;
        delta.y = cursorPos.y - windowCenter.y;

        // Update the camera with the delta
        // (You may need to convert POINT to POINTS or use the deltas as is)
        g_pCamera->UpdateLookAt({ static_cast<short>(delta.x), static_cast<short>(delta.y) });

        // Recenter the cursor
        SetCursorPos(windowCenter.x, windowCenter.y);
    }
    break;
   
    case WM_ACTIVATE:
        if (LOWORD(wParam) != WA_INACTIVE) {
            CenterMouseInWindow(hWnd);
        }
        break;
    case WM_PAINT:
        hdc = BeginPaint( hWnd, &ps );
        EndPaint( hWnd, &ps );
        break;

    case WM_DESTROY:
        PostQuitMessage( 0 );
        break;

        // Note that this tutorial does not handle resizing (WM_SIZE) requests,
        // so we created the window without the resize border.

    default:
        return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}

void setupLightForRender()
{
    Light light;
    light.Enabled = static_cast<int>(true);
    light.LightType = PointLight;
    light.Color = XMFLOAT4(Colors::White);
    light.SpotAngle = XMConvertToRadians(45.0f);
    light.ConstantAttenuation = 1.0f;
    light.LinearAttenuation = 1;
    light.QuadraticAttenuation = 1;
    light.Position = XMFLOAT4(Light1Pos.x, Light1Pos.y, Light1Pos.z, 1);

    g_GameObject5.setPosition(Light1Pos);

    Light light2;
    light2.Enabled = static_cast<int>(true);
    light2.LightType = SpotLight;
    light2.Color = XMFLOAT4(Colors::Red);
    light2.SpotAngle = XMConvertToRadians(45.0f);
    light2.Direction = XMFLOAT4(cos(Light2Rot.x * 3.145/180)* sin(Light2Rot.y * 3.145 / 180), sin(Light2Rot.x * 3.145 / 180), cos(Light2Rot.x * 3.145 / 180)* cos(Light2Rot.y * 3.145 / 180), 1);
    light2.ConstantAttenuation = 1.0f;
    light2.LinearAttenuation = 1.0f;
    light2.QuadraticAttenuation = 1;
    light2.Position = XMFLOAT4(Light2Pos.x, Light2Pos.y, Light2Pos.z, 1);

    g_GameObject6.setPosition(Light2Pos);

    LightPropertiesConstantBuffer lightProperties;
    lightProperties.EyePosition = XMFLOAT4(g_pCamera->GetPosition().x, g_pCamera->GetPosition().y, g_pCamera->GetPosition().z, 1);
    lightProperties.Lights[0] = light;
    lightProperties.Lights[1] = light2;
    g_pImmediateContext->UpdateSubresource(g_pLightConstantBuffer, 0, nullptr, &lightProperties, 0, 0);
}

float calculateDeltaTime()
{
    // Update our time
    static float deltaTime = 0.0f;
    static ULONGLONG timeStart = 0;
    ULONGLONG timeCur = GetTickCount64();
    if (timeStart == 0)
        timeStart = timeCur;
    deltaTime = (timeCur - timeStart) / 1000.0f;
    timeStart = timeCur;

    float FPS60 = 1.0f / 60.0f;
    static float cummulativeTime = 0;

    // cap the framerate at 60 fps 
    cummulativeTime += deltaTime;
    if (cummulativeTime >= FPS60) {
        cummulativeTime = cummulativeTime - FPS60;
    }
    else {
        return 0;
    }

    return deltaTime;
}
void RenderDebugWindow(float deltaTime) {
    ImGui::Begin("Debug Window");
    ImGui::Text("To move around use the wasd keys\nand the right click for the camera.");
    ImGui::Text("For the sliders just hold \nleft click on them.");

    ImGui::Checkbox("Deferred Rendering", &Rendering);

    ImGui::Text("");

    ImGui::Text("Normal Maps");
    const char* items[] = { "0", "1", "2", "3", "4", "5"};
    static const char* current_item1 = 0;
    static const char* current_item2 = 0;
    static const char* current_item3 = 0;

    if (ImGui::BeginCombo("Cube 1", current_item1)) 
    {
        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
        {
            bool is_selected = (current_item1 == items[n]); 
            if (ImGui::Selectable(items[n], is_selected))
            {
                current_item1 = items[n];
                int temp = atoi(items[n]);
                g_GameObject.setNormalMapID(temp);
                g_GameObject.initMesh(g_pd3dDevice, g_pImmediateContext);
            }
                
            if (is_selected)
                ImGui::SetItemDefaultFocus();   
        }
        ImGui::EndCombo();
    }

    if (ImGui::BeginCombo("Cube 2", current_item2))
    {
        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
        {
            bool is_selected = (current_item2 == items[n]); 
            if (ImGui::Selectable(items[n], is_selected))
            {
                current_item2 = items[n];
                int temp = atoi(items[n]);
                g_GameObject2.setNormalMapID(temp);
                g_GameObject2.initMesh(g_pd3dDevice, g_pImmediateContext);
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();  
        }
        ImGui::EndCombo();
    }

    if (ImGui::BeginCombo("Cube 3", current_item3))
    {
        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
        {
            bool is_selected = (current_item3 == items[n]);
            if (ImGui::Selectable(items[n], is_selected))
            {
                current_item3 = items[n];
                int temp = atoi(items[n]);
                g_GameObject3.setNormalMapID(temp);
                g_GameObject3.initMesh(g_pd3dDevice, g_pImmediateContext);
            }

            if (is_selected)
                ImGui::SetItemDefaultFocus();  
        }
        ImGui::EndCombo();
    }

    ImGui::Text("Post Processing");
    ImGui::Checkbox("Gray Scale", &GrayScale);
    ImGui::Checkbox("Warm", &Warm);
    ImGui::Checkbox("Cold", &Cold);
    ImGui::Checkbox("GausianBlur", &GausianBlur);
    ImGui::Checkbox("Depth of Field", &DepthOfField);
    ImGui::Checkbox("Render to Cube", &RenderToCube);
    ImGui::Text("");
    ImGui::Text("Object Controls");
    ImGui::Text("Object 1");
    ImGui::Text("XYZ");
    ImGui::SetNextItemWidth(70);
    ImGui::DragFloat("##xxx", &Object1Pos.x, 0.1F);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::DragFloat("##yyy", &Object1Pos.y, 0.1F);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::DragFloat("##zzz", &Object1Pos.z, 0.1F);
    ImGui::Text("XYZ Rotation");
    ImGui::SetNextItemWidth(70);
    ImGui::DragInt("##xxxx", &Object1Rot.x);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::DragInt("##yyyy", &Object1Rot.y);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::DragInt("##zzzz", &Object1Rot.z);
    ImGui::Text("Object 2");
    ImGui::Text("XYZ Movement");
    ImGui::SetNextItemWidth(70);
    ImGui::DragFloat("##xx", &Object2Pos.x, 0.1F);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::DragFloat("##yy", &Object2Pos.y, 0.1F);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::DragFloat("##zz", &Object2Pos.z, 0.1F);
    ImGui::Text("XYZ Rotation");
    ImGui::SetNextItemWidth(70);
    ImGui::DragInt("##xxxxx", &Object2Rot.x);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::DragInt("##yyyyy", &Object2Rot.y);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::DragInt("##zzzzz", &Object2Rot.z);
    ImGui::Text("Object 3");
    ImGui::Text("XYZ Movement");
    ImGui::SetNextItemWidth(70);
    ImGui::DragFloat("##x", &Object3Pos.x, 0.1F);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::DragFloat("##y", &Object3Pos.y, 0.1F);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::DragFloat("##z", &Object3Pos.z, 0.1F);
    ImGui::Text("XYZ Rotation");
    ImGui::SetNextItemWidth(70);
    ImGui::DragInt("##xxxxxx", &Object3Rot.x);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::DragInt("##yyyyyy", &Object3Rot.y);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::DragInt("##zzzzzz", &Object3Rot.z);
    
    ImGui::Text("");
    ImGui::Text("Light Controls");
    ImGui::Text("Light 1 (Point)");
    ImGui::Text("XYZ Movement");
    ImGui::SetNextItemWidth(70);
    ImGui::DragFloat("##a", &Light1Pos.x, 0.1F);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::DragFloat("##b", &Light1Pos.y, 0.1F);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::DragFloat("##c", &Light1Pos.z, 0.1F);
    ImGui::Text("Light 2 (Directional)");
    ImGui::Text("XYZ Movement");
    ImGui::SetNextItemWidth(70);
    ImGui::DragFloat("##aa", &Light2Pos.x, 0.1F);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::DragFloat("##bb", &Light2Pos.y, 0.1F);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::DragFloat("##cv", &Light2Pos.z, 0.1F);
    ImGui::Text("Pitch and Yaw");
    ImGui::SetNextItemWidth(70);
    ImGui::DragInt("##aaA", &Light2Rot.x);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(70);
    ImGui::DragInt("##bbB", &Light2Rot.y);



    ImGui::End();

    ImGui::Begin("Buffers");
    ImGui::Text("Albedo");
    ImGui::Image((ImTextureID)g_pAlbedoSRV, ImVec2(256, 144));
    ImGui::Text("Specular");
    ImGui::Image((ImTextureID)g_pSpecularSRV, ImVec2(256, 144));
    ImGui::Text("Normal");
    ImGui::Image((ImTextureID)g_pNormalSRV, ImVec2(256, 144));
    ImGui::Text("Position");
    ImGui::Image((ImTextureID)g_pPositionSRV, ImVec2(256, 144));
    ImGui::Text("Lighting");
    ImGui::Image((ImTextureID)g_pLightingSRV, ImVec2(256, 144));
    ImGui::End();
}

void NormalRendering()
{

    XMMATRIX mGO = XMLoadFloat4x4(g_GameObject.getTransform());

    ConstantBuffer cb1;
    cb1.mWorld = XMMatrixTranspose(mGO);
    cb1.mView = XMMatrixTranspose(g_pCamera->GetViewMatrix());
    cb1.mProjection = XMMatrixTranspose(g_Projection);
    cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);
    g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb1, 0, 0);

    ID3D11Buffer* materialCB = g_GameObject.getMaterialConstantBuffer();
    g_pImmediateContext->PSSetConstantBuffers(1, 1, &materialCB);

    //Render Cube
    g_pImmediateContext->OMSetRenderTargets(1, nullRTV, nullptr);
    g_pImmediateContext->OMSetRenderTargets(1, &g_pRTTRenderTargetView, g_pDepthStencilView2);
    g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

    g_pImmediateContext->ClearRenderTargetView(g_pRTTRenderTargetView, Colors::MidnightBlue);
    g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView2, D3D11_CLEAR_DEPTH, 1.0f, 0);

    g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
    g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pLightConstantBuffer);

    g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
    g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pLightConstantBuffer);

    g_GameObject.draw(g_pImmediateContext, g_pFinalSRV, RenderToCube);


    //Render Cube 2
    mGO = XMLoadFloat4x4(g_GameObject2.getTransform());

    cb1.mWorld = XMMatrixTranspose(mGO);
    cb1.mView = XMMatrixTranspose(g_pCamera->GetViewMatrix());
    cb1.mProjection = XMMatrixTranspose(g_Projection);
    cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);
    g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb1, 0, 0);

    materialCB = g_GameObject2.getMaterialConstantBuffer();
    g_pImmediateContext->PSSetConstantBuffers(1, 1, &materialCB);

    g_pImmediateContext->OMSetRenderTargets(1, nullRTV, nullptr);
    g_pImmediateContext->OMSetRenderTargets(1, &g_pRTTRenderTargetView, g_pDepthStencilView2);
    g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

    g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
    g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pLightConstantBuffer);

    g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
    g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pLightConstantBuffer);

    g_GameObject2.draw(g_pImmediateContext, g_pFinalSRV, RenderToCube);

    //Render Cube 3
    mGO = XMLoadFloat4x4(g_GameObject3.getTransform());

    cb1.mWorld = XMMatrixTranspose(mGO);
    cb1.mView = XMMatrixTranspose(g_pCamera->GetViewMatrix());
    cb1.mProjection = XMMatrixTranspose(g_Projection);
    cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);
    g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb1, 0, 0);

    materialCB = g_GameObject3.getMaterialConstantBuffer();
    g_pImmediateContext->PSSetConstantBuffers(1, 1, &materialCB);

    g_pImmediateContext->OMSetRenderTargets(1, nullRTV, nullptr);
    g_pImmediateContext->OMSetRenderTargets(1, &g_pRTTRenderTargetView, g_pDepthStencilView2);
    g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

    g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
    g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pLightConstantBuffer);

    g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
    g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pLightConstantBuffer);

    g_GameObject3.draw(g_pImmediateContext, g_pFinalSRV, RenderToCube);

    //Render Plane
    mGO = XMLoadFloat4x4(g_GameObject4.getTransform());

    cb1.mWorld = XMMatrixTranspose(mGO);
    cb1.mView = XMMatrixTranspose(g_pCamera->GetViewMatrix());
    cb1.mProjection = XMMatrixTranspose(g_Projection);
    cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);
    g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb1, 0, 0);

    materialCB = g_GameObject4.getMaterialConstantBuffer();
    g_pImmediateContext->PSSetConstantBuffers(1, 1, &materialCB);

    g_pImmediateContext->OMSetRenderTargets(1, nullRTV, nullptr);
    g_pImmediateContext->OMSetRenderTargets(1, &g_pRTTRenderTargetView, g_pDepthStencilView2);
    g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

    g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
    g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pLightConstantBuffer);

    g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
    g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pLightConstantBuffer);

    g_GameObject4.draw(g_pImmediateContext, g_pFinalSRV, RenderToCube);

    //Render Light Cube 1
    mGO = XMLoadFloat4x4(g_GameObject5.getTransform());

    cb1.mWorld = XMMatrixTranspose(mGO);
    cb1.mView = XMMatrixTranspose(g_pCamera->GetViewMatrix());
    cb1.mProjection = XMMatrixTranspose(g_Projection);
    cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);
    g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb1, 0, 0);

    materialCB = g_GameObject5.getMaterialConstantBuffer();
    g_pImmediateContext->PSSetConstantBuffers(1, 1, &materialCB);

    g_pImmediateContext->OMSetRenderTargets(1, nullRTV, nullptr);
    g_pImmediateContext->OMSetRenderTargets(1, &g_pRTTRenderTargetView, g_pDepthStencilView2);
    g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

    g_pImmediateContext->VSSetShader(g_pVertexShader, nullptr, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
    g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pLightConstantBuffer);

    g_pImmediateContext->PSSetShader(g_pPixelShader, nullptr, 0);
    g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pLightConstantBuffer);

    g_GameObject5.draw(g_pImmediateContext, g_pFinalSRV, RenderToCube);

    g_pImmediateContext->CopyResource(g_pFinalTexture, g_pRTTRenderTargetTexture);
    // Render To Target

    PostProcessingBuffer pPB;
    pPB.GrayScale = GrayScale;
    pPB.Cold = Cold;
    pPB.ColdAmount = ColdAmount;
    pPB.Warm = Warm;
    pPB.WarmAmount = WarmAmount;
    pPB.DepthOfField = false;
    pPB.GausianBlur = GausianBlur;
    if (DepthOfField) { pPB.GausianBlur = true; }
    pPB.Bloom = Bloom;

    UINT stride = sizeof(SCREEN_VERTEX);
    UINT offset = 0;
    g_pImmediateContext->IASetVertexBuffers(0, 1, &m_pQuadVertexBuffer, &stride, &offset);
    g_pImmediateContext->IASetIndexBuffer(m_pQuadIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    g_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    g_pImmediateContext->IASetInputLayout(p_gQuadLayout);

    g_pImmediateContext->VSSetShader(g_pQuadVS, nullptr, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
    g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pLightConstantBuffer);

    g_pImmediateContext->PSSetShader(g_pQuadPS, nullptr, 0);
    g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pLightConstantBuffer);
    g_pImmediateContext->PSSetConstantBuffers(3, 1, &g_pPostProcessingBuffer);

    if (DepthOfField || GausianBlur) {
        g_pImmediateContext->OMSetRenderTargets(1, nullRTV, nullptr);
        g_pImmediateContext->OMSetRenderTargets(1, &g_pRTT2RenderTargetView, NULL);
        g_pImmediateContext->ClearRenderTargetView(g_pRTT2RenderTargetView, Colors::Pink);
        pPB.GausianW = true;
    }
    else {
        g_pImmediateContext->OMSetRenderTargets(1, nullRTV, nullptr);
        g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);
        g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::Pink);
    }

    /*g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 0, 0);*/

    g_pImmediateContext->UpdateSubresource(g_pPostProcessingBuffer, 0, nullptr, &pPB, 0, 0);
    g_pImmediateContext->PSSetShaderResources(2, 1, &g_pRTTShaderResourceView);
    
    g_pImmediateContext->DrawIndexed(6, 0, 0);
    
    g_pImmediateContext->PSSetShaderResources(2, 1, nullSRV);

    if (pPB.GausianBlur)
    {
        pPB.GausianW = false;
        pPB.GausianH = true;
        if (!DepthOfField)
        {
            g_pImmediateContext->OMSetRenderTargets(1, nullRTV, nullptr);
            g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);

            g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::Pink);
        }
        else {
            g_pImmediateContext->OMSetRenderTargets(1, nullRTV, nullptr);
            g_pImmediateContext->OMSetRenderTargets(1, &g_pRTT3RenderTargetView, NULL);

            g_pImmediateContext->ClearRenderTargetView(g_pRTT3RenderTargetView, Colors::Pink);
        }
        
        /*g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);*/

        g_pImmediateContext->UpdateSubresource(g_pPostProcessingBuffer, 0, nullptr, &pPB, 0, 0);

        g_pImmediateContext->PSSetShaderResources(2, 1, &g_pRTT2ShaderResourceView);

        g_pImmediateContext->DrawIndexed(6, 0, 0);

        g_pImmediateContext->PSSetShaderResources(2, 1, nullSRV);

        if (DepthOfField)
        {
            pPB.GausianH = false;
            pPB.GausianBlur = false;
            pPB.DepthOfField = true;

            g_pImmediateContext->OMSetRenderTargets(1, nullRTV, nullptr);
            g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, NULL);

            g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::Pink);
            //g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 0, 0);

            g_pImmediateContext->UpdateSubresource(g_pPostProcessingBuffer, 0, nullptr, &pPB, 0, 0);

            g_pImmediateContext->PSSetShaderResources(2, 1, &g_pRTTShaderResourceView);
            g_pImmediateContext->PSSetShaderResources(3, 1, &g_pRTT3ShaderResourceView);
            g_pImmediateContext->PSSetShaderResources(4, 1, &g_pDepthShaderResourceView);

            g_pImmediateContext->DrawIndexed(6, 0, 0);

            g_pImmediateContext->PSSetShaderResources(2, 1, nullSRV);
            g_pImmediateContext->PSSetShaderResources(3, 1, nullSRV);
            g_pImmediateContext->PSSetShaderResources(4, 1, nullSRV);
        }
    }
}

void DefferedRendering()
{
    for (int i = 0; i < 10; i++)
    {
        g_pImmediateContext->PSSetShaderResources(i, 1, nullSRV);
    }

    ID3D11RenderTargetView* m_pRenderViews[4];
    m_pRenderViews[0] = g_pAlbedoRTV;
    m_pRenderViews[1] = g_pSpecularRTV;
    m_pRenderViews[2] = g_pNormalRTV;
    m_pRenderViews[3] = g_pPositionRTV;

    g_pImmediateContext->OMSetRenderTargets(1, nullRTV, nullptr);
    g_pImmediateContext->OMSetRenderTargets(4, m_pRenderViews, g_pDepthStencilView2);

    g_pImmediateContext->ClearRenderTargetView(g_pAlbedoRTV, Colors::MidnightBlue);
    g_pImmediateContext->ClearRenderTargetView(g_pSpecularRTV, Colors::MidnightBlue);
    g_pImmediateContext->ClearRenderTargetView(g_pNormalRTV, Colors::MidnightBlue);
    g_pImmediateContext->ClearRenderTargetView(g_pPositionRTV, Colors::MidnightBlue);

    g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView2, D3D11_CLEAR_DEPTH, 1.0f, 0);

    g_pImmediateContext->IASetInputLayout(g_pVertexLayout);

    g_pImmediateContext->VSSetShader(g_pGBUFFERVS, nullptr, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
    g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pLightConstantBuffer);

    g_pImmediateContext->PSSetShader(g_pGBUFFERPS, nullptr, 0);
    g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pLightConstantBuffer);
    g_pImmediateContext->PSSetConstantBuffers(0, 1, &g_pConstantBuffer);

    XMMATRIX mGO = XMLoadFloat4x4(g_GameObject.getTransform());

    ConstantBuffer cb1;
    cb1.mWorld = XMMatrixTranspose(mGO);
    cb1.mView = XMMatrixTranspose(g_pCamera->GetViewMatrix());
    cb1.mProjection = XMMatrixTranspose(g_Projection);
    cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);
    g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb1, 0, 0);

    ID3D11Buffer* materialCB = g_GameObject.getMaterialConstantBuffer();
    g_pImmediateContext->PSSetConstantBuffers(1, 1, &materialCB);

    //Render Cube

    g_GameObject.draw(g_pImmediateContext, *nullSRV, false);


    //Render Cube 2
    mGO = XMLoadFloat4x4(g_GameObject2.getTransform());

    cb1.mWorld = XMMatrixTranspose(mGO);
    cb1.mView = XMMatrixTranspose(g_pCamera->GetViewMatrix());
    cb1.mProjection = XMMatrixTranspose(g_Projection);
    cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);
    g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb1, 0, 0);

    materialCB = g_GameObject2.getMaterialConstantBuffer();
    g_pImmediateContext->PSSetConstantBuffers(1, 1, &materialCB);

    g_GameObject2.draw(g_pImmediateContext, * nullSRV, false);

    //Render Cube 3
    mGO = XMLoadFloat4x4(g_GameObject3.getTransform());

    cb1.mWorld = XMMatrixTranspose(mGO);
    cb1.mView = XMMatrixTranspose(g_pCamera->GetViewMatrix());
    cb1.mProjection = XMMatrixTranspose(g_Projection);
    cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);
    g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb1, 0, 0);

    materialCB = g_GameObject3.getMaterialConstantBuffer();
    g_pImmediateContext->PSSetConstantBuffers(1, 1, &materialCB);

    g_GameObject3.draw(g_pImmediateContext, * nullSRV, false);
    
    //Render FlatPlane
    mGO = XMLoadFloat4x4(g_GameObject4.getTransform());

    cb1.mWorld = XMMatrixTranspose(mGO);
    cb1.mView = XMMatrixTranspose(g_pCamera->GetViewMatrix());
    cb1.mProjection = XMMatrixTranspose(g_Projection);
    cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);
    g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb1, 0, 0);

    materialCB = g_GameObject4.getMaterialConstantBuffer();
    g_pImmediateContext->PSSetConstantBuffers(1, 1, &materialCB);

    g_GameObject4.draw(g_pImmediateContext, * nullSRV, false);

    //Render Light object 
    mGO = XMLoadFloat4x4(g_GameObject5.getTransform());

    cb1.mWorld = XMMatrixTranspose(mGO);
    cb1.mView = XMMatrixTranspose(g_pCamera->GetViewMatrix());
    cb1.mProjection = XMMatrixTranspose(g_Projection);
    cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);
    g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb1, 0, 0);

    materialCB = g_GameObject5.getMaterialConstantBuffer();
    g_pImmediateContext->PSSetConstantBuffers(1, 1, &materialCB);

    g_GameObject5.draw(g_pImmediateContext, *nullSRV, false);
    
    //Render Light Object 2
    mGO = XMLoadFloat4x4(g_GameObject6.getTransform());

    cb1.mWorld = XMMatrixTranspose(mGO);
    cb1.mView = XMMatrixTranspose(g_pCamera->GetViewMatrix());
    cb1.mProjection = XMMatrixTranspose(g_Projection);
    cb1.vOutputColor = XMFLOAT4(0, 0, 0, 0);
    g_pImmediateContext->UpdateSubresource(g_pConstantBuffer, 0, nullptr, &cb1, 0, 0);

    materialCB = g_GameObject6.getMaterialConstantBuffer();
    g_pImmediateContext->PSSetConstantBuffers(1, 1, &materialCB);

    g_GameObject6.draw(g_pImmediateContext, *nullSRV, false);

    //Light Render
    g_pImmediateContext->OMSetRenderTargets(1, &g_pLightingRTV, g_pDepthStencilView);
    g_pImmediateContext->ClearRenderTargetView(g_pLightingRTV, Colors::AliceBlue);
    g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);


    UINT stride = sizeof(SCREEN_VERTEX);
    UINT offset = 0;

    g_pImmediateContext->IASetInputLayout(p_gQuadLayout);
    g_pImmediateContext->IASetVertexBuffers(0, 1, &m_pQuadVertexBuffer, &stride, &offset);
    g_pImmediateContext->IASetIndexBuffer(m_pQuadIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    g_pImmediateContext->VSSetShader(g_pLightingVS, nullptr, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
    g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pLightConstantBuffer);

    g_pImmediateContext->PSSetShader(g_pLightingPS, nullptr, 0);
    g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pLightConstantBuffer);
    g_pImmediateContext->PSSetShaderResources(6, 1, &g_pSpecularSRV);
    g_pImmediateContext->PSSetShaderResources(7, 1, &g_pNormalSRV);
    g_pImmediateContext->PSSetShaderResources(8, 1, &g_pPositionSRV);

    g_pImmediateContext->DrawIndexed(6, 0, 0);

    // Final Render
    g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);
    g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, Colors::AliceBlue);
    g_pImmediateContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    g_pImmediateContext->IASetInputLayout(p_gQuadLayout);
    g_pImmediateContext->IASetVertexBuffers(0, 1, &m_pQuadVertexBuffer, &stride, &offset);
    g_pImmediateContext->IASetIndexBuffer(m_pQuadIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    g_pImmediateContext->VSSetShader(g_pFinalVS, nullptr, 0);
    g_pImmediateContext->VSSetConstantBuffers(0, 1, &g_pConstantBuffer);
    g_pImmediateContext->VSSetConstantBuffers(2, 1, &g_pLightConstantBuffer);

    g_pImmediateContext->PSSetShader(g_pFinalPS, nullptr, 0);
    g_pImmediateContext->PSSetConstantBuffers(2, 1, &g_pLightConstantBuffer);
    g_pImmediateContext->PSSetShaderResources(5, 1, &g_pAlbedoSRV);
    g_pImmediateContext->PSSetShaderResources(9, 1, &g_pLightingSRV);

    g_pImmediateContext->DrawIndexed(6, 0, 0);
}
void Render()
{
    float t = calculateDeltaTime(); // capped at 60 fps
    if (t == 0.0f)
        return;

    // Start the Dear ImGui frame

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    g_GameObject.update(t, g_pImmediateContext, XMMatrixRotationX(XMConvertToRadians(Object1Rot.x)) * XMMatrixRotationY(XMConvertToRadians(Object1Rot.y)) * XMMatrixRotationZ(XMConvertToRadians(Object1Rot.z)), Object1Pos);
    g_GameObject2.update(t, g_pImmediateContext, XMMatrixRotationX(XMConvertToRadians(Object2Rot.x)) * XMMatrixRotationY(XMConvertToRadians(Object2Rot.y)) * XMMatrixRotationZ(XMConvertToRadians(Object2Rot.z)), Object2Pos);
    g_GameObject3.update(t, g_pImmediateContext, XMMatrixRotationX(XMConvertToRadians(Object3Rot.x)) * XMMatrixRotationY(XMConvertToRadians(Object3Rot.y)) * XMMatrixRotationZ(XMConvertToRadians(Object3Rot.z)), Object3Pos);
    g_GameObject4.update(t, g_pImmediateContext, XMMatrixRotationX(0), Object4Pos);
    g_GameObject5.update(t, g_pImmediateContext, XMMatrixRotationX(0), Light1Pos);
    g_GameObject6.update(t, g_pImmediateContext, XMMatrixRotationX(0), Light2Pos);

    RenderDebugWindow(t);

    setupLightForRender();

    if (!Rendering)
    {
        NormalRendering();
    }
    else {
        DefferedRendering();
    }
    

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // Present our back buffer to our front buffer
    g_pSwapChain->Present( 0, 0 );
}



