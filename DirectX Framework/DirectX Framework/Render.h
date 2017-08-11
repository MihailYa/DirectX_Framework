#pragma once
#include "stdafx.h"

namespace D3D11Framework
{
	//------------------------------------------------------------------

	class Render
	{
		friend class StaticMesh;
		friend class Image;

	public:
		Render();
		virtual ~Render();

		bool CreateDevice(HWND hwnd);
		void BeginFrame();
		void EndFrame();
		void Shutdown();

		virtual bool Init(HWND hwnd) = 0;
		virtual bool Draw() = 0;
		virtual void Close() = 0;

		void TurnZBufferOn();
		void TurnZBufferOff();

		void* operator new(size_t i)
		{
			return _aligned_malloc(i, 16);
		}

		void operator delete(void* p)
		{
			_aligned_free(p);
		}
	protected:
		HRESULT m_compileshaderfromfile(WCHAR* FileName, LPCSTR EntryPoint, LPCSTR ShaderModel, ID3DBlob** ppBlobOut);

		D3D_DRIVER_TYPE m_driverType;
		D3D_FEATURE_LEVEL m_featureLevel;
		ID3D11Device *m_pd3dDevice;
		ID3D11DeviceContext *m_pImmediateContext;
		IDXGISwapChain *m_pSwapChain;
		ID3D11RenderTargetView *m_pRenderTargetView;

		ID3D11Texture2D *m_pDepthStencil;
		ID3D11DepthStencilView *m_pDepthStencilView;
		ID3D11DepthStencilState *m_pDepthStencilState;
		ID3D11DepthStencilState *m_pDepthDisabledStencilState;

		XMMATRIX m_Projection;

	};

	//------------------------------------------------------------------
}