#pragma once

class WindowPtr;

class App
{
public:
	WindowPtr* winptr = nullptr;
public:
	virtual ~App() {}
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() const = 0;
	virtual void Finalize() = 0;
};
