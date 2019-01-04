#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cassert>
#include <iostream>
#include <chrono>

void glfw_error_callback(int error, const char *description) {
	std::cerr << "glfw error: " << error << " " << description << std::endl;
}

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;

	// init glfw
	glfwSetErrorCallback(glfw_error_callback);
	assert(glfwInit());

	// open window
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	auto *window = glfwCreateWindow(800, 400, "oscilloscope", nullptr, nullptr);
	assert(window);
	glfwMakeContextCurrent(window);

	// init glew
	glewExperimental = GL_TRUE;
	if (GLenum err = glewInit(); err != GLEW_OK)
		std::cerr << glewGetErrorString(err) << std::endl;

	// time management for main loop (max 9223372036854775808 milliseconds)
	using namespace std::chrono_literals;
	using clock = std::chrono::steady_clock;
	auto startTime = clock::now();
	auto totalDeltaTime = 0ms;

	// main loop
	while (!glfwWindowShouldClose(window)) {
		auto curTime = clock::now();
		auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(
			clock::now() - (startTime + totalDeltaTime));
		totalDeltaTime += deltaTime;

		int width, height;
		glfwGetWindowSize(window, &width, &height);
		glViewport(0, 0, width, height);

		// update
		// TODO

		// render
		glClear(GL_COLOR_BUFFER_BIT);
		// TODO
		glfwSwapBuffers(window);

		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_Q))
			glfwSetWindowShouldClose(window, true);
	}

	// terminate glfw
	glfwTerminate();
	return 0;
}
