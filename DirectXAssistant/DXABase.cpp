#include "DXABase.h"
#include <wincodec.h>
#include <wincodecsdk.h>

DXAWICFactory::DXAWICFactory()
{
	pImagingFactory = nullptr;
}

DXAWICFactory::~DXAWICFactory()
{
	Uninitialize();
}

bool DXAWICFactory::Initialize()
{
	if (pImagingFactory) return true;
	HRESULT hr = CoCreateInstance(
		CLSID_WICImagingFactory,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pImagingFactory)
	);
	return SUCCEEDED(hr);
}

void DXAWICFactory::Uninitialize()
{
	if (pImagingFactory) pImagingFactory->Release();
	pImagingFactory = nullptr;
}

DXAWICFactory::WICFactory* DXAWICFactory::ImagingFactory()
{
	return pImagingFactory;
}

DXAWICFactory::operator WICFactory* ()
{
	return pImagingFactory;
}

HRESULT DXACreateBitmap(IWICImagingFactory* pImagingFactory, ID2D1RenderTarget* pRenderTarget, LPCWSTR szPath, ID2D1Bitmap** ppBitmap)
{
	if (!pImagingFactory) return E_FAIL;
	IWICBitmapDecoder* pDecoder = nullptr;
	IWICBitmapFrameDecode* pSource = nullptr;
	IWICFormatConverter* pConverter = nullptr;
	HRESULT hr = pImagingFactory->CreateDecoderFromFilename(
		szPath,
		NULL,
		GENERIC_READ,
		WICDecodeMetadataCacheOnLoad,
		&pDecoder
	);
	if (SUCCEEDED(hr))
	{
		hr = pDecoder->GetFrame(0, &pSource);
	}
	if (SUCCEEDED(hr))
	{
		hr = pImagingFactory->CreateFormatConverter(&pConverter);
	}
	if (SUCCEEDED(hr))
	{
		hr = pConverter->Initialize(
			pSource,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.f,
			WICBitmapPaletteTypeMedianCut
		);
	}
	if (SUCCEEDED(hr))
	{
		// Create a Direct2D bitmap from the WIC bitmap.
		hr = pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			ppBitmap
		);
	}
	if (pDecoder) pDecoder->Release();
	if (pSource) pSource->Release();
	if (pConverter) pConverter->Release();
	return hr;
}

HRESULT DXACreateBitmap(IWICImagingFactory* pImagingFactory, ID2D1RenderTarget* pRenderTarget, HMODULE hModule, LPCWSTR szResourceName, LPCWSTR szResourceType, ID2D1Bitmap** ppBitmap)
{
	if (!pImagingFactory) return E_FAIL;
	IWICBitmapDecoder* pDecoder = nullptr;
	IWICBitmapFrameDecode* pSource = nullptr;
	IWICFormatConverter* pConverter = nullptr;
	IWICStream* pStream = nullptr;
	HRSRC hRscBitmap = nullptr;
	HGLOBAL hGlobalBitmap = nullptr;
	void* pImage = nullptr;
	DWORD nImageSize = 0;
	hRscBitmap = FindResourceW(hModule, szResourceName, szResourceType);
	HRESULT hr = hRscBitmap ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		// Load the resource.
		hGlobalBitmap = LoadResource(hModule, hRscBitmap);
		hr = hGlobalBitmap ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		// Lock it to get a system memory pointer.
		pImage = LockResource(hGlobalBitmap);
		hr = pImage ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		// Calculate the size.
		nImageSize = SizeofResource(hModule, hRscBitmap);
		hr = nImageSize ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		// Create a WIC stream to map onto the memory.
		hr = pImagingFactory->CreateStream(&pStream);
	}
	if (SUCCEEDED(hr))
	{
		// Initialize the stream with the memory pointer and size.
		hr = pStream->InitializeFromMemory(
			reinterpret_cast<BYTE*>(pImage),
			nImageSize
		);
	}
	if (SUCCEEDED(hr))
	{
		// Create a decoder for the stream.
		hr = pImagingFactory->CreateDecoderFromStream(
			pStream,
			NULL,
			WICDecodeMetadataCacheOnLoad,
			&pDecoder
		);
	}
	if (SUCCEEDED(hr))
	{
		hr = pDecoder->GetFrame(0, &pSource);
	}
	if (SUCCEEDED(hr))
	{
		hr = pImagingFactory->CreateFormatConverter(&pConverter);
	}
	if (SUCCEEDED(hr))
	{
		hr = pConverter->Initialize(
			pSource,
			GUID_WICPixelFormat32bppPBGRA,
			WICBitmapDitherTypeNone,
			NULL,
			0.f,
			WICBitmapPaletteTypeMedianCut
		);
	}
	if (SUCCEEDED(hr))
	{
		// Create a Direct2D bitmap from the WIC bitmap.
		hr = pRenderTarget->CreateBitmapFromWicBitmap(
			pConverter,
			NULL,
			ppBitmap
		);
	}
	if (pDecoder) pDecoder->Release();
	if (pSource) pSource->Release();
	if (pConverter) pConverter->Release();
	if (pStream) pStream->Release();
	return hr;
}

ATOM RegisterWndClass(HINSTANCE hInstance, LPCWSTR lpszClassName, WNDPROC lpfnWndProc, HICON hIcon, HICON hIconSm, HCURSOR hCursor, LPCWSTR lpszMenuName)
{
	WNDCLASSEXW wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEXW);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = lpszClassName;
	wcex.lpfnWndProc = lpfnWndProc;
	wcex.hIcon = hIcon;
	wcex.hIconSm = hIconSm;
	wcex.hCursor = hCursor ? hCursor : LoadCursorW(nullptr, MAKEINTRESOURCEW(IDC_ARROW));
	wcex.lpszMenuName = lpszMenuName;
	return RegisterClassExW(&wcex);
}
