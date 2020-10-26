#include "SaitekX52.h"
#include "InputButton.h"
#include "InputAxis.h"
#include <string>

SaitekX52::SaitekX52(HANDLE device) : TChapman500::JoystickAPI::Joystick(device)
{
	DeviceName = L"X52 H.O.T.A.S.";

	ButtonList[0]->SetName(L"Trigger");
	ButtonList[1]->SetName(L"Fire");
	ButtonList[2]->SetName(L"Fire A");
	ButtonList[3]->SetName(L"Fire B");
	ButtonList[4]->SetName(L"Fire C");
	ButtonList[5]->SetName(L"Pinky");
	ButtonList[6]->SetName(L"Fire D");
	ButtonList[7]->SetName(L"Fire E");
	ButtonList[8]->SetName(L"Toggle 1");
	ButtonList[9]->SetName(L"Toggle 2");
	ButtonList[10]->SetName(L"Toggle 3");
	ButtonList[11]->SetName(L"Toggle 4");
	ButtonList[12]->SetName(L"Toggle 5");
	ButtonList[13]->SetName(L"Toggle 6");
	ButtonList[14]->SetName(L"Second Trigger");
	ButtonList[15]->SetName(L"POV 2 (Up)");
	ButtonList[16]->SetName(L"POV 2 (Right)");
	ButtonList[17]->SetName(L"POV 2 (Down)");
	ButtonList[18]->SetName(L"POV 2 (Left)");
	ButtonList[19]->SetName(L"Throttle HAT (Up)");
	ButtonList[20]->SetName(L"Throttle HAT (Right)");
	ButtonList[21]->SetName(L"Throttle HAT (Down)");
	ButtonList[22]->SetName(L"Throttle HAT (Left)");
	ButtonList[23]->SetName(L"Mode 1");
	ButtonList[24]->SetName(L"Mode 2");
	ButtonList[25]->SetName(L"Mode 3");
	ButtonList[26]->SetName(L"MFD Mode");
	ButtonList[27]->SetName(L"MFD Down");
	ButtonList[28]->SetName(L"MFD Up");
	ButtonList[29]->SetName(L"Clutch");
	ButtonList[30]->SetName(L"Left Click");
	ButtonList[31]->SetName(L"Right Click");
	ButtonList[32]->SetName(L"Scroll Down");
	ButtonList[33]->SetName(L"Scroll Up");

	AxisList[7]->SetName(L"Mouse X");
	AxisList[8]->SetName(L"Mouse Y");

	AddCustomHAT(15, 17, 16, 18, L"POV 2");
	AddCustomHAT(19, 21, 20, 22, L"Throttle HAT");
}