#include "Screen.h"

Screen::Screen(int x, int y, int width, int height, const WCHAR* name)
{
	this->X = x;
	this->Y = y;
	this->Width = width;
	this->Height = height;
	wcsncpy_s(this->name, name, 32);
}