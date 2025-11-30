#include <iostream>
#include <glad/glad.h>	
#include <GLFW/glfw3.h>
#include "Util/loadShaders.h"

//////////////////////////////////////

GLuint
VaoId,
VboId,
ColorBufferId,
ProgramId;

GLFWwindow* window;

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 600

void CreateVBO(void)
{
	// vertices 
	GLfloat Vertices[] = {
		0.0f,  0.75f, 0.0f, 1.0f,
		0.866025f, -0.75f, 0.0f, 1.0f,
		-0.866025f, -0.75f, 0.0f, 1.0f,
	};

	// colors of the vertices
	GLfloat Colors[] = {
	  1.0f, 0.0f, 0.0f, 1.0f,
	  0.0f, 1.0f, 0.0f, 1.0f,
	  0.0f, 0.0f, 1.0f, 1.0f,
	};

	// create new buffer id
	glGenBuffers(1, &VboId);
	// set buffer as the current one
	glBindBuffer(GL_ARRAY_BUFFER, VboId);
	// the vertices are copied into the buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	// a VAO is created
	glGenVertexArrays(1, &VaoId);
	glBindVertexArray(VaoId);
	// set attribute 0 (position) as current
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	// new buffer for colors
	glGenBuffers(1, &ColorBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, ColorBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Colors), Colors, GL_STATIC_DRAW);
	// set attribute 1 (color) as current
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
}
void DestroyVBO(void)
{
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &ColorBufferId);
	glDeleteBuffers(1, &VboId);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VaoId);
}

void CreateShaders(void)
{
	ProgramId = LoadShaders("resources/shaders/example/example.vert", "resources/shaders/example/example.frag");
	glUseProgram(ProgramId);
}
void DestroyShaders(void)
{
	glDeleteProgram(ProgramId);
}

void Initialize(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //background color
	CreateVBO();
	CreateShaders();
}
void RenderFunction(void)
{
	glClear(GL_COLOR_BUFFER_BIT);       

	//Drawing function
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glFlush();
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	RenderFunction();
	glfwSwapBuffers(window);
}

void Cleanup(void)
{
	DestroyShaders();
	DestroyVBO();
}

int main(int argc, char* argv[])
{

	// Initialize GLFW
	if (!glfwInit()) {
		std::cerr << "Failed to init GLFW\n";
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL RGB Triangle", nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create GLFW window\n";
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// Load OpenGL functions with GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD\n";
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	Initialize();

	while (!glfwWindowShouldClose(window)) {
		// input
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		// render
		RenderFunction();

		// swap + poll
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	Cleanup();
	glfwDestroyWindow(window);
	glfwTerminate();
}

