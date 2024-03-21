#pragma once

#ifndef DXAWRITE_H
#define DXAWRITE_H

#include "DXABase.h"
#include <dwrite_3.h>

#ifdef __cplusplus

class DXA_EXPORT DXAWriteFactory
{
public:
	using DWriteFactory = IDWriteFactory8;

	DXAWriteFactory();
	~DXAWriteFactory();
	DXAWriteFactory(const DXAWriteFactory&) = delete;
	DXAWriteFactory& operator=(const DXAWriteFactory&) = delete;
	HRESULT Initialize();
	void Release();
	DWriteFactory* WriteFactory();
	operator DWriteFactory* ();
private:
	DWriteFactory* pFactory;
};

class DXA_EXPORT DXAFontCollection
{
public:
	using DWriteMemoryLoader = IDWriteInMemoryFontFileLoader;
	using DWriteFontCollection = IDWriteFontCollection1;

	friend DXA_EXPORT HRESULT DXACreateFontCollection(DXAWriteFactory::DWriteFactory* pWriteFactory, LPCWSTR szPath, DXAFontCollection* pFontCollection, LPWSTR FontFamilyBuffer, UINT* BufElemNum);
	friend DXA_EXPORT HRESULT DXACreateFontCollection(DXAWriteFactory::DWriteFactory* pWriteFactory, HMODULE hModule, LPCWSTR szResourceName, LPCWSTR szResourceType, DXAFontCollection* pFontCollection, LPWSTR FontFamilyBuffer, UINT* BufElemNum);

	DXAFontCollection();
	~DXAFontCollection();
	DXAFontCollection(const DXAFontCollection&) = delete;
	DXAFontCollection& operator=(const DXAFontCollection&) = delete;
	HRESULT Initialize(DXAWriteFactory::DWriteFactory* pWriteFactory, LPCWSTR szPath, LPWSTR FontFamilyBuffer, UINT* BufElemNum);
	HRESULT Initialize(DXAWriteFactory::DWriteFactory* pWriteFactory, HMODULE hModule, LPCWSTR szResourceName, LPCWSTR szResourceType, LPWSTR FontFamilyBuffer, UINT* BufElemNum);
	void Release();
	DWriteFontCollection* FontCollection();
	DXAWriteFactory::DWriteFactory* WriteFactory();
	operator DWriteFontCollection* ();
	operator DXAWriteFactory::DWriteFactory* ();
private:
	DWriteMemoryLoader* pMemoryLoader;
	DWriteFontCollection* pFontCollection;
	DXAWriteFactory::DWriteFactory* pWriteFactory;
};

class DXA_EXPORT DXATextFormat
{
public:
	using DWriteTextFormat = IDWriteTextFormat;

	DXATextFormat();
	~DXATextFormat();
	DXATextFormat(const DXATextFormat&) = delete;
	DXATextFormat& operator=(const DXATextFormat&) = delete;
	HRESULT Initialize(DXAWriteFactory::DWriteFactory* pWriteFactory, LPCWSTR szFontFamilyName, DXAFontCollection::DWriteFontCollection* pFontCollection, float fFontSize, LPCWSTR szLocaleName);
	void Release();
	DWriteTextFormat* TextFormat();
	DXAWriteFactory::DWriteFactory* WriteFactory();
	float FontSize();
	operator DWriteTextFormat* ();
	operator DXAWriteFactory::DWriteFactory* ();
private:
	DWriteTextFormat* pTextFormat;
	float fFontSize;
	DXAWriteFactory::DWriteFactory* pWriteFactory;
};

DXA_EXPORT HRESULT DXACreateFontCollection(DXAWriteFactory::DWriteFactory* pWriteFactory, LPCWSTR szPath, DXAFontCollection* pFontCollection, LPWSTR FontFamilyBuffer, UINT* BufElemNum);
DXA_EXPORT HRESULT DXACreateFontCollection(DXAWriteFactory::DWriteFactory* pWriteFactory, HMODULE hModule, LPCWSTR szResourceName, LPCWSTR szResourceType, DXAFontCollection* pFontCollection, LPWSTR FontFamilyBuffer, UINT* BufElemNum);

#endif // __cplusplus

#endif // !DXAWRITE_H
