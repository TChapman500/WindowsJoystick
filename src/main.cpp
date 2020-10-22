#include "ProcDefs.h"
#include "resource.h"
#include "InputSystem.h"
#include <commctrl.h>
#include <Windows.h>

#include "Joystick.h"
#include "InputAxis.h"
#include "InputButton.h"
#include "InputHAT.h"

// Hmm.  I wonder what this could be for...

static PROCFUNC WndProc[65536];
HWND MainHWnd;

// lol!
INT_PTR CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return WndProc[message](hWnd, wParam, lParam);
}

inputMode InputMode = inputMode::WindowProc;
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{

	// Initialize proc list.
	for (int i = 0; i < 65536; i++) WndProc[i] = Unprocessed;	// Default message handler.
	WndProc[WM_INPUT] = OnInput;					// For input loop.
	WndProc[WM_INPUT_DEVICE_CHANGE] = OnInputDeviceChanged;		// Input device changed.
	WndProc[WM_COMMAND] = OnCommand;				// Button pressed.
	WndProc[LBN_SELCHANGE] = OnSelectChanged;			// Device select changed.
	WndProc[WM_CLOSE] = OnClose;
	WndProc[WM_INITDIALOG] = OnCreate;
	WndProc[WM_DESTROY] = OnDestroy;

	// Initialize Window
	MainHWnd = CreateDialogParamW(hInstance, MAKEINTRESOURCEW(IDD_JOYOUT), nullptr, WindowProc, 0);
	CheckRadioButton(MainHWnd, IDC_MESSAGE_LOOP, IDC_INPUT_LOOP, IDC_MESSAGE_LOOP);
	ShowWindow(MainHWnd, nCmdShow);

	// Initialize Button List Columns
	HWND btnList = GetDlgItem(MainHWnd, IDC_BUTTON_LIST);
	LVCOLUMNW column;
	ZeroMemory(&column, sizeof(LVCOLUMNW));
	column.mask = LVCF_TEXT | LVCF_WIDTH;
	column.pszText = (LPWSTR)L"Button";
	column.cx = 100;
	SendMessageW(btnList, LVM_INSERTCOLUMN, 0, (LPARAM)&column);
	column.pszText = (LPWSTR)L"Pressed";
	column.cx = 60;
	SendMessageW(btnList, LVM_INSERTCOLUMN, 1, (LPARAM)&column);

	// Initialize Axis List Columns.
	HWND axisList = GetDlgItem(MainHWnd, IDC_AXIS_LIST);
	column.pszText = (LPWSTR)L"Axis";
	column.cx = 70;
	SendMessageW(axisList, LVM_INSERTCOLUMN, 1, (LPARAM)&column);
	column.pszText = (LPWSTR)L"Min";
	column.cx = 50;
	SendMessageW(axisList, LVM_INSERTCOLUMN, 2, (LPARAM)&column);
	column.pszText = (LPWSTR)L"Max";
	SendMessageW(axisList, LVM_INSERTCOLUMN, 3, (LPARAM)&column);
	column.pszText = (LPWSTR)L"Raw";
	SendMessageW(axisList, LVM_INSERTCOLUMN, 4, (LPARAM)&column);
	column.pszText = (LPWSTR)L"Center";
	column.cx = 60;
	SendMessageW(axisList, LVM_INSERTCOLUMN, 5, (LPARAM)&column);
	column.pszText = (LPWSTR)L"End";
	SendMessageW(axisList, LVM_INSERTCOLUMN, 6, (LPARAM)&column);
	column.pszText = (LPWSTR)L"Usage";
	SendMessageW(axisList, LVM_INSERTCOLUMN, 7, (LPARAM)&column);

	// Initialize HAT Switch List Columns
	HWND hatList = GetDlgItem(MainHWnd, IDC_HAT_LIST);
	column.pszText = (LPWSTR)L"HAT Switch";
	column.cx = 100;
	SendMessageW(hatList, LVM_INSERTCOLUMN, 0, (LPARAM)&column);
	column.pszText = (LPWSTR)L"Direction";
	column.cx = 100;
	SendMessageW(hatList, LVM_INSERTCOLUMN, 1, (LPARAM)&column);

	// Main Loop
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	int PrevSel = SendMessageW(GetDlgItem(MainHWnd, IDC_JOYSTICK_LIST), LB_GETCURSEL, 0, 0);

	HWND hatUp = GetDlgItem(MainHWnd, IDC_HAT_UP);
	HWND hatDown = GetDlgItem(MainHWnd, IDC_HAT_DOWN);
	HWND hatLeft = GetDlgItem(MainHWnd, IDC_HAT_LEFT);
	HWND hatRight = GetDlgItem(MainHWnd, IDC_HAT_RIGHT);

	UINT btnColumns[2] = { 0, 1 };
	UINT axisColumns[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };

	// Main Loop
	while (true)
	{

		// Process Input
		if (InputMode == inputMode::InputLoop) TChapman500::JoystickAPI::InputSystem::System->InputLoop();
		TChapman500::JoystickAPI::InputSystem::System->ProcessXInputDevices();
		while (PeekMessageW(&msg, nullptr, NULL, NULL, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) goto CLEAN_UP;
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}


		// Display Results
		int CurrSel = SendMessageW(GetDlgItem(MainHWnd, IDC_JOYSTICK_LIST), LB_GETCURSEL, 0, 0);
		
		// Reset list if needed.
		if (CurrSel != PrevSel)
		{
			PrevSel = CurrSel;

			SendMessageW(btnList, LVM_DELETEALLITEMS, 0, 0);
			SendMessageW(axisList, LVM_DELETEALLITEMS, 0, 0);
			SendMessageW(hatList, LVM_DELETEALLITEMS, 0, 0);

			LVITEMW item;
			ZeroMemory(&item, sizeof(LVITEMW));
			item.mask = LVIF_TEXT;
			item.puColumns = btnColumns;

			if (CurrSel >= 0 && CurrSel < TChapman500::JoystickAPI::InputSystem::System->JoystickList.size())
			{
				// Get Button List.
				for (int i = 0; i < TChapman500::JoystickAPI::InputSystem::System->JoystickList[CurrSel]->ButtonList.size(); i++)
				{
					item.pszText = (LPWSTR)TChapman500::JoystickAPI::InputSystem::System->JoystickList[CurrSel]->ButtonList[i]->ControlName.data();
					item.iItem = i;

					SendMessageW(btnList, LVM_INSERTITEM, 0, (LPARAM)&item);
				}

				// Get Axis List.
				for (int i = 0; i < TChapman500::JoystickAPI::InputSystem::System->JoystickList[CurrSel]->AxisList.size(); i++)
				{
					item.pszText = (LPWSTR)TChapman500::JoystickAPI::InputSystem::System->JoystickList[CurrSel]->AxisList[i]->ControlName.data();
					item.iItem = i;

					SendMessageW(axisList, LVM_INSERTITEM, 0, (LPARAM)&item);
				}

				// Get HAT List.
				for (int i = 0; i < TChapman500::JoystickAPI::InputSystem::System->JoystickList[CurrSel]->HATList.size(); i++)
				{
					item.pszText = (LPWSTR)TChapman500::JoystickAPI::InputSystem::System->JoystickList[CurrSel]->HATList[i]->ControlName.data();
					item.iItem = i;
					SendMessageW(hatList, LVM_INSERTITEM, 0, (LPARAM)&item);
				}
			}
		}

		// Set list data

		LVITEMW item;
		ZeroMemory(&item, sizeof(LVITEMW));
		item.mask = LVIF_TEXT;
		item.iSubItem = 1;
		item.puColumns = btnColumns;
		item.cchTextMax = 6;

		wchar_t btnBuffer[6];
		wchar_t axisBuffer[32];
		wchar_t hatBuffer[32];
		wchar_t axisString[32];
		wchar_t hatString[32];

		if (CurrSel >= 0 && CurrSel < TChapman500::JoystickAPI::InputSystem::System->JoystickList.size())
		{
			// Print Button States.
			for (int i = 0; i < TChapman500::JoystickAPI::InputSystem::System->JoystickList[CurrSel]->ButtonList.size(); i++)
			{
				item.pszText = btnBuffer;
				SendMessageW(btnList, LVM_GETITEMTEXT, i, (LPARAM)&item);

				if (wcscmp((wchar_t *)L"", btnBuffer) == 0)
				{
					if (TChapman500::JoystickAPI::InputSystem::System->JoystickList[CurrSel]->ButtonList[i]->Pressed) item.pszText = (wchar_t *)L"true";
					else item.pszText = (wchar_t *)L"false";
					SendMessageW(btnList, LVM_SETITEMTEXT, i, (LPARAM)&item);
				}
				else
				{
					if (TChapman500::JoystickAPI::InputSystem::System->JoystickList[CurrSel]->ButtonList[i]->Pressed && wcscmp((wchar_t *)L"false", btnBuffer) == 0)
					{
						item.pszText = (wchar_t *)L"true";
						SendMessageW(btnList, LVM_SETITEMTEXT, i, (LPARAM)&item);
					}
					else if (!TChapman500::JoystickAPI::InputSystem::System->JoystickList[CurrSel]->ButtonList[i]->Pressed && wcscmp((wchar_t *)L"true", btnBuffer) == 0)
					{
						item.pszText = (wchar_t *)L"false";
						SendMessageW(btnList, LVM_SETITEMTEXT, i, (LPARAM)&item);
					}
				}
			}
			item.cchTextMax = 32;

			// Print Axis States.
			for (int i = 0; i < TChapman500::JoystickAPI::InputSystem::System->JoystickList[CurrSel]->AxisList.size(); i++)
			{
				item.iSubItem = 1;
				item.pszText = axisBuffer;
				SendMessageW(axisList, LVM_GETITEMTEXT, i, (LPARAM)&item);
				swprintf_s(axisString, 32, L"%d", TChapman500::JoystickAPI::InputSystem::System->JoystickList[CurrSel]->AxisList[i]->MinValue);
				if (wcscmp(axisString, axisBuffer) != 0)
				{
					item.pszText = axisString;
					SendMessageW(axisList, LVM_SETITEMTEXT, i, (LPARAM)&item);
				}

				item.iSubItem = 2;
				item.pszText = axisBuffer;
				SendMessageW(axisList, LVM_GETITEMTEXT, i, (LPARAM)&item);
				swprintf_s(axisString, 32, L"%d", TChapman500::JoystickAPI::InputSystem::System->JoystickList[CurrSel]->AxisList[i]->MaxValue);
				if (wcscmp(axisString, axisBuffer) != 0)
				{
					item.pszText = axisString;
					SendMessageW(axisList, LVM_SETITEMTEXT, i, (LPARAM)&item);
				}

				item.iSubItem = 3;
				item.pszText = axisBuffer;
				SendMessageW(axisList, LVM_GETITEMTEXT, i, (LPARAM)&item);
				swprintf_s(axisString, 32, L"%d", TChapman500::JoystickAPI::InputSystem::System->JoystickList[CurrSel]->AxisList[i]->RawValue);
				if (wcscmp(axisString, axisBuffer) != 0)
				{
					item.pszText = axisString;
					SendMessageW(axisList, LVM_SETITEMTEXT, i, (LPARAM)&item);
				}

				item.iSubItem = 4;
				item.pszText = axisBuffer;
				SendMessageW(axisList, LVM_GETITEMTEXT, i, (LPARAM)&item);
				swprintf_s(axisString, 32, L"%.4f", TChapman500::JoystickAPI::InputSystem::System->JoystickList[CurrSel]->AxisList[i]->CenterRelative);
				if (wcscmp(axisString, axisBuffer) != 0)
				{
					item.pszText = axisString;
					SendMessageW(axisList, LVM_SETITEMTEXT, i, (LPARAM)&item);
				}

				item.iSubItem = 5;
				item.pszText = axisBuffer;
				SendMessageW(axisList, LVM_GETITEMTEXT, i, (LPARAM)&item);
				swprintf_s(axisString, 32, L"%.4f", TChapman500::JoystickAPI::InputSystem::System->JoystickList[CurrSel]->AxisList[i]->EndRelative);
				if (wcscmp(axisString, axisBuffer) != 0)
				{
					item.pszText = axisString;
					SendMessageW(axisList, LVM_SETITEMTEXT, i, (LPARAM)&item);
				}

				item.iSubItem = 6;
				item.pszText = axisBuffer;
				SendMessageW(axisList, LVM_GETITEMTEXT, i, (LPARAM)&item);
				swprintf_s(axisString, 32, L"%d", TChapman500::JoystickAPI::InputSystem::System->JoystickList[CurrSel]->AxisList[i]->Usage);
				if (wcscmp(axisString, axisBuffer) != 0)
				{
					item.pszText = axisString;
					SendMessageW(axisList, LVM_SETITEMTEXT, i, (LPARAM)&item);
				}
			}

			// Print HAT States
			for (int i = 0; i < TChapman500::JoystickAPI::InputSystem::System->JoystickList[CurrSel]->HATList.size(); i++)
			{
				item.iSubItem = 1;
				item.pszText = hatBuffer;
				SendMessageW(hatList, LVM_GETITEMTEXT, i, (LPARAM)&item);

				switch (TChapman500::JoystickAPI::InputSystem::System->JoystickList[CurrSel]->HATList[i]->Direction)
				{
				case TChapman500::JoystickAPI::InputHAT::direction::None:
					swprintf_s(hatString, 32, L"Centered");
					break;
				case TChapman500::JoystickAPI::InputHAT::direction::Top:
					swprintf_s(hatString, 32, L"Up");
					break;
				case TChapman500::JoystickAPI::InputHAT::direction::TopRight:
					swprintf_s(hatString, 32, L"Up & Right");
					break;
				case TChapman500::JoystickAPI::InputHAT::direction::Right:
					swprintf_s(hatString, 32, L"Right");
					break;
				case TChapman500::JoystickAPI::InputHAT::direction::BottomRight:
					swprintf_s(hatString, 32, L"Down & Right");
					break;
				case TChapman500::JoystickAPI::InputHAT::direction::Bottom:
					swprintf_s(hatString, 32, L"Down");
					break;
				case TChapman500::JoystickAPI::InputHAT::direction::BottomLeft:
					swprintf_s(hatString, 32, L"Down & Left");
					break;
				case TChapman500::JoystickAPI::InputHAT::direction::Left:
					swprintf_s(hatString, 32, L"Left");
					break;
				case TChapman500::JoystickAPI::InputHAT::direction::TopLeft:
					swprintf_s(hatString, 32, L"Up & Left");
					break;
				}

				if (wcscmp(hatString, hatBuffer) != 0)
				{
					item.pszText = hatString;
					SendMessageW(hatList, LVM_SETITEMTEXT, i, (LPARAM)&item);
				}
			}


			// Get HAT State
			if (TChapman500::JoystickAPI::InputSystem::System->JoystickList[CurrSel]->HATList[0])
			{
				TChapman500::JoystickAPI::InputHAT::direction direction = TChapman500::JoystickAPI::InputSystem::System->JoystickList[CurrSel]->HATList[0]->Direction;

				LRESULT state = SendMessageW(hatUp, BM_GETCHECK, 0, 0);
				if (direction == TChapman500::JoystickAPI::InputHAT::direction::Top || direction == TChapman500::JoystickAPI::InputHAT::direction::TopLeft || direction == TChapman500::JoystickAPI::InputHAT::direction::TopRight)
				{
					if (state == BST_UNCHECKED) SendMessageW(hatUp, BM_SETCHECK, BST_CHECKED, 0);
				}
				else
				{
					if (state == BST_CHECKED) SendMessageW(hatUp, BM_SETCHECK, BST_UNCHECKED, 0);
				}

				state = SendMessageW(hatLeft, BM_GETCHECK, 0, 0);
				if (direction == TChapman500::JoystickAPI::InputHAT::direction::Left || direction == TChapman500::JoystickAPI::InputHAT::direction::TopLeft || direction == TChapman500::JoystickAPI::InputHAT::direction::BottomLeft)
				{
					if (state == BST_UNCHECKED) SendMessageW(hatLeft, BM_SETCHECK, BST_CHECKED, 0);
				}
				else
				{
					if (state == BST_CHECKED) SendMessageW(hatLeft, BM_SETCHECK, BST_UNCHECKED, 0);
				}

				state = SendMessageW(hatRight, BM_GETCHECK, 0, 0);
				if (direction == TChapman500::JoystickAPI::InputHAT::direction::Right || direction == TChapman500::JoystickAPI::InputHAT::direction::TopRight || direction == TChapman500::JoystickAPI::InputHAT::direction::BottomRight)
				{
					if (state == BST_UNCHECKED) SendMessageW(hatRight, BM_SETCHECK, BST_CHECKED, 0);
				}
				else
				{
					if (state == BST_CHECKED) SendMessageW(hatRight, BM_SETCHECK, BST_UNCHECKED, 0);
				}

				state = SendMessageW(hatDown, BM_GETCHECK, 0, 0);
				if (direction == TChapman500::JoystickAPI::InputHAT::direction::Bottom || direction == TChapman500::JoystickAPI::InputHAT::direction::BottomLeft || direction == TChapman500::JoystickAPI::InputHAT::direction::BottomRight)
				{
					if (state == BST_UNCHECKED) SendMessageW(hatDown, BM_SETCHECK, BST_CHECKED, 0);
				}
				else
				{
					if (state == BST_CHECKED) SendMessageW(hatDown, BM_SETCHECK, BST_UNCHECKED, 0);
				}
			}
		}
		else
		{
			HRESULT count = SendMessageW(btnList, LVM_GETITEMCOUNT, 0, 0);
			if (count > 0)
			{
				SendMessageW(btnList, LVM_DELETEALLITEMS, 0, 0);
				SendMessageW(axisList, LVM_DELETEALLITEMS, 0, 0);
				SendMessageW(hatList, LVM_DELETEALLITEMS, 0, 0);
			}
		}

	}
CLEAN_UP:



	return 0;
}

