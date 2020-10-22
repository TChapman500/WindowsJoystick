#include "ProcDefs.h"
#include "InputSystem.h"
#include "InputDevice.h"
#include "Joystick.h"
#include "SaitekX52.h"

extern HWND MainHWnd;

INT_PTR CALLBACK Unprocessed(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

INT_PTR CALLBACK OnClose(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	DestroyWindow(hWnd);
	return TRUE;
}

INT_PTR CALLBACK OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	TChapman500::JoystickAPI::InputSystem::System = new TChapman500::JoystickAPI::InputSystem(hWnd);


	return TRUE;
}


INT_PTR CALLBACK OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	delete TChapman500::JoystickAPI::InputSystem::System;
	PostQuitMessage(0);
	return TRUE;
}

// Process RAW input via the message loop.
INT_PTR CALLBACK OnInput(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (InputMode == inputMode::WindowProc) TChapman500::JoystickAPI::InputSystem::System->MessageLoop(lParam);
	return TRUE;
}

INT_PTR CALLBACK OnInputDeviceChanged(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HANDLE device = (HANDLE)lParam;

	UINT dataSize;
	int deviceIndex = -1;
	RID_DEVICE_INFO *deviceInfo;
	GetRawInputDeviceInfoA(device, RIDI_DEVICEINFO, nullptr, &dataSize);
	deviceInfo = (RID_DEVICE_INFO *)new char[dataSize];
	GetRawInputDeviceInfoA(device, RIDI_DEVICEINFO, deviceInfo, &dataSize);

	if (wParam == GIDC_ARRIVAL)
	{
		TChapman500::JoystickAPI::InputSystem::System->DeviceAdded(device);

		for (int i = 0; i < TChapman500::JoystickAPI::InputSystem::System->JoystickList.size(); i++)
		{
			if (TChapman500::JoystickAPI::InputSystem::System->JoystickList[i]->Device == device)
			{
				// Detect X52
				{
					UINT dataSize = 0;
					std::string devName;
					GetRawInputDeviceInfoA(device, RIDI_DEVICENAME, nullptr, &dataSize);
					devName.resize(dataSize);
					GetRawInputDeviceInfoA(device, RIDI_DEVICENAME, (void *)devName.data(), &dataSize);

					std::wstring DeviceName;
					DeviceName.resize(126);
					HANDLE HIDHandle = CreateFileA(devName.data(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);
					if (HIDHandle)
					{
						BOOLEAN productString = HidD_GetProductString(HIDHandle, (void *)DeviceName.data(), sizeof(wchar_t) * 126);
						CloseHandle(HIDHandle);
					}

					if (wcscmp(DeviceName.data(), (wchar_t*)L"Saitek X52 Flight Control System") == 0)
					{
						TChapman500::JoystickAPI::InputSystem::System->ReplaceJoystick(i, new SaitekX52(device));
					}
				}


				SendMessageW(GetDlgItem(MainHWnd, IDC_JOYSTICK_LIST), LB_INSERTSTRING, i, (LPARAM)TChapman500::JoystickAPI::InputSystem::System->JoystickList[i]->DeviceName.data());
				break;
			}
		}
	}
	else if (wParam == GIDC_REMOVAL)
	{
		for (int i = 0; i < TChapman500::JoystickAPI::InputSystem::System->JoystickList.size(); i++)
		{
			if (TChapman500::JoystickAPI::InputSystem::System->JoystickList[i]->Device == device)
			{
				SendMessageW(GetDlgItem(MainHWnd, IDC_JOYSTICK_LIST), LB_DELETESTRING, i, 0);
				break;
			}
		}

		TChapman500::JoystickAPI::InputSystem::System->DeviceRemoved(device);
	}

	delete[] deviceInfo;

	return TRUE;
}

INT_PTR CALLBACK OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case IDOK:
		DestroyWindow(hWnd);
		break;
	case IDC_MESSAGE_LOOP:
		InputMode = inputMode::WindowProc;
		break;
	case IDC_INPUT_LOOP:
		InputMode = inputMode::InputLoop;
		break;
	}
	return TRUE;
}

INT_PTR CALLBACK OnSelectChanged(HWND hWnd, WPARAM wParam, LPARAM lParam)
{



	return TRUE;
}
