

#include "resource.h"
#include "ProcDefs.h"

#include <commctrl.h>
#include <Windows.h>
#include "../InputSystem/WinInputSystem.h"
#include "../InputSystem/Joystick.h"
#include "../InputSystem/InputAxis.h"
#include "../InputSystem/InputButton.h"
#include "../InputSystem/InputHAT.h"

#include <string>
#include <vector>

TChapman500::Input::IInputSystem *InputSystem = nullptr;
TChapman500::Input::Joystick *currDev = nullptr;
bool refreshCycle = true;

static PROCFUNC WndProc[65536];
HWND MainHWnd;

HWND DevList;
HWND BtnList;
HWND AxisList;
HWND HATList;
HWND DevCountText;

INT_PTR CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (!InputSystem) return FALSE;
	BOOL result = ((TChapman500::Input::Windows::WinInputSystem *)InputSystem)->WindowProc(message, wParam, lParam);
	if (!result) return WndProc[message](hWnd, wParam, lParam);
}

int CurrSelection = -1;


unsigned prevSel = -1;
unsigned currSel = -1;

bool refresh = true;

void InitializeInputSystem()
{
	//if (IsWindow(MainHWnd))
	//{
	//	MessageBoxW(MainHWnd, L"This is a valid handle.", L"Valid HWND", MB_OK);
	//}

	refreshCycle = true;
	prevSel = -1;
	currSel = -1;
	currDev = nullptr;

	// Initialize Input System
	if (InputSystem) delete InputSystem;
	InputSystem = new TChapman500::Input::Windows::WinInputSystem(MainHWnd);

}

std::wstring HatStates[9] = {
	L"Up",
	L"Up and Right",
	L"Right",
	L"Down and Right",
	L"Down",
	L"Down and Left",
	L"Left",
	L"Up and Left",
	L"Centered"
};

void PopulateJoystickList()
{
	// Clear all lists
	ListView_DeleteAllItems(DevList);
	ListView_DeleteAllItems(BtnList);
	ListView_DeleteAllItems(AxisList);
	ListView_DeleteAllItems(HATList);

	// Set Joystick Count
	unsigned devCount = InputSystem->GetJoystickCount();
	wchar_t devCountStr[16];
	swprintf_s(devCountStr, 16, L"%d", devCount);
	SetWindowTextW(DevCountText, devCountStr);

	unsigned devColumns[7] = { 0, 1, 2, 3, 4, 5, 6 };

	LVITEMW item;
	ZeroMemory(&item, sizeof(LVITEMW));
	item.mask = LVIF_TEXT;
	item.puColumns = devColumns;

	// Initialize Joystick List
	for (unsigned i = 0; i < devCount; i++)
	{
		// Current Joystick
		TChapman500::Input::Joystick *currDevice = InputSystem->GetJoystick(i);

		// Name Buffer
		wchar_t *nameStr;
		item.iItem = i;

		// Poduct Name
		nameStr = (wchar_t *)currDevice->GetProductName();
		item.pszText = nameStr;
		item.iSubItem = 0;
		ListView_InsertItem(DevList, &item);

		// Vendor Name
		nameStr = (wchar_t *)currDevice->GetVendorName();
		item.pszText = nameStr;
		item.iSubItem = 1;
		ListView_SetItemText(DevList, i, 1, nameStr);

		unsigned short usage = currDevice->GetUsage();
		if (usage == 4) { ListView_SetItemText(DevList, i, 2, (LPWSTR)L"Joystick"); }
		else if (usage == 5) { ListView_SetItemText(DevList, i, 2, (LPWSTR)L"Gamepad"); }

		nameStr = (wchar_t *)currDevice->GetInterfaceName();
		item.pszText = nameStr;
		ListView_SetItemText(DevList, i, 3, item.pszText);

		wchar_t printStr[16];
		item.pszText = printStr;
		swprintf_s(printStr, 16, L"%d", (unsigned)currDevice->GetAxisCount());
		ListView_SetItemText(DevList, i, 4, item.pszText);

		swprintf_s(printStr, 16, L"%d", (unsigned)currDevice->GetButtonCount());
		ListView_SetItemText(DevList, i, 5, item.pszText);

		swprintf_s(printStr, 16, L"%d", (unsigned)currDevice->GetHATCount());
		ListView_SetItemText(DevList, i, 6, item.pszText);
	}
}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	// Initialize proc list.
	for (int i = 0; i < 65536; i++) WndProc[i] = &Unprocessed;	// Default message handler.
	WndProc[WM_COMMAND] = &OnCommand;				// Button pressed.
	WndProc[WM_CLOSE] = &OnClose;
	WndProc[WM_INITDIALOG] = &OnCreate;
	WndProc[WM_DESTROY] = &OnDestroy;

	// Initialize Window
	MainHWnd = CreateDialogParamW(hInstance, MAKEINTRESOURCEW(IDD_JOYVIEW), nullptr, &WindowProc, 0);
	if (!MainHWnd)
	{
		DWORD error = GetLastError();

		wchar_t msg[16];
		swprintf_s(msg, 16, L"%d", error);

		MessageBoxExW(nullptr, msg, L"Error", MB_ICONERROR | MB_OK, NULL);
		return -1;
	}
	ShowWindow(MainHWnd, nShowCmd);

	// Get Child Windows
	DevList = GetDlgItem(MainHWnd, IDC_DEVLIST);
	BtnList = GetDlgItem(MainHWnd, IDC_BTNLIST);
	AxisList = GetDlgItem(MainHWnd, IDC_AXISLIST);
	HATList = GetDlgItem(MainHWnd, IDC_HATLIST);
	DevCountText = GetDlgItem(MainHWnd, IDC_DEVCOUNT);

	LVCOLUMNW column;
	column.mask = LVCF_TEXT | LVCF_WIDTH;

	// Initialize columns of all list views
	{
		// Initialize Joystick List
		column.pszText = (LPWSTR)L"Device Name";
		column.cx = 200;
		ListView_InsertColumn(DevList, 0, &column);

		column.pszText = (LPWSTR)L"Vendor Name";
		column.cx = 90;
		ListView_InsertColumn(DevList, 1, &column);

		column.pszText = (LPWSTR)L"Device Type";
		column.cx = 80;
		ListView_InsertColumn(DevList, 2, &column);

		column.pszText = (LPWSTR)L"Interface Type";
		column.cx = 140;
		ListView_InsertColumn(DevList, 3, &column);

		column.pszText = (LPWSTR)L"#Axes";
		column.cx = 60;
		ListView_InsertColumn(DevList, 4, &column);

		column.pszText = (LPWSTR)L"#Buttons";
		column.cx = 60;
		ListView_InsertColumn(DevList, 5, &column);

		column.pszText = (LPWSTR)L"#HATs";
		column.cx = 60;
		ListView_InsertColumn(DevList, 6, &column);

		// Initialize Button List
		column.pszText = (LPWSTR)L"Button";
		column.cx = 110;
		ListView_InsertColumn(BtnList, 0, &column);

		column.pszText = (LPWSTR)L"Pressed";
		column.cx = 50;
		ListView_InsertColumn(BtnList, 1, &column);

		// Initialize Axis List
		column.pszText = (LPWSTR)L"Axis Type";
		column.cx = 60;
		ListView_InsertColumn(AxisList, 0, &column);

		column.pszText = (LPWSTR)L"Bits";
		column.cx = 40;
		ListView_InsertColumn(AxisList, 1, &column);

		column.pszText = (LPWSTR)L"Min Value";
		column.cx = 70;
		ListView_InsertColumn(AxisList, 2, &column);

		column.pszText = (LPWSTR)L"Max Value";
		column.cx = 70;
		ListView_InsertColumn(AxisList, 3, &column);

		column.pszText = (LPWSTR)L"Raw Value";
		column.cx = 70;
		ListView_InsertColumn(AxisList, 4, &column);

		column.pszText = (LPWSTR)L"Center-Relative";
		column.cx = 90;
		ListView_InsertColumn(AxisList, 5, &column);

		column.pszText = (LPWSTR)L"End-Relative";
		column.cx = 90;
		ListView_InsertColumn(AxisList, 6, &column);

		// Initialize HAT List
		column.pszText = (LPWSTR)L"HAT Name";
		column.cx = 120;
		ListView_InsertColumn(HATList, 0, &column);

		column.pszText = (LPWSTR)L"Position";
		column.cx = 90;
		ListView_InsertColumn(HATList, 1, &column);
	}

	InitializeInputSystem();



	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	// Get the current device
	std::vector<int> prevAxisStates;
	std::vector<bool> prevButtonStates;
	std::vector<unsigned> prevHatStates;

	unsigned prevCount = InputSystem->GetJoystickCount();
	PopulateJoystickList();

	while (true)
	{
		// Process Messages
		if (PeekMessageW(&msg, nullptr, NULL, NULL, PM_REMOVE))
		{
			if (msg.message == WM_QUIT) break;
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
			continue;
		}

		if (InputSystem->GetJoystickCount() != prevCount)
		{
			prevCount = InputSystem->GetJoystickCount();
			PopulateJoystickList();
		}
		
		// Get selected joystick
		for (unsigned i = 0; i < InputSystem->GetJoystickCount(); i++)
		{
			unsigned column = 0;
			LVITEMA item;
			ZeroMemory(&item, sizeof(LVITEMA));
			item.mask = LVIF_STATE;
			item.stateMask = LVIS_SELECTED;
			item.iItem = i;

			ListView_GetItem(DevList, &item);

			if ((item.state & LVIS_SELECTED) > 0)
			{
				currSel = i;
				break;
			}
		}

		// Only populate object lists if selection has changed and is valid.
		if (currSel != prevSel && currSel < InputSystem->GetJoystickCount())
		{
			// Clear All Lists
			ListView_DeleteAllItems(BtnList);
			ListView_DeleteAllItems(AxisList);
			ListView_DeleteAllItems(HATList);

			// Get the current device
			currDev = InputSystem->GetJoystick(currSel);

			prevButtonStates.clear();
			prevButtonStates.resize(currDev->GetButtonCount());
			for (unsigned i = 0; i < prevButtonStates.size(); i++)
				prevButtonStates[i] = currDev->GetButtonState(i);

			prevAxisStates.clear();
			prevAxisStates.resize(currDev->GetAxisCount());
			for (unsigned i = 0; i < prevAxisStates.size(); i++)
			{
				prevAxisStates[i] = currDev->GetAxisValue(i);
			}

			prevHatStates.clear();
			prevHatStates.resize(currDev->GetHATCount());
			for (unsigned i = 0; i < prevHatStates.size(); i++)
			{
				prevHatStates[i] = (unsigned)currDev->GetHATPosition(i);
			}

			unsigned btnColumns[2] = { 0, 1 };
			unsigned axisColumns[6] = { 0, 1, 2, 3, 4, 5 };

			LVITEMW item;
			ZeroMemory(&item, sizeof(LVITEMW));
			item.mask = LVIF_TEXT;
			item.puColumns = btnColumns;

			// Populate button list
			{
				unsigned btnCount = (unsigned)currDev->GetButtonCount();

				// Name Buffer
				wchar_t btnName[32];
				ZeroMemory(btnName, 64);

				item.pszText = btnName;
				item.iSubItem = 0;

				for (unsigned i = 0; i < btnCount; i++)
				{
					switch (currDev->GetButtonUsage(i))
					{
					case 60001:
						swprintf(btnName, 32, L"PoV Up");
						break;
					case 60002:
						swprintf(btnName, 32, L"PoV Down");
						break;
					case 60003:
						swprintf(btnName, 32, L"PoV Right");
						break;
					case 60004:
						swprintf(btnName, 32, L"PoV Left");
						break;
					default:
						swprintf(btnName, 32, L"Button %d", i);
						break;
					}


					item.iItem = i;
					ListView_InsertItem(BtnList, &item);
					ZeroMemory(btnName, 64);

					if (prevButtonStates[i])
					{
						ListView_SetItemText(BtnList, i, 1, (wchar_t *)L"true");
					}
					else
					{
						ListView_SetItemText(BtnList, i, 1, (wchar_t *)L"false");
					}
				}
			}

			// Populate axis list
			{
				unsigned btnCount = (unsigned)currDev->GetAxisCount();

				item.iSubItem = 0;

				TChapman500::Input::value_properties properties;
				
				for (unsigned i = 0; i < btnCount; i++)
				{
					currDev->GetAxisProperties(properties, i);
					unsigned short usage = properties.Usage;

					switch (usage)
					{
					case 0x30:
						item.pszText = (wchar_t*)L"X";
						break;
					case 0x31:
						item.pszText = (wchar_t *)L"Y";
						break;
					case 0x32:
						item.pszText = (wchar_t *)L"Z";
						break;
					case 0x33:
						item.pszText = (wchar_t *)L"Rx";
						break;
					case 0x34:
						item.pszText = (wchar_t *)L"Ry";
						break;
					case 0x35:
						item.pszText = (wchar_t *)L"Rz";
						break;
					case 0x36:
						item.pszText = (wchar_t *)L"Slider";
						break;
					case 0x37:
						item.pszText = (wchar_t *)L"Dial";
						break;
					case 0x38:
						item.pszText = (wchar_t *)L"Wheel";
						break;
					default:
						item.pszText = (wchar_t *)L"Unknown";
						break;
					}

					item.iItem = i;
					ListView_InsertItem(AxisList, &item);

					wchar_t btnName[32];
					swprintf(btnName, 32, L"%d", properties.Bits);
					ListView_SetItemText(AxisList, i, 1, btnName);

					swprintf(btnName, 32, L"%d", properties.MinValue);
					ListView_SetItemText(AxisList, i, 2, btnName);

					swprintf(btnName, 32, L"%d", properties.MaxValue);
					ListView_SetItemText(AxisList, i, 3, btnName);

					swprintf(btnName, 32, L"%d", currDev->GetAxisValue(i));
					ListView_SetItemText(AxisList, i, 4, btnName);

					swprintf(btnName, 32, L"%.4f", currDev->GetAxisCenterRelative(i));
					ListView_SetItemText(AxisList, i, 5, btnName);

					swprintf(btnName, 32, L"%.4f", currDev->GetAxisEndRelative(i));
					ListView_SetItemText(AxisList, i, 6, btnName);
				}
			}


			// Populate hat list
			{
				unsigned btnCount = (unsigned)currDev->GetHATCount();

				// Name Buffer
				wchar_t btnName[32];
				ZeroMemory(btnName, 64);

				item.pszText = btnName;
				item.iSubItem = 0;

				for (unsigned i = 0; i < btnCount; i++)
				{
					swprintf(btnName, 32, L"HAT %d", i);
					item.iItem = i;
					ListView_InsertItem(HATList, &item);
					ZeroMemory(btnName, 64);

					unsigned pos = (unsigned)currDev->GetHATPosition(i);
					ListView_SetItemText(HATList, i, 1, (wchar_t *)HatStates[pos].c_str());
				}
			}

			prevSel = currSel;
		}

		// Process Joysticks
		if (refreshCycle) refreshCycle = false;
		else InputSystem->UpdateJoystickStates();

		if (currDev)
		{
			// Print Button States
			for (unsigned i = 0; i < (unsigned)currDev->GetButtonCount(); i++)
			{
				if (prevButtonStates[i] != currDev->GetButtonState(i))
				{
					if (currDev->GetButtonState(i))
					{
						ListView_SetItemText(BtnList, i, 1, (wchar_t *)L"true");
					}
					else
					{
						ListView_SetItemText(BtnList, i, 1, (wchar_t *)L"false");
					}
					prevButtonStates[i] = currDev->GetButtonState(i);
				}
			}

			// Print Axis States
			for (unsigned i = 0; i < currDev->GetAxisCount(); i++)
			{
				if (prevAxisStates[i] != currDev->GetAxisValue(i))
				{
					wchar_t btnName[32];

					swprintf(btnName, 32, L"%d", currDev->GetAxisValue(i));
					ListView_SetItemText(AxisList, i, 4, btnName);

					swprintf(btnName, 32, L"%.4f", currDev->GetAxisCenterRelative(i));
					ListView_SetItemText(AxisList, i, 5, btnName);

					swprintf(btnName, 32, L"%.4f", currDev->GetAxisEndRelative(i));
					ListView_SetItemText(AxisList, i, 6, btnName);

					prevAxisStates[i] = currDev->GetAxisValue(i);
				}
			}


			// Print HAT States
			for (unsigned i = 0; i < currDev->GetHATCount(); i++)
			{
				if (prevHatStates[i] != (unsigned)currDev->GetHATPosition(i))
				{
					unsigned pos = (unsigned)currDev->GetHATPosition(i);
					ListView_SetItemText(HATList, i, 1, (wchar_t*)HatStates[pos].c_str());

					prevHatStates[i] = (unsigned)currDev->GetHATPosition(i);
				}
			}
		}





	}

	delete InputSystem;
	return (int)msg.wParam;
}

