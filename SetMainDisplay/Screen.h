#pragma once
#include <string>
#include <Windows.h>
class Screen
{
public:
	int X = 0;
	int Y = 0;
	int Width = 0;
	int Height = 0;
	WCHAR name[32];

	Screen(int x, int y, int width, int height, const WCHAR* name);
};

