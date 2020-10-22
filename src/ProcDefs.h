#pragma once
#include "resource.h"
#include <Windows.h>

enum class inputMode
{
	Invalid,
	WindowProc,
	InputLoop,
};
extern inputMode InputMode;

typedef INT_PTR(CALLBACK *PROCFUNC)(HWND, WPARAM, LPARAM);

INT_PTR CALLBACK Unprocessed(HWND, WPARAM, LPARAM);

INT_PTR CALLBACK OnClose(HWND, WPARAM, LPARAM);
INT_PTR CALLBACK OnCreate(HWND, WPARAM, LPARAM);
INT_PTR CALLBACK OnDestroy(HWND, WPARAM, LPARAM);


INT_PTR CALLBACK OnInput(HWND, WPARAM, LPARAM);
INT_PTR CALLBACK OnInputDeviceChanged(HWND, WPARAM, LPARAM);
INT_PTR CALLBACK OnCommand(HWND, WPARAM, LPARAM);
INT_PTR CALLBACK OnSelectChanged(HWND, WPARAM, LPARAM);
