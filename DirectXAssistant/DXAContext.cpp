#include "DXAContext.h"

// 计算位图的裁剪区域以保证位图在绘制到指定位置时不会拉伸
static D2D1_RECT_F CalcBitmapFillRect(D2D1_RECT_F bitmapRect, D2D1_RECT_F targetRect)
{
	FLOAT xS = targetRect.right - targetRect.left; // xBitmap
	FLOAT yS = targetRect.bottom - targetRect.top; // yBitmap
	FLOAT rS = xS / yS;
	FLOAT xB = bitmapRect.right - bitmapRect.left; // xScreen
	FLOAT yB = bitmapRect.bottom - bitmapRect.top; // yScreen
	FLOAT rB = xB / yB;
	D2D1_RECT_F rR = bitmapRect;
	if (rS > rB)
	{
		yB = xB / rS;
		rR.top = (rR.top + rR.bottom - yB) / 2;
		rR.bottom = rR.top + yB;
	}
	else if (rS < rB)
	{
		xB = yB * rS;
		rR.left = (rR.left + rR.right - xB) / 2;
		rR.right = rR.left + xB;
	}
	return rR;
}

// 计算位图的放置区域以保证位图在绘制到指定位置时不会拉伸
static D2D1_RECT_F CalcBitmapPutRect(D2D1_RECT_F bitmapRect, D2D1_RECT_F targetRect)
{
	FLOAT xS = targetRect.right - targetRect.left; // xBitmap
	FLOAT yS = targetRect.bottom - targetRect.top; // yBitmap
	FLOAT rS = xS / yS;
	FLOAT xB = bitmapRect.right - bitmapRect.left; // xScreen
	FLOAT yB = bitmapRect.bottom - bitmapRect.top; // yScreen
	FLOAT rB = xB / yB;
	D2D1_RECT_F rR = targetRect;
	if (rS > rB)
	{
		xS = yS * rB;
		rR.left = (rR.left + rR.right - xS) / 2;
		rR.right = rR.left + xS;
	}
	else if (rS < rB)
	{
		yS = xS / rB;
		rR.top = (rR.top + rR.bottom - yS) / 2;
		rR.bottom = rR.top + yS;
	}
	return rR;
}

DXAContext::DXAContext()
{
	ZeroData();
}

DXAContext::~DXAContext()
{
	Uninitialize();
}

void DXAContext::Initialize(DXADevice* Device, HWND hWnd)
{
	pDevice = Device;
	handle_hWnd = hWnd;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
	swapChainDesc.Width = 0;
	swapChainDesc.Height = 0;
	swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swapChainDesc.Flags = 0;
	pDevice->DXGIFactory()->CreateSwapChainForHwnd(pDevice->D3Device(), hWnd, &swapChainDesc, nullptr, nullptr, &pDXGISwapChain);
	pDXGISwapChain->ResizeBuffers(2, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), DXGI_FORMAT_UNKNOWN, 0);
	pDevice->DXGIDevice()->SetMaximumFrameLatency(1);
	//pDXGISwapChain->GetBuffer(0, IID_PPV_ARGS(&p3DTexture2D));
	bitmapProperties =
		D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
			GetDpiForWindow(hWnd),
			GetDpiForWindow(hWnd)
		);
	pDXGISwapChain->GetBuffer(0, IID_PPV_ARGS(&pDXGISurface));
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &p2DFactory);
	p2DFactory->CreateDevice(pDevice->DXGIDevice(), &p2Device);
	p2Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &p2DContext);
	p2DContext->CreateBitmapFromDxgiSurface(pDXGISurface, bitmapProperties, &pDrawBitmap);
	p2DContext->SetTarget(pDrawBitmap);
}

void DXAContext::Uninitialize()
{
	if (pDrawBitmap) pDrawBitmap->Release();
	if (p2DContext) p2DContext->Release();
	if (p2Device) p2Device->Release();
	if (p2DFactory) p2DFactory->Release();
	if (pDXGISurface) pDXGISurface->Release();
	if (pDXGISwapChain) pDXGISwapChain->Release();
	ZeroData();
}

void DXAContext::BeginDraw()
{
	if (!p2DContext) return;
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(handle_hWnd, &ps);
	EndPaint(handle_hWnd, &ps);
	p2DContext->BeginDraw();
}

bool DXAContext::EndDraw()
{
	HRESULT hr = p2DContext->EndDraw();
	if (SUCCEEDED(hr)) hr = pDXGISwapChain->Present(1, 0);
	return SUCCEEDED(hr);
}

void DXAContext::Clear(D2D1_COLOR_F Color)
{
	p2DContext->Clear(Color);
}

void DXAContext::FillBitmap(ID2D1Bitmap* pBitmap, D2D1_RECT_F dstRect, D2D1_RECT_F srcRect)
{
	if (srcRect.left == 0 && srcRect.right == 0 && srcRect.top == 0 && srcRect.bottom == 0)
	{
		srcRect.right = pBitmap->GetSize().width;
		srcRect.bottom = pBitmap->GetSize().height;
	}
	srcRect = CalcBitmapFillRect(srcRect, dstRect);
	p2DContext->DrawBitmap(pBitmap, dstRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, srcRect);
}

D2D1_RECT_F DXAContext::PutBitmap(ID2D1Bitmap* pBitmap, D2D1_RECT_F dstRect, D2D1_RECT_F srcRect)
{
	if (srcRect.left == 0 && srcRect.right == 0 && srcRect.top == 0 && srcRect.bottom == 0)
	{
		srcRect.right = pBitmap->GetSize().width;
		srcRect.bottom = pBitmap->GetSize().height;
	}
	dstRect = CalcBitmapPutRect(srcRect, dstRect);
	p2DContext->DrawBitmap(pBitmap, dstRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, srcRect);
	return dstRect;
}

void DXAContext::DrawProgress(D2D1_RECT_F Rect, float Percentage, D2D1_COLOR_F BackColor, D2D1_COLOR_F FrontColor)
{
	ID2D1SolidColorBrush* pBrush = nullptr;
	p2DContext->CreateSolidColorBrush(BackColor, &pBrush);
	if (!pBrush) return;
	p2DContext->FillRectangle(Rect, pBrush);
	pBrush->Release();
	Rect.right = Rect.left + ((Rect.right - Rect.left) * Percentage);
	p2DContext->CreateSolidColorBrush(FrontColor, &pBrush);
	if (!pBrush) return;
	p2DContext->FillRectangle(Rect, pBrush);
	pBrush->Release();
	pBrush = nullptr;
}

bool DXAContext::DrawButton(D2D1_RECT_F Rect, LPCWSTR szText, DXATextFormat* TextFormat, ID2D1Bitmap* pBackgroundBitmap, D2D1_COLOR_F FrontColor, D2D1_COLOR_F BackColor)
{
	// 先使用IDWriteTextLayout计算结果，如果不会溢出则直接输出到窗口
	// 如果计算发现会溢出则使用Layout减小字号再次计算，直至不会溢出为止
	// 如果字号减小到预设的最小值还是会溢出就返回false
	if (!szText || !TextFormat) return false;
	IDWriteTextLayout* pLayout = nullptr;
	HRESULT hr = TextFormat->WriteFactory()->CreateTextLayout(
		szText,
		wcslen(szText),
		TextFormat->TextFormat(),
		Rect.right - Rect.left,
		Rect.bottom - Rect.top,
		&pLayout
	);
	pLayout->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	float fFontSize = TextFormat->FontSize();
	ID2D1SolidColorBrush* pBrush = nullptr;
	if (SUCCEEDED(hr)) {
		if (pBackgroundBitmap) FillBitmap(pBackgroundBitmap, Rect);
		else {
			hr = p2DContext->CreateSolidColorBrush(BackColor, &pBrush);
			if (pBrush) p2DContext->FillRectangle(Rect, pBrush);
			pBrush->Release();
		}
		hr = p2DContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &pBrush);
		if (pBrush) p2DContext->DrawRectangle(Rect, pBrush);
		pBrush->Release();
	}
	if (SUCCEEDED(hr)) {
		DWRITE_TEXT_METRICS textMetrics = { 0 };
		DWRITE_TEXT_RANGE textRange = { 0, wcslen(szText) };
		D2D1_SIZE_F size = { 0 };
		while (true) {
			hr = pLayout->GetMetrics(&textMetrics);
			if (textMetrics.widthIncludingTrailingWhitespace > Rect.right - Rect.left ||
				textMetrics.height > Rect.bottom - Rect.top) {
				if (fFontSize <= 8) return false;
				fFontSize -= 8;
				hr = pLayout->SetFontSize(fFontSize, textRange);
			}
			else break;
		}
		float vstart = ((Rect.bottom - Rect.top) - textMetrics.height) / 2;
		p2DContext->CreateSolidColorBrush(FrontColor, &pBrush);
		if (pLayout && pBrush) p2DContext->DrawTextLayout({ Rect.left, Rect.top + vstart }, pLayout, pBrush);
		pBrush->Release();
		return true;
	}
	return false;
}

HWND DXAContext::hWnd() const
{
	return handle_hWnd;
}

ID3D11Device* DXADevice::D3Device()
{
	return p3Device;
}

ID3D11DeviceContext* DXADevice::D3DContext()
{
	return p3DContext;
}

IDXGIDevice3* DXADevice::DXGIDevice()
{
	return pDXGIDevice;
}

IDXGIAdapter* DXADevice::DXGIAdapter()
{
	return pDXGIAdapter;
}

IDXGIFactory2* DXADevice::DXGIFactory()
{
	return pDXGIFactory;
}

ID2D1Factory1* DXAContext::D2DFactory()
{
	return p2DFactory;
}

ID2D1Device* DXAContext::D2Device()
{
	return p2Device;
}

ID2D1DeviceContext* DXAContext::D2DContext()
{
	return p2DContext;
}

DXAContext::operator HWND()
{
	return handle_hWnd;
}

DXADevice::operator ID3D11Device* ()
{
	return p3Device;
}

DXADevice::operator ID3D11DeviceContext* ()
{
	return p3DContext;
}

DXADevice::operator IDXGIDevice3* ()
{
	return pDXGIDevice;
}

DXADevice::operator IDXGIAdapter* ()
{
	return pDXGIAdapter;
}

DXADevice::operator IDXGIFactory2* ()
{
	return pDXGIFactory;
}

DXAContext::operator ID2D1Factory1* ()
{
	return p2DFactory;
}

DXAContext::operator ID2D1Device* ()
{
	return p2Device;
}

DXAContext::operator ID2D1DeviceContext* ()
{
	return p2DContext;
}

void DXAContext::ZeroData()
{
	p2DContext = nullptr;
	p2Device = nullptr;
	p2DFactory = nullptr;
	pDXGISurface = nullptr;
	pDXGISwapChain = nullptr;
	pDrawBitmap = nullptr;
	bitmapProperties = D2D1_BITMAP_PROPERTIES1();
}

DXADevice::DXADevice()
{
	ZeroData();
}

DXADevice::~DXADevice()
{
	Uninitialize();
}

void DXADevice::Initialize()
{
	D3D_FEATURE_LEVEL featureLevels[] =
	{
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0,
	D3D_FEATURE_LEVEL_10_1,
	D3D_FEATURE_LEVEL_10_0,
	D3D_FEATURE_LEVEL_9_3,
	D3D_FEATURE_LEVEL_9_2,
	D3D_FEATURE_LEVEL_9_1
	};
	D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &p3Device, &currentLevel, &p3DContext);
	p3Device->QueryInterface(&pDXGIDevice);
	pDXGIDevice->GetAdapter(&pDXGIAdapter);
	pDXGIAdapter->GetParent(IID_PPV_ARGS(&pDXGIFactory));
}

void DXADevice::Uninitialize()
{
	if (pDXGIAdapter) pDXGIAdapter->Release();
	if (pDXGIDevice) pDXGIDevice->Release();
	if (pDXGIFactory) pDXGIFactory->Release();
	if (p3Device) p3Device->Release();
	ZeroData();
}

void DXADevice::ZeroData()
{
	p3DContext = nullptr;
	p3Device = nullptr;
	pDXGIAdapter = nullptr;
	pDXGIDevice = nullptr;
	pDXGIFactory = nullptr;
	currentLevel = D3D_FEATURE_LEVEL();
}
