#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cassert>
#include <iostream>
#include <chrono>
#include <memory>
#include <fstream>
#include <sstream>
#include <vector>
#include "ring.h"

void glfw_error_callback(int error, const char *description) {
	std::cerr << "glfw error: " << error << " " << description << std::endl;
}

constexpr auto pi()
{
	return std::atan(1)*4;
}

GLuint loadShaderFromSourceCode(GLenum type, const char* sourcecode, int length)
{
	GLuint shaderId = glCreateShader(type);

	glShaderSource(shaderId, 1, &sourcecode, &length);
	glCompileShader(shaderId);

	GLint isCompiled = 0;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);

		auto errorLog = std::make_unique<GLchar[]>(maxLength);
		glGetShaderInfoLog(shaderId, maxLength, &maxLength, &errorLog[0]);

		std::cout << "Error compiling " << std::endl
			<< &errorLog[0] << std::endl;
		glDeleteShader(shaderId); // Don't leak the shader.
		return 0;
	}

	return shaderId;
}

GLuint loadShaderFromFile(GLenum type, const char* filepath)
{
	std::ifstream fstream;
	fstream.open(filepath);

	if (!fstream.is_open())
	{
		std::cout << "Unable to open file '" << filepath << "'" << std::endl;
		return 0;
	}

	std::stringstream sstream;
	std::string line;
	while (std::getline(fstream, line))
		sstream << line << '\n';
	line = sstream.str();

	GLuint shaderId = loadShaderFromSourceCode(type, line.c_str(), line.length());
	if (!shaderId)
		std::cout << "...with filepath '" << filepath << "'"; 

	return shaderId;
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

	// init program
	auto program = glCreateProgram();
	{
		auto vertexShader = loadShaderFromFile(GL_VERTEX_SHADER, "res/vertex.glsl");
		auto fragmentShader = loadShaderFromFile(GL_FRAGMENT_SHADER, "res/fragment.glsl");
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);
		glLinkProgram(program);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}
	glUseProgram(program);

	// locations
	auto mvpLocation = glGetUniformLocation(program, "mvp");
	assert(mvpLocation != -1);
	auto colorLocation = glGetUniformLocation(program, "color");
	assert(colorLocation != -1);

	// create info buffers
	GLuint info_vao;
	GLuint info_vbos[1];
	glGenBuffers(1, info_vbos);
	glGenVertexArrays(1, &info_vao);
	glBindVertexArray(info_vao);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, info_vbos[0]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// generate info
	glm::vec2 infoLines[] {
		{-1,  0},
		{ 1,  0},
		{ 0, -1},
		{ 0,  1},
	};
	glBindBuffer(GL_ARRAY_BUFFER, info_vbos[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof infoLines, infoLines, GL_STATIC_DRAW);

	// create buffers
	GLuint vao;
	GLuint vbos[1];
	glGenBuffers(1, vbos);
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// unbind vao
	glBindVertexArray(0);

	// time management for main loop (max 9223372036854775808 milliseconds)
	using namespace std::chrono_literals;
	using clock = std::chrono::steady_clock;
	auto startTime = clock::now();
	auto totalDeltaTime = 0ms;

	// variables
	float scale = 1.f;
	glm::mat4 mvp;
	ring<float> data(1024);

	// main loop
	while (!glfwWindowShouldClose(window)) {
		auto deltaTime = std::chrono::duration_cast<std::chrono::milliseconds>(
			clock::now() - (startTime + totalDeltaTime));
		totalDeltaTime += deltaTime;

		// generate data
		data.push(
			std::sin(totalDeltaTime.count() / 1000.f * pi() * 2.f) * 0.5
			+ std::sin(totalDeltaTime.count() / 778.f * pi() * 2.f) * 0.2
			+ std::sin(totalDeltaTime.count() / 4431.f * pi() * 2.f) * 0.3
			+ (int(totalDeltaTime.count() / 143.f) % 2 == 0 ? 0.1 : -0.1)
			+ (totalDeltaTime.count() % 333 - 333/2.f) / 10000.f
		);

		// handle input
		if (glfwGetKey(window, GLFW_KEY_S))
			scale = scale * 1.05;
		if (glfwGetKey(window, GLFW_KEY_W))
			scale = scale / 1.05;

		// update viewport
		int width, height;
		glfwGetWindowSize(window, &width, &height);
		glViewport(0, 0, width, height);

		// clear
		glClear(GL_COLOR_BUFFER_BIT);

		// render info
		mvp = glm::ortho<float>(-1, 1, -1, 1);
		glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &mvp[0][0]);
		glUniform4f(colorLocation, 0, .2f, 0, 1);
		glBindVertexArray(info_vao);
		glDrawArrays(GL_LINES, 0, 4);
		glBindVertexArray(0);

		// generate data
		const auto numSamples = data.size();
		glm::vec2 positions[numSamples];
		for (std::size_t i = 0; i < numSamples; i++) {
			positions[i] = glm::vec2(
				(1.f + i - numSamples) / 512.f,
				data[i]);
		}
		glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof positions, positions, GL_STATIC_DRAW);

		// render data
		mvp = glm::ortho<float>(-1 * scale, 0, -1, 1);
		glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, &mvp[0][0]);
		glUniform4f(colorLocation, 1, 0, 0, 1);
		glBindVertexArray(vao);
		glDrawArrays(GL_LINE_STRIP, 0, numSamples);
		glBindVertexArray(0);

		// display
		glfwSwapBuffers(window);

		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_Q))
			glfwSetWindowShouldClose(window, true);
	}

	// terminate
	glDeleteVertexArrays(1, &info_vao);
	glDeleteBuffers(1, info_vbos);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, vbos);
	glDeleteProgram(program);
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
