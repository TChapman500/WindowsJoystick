#include "resource.h"
#include "ProcDefs.h"
#include "../InputSystem/WinInputSystem.h"

extern HWND MainHWnd;
extern TChapman500::Input::IInputSystem *InputSystem;

extern void InitializeInputSystem();

INT_PTR CALLBACK Unprocessed(HWND hWnd, WPARAM wParam, LPARAM lParam) { return FALSE; }

INT_PTR CALLBACK OnClose(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	DestroyWindow(hWnd);
	return TRUE;
}

INT_PTR CALLBACK OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	//InputSystem = TChapman500::JoystickAPI::CreateInputSystem();
	return TRUE;
}


INT_PTR CALLBACK OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	//TChapman500::JoystickAPI::DestroyInputSystem(InputSystem);
	PostQuitMessage(0);
	return TRUE;
}

extern bool refresh;
INT_PTR CALLBACK OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case IDC_REFRESH:
		refresh = true;
		break;
	}
	return TRUE;
}

INT_PTR CALLBACK OnSelectChanged(HWND hWnd, WPARAM wParam, LPARAM lParam)
{



	return TRUE;
}
