#include "stdafx.h"
#include "Render.h"

namespace D3D11Framework
{
	//------------------------------------------------------------------


	Render::Render()
	{
		m_driverType = D3D_DRIVER_TYPE_NULL;
		m_featureLevel = D3D_FEATURE_LEVEL_11_0;
		m_pd3dDevice = nullptr;
		m_pImmediateContext = nullptr;
		m_pSwapChain = nullptr;
		m_pRenderTargetView = nullptr;
		m_pDepthStencil = nullptr;
		m_pDepthStencilView = nullptr;
	}

	Render::~Render()
	{
	}

	bool Render::CreateDevice(HWND hwnd)
	{
		//Успешность выполнения команд
		HRESULT hr = S_OK;

		//Описание дисплея
		RECT rc;
		GetClientRect(hwnd, &rc);
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
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};
		UINT numFeatureLevels = ARRAYSIZE(featureLevels);

		//Заполняем Swap Chain
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));	// очищаем структуру
		sd.BufferCount = 1;				// у нас один задний буфер
		sd.BufferDesc.Width = width;	// устанавливаем ширину буфера
		sd.BufferDesc.Height = height;	// устанавливаем высоту
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // формат пикселя
		sd.BufferDesc.RefreshRate.Numerator = 60; // частота обновления экрана
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // назначение буфера
		sd.OutputWindow = hwnd;			// десктриптор окна
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;			// устанавливаем оконный режим

									//Создание ID3D11Device и IDXGISwapChain
		for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
		{
			m_driverType = driverTypes[driverTypeIndex];
			hr = D3D11CreateDeviceAndSwapChain(NULL, m_driverType,
				NULL, createDeviceFlags,
				featureLevels, numFeatureLevels,
				D3D11_SDK_VERSION, &sd,
				&m_pSwapChain, &m_pd3dDevice,
				&m_featureLevel, &m_pImmediateContext);
			if (SUCCEEDED(hr))
				break;
		}
		if (FAILED(hr))
			return false;

		//Render target
		ID3D11Texture2D* pBackBuffer = NULL;
		hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
		if (FAILED(hr))
			return false;

		hr = m_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRenderTargetView);
		_RELEASE(pBackBuffer);
		if (FAILED(hr))
			return false;

		D3D11_TEXTURE2D_DESC descDepth;
		ZeroMemory(&descDepth, sizeof(descDepth));
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
		hr = m_pd3dDevice->CreateTexture2D(&descDepth, NULL, &m_pDepthStencil);
		if (FAILED(hr))
			return false;

		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory(&descDSV, sizeof(descDSV));
		descDSV.Format = descDepth.Format;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;
		hr = m_pd3dDevice->CreateDepthStencilView(m_pDepthStencil, &descDSV, &m_pDepthStencilView);
		if (FAILED(hr))
			return false;

		//Присоединение render target к конвееру рендеринга (rendering pipeline)
		m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		//Viewport
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)width;
		vp.Height = (FLOAT)height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		m_pImmediateContext->RSSetViewports(1, &vp);
	}

	void Render::BeginFrame() {
		//Очистка окна
		float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; //красный,зеленый,синий,альфа
		m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, ClearColor);
		m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	void Render::EndFrame()
	{
		//Swap Buffers
		m_pSwapChain->Present(0, 0);
	}

	void Render::Shutdown()
	{
		Close();

		if (m_pImmediateContext)
			m_pImmediateContext->ClearState();

		_RELEASE(m_pDepthStencil);
		_RELEASE(m_pDepthStencilView);
		_RELEASE(m_pRenderTargetView);
		_RELEASE(m_pSwapChain);
		_RELEASE(m_pImmediateContext);
		_RELEASE(m_pd3dDevice);
	}

	HRESULT Render::m_compileshaderfromfile(WCHAR* FileName, LPCSTR EntryPoint, LPCSTR ShaderModel, ID3DBlob** ppBlobOut)
	{
		HRESULT hr = S_OK;

		DWORD ShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
		ShaderFlags |= D3DCOMPILE_DEBUG;
#endif

		ID3DBlob* pErrorBlob;
		hr = D3DX11CompileFromFile(FileName, NULL, NULL, EntryPoint, ShaderModel, ShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
		if (FAILED(hr) && pErrorBlob != NULL)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());

		_RELEASE(pErrorBlob);
		return hr;
	}

	//------------------------------------------------------------------
}