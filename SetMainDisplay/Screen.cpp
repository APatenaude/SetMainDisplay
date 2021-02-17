#include "Screen.h"

Screen::Screen(int x, int y, int width, int height, const std::wstring& name)
{
	this->X = x;
	this->Y = y;
	this->Width = width;
	this->Height = height;
	this->name = name;
}