#include "stdafx.h"

#include "Image.h"

#include "macros.h"

namespace D3D11Framework
{
	//------------------------------------------------------------------

	struct Vertex
	{
		XMFLOAT3 pos;
		XMFLOAT2 tex;
	};

	struct ConstantBuffer
	{
		XMMATRIX Ortho;
	};

	Image::Image()
	{
		m_vertexBuffer = nullptr;
		m_indexBuffer = nullptr;
		m_texture = nullptr;
		m_vertexShader = nullptr;
		m_pixelShader = nullptr;
		m_layout = nullptr;
		m_constantBuffer = nullptr;
		m_sampleState = nullptr;
	}

	bool Image::Init(Render *render, int screenWidth, int screenHeight, const wchar_t *textureFilename, int bitmapWidth, int bitmapHeight)
	{
		m_screenWidth = screenWidth;
		m_screenHeight = screenHeight;
		m_bitmapWidth = bitmapWidth;
		m_bitmapHeight = bitmapHeight;
		m_previousPosX = -1;
		m_previousPosY = -1;
		m_render = render;

		if (!m_InitBuffers())
			return false;

		if (!m_LoadTexture(textureFilename))
			return false;

		if (!m_InitShader(L"image.vs", L"image.ps"))
			return false;

		return true;
	}

	void Image::Render_Image(int positionX, int positionY)
	{
		m_UpdateBuffers(positionX, positionY);
		m_RenderBuffers();
		m_SetShaderParameters();
		m_RenderShader();
	}

	void Image::Close()
	{
		_RELEASE(m_vertexBuffer);
		_RELEASE(m_indexBuffer);
		_RELEASE(m_constantBuffer);
		_RELEASE(m_vertexShader);
		_RELEASE(m_pixelShader);
		_RELEASE(m_layout);
		_RELEASE(m_texture);
		_RELEASE(m_sampleState);
	}

	bool Image::m_InitBuffers()
	{
		Vertex *vertices = new Vertex[4];
		if (!vertices)
			return false;

		unsigned long indices[] = 
		{
			0, 1, 2,
			0, 2, 3
		};

		memset(vertices, 0, (sizeof(Vertex) * 4));

		HRESULT result;
		D3D11_BUFFER_DESC BufferDesc;
		D3D11_SUBRESOURCE_DATA Data;

		BufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		BufferDesc.ByteWidth = sizeof(Vertex) * 4;
		BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		BufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		BufferDesc.MiscFlags = 0;
		BufferDesc.StructureByteStride = 0;

		Data.pSysMem = vertices;
		Data.SysMemPitch = 0;
		Data.SysMemSlicePitch = 0;

		result = m_render->m_pd3dDevice->CreateBuffer(&BufferDesc, &Data, &m_vertexBuffer);
		if (FAILED(result))
			return false;

		BufferDesc.Usage = D3D11_USAGE_DEFAULT;
		BufferDesc.ByteWidth = sizeof(unsigned long) * 6;
		BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		BufferDesc.CPUAccessFlags = 0;
		BufferDesc.MiscFlags = 0;
		BufferDesc.StructureByteStride = 0;

		Data.pSysMem = &indices;
		Data.SysMemPitch = 0;
		Data.SysMemSlicePitch = 0;

		result = m_render->m_pd3dDevice->CreateBuffer(&BufferDesc, &Data, &m_indexBuffer);
		if (FAILED(result))
			return false;

		BufferDesc.Usage = D3D11_USAGE_DEFAULT;
		BufferDesc.ByteWidth = sizeof(ConstantBuffer);
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		BufferDesc.CPUAccessFlags = 0;
		result = m_render->m_pd3dDevice->CreateBuffer(&BufferDesc, NULL, &m_constantBuffer);
		if (FAILED(result))
			return false;

		_DELETE_ARRAY(vertices);

		return true;
	}

	bool Image::m_LoadTexture(const wchar_t *textureFilename)
	{
		HRESULT hr = D3DX11CreateShaderResourceViewFromFile(m_render->m_pd3dDevice, textureFilename, NULL, NULL, &m_texture, NULL);
		if (FAILED(hr))
			return false;

		return true;
	}

	bool Image::m_InitShader(wchar_t* vsFilename, wchar_t* psFilename)
	{
		ID3DBlob *vertexShaderBuffer = nullptr;
		HRESULT hr = m_render->m_compileshaderfromfile(vsFilename, "VS", "vs_4_0", &vertexShaderBuffer);
		if (FAILED(hr))
			return false;

		ID3DBlob *pixelShaderBuffer = nullptr;
		HRESULT result = m_render->m_compileshaderfromfile(psFilename, "PS", "ps_4_0", &pixelShaderBuffer);
		if (FAILED(hr))
			return false;

		result = m_render->m_pd3dDevice->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
		if (FAILED(result))
			return false;

		result = m_render->m_pd3dDevice->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
		if (FAILED(result))
			return false;

		D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
		polygonLayout[0].SemanticName = "POSITION";
		polygonLayout[0].SemanticIndex = 0;
		polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		polygonLayout[0].InputSlot = 0;
		polygonLayout[0].AlignedByteOffset = 0;
		polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[0].InstanceDataStepRate = 0;
		polygonLayout[1].SemanticName = "TEXCOORD";
		polygonLayout[1].SemanticIndex = 0;
		polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
		polygonLayout[1].InputSlot = 0;
		polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[1].InstanceDataStepRate = 0;

		unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

		result = m_render->m_pd3dDevice->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
		if (FAILED(result))
			return false;

		_RELEASE(vertexShaderBuffer);
		_RELEASE(pixelShaderBuffer);

		D3D11_SAMPLER_DESC samplerDesc;
		samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[0] = 0;
		samplerDesc.BorderColor[1] = 0;
		samplerDesc.BorderColor[2] = 0;
		samplerDesc.BorderColor[3] = 0;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		result = m_render->m_pd3dDevice->CreateSamplerState(&samplerDesc, &m_sampleState);
		if (FAILED(result))
			return false;

		return true;
	}

	void Image::m_UpdateBuffers(int positionX, int positionY)
	{
		if ((positionX == m_previousPosX) && (positionY == m_previousPosY))
			return;

		m_previousPosX = positionX;
		m_previousPosY = positionY;

		float left = (float)((m_screenWidth / 2) * -1) + (float)positionX;
		float right = left + (float)m_bitmapWidth;
		float top = (float)(m_screenHeight / 2) - (float)positionY;
		float bottom = top - (float)m_bitmapHeight;

		D3D11_MAPPED_SUBRESOURCE mappedResource;
		HRESULT result = m_render->m_pImmediateContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
			return;

		Vertex *verticesPtr = (Vertex*)mappedResource.pData;

		verticesPtr[0].pos = XMFLOAT3(left, bottom, 0.0f);
		verticesPtr[0].tex = XMFLOAT2(0.0f, 1.0f);

		verticesPtr[1].pos = XMFLOAT3(left, top, 0.0f);
		verticesPtr[1].tex = XMFLOAT2(0.0f, 0.0f);

		verticesPtr[2].pos = XMFLOAT3(right, top, 0.0f);
		verticesPtr[2].tex = XMFLOAT2(1.0f, 0.0f);

		verticesPtr[3].pos = XMFLOAT3(right, bottom, 0.0f);
		verticesPtr[3].tex = XMFLOAT2(1.0f, 1.0f);
	
		m_render->m_pImmediateContext->Unmap(m_vertexBuffer, 0);
	}

	void Image::m_RenderBuffers()
	{
		unsigned int stride = sizeof(Vertex);
		unsigned int offset = 0;
		m_render->m_pImmediateContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
		m_render->m_pImmediateContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
		m_render->m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void Image::m_SetShaderParameters()
	{
		ConstantBuffer cb;

		// !!!!!!!!!!!!!!!!!
		// Changed next line:
		cb.Ortho = XMMatrixTranspose(XMMatrixOrthographicLH(m_screenWidth, m_screenHeight, 0.0f, 1000.0f)/*m_render->m_Ortho*/);
		
		m_render->m_pImmediateContext->UpdateSubresource(m_constantBuffer, 0, NULL, &cb, 0, 0);

		m_render->m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);

		m_render->m_pImmediateContext->PSSetShaderResources(0, 1, &m_texture);
	}

	void Image::m_RenderShader()
	{
		m_render->m_pImmediateContext->IASetInputLayout(m_layout);
		m_render->m_pImmediateContext->VSSetShader(m_vertexShader, NULL, 0);
		m_render->m_pImmediateContext->PSSetShader(m_pixelShader, NULL, 0);
		m_render->m_pImmediateContext->PSSetSamplers(0, 1, &m_sampleState);
		m_render->m_pImmediateContext->DrawIndexed(6, 0, 0);
	}

	//------------------------------------------------------------------
}
