#include <Windows.h>
#include <strsafe.h>
#include <vector>
#include "Screen.h"

int main()
{
	//WinAPI reference Parameters
	DWORD DispNum = 0;
	DISPLAY_DEVICE DisplayDevice;
	DEVMODE displayInfo;

	int i = 0;
	std::vector<Screen> Screens;
	int newMainDeviceNum = -1;
	int itDoesntWorkOtherwise = 0;
	POINT cursor;
	bool withinX, withinY;

	GetCursorPos(&cursor);

	//initialize displayInfo and DisplayDevice
	ZeroMemory(&displayInfo, sizeof(DEVMODE));
	displayInfo.dmSize = sizeof(DEVMODE);
	ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
	DisplayDevice.cb = sizeof(DisplayDevice);

	//establish new main display
	//iterates over each display device in the current session until it fails when DispNum is greater than the greatest registered display index
	//information about the current iterated display into DisplayDevice
	while (EnumDisplayDevices(NULL, DispNum, &DisplayDevice, 0)) {

		//retrieves more information about the current iterated display into displayInfo using DisplayDevice.DeviceName
		EnumDisplaySettings((LPCWSTR)DisplayDevice.DeviceName, ENUM_REGISTRY_SETTINGS, &displayInfo);

		//DISPLAY_DEVICE_ACTIVE is the same as DISPLAY_DEVICE_ATTACHED_TO_DESKTOP refered to in the WinAPI documentation
		if ((DisplayDevice.StateFlags & DISPLAY_DEVICE_ACTIVE)) {
			Screens.push_back(Screen(displayInfo.dmPosition.x, displayInfo.dmPosition.y, displayInfo.dmPelsWidth, displayInfo.dmPelsHeight, DisplayDevice.DeviceName));

			//line below does not work for some reason
			//withinX = displayInfo.dmPosition.x < cursor.x and cursor.x < (displayInfo.dmPosition.x + displayInfo.dmPelsWidth);
			itDoesntWorkOtherwise = displayInfo.dmPosition.x + displayInfo.dmPelsWidth;
			withinX = displayInfo.dmPosition.x < cursor.x and cursor.x < itDoesntWorkOtherwise;
			itDoesntWorkOtherwise = displayInfo.dmPosition.y + displayInfo.dmPelsHeight;
			withinY = displayInfo.dmPosition.y < cursor.y and cursor.y < itDoesntWorkOtherwise;

			//device in cursor position 
			//~ used isntead of ! for flags according to C6317
			if(withinX and withinY and (DisplayDevice.StateFlags & ~DISPLAY_DEVICE_PRIMARY_DEVICE & ~DISPLAY_DEVICE_REMOVABLE))
				newMainDeviceNum =  i;
		}

		//reset displayInfo and DisplayDevice
		ZeroMemory(&displayInfo, sizeof(DEVMODE));
		displayInfo.dmSize = sizeof(DEVMODE);
		ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
		DisplayDevice.cb = sizeof(DisplayDevice);

		DispNum++;
		i++;
	}

	//repositions displays around new main display
	if (newMainDeviceNum >= 0) {
		Screen newMainScreen = Screens.at(newMainDeviceNum);
		DispNum = 0;
		i = 0;

		while (EnumDisplayDevices(NULL, DispNum, &DisplayDevice, 0)) {
			ZeroMemory(&displayInfo, sizeof(DEVMODE));
			displayInfo.dmSize = sizeof(DEVMODE);

			EnumDisplaySettings((LPCWSTR)DisplayDevice.DeviceName, ENUM_REGISTRY_SETTINGS, &displayInfo);
			
			if ((DisplayDevice.StateFlags & DISPLAY_DEVICE_ACTIVE)) {

				Screen currentScreen = Screens.at(i);
				displayInfo.dmFields = DM_POSITION;

				//new main display, set 0,0
				if (i == newMainDeviceNum) {
					displayInfo.dmPosition.x = 0;
					displayInfo.dmPosition.y = 0;
					ChangeDisplaySettingsEx(DisplayDevice.DeviceName, &displayInfo, NULL, CDS_UPDATEREGISTRY | CDS_SET_PRIMARY | CDS_NORESET, NULL);
				}
				//set position relative to new main display
				else {
					displayInfo.dmPosition.x = currentScreen.X - newMainScreen.X;
					displayInfo.dmPosition.y = currentScreen.Y - newMainScreen.Y;
					ChangeDisplaySettingsEx(DisplayDevice.DeviceName, &displayInfo, NULL, CDS_UPDATEREGISTRY | CDS_NORESET, NULL);
				}	
			}

			ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
			DisplayDevice.cb = sizeof(DisplayDevice);
			DispNum++;
			i++;
		}
	}

	//apply changes
	ChangeDisplaySettingsEx(NULL, NULL, NULL, NULL, NULL);
}