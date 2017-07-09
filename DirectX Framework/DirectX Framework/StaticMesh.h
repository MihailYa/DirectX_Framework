#pragma once

#include "Render.h"

namespace D3D11Framework
{
	//------------------------------------------------------------------

	class StaticMesh
	{
	public:
		StaticMesh();

		bool Init(Render *render, wchar_t *name);
		void Draw(CXMMATRIX viewmatrix);
		void Close();

		void Translate(float x, float y, float z);
		void Rotate(float angle, float x, float y, float z);
		void Scale(float x, float y, float z);
		void Identity();

		void* operator new(size_t i)
		{
			return _aligned_malloc(i, 16);
		}

		void operator delete(void* p)
		{
			_aligned_free(p);
		}

	private:
		bool m_loadMS3DFile(wchar_t* name);
		bool m_LoadTextures(wchar_t* name);
		bool m_InitShader(wchar_t* namevs, wchar_t* nameps);

		void m_RenderBuffers();
		void m_SetShaderParameters(CXMMATRIX viewmatrix);
		void m_RenderShader();

		Render *m_render;

		ID3D11Buffer *m_vertexBuffer;
		ID3D11Buffer *m_indexBuffer;
		ID3D11VertexShader *m_vertexShader;
		ID3D11PixelShader *m_pixelShader;
		ID3D11InputLayout *m_layout;
		ID3D11Buffer *m_pConstantBuffer;
		ID3D11SamplerState* m_sampleState;
		ID3D11ShaderResourceView *m_texture;

		XMMATRIX m_objMatrix;
		unsigned short m_indexCount;
	};

	//------------------------------------------------------------------
}
