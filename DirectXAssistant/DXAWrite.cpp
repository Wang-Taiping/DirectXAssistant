#include "DXAWrite.h"
#include <wincodec.h>
#include <wincodecsdk.h>

DXAWriteFactory::DXAWriteFactory()
{
	pFactory = nullptr;
}

DXAWriteFactory::~DXAWriteFactory()
{
	Release();
}

HRESULT DXAWriteFactory::Initialize()
{
	if (pFactory) return E_FAIL;
	return DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(&pFactory)
	);
}

void DXAWriteFactory::Release()
{
	if (pFactory) {
		pFactory->Release();
		pFactory = nullptr;
	}
}

DXAWriteFactory::DWriteFactory* DXAWriteFactory::WriteFactory()
{
	return pFactory;
}

DXAWriteFactory::operator DWriteFactory* ()
{
	return pFactory;
}

DXAFontCollection::DXAFontCollection()
{
	pMemoryLoader = nullptr;
	pFontCollection = nullptr;
	pWriteFactory = nullptr;
}

DXAFontCollection::~DXAFontCollection()
{
	Release();
}

HRESULT DXAFontCollection::Initialize(DXAWriteFactory::DWriteFactory* pWriteFactory, LPCWSTR szPath, LPWSTR FontFamilyBuffer, UINT* BufElemNum)
{
	if (pFontCollection) return E_FAIL;
	IDWriteFontSetBuilder1* pFSBuilder = nullptr;
	IDWriteFontSet* pFontSet = nullptr;
	IDWriteFontFile* pFontFile = nullptr;
	IDWriteFontCollection1* pFCollection = nullptr;
	IDWriteFontFamily* pFFamily = nullptr;
	IDWriteLocalizedStrings* pFFName = nullptr;
	HRESULT hr = pWriteFactory->CreateFontSetBuilder(&pFSBuilder);
	if (SUCCEEDED(hr)) hr = pWriteFactory->CreateFontFileReference(szPath, /* lastWriteTime*/ nullptr, &pFontFile);
	if (SUCCEEDED(hr)) hr = pFSBuilder->AddFontFile(pFontFile);
	if (SUCCEEDED(hr)) hr = pFSBuilder->CreateFontSet(&pFontSet);
	if (SUCCEEDED(hr)) hr = pWriteFactory->CreateFontCollectionFromFontSet(pFontSet, &pFCollection);
	if (SUCCEEDED(hr)) hr = pFCollection->GetFontFamily(0, &pFFamily);
	if (SUCCEEDED(hr)) hr = pFFamily->GetFamilyNames(&pFFName);
	UINT32 len = 0;
	if (SUCCEEDED(hr)) hr = pFFName->GetStringLength(0, &len);
	if (*BufElemNum <= len) {
		*BufElemNum = len + 1;
		hr = E_FAIL;
	}
	if (SUCCEEDED(hr)) hr = pFFName->GetString(0, FontFamilyBuffer, *BufElemNum);
	if (pFSBuilder) pFSBuilder->Release();
	if (pFontSet) pFontSet->Release();
	if (pFontFile) pFontFile->Release();
	if (pFFamily) pFFamily->Release();
	if (pFFName) pFFName->Release();
	if (!SUCCEEDED(hr) && pFCollection) pFCollection->Release();
	if (SUCCEEDED(hr)) {
		this->pFontCollection = pFCollection;
		this->pWriteFactory = pWriteFactory;
	}
	return hr;
}

HRESULT DXAFontCollection::Initialize(DXAWriteFactory::DWriteFactory* pWriteFactory, HMODULE hModule, LPCWSTR szResourceName, LPCWSTR szResourceType, LPWSTR FontFamilyBuffer, UINT* BufElemNum)
{
	if (pFontCollection) return E_FAIL;
	IDWriteFontSetBuilder1* pFSBuilder = nullptr;
	IDWriteFontSet* pFontSet = nullptr;
	IDWriteFontFile* pFontFile = nullptr;
	IDWriteFontCollection1* pFCollection = nullptr;
	IDWriteFontFamily* pFFamily = nullptr;
	IDWriteLocalizedStrings* pFFName = nullptr;
	HRSRC hRscFont = nullptr;
	HGLOBAL hGlobalFont = nullptr;
	void* pFontData = nullptr;
	DWORD nFDSize = 0;
	hRscFont = FindResourceW(hModule, szResourceName, szResourceType);
	HRESULT hr = hRscFont ? S_OK : E_FAIL;
	if (SUCCEEDED(hr)) {
		// Load the resource.
		hGlobalFont = LoadResource(hModule, hRscFont);
		hr = hGlobalFont ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr)) {
		// Lock it to get a system memory pointer.
		pFontData = LockResource(hGlobalFont);
		hr = pFontData ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr)) {
		// Calculate the size.
		nFDSize = SizeofResource(hModule, hRscFont);
		hr = nFDSize ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr)) hr = pWriteFactory->CreateFontSetBuilder(&pFSBuilder);
	if (SUCCEEDED(hr)) hr = pWriteFactory->CreateInMemoryFontFileLoader(&this->pMemoryLoader);
	if (SUCCEEDED(hr)) hr = pWriteFactory->RegisterFontFileLoader(this->pMemoryLoader);
	if (SUCCEEDED(hr)) hr = this->pMemoryLoader->CreateInMemoryFontFileReference(pWriteFactory, pFontData, nFDSize, nullptr, &pFontFile);
	if (SUCCEEDED(hr)) hr = pFSBuilder->AddFontFile(pFontFile);
	if (SUCCEEDED(hr)) hr = pFSBuilder->CreateFontSet(&pFontSet);
	if (SUCCEEDED(hr)) hr = pWriteFactory->CreateFontCollectionFromFontSet(pFontSet, &pFCollection);
	if (SUCCEEDED(hr)) hr = pFCollection->GetFontFamily(0, &pFFamily);
	if (SUCCEEDED(hr)) hr = pFFamily->GetFamilyNames(&pFFName);
	UINT32 len = 0;
	if (SUCCEEDED(hr)) hr = pFFName->GetStringLength(0, &len);
	if (*BufElemNum <= len) {
		*BufElemNum = len + 1;
		hr = E_FAIL;
	}
	if (SUCCEEDED(hr)) hr = pFFName->GetString(0, FontFamilyBuffer, *BufElemNum);
	if (pFSBuilder) pFSBuilder->Release();
	if (pFontSet) pFontSet->Release();
	if (pFontFile) pFontFile->Release();
	if (pFFamily) pFFamily->Release();
	if (pFFName) pFFName->Release();
	if (!SUCCEEDED(hr) && pFCollection) pFCollection->Release();
	if (SUCCEEDED(hr)) {
		this->pFontCollection = pFCollection;
		this->pWriteFactory = pWriteFactory;
	}
	return hr;
}

void DXAFontCollection::Release()
{
	if (pFontCollection) pFontCollection->Release();
	if (pMemoryLoader)
	{
		pWriteFactory->UnregisterFontFileLoader(pMemoryLoader);
		pMemoryLoader->Release();
	}
	pFontCollection = nullptr;
	pMemoryLoader = nullptr;
	pWriteFactory = nullptr;
}

DXAFontCollection::DWriteFontCollection* DXAFontCollection::FontCollection()
{
	return pFontCollection;
}

DXAWriteFactory::DWriteFactory* DXAFontCollection::WriteFactory()
{
	return pWriteFactory;
}

DXAFontCollection::operator DWriteFontCollection* ()
{
	return pFontCollection;
}

HRESULT DXACreateFontCollection(DXAWriteFactory::DWriteFactory* pWriteFactory, LPCWSTR szPath, DXAFontCollection* pFontCollection, LPWSTR FontFamilyBuffer, UINT* BufElemNum)
{
	return pFontCollection->Initialize(pWriteFactory, szPath, FontFamilyBuffer, BufElemNum);
}

HRESULT DXACreateFontCollection(DXAWriteFactory::DWriteFactory* pWriteFactory, HMODULE hModule, LPCWSTR szResourceName, LPCWSTR szResourceType, DXAFontCollection* pFontCollection, LPWSTR FontFamilyBuffer, UINT* BufElemNum)
{
	return pFontCollection->Initialize(pWriteFactory, hModule, szResourceName, szResourceType, FontFamilyBuffer, BufElemNum);
}

DXATextFormat::DXATextFormat()
{
	pTextFormat = nullptr;
	fFontSize = 0.0f;
	pWriteFactory = nullptr;
}

DXATextFormat::~DXATextFormat()
{
	Release();
}

HRESULT DXATextFormat::Initialize(DXAWriteFactory::DWriteFactory* pWriteFactory, LPCWSTR szFontFamilyName, DXAFontCollection::DWriteFontCollection* pFontCollection, float fFontSize, LPCWSTR szLocaleName)
{
	this->pWriteFactory = pWriteFactory;
	this->fFontSize = fFontSize;
	return pWriteFactory->CreateTextFormat(
		szFontFamilyName ? szFontFamilyName : L"Microsoft YaHei",
		pFontCollection,
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		fFontSize,
		szLocaleName,
		&pTextFormat
	);
}

void DXATextFormat::Release()
{
	if (pTextFormat) {
		pTextFormat->Release();
		pTextFormat = nullptr;
		fFontSize = 0.0f;
	}
}

DXATextFormat::DWriteTextFormat* DXATextFormat::TextFormat()
{
	return pTextFormat;
}

DXAWriteFactory::DWriteFactory* DXATextFormat::WriteFactory()
{
	return pWriteFactory;
}

float DXATextFormat::FontSize()
{
	return fFontSize;
}

DXATextFormat::operator DWriteTextFormat* ()
{
	return pTextFormat;
}

DXATextFormat::operator DXAWriteFactory::DWriteFactory* ()
{
	return pWriteFactory;
}
