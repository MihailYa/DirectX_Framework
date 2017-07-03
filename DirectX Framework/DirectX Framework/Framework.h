#pragma once

#include "Window.h"
#include "Render.h"
#include "InputMgr.h"
#include "Log.h"

namespace D3D11Framework
{
	//------------------------------------------------------------------

	class Framework
	{
	public:
		Framework();
		~Framework();

		bool Init();
		void Run();
		void Close();

		void SetRender(Render *render) { m_render = render; }
		void AddInputListener(InputListener *listener);
	protected:
		bool m_frame();

		Window *m_wnd;
		Render *m_render;
		InputMgr *m_input;
		Log m_log;
		bool m_init;		// если было инициализировано
	};

	//------------------------------------------------------------------
}