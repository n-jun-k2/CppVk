#pragma once

#include <Windows.h>
#include <vector>

///アプリケーションとウィンドウの窓口
class WindowPtr
{
public:
	~WindowPtr() {}
	virtual HWND getWin32() = 0;
	virtual std::vector<const char*> GetRequired() = 0;
};