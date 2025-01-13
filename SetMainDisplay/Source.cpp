#include <Windows.h>
#include <strsafe.h>
#include <vector>
#include "Screen.h"
#include <iostream>

int main()
{
	// WinAPI reference Parameters
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

	std::cout << "Cursor : " << cursor.x << ", " << cursor.y << std::endl;

	// initialize displayInfo and DisplayDevice
	ZeroMemory(&displayInfo, sizeof(DEVMODE));
	displayInfo.dmSize = sizeof(DEVMODE);
	ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
	DisplayDevice.cb = sizeof(DisplayDevice);

	// establish new main display
	// iterates over each display device in the current session until it fails when DispNum is greater than the greatest registered display index
	// information about the current iterated display into DisplayDevice
	while (EnumDisplayDevices(NULL, DispNum, &DisplayDevice, 0)) {

		// retrieves more information about the current iterated display into displayInfo using DisplayDevice.DeviceName
		EnumDisplaySettings((LPCWSTR)DisplayDevice.DeviceName, ENUM_REGISTRY_SETTINGS, &displayInfo);

		// DISPLAY_DEVICE_ACTIVE is the same as DISPLAY_DEVICE_ATTACHED_TO_DESKTOP refered to in the WinAPI documentation
		if ((DisplayDevice.StateFlags & DISPLAY_DEVICE_ACTIVE)) {
			std::wcout << "Found Display : " << DisplayDevice.DeviceName << ", x: " << displayInfo.dmPosition.x << ", y: " << displayInfo.dmPosition.y << ", width: " << displayInfo.dmPelsWidth << ", height: " << displayInfo.dmPelsHeight << std::endl;
			Screens.push_back(Screen(displayInfo.dmPosition.x, displayInfo.dmPosition.y, displayInfo.dmPelsWidth, displayInfo.dmPelsHeight, DisplayDevice.DeviceName));

			// line below does not work for some reason
			// (it doesnt work because dmPosition.x is a signed int (long) and dmPelsWidth is an unsigned int (dword)
			// the addition converts dmPosition.x to an unsigned int and so it loses it's sign)
			// withinX = displayInfo.dmPosition.x < cursor.x && cursor.x < (displayInfo.dmPosition.x + displayInfo.dmPelsWidth);

			//itDoesntWorkOtherwise = displayInfo.dmPosition.x + displayInfo.dmPelsWidth;
			//withinX = displayInfo.dmPosition.x < cursor.x && cursor.x < itDoesntWorkOtherwise;
			//itDoesntWorkOtherwise = displayInfo.dmPosition.y + displayInfo.dmPelsHeight;
			//withinY = displayInfo.dmPosition.y < cursor.y && cursor.y < itDoesntWorkOtherwise;

			// proper fix for above
			withinX = displayInfo.dmPosition.x < cursor.x && cursor.x < (displayInfo.dmPosition.x + static_cast<LONG>(displayInfo.dmPelsWidth));
			withinY = displayInfo.dmPosition.y < cursor.y && cursor.y < (displayInfo.dmPosition.y + static_cast<LONG>(displayInfo.dmPelsHeight));


			// device in cursor position and not current main display
			// ~ used isntead of ! for flags according to C6317
			if ((displayInfo.dmPosition.x + displayInfo.dmPosition.y) != 0 && withinX && withinY
				&& (DisplayDevice.StateFlags & ~DISPLAY_DEVICE_PRIMARY_DEVICE & ~DISPLAY_DEVICE_REMOVABLE))
			{
				newMainDeviceNum = i;
			}
		}

		// reset displayInfo and DisplayDevice
		ZeroMemory(&displayInfo, sizeof(DEVMODE));
		displayInfo.dmSize = sizeof(DEVMODE);
		ZeroMemory(&DisplayDevice, sizeof(DisplayDevice));
		DisplayDevice.cb = sizeof(DisplayDevice);

		DispNum++;
		i++;
	}

	// repositions displays around new main display
	if (newMainDeviceNum >= 0) {
		Screen newMainScreen = Screens.at(newMainDeviceNum);
		std::cout << "New Main Display Found : " << newMainScreen.name << std::endl;

		// Reposition the new main display to (0, 0)
		ZeroMemory(&displayInfo, sizeof(DEVMODE));
		displayInfo.dmSize = sizeof(DEVMODE);

		// Find the device using EnumDisplayDevices (still necessary for accessing settings)
		if (EnumDisplaySettings((LPCWSTR)Screens.at(newMainDeviceNum).name, ENUM_REGISTRY_SETTINGS, &displayInfo)) {
			// Set new main display to 0, 0 directly
			displayInfo.dmFields = DM_POSITION;
			displayInfo.dmPosition.x = 0;
			displayInfo.dmPosition.y = 0;

			// Apply the change using ChangeDisplaySettingsEx
			long res = ChangeDisplaySettingsEx(Screens.at(newMainDeviceNum).name, &displayInfo, NULL,
				CDS_UPDATEREGISTRY | CDS_SET_PRIMARY | CDS_NORESET, NULL);
			std::wcout << "(" << res << ") New Main Display " << Screens.at(newMainDeviceNum).name
				<< " : x:" << displayInfo.dmPosition.x << ", y:" << displayInfo.dmPosition.y << std::endl;
		}

		for (int j = 0; j < Screens.size(); ++j) {
			if (j != newMainDeviceNum) {
				ZeroMemory(&displayInfo, sizeof(DEVMODE));
				displayInfo.dmSize = sizeof(DEVMODE);

				// Get the settings for the other screens
				displayInfo.dmFields = DM_POSITION;

				// Set position relative to the new main display
				displayInfo.dmPosition.x = Screens[j].X - newMainScreen.X;
				displayInfo.dmPosition.y = Screens[j].Y - newMainScreen.Y;

				// Apply the changes using ChangeDisplaySettingsEx
				long res = ChangeDisplaySettingsEx(Screens[j].name, &displayInfo, NULL,
					CDS_UPDATEREGISTRY | CDS_NORESET, NULL);
				std::wcout << "(" << res << ") New Position Display " << Screens[j].name
					<< " : x:" << displayInfo.dmPosition.x
					<< ", y:" << displayInfo.dmPosition.y << std::endl;
			}
		}

		// apply changes
		ChangeDisplaySettingsEx(NULL, NULL, NULL, 0, NULL);
	}
	else
	{
		std::cout << "Already Main Display - No Change" << std::endl;
	}
}