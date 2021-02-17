#pragma once
#include <string>
class Screen
{
public:
	int X = 0;
	int Y = 0;
	int Width = 0;
	int Height = 0;
	std::wstring name;


	Screen(int x, int y, int width, int height, const std::wstring& name);

};

