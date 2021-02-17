#include <Windows.h>
#include <strsafe.h>
#include <vector>
#include "Screen.h"

int main()
{
	DWORD DispNum = 0;
	DISPLAY_DEVICE DisplayDevice;
	DEVMODE displayInfo;
	int i = 0;
	std::vector<Screen> Screens;
	int newDefaultDeviceNum = -1;
	int itDoesntWorkOtherwise;
	POINT cursor;

	GetCursorPos(&cursor);

	// initialize DisplayDevice
	ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
	DisplayDevice.cb = sizeof(DisplayDevice);

	while (EnumDisplayDevices(NULL, DispNum, &DisplayDevice, 0)) {
		ZeroMemory(&displayInfo, sizeof(DEVMODE));
		displayInfo.dmSize = sizeof(DEVMODE);
		EnumDisplaySettings((LPCWSTR)DisplayDevice.DeviceName, ENUM_REGISTRY_SETTINGS, &displayInfo);

		if ((DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) {
			Screens.push_back(Screen(displayInfo.dmPosition.x, displayInfo.dmPosition.y, displayInfo.dmPelsWidth, displayInfo.dmPelsHeight, DisplayDevice.DeviceName));

			itDoesntWorkOtherwise = displayInfo.dmPosition.x + displayInfo.dmPelsWidth;
			bool withinX = displayInfo.dmPosition.x < cursor.x and cursor.x < itDoesntWorkOtherwise;
			itDoesntWorkOtherwise = displayInfo.dmPosition.y + displayInfo.dmPelsHeight;
			bool withinY = displayInfo.dmPosition.y < cursor.y and cursor.y < itDoesntWorkOtherwise;

			newDefaultDeviceNum = withinX and withinY and !(DisplayDevice.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE) ? i : newDefaultDeviceNum;
		}

		ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
		DisplayDevice.cb = sizeof(DisplayDevice);
		DispNum++;
		i++;
	}

	if (newDefaultDeviceNum >= 0) {
		Screen newDefaultScreen = Screens.at(newDefaultDeviceNum);
		DispNum = 0;
		i = 0;
		while (EnumDisplayDevices(NULL, DispNum, &DisplayDevice, 0)) {
			ZeroMemory(&displayInfo, sizeof(DEVMODE));
			displayInfo.dmSize = sizeof(DEVMODE);

			EnumDisplaySettings((LPCWSTR)DisplayDevice.DeviceName, ENUM_REGISTRY_SETTINGS, &displayInfo);
			
			if ((DisplayDevice.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) {

				Screen currentScreen = Screens.at(i);

				displayInfo.dmFields = DM_POSITION;

				//x
				if (i == newDefaultDeviceNum) {
					displayInfo.dmPosition.x = 0;
					displayInfo.dmPosition.y = 0;
					ChangeDisplaySettingsEx(DisplayDevice.DeviceName, &displayInfo, NULL, CDS_UPDATEREGISTRY | CDS_SET_PRIMARY | CDS_NORESET, NULL);
					ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
					DisplayDevice.cb = sizeof(DisplayDevice);
					DispNum++;
					i++;
					continue;
				}
				else if (currentScreen.X == newDefaultScreen.X) {
					displayInfo.dmPosition.x = 0;
				}
				else if (currentScreen.X < newDefaultScreen.X) {
					displayInfo.dmPosition.x = currentScreen.X - newDefaultScreen.X;
				}
				else if (currentScreen.X > newDefaultScreen.X) {
					displayInfo.dmPosition.x = currentScreen.X - newDefaultScreen.X;
				}
				//y
				if (currentScreen.Y == newDefaultScreen.Y) {
					displayInfo.dmPosition.y = 0;
				}
				else if (currentScreen.Y < newDefaultScreen.Y) {
					displayInfo.dmPosition.y = currentScreen.Y - newDefaultScreen.Y;
				}
				else if (currentScreen.Y > newDefaultScreen.Y) {
					displayInfo.dmPosition.y = currentScreen.Y - newDefaultScreen.Y;
				}
				
				ChangeDisplaySettingsEx(DisplayDevice.DeviceName, &displayInfo, NULL, CDS_UPDATEREGISTRY | CDS_NORESET, NULL);
			}

			ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
			DisplayDevice.cb = sizeof(DisplayDevice);
			DispNum++;
			i++;
		}
	}
	ChangeDisplaySettingsEx(NULL, NULL, NULL, NULL, NULL);
}