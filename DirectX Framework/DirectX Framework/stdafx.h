#pragma once

#include <clocale>
#include <ctime>

#include <string>
#include <list>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "macros.h"

#include <d3d11.h>

#include <d3dcompiler.h>
#include <D3DX11async.h>

#pragma comment(lib, "d3d11.lib")

#ifdef _DEBUG
#	pragma comment(lib, "d3dx11d.lib")
#else
#	pragma comment(lib, "d3dx11.lib")
#endif