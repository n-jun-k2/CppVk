#pragma once

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WIN32
#include "WindowPtr.h"
#include <glfw3.h>
#include <glfw3native.h>
#include <iostream>

class App;

/*
	アプリケーション用ウィンドウを作成する。
	コンストラクタのAppにアプリケーションオブジェクトを渡し
	アプリケーションオブジェクトの処理を行う。
*/
class AppWindow:public WindowPtr
{
protected:

	GLFWwindow* window;
	App* application;
	int Wide, Heide;
	std::string Title;

public:
	AppWindow(int w, int h, const std::string& t, App* app);
	AppWindow(const AppWindow&) = delete;
	AppWindow& operator = (const AppWindow&) = delete;
	AppWindow(AppWindow&&) = delete;
	AppWindow& operator = (AppWindow&&) = delete;

	virtual ~AppWindow();

	void Run();
	virtual HWND getWin32()override
	{
		return glfwGetWin32Window(window);
	}
	virtual std::vector<const char*> GetRequired() override 
	{
		uint32_t gcount = 0;
		const char** gextensions;

		gextensions = glfwGetRequiredInstanceExtensions(&gcount);

		std::vector<const char*> extensions(gextensions, gextensions + gcount);

		return extensions;
	}
private:
	bool init();
};