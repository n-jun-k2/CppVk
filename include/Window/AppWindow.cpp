
#include "App.h"
#include "AppWindow.h"

#include <string>

#if _DEBUG
void error_callback( int error, const char *msg ) {
    std::string s;
    s = " [" + std::to_string(error) + "] " + msg + '\n';
    std::cerr << s << std::endl;
}
#endif

AppWindow::AppWindow(int w, int h, const std::string& t, App* app)
	:Wide(w), Heide(h), Title(t), application(app)
{

}

AppWindow::~AppWindow()
{
	if (window != nullptr)
	{
		glfwDestroyWindow(window);
		window = nullptr;
	}
	glfwTerminate();
}

void AppWindow::Run()
{
	if (init())
	{
		application->winptr = this;
		application->Initialize();

		while (!glfwWindowShouldClose(window))
		{
			application->Update();
			application->Draw();

			glfwSwapBuffers(window);
			glfwPollEvents();
		}
		application->Finalize();
	}
}

bool AppWindow::init()
{
#if _DEBUG
  glfwSetErrorCallback( error_callback );
#endif

	if (glfwInit() == GLFW_FALSE)
		return false;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(Wide, Heide, Title.c_str(), nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return false;
	}

	glfwSwapInterval(GLFW_TRUE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

	return true;
}