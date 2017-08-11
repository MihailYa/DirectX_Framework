#pragma once
#include "Render.h"

namespace D3D11Framework
{
	//------------------------------------------------------------------

	class Image
	{
	public:
		Image();

		bool Init(Render *render, int screenWidth, int screenHeight, const wchar_t *textureFilename, int bitmapWidth, int bitmapHeight);
		void Render_Image(int positionX, int positionY);
		void Close();

	private:
		bool m_InitBuffers();
		bool m_LoadTexture(const wchar_t *name);
		bool m_InitShader(wchar_t *namevs, wchar_t *nameps);
		void m_UpdateBuffers(int positionX, int positionY);
		void m_RenderBuffers();
		void m_SetShaderParameters();
		void m_RenderShader();

		Render *m_render;

		ID3D11Buffer *m_vertexBuffer;
		ID3D11Buffer *m_indexBuffer;
		ID3D11Buffer *m_constantBuffer;
		ID3D11VertexShader *m_vertexShader;
		ID3D11PixelShader *m_pixelShader;
		ID3D11InputLayout *m_layout;
		ID3D11ShaderResourceView *m_texture;
		ID3D11SamplerState *m_sampleState;

		int m_screenWidth, m_screenHeight;
		int m_bitmapWidth, m_bitmapHeight;
		int m_previousPosX, m_previousPosY;
	};

	//------------------------------------------------------------------
}