#include "stdafx.h"
#include "StaticMesh.h"
#include "ms3dspec.h"
#include <fstream>
#include "macros.h"
#include "Util.h"

using namespace D3D11Framework;
using namespace std;

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT2 Tex;
};

struct ConstantBuffer
{
	XMMATRIX WVP;
};

StaticMesh::StaticMesh()
{
	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;
	m_vertexShader = nullptr;
	m_pixelShader = nullptr;
	m_layout = nullptr;
	m_pConstantBuffer = nullptr;
	m_sampleState = nullptr;
	m_texture = nullptr;
	Identity();
}

bool StaticMesh::Init(Render *render, wchar_t *name)
{
	m_objMatrix = XMMatrixIdentity();
	m_render = render;
	if (!m_loadMS3DFile(name))
		return false;
	if (!m_InitShader(L"mesh.vs", L"mesh.ps"))
		return false;

	return true;
}

bool StaticMesh::m_loadMS3DFile(wchar_t *Filename)
{
	unsigned short VertexCount = 0;
	unsigned short TriangleCount = 0;
	unsigned short GroupCount = 0;
	unsigned short MaterialCount = 0;
	MS3DVertex *pMS3DVertices = nullptr;
	MS3DTriangle *pMS3DTriangles = nullptr;
	MS3DGroup *pMS3DGroups = nullptr;
	MS3DMaterial *pMS3DMaterials = nullptr;

	ifstream fin;
	MS3DHeader header;

	fin.open(Filename, std::ios::binary);
	fin.read((char*)(&(header)), sizeof(header));
	if (header.version != 3 && header.version != 4)
		return false;

	fin.read((char*)(&VertexCount), sizeof(unsigned short));
	pMS3DVertices = new MS3DVertex[VertexCount];
	fin.read((char*)pMS3DVertices, VertexCount * sizeof(MS3DVertex));

	fin.read((char*)(&TriangleCount), sizeof(unsigned short));
	pMS3DTriangles = new MS3DTriangle[TriangleCount];
	fin.read((char*)pMS3DTriangles, TriangleCount * sizeof(MS3DTriangle));

	fin.read((char*)(&GroupCount), sizeof(unsigned short));
	pMS3DGroups = new MS3DGroup[GroupCount];
	for (int i = 0; i < GroupCount; i++)
	{
		fin.read((char*)&(pMS3DGroups[i].flags), sizeof(unsigned char));
		fin.read((char*)&(pMS3DGroups[i].name), sizeof(char[32]));
		fin.read((char*)&(pMS3DGroups[i].numtriangles), sizeof(unsigned short));
		unsigned short triCount = pMS3DGroups[i].numtriangles;
		pMS3DGroups[i].triangleIndices = new unsigned short[triCount];
		fin.read((char*)(pMS3DGroups[i].triangleIndices), sizeof(unsigned short) * triCount);
		fin.read((char*)&(pMS3DGroups[i].materialIndex), sizeof(char));
	}

	fin.read((char*)(&MaterialCount), sizeof(unsigned short));
	pMS3DMaterials = new MS3DMaterial[MaterialCount];
	fin.read((char*)pMS3DMaterials, MaterialCount * sizeof(MS3DMaterial));

	fin.close();

	m_indexCount = TriangleCount * 3;
	WORD *indices = new WORD[m_indexCount];
	if (!indices)
		return false;
	Vertex *vertices = new Vertex[VertexCount];
	if (!vertices)
		return false;

	for (int i = 0; i < TriangleCount; i++)
	{
		indices[3 * i + 0] = pMS3DTriangles[i].vertexIndices[0];
		indices[3 * i + 1] = pMS3DTriangles[i].vertexIndices[1];
		indices[3 * i + 2] = pMS3DTriangles[i].vertexIndices[2];
	}

	for (int i = 0; i < VertexCount; i++)
	{
		vertices[i].Pos.x = pMS3DVertices[i].vertex[0];
		vertices[i].Pos.y = pMS3DVertices[i].vertex[1];
		vertices[i].Pos.z = pMS3DVertices[i].vertex[2];

		for (int j = 0; j < TriangleCount; j++)
		{
			if (i == pMS3DTriangles[j].vertexIndices[0])
			{
				vertices[i].Tex.x = pMS3DTriangles[j].s[0];
				vertices[i].Tex.y = pMS3DTriangles[j].t[0];
			}
			else if (i == pMS3DTriangles[j].vertexIndices[1])
			{
				vertices[i].Tex.x = pMS3DTriangles[j].s[1];
				vertices[i].Tex.y = pMS3DTriangles[j].t[1];
			}
			else if (i == pMS3DTriangles[j].vertexIndices[2])
			{
				vertices[i].Tex.x = pMS3DTriangles[j].s[2];
				vertices[i].Tex.y = pMS3DTriangles[j].t[2];
			}
			else
				continue;
			break;
		}
	}

	if (!m_LoadTextures(CharToWChar(pMS3DMaterials[0].texture)))
		return false;

	_DELETE_ARRAY(pMS3DMaterials);
	if (pMS3DGroups != nullptr)
	{
		for (int i = 0; i < GroupCount; i++)
			_DELETE_ARRAY(pMS3DGroups[i].triangleIndices);
		_DELETE_ARRAY(pMS3DGroups);
	}
	_DELETE_ARRAY(pMS3DTriangles);
	_DELETE_ARRAY(pMS3DVertices);

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex) * VertexCount;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA Data;
	ZeroMemory(&Data, sizeof(Data));
	Data.pSysMem = vertices;
	HRESULT hr = m_render->m_pd3dDevice->CreateBuffer(&bd, &Data, &m_vertexBuffer);
	if (FAILED(hr))
		return false;

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(WORD) * m_indexCount;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	Data.pSysMem = indices;
	hr = m_render->m_pd3dDevice->CreateBuffer(&bd, &Data, &m_indexBuffer);
	if (FAILED(hr))
		return false;

	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = m_render->m_pd3dDevice->CreateBuffer(&bd, NULL, &m_pConstantBuffer);
	if (FAILED(hr))
		return false;

	_DELETE_ARRAY(vertices);
	_DELETE_ARRAY(indices);

	return true;
}

bool StaticMesh::m_LoadTextures(wchar_t *textureFilename)
{
	HRESULT hr = D3DX11CreateShaderResourceViewFromFile(m_render->m_pd3dDevice, textureFilename, NULL, NULL, &m_texture, NULL);
	if (FAILED(hr))
		return false;

	return true;
}

bool StaticMesh::m_InitShader(wchar_t* vsFilename, wchar_t* psFilename)
{
	ID3D10Blob *vertexShaderBuffer = nullptr;
	HRESULT hr = m_render->m_compileshaderfromfile(vsFilename, "VS", "vs_4_0", &vertexShaderBuffer);
	if (FAILED(hr))
		return false;

	ID3D10Blob *pixelShaderBuffer = nullptr;
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

void StaticMesh::Draw(CXMMATRIX viewmatrix)
{
	m_RenderBuffers();
	m_SetShaderParameters(viewmatrix);
	m_RenderShader();
}

void StaticMesh::m_RenderBuffers()
{
	unsigned int stride = sizeof(Vertex);
	unsigned int offset = 0;
	m_render->m_pImmediateContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	m_render->m_pImmediateContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	m_render->m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void StaticMesh::m_SetShaderParameters(CXMMATRIX viewmatrix)
{
	XMMATRIX WVP = m_objMatrix * viewmatrix * m_render->m_Projection;
	ConstantBuffer cb;
	cb.WVP = XMMatrixTranspose(WVP);
	m_render->m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, NULL, &cb, 0, 0);

	m_render->m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

	m_render->m_pImmediateContext->PSSetShaderResources(0, 1, &m_texture);
}

void StaticMesh::m_RenderShader()
{
	m_render->m_pImmediateContext->IASetInputLayout(m_layout);
	m_render->m_pImmediateContext->VSSetShader(m_vertexShader, NULL, 0);
	m_render->m_pImmediateContext->PSSetShader(m_pixelShader, NULL, 0);
	m_render->m_pImmediateContext->PSSetSamplers(0, 1, &m_sampleState);
	m_render->m_pImmediateContext->DrawIndexed(m_indexCount, 0, 0);
}

void StaticMesh::Close()
{
	_RELEASE(m_texture);
	_RELEASE(m_indexBuffer);
	_RELEASE(m_vertexBuffer);
	_RELEASE(m_pConstantBuffer);
	_RELEASE(m_sampleState);
	_RELEASE(m_layout);
	_RELEASE(m_pixelShader);
	_RELEASE(m_vertexShader);
}

void StaticMesh::Translate(float x, float y, float z)
{
	m_objMatrix *= XMMatrixTranslation(x, y, z);
}
void StaticMesh::Rotate(float angle, float x, float y, float z)
{
	XMVECTOR v = XMVectorSet(x, y, z, 0.0f);
	m_objMatrix *= XMMatrixRotationAxis(v, angle);
}
void StaticMesh::Scale(float x, float y, float z)
{
	m_objMatrix *= XMMatrixScaling(x, y, z);
}

void StaticMesh::Identity()
{
	m_objMatrix = XMMatrixIdentity();
}
