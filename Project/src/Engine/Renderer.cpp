#include "Engine/Renderer.h"

#include "Engine/App.h"

// temporary functions

GLuint
VaoId,
VboId,
ColorBufferId,
ProgramId;

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
	CreateVBO();
	CreateShaders();
}
void RenderFunction(void)
{

	//Drawing function
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glFlush();
}

void Cleanup(void)
{
	DestroyShaders();
	DestroyVBO();
}

Renderer::Renderer(App& app) : app(app)
{
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glClearColor(DEFAULT_CLEAR_COLOR_R, DEFAULT_CLEAR_COLOR_G, DEFAULT_CLEAR_COLOR_B, 1.0f);

	Initialize();
}

Renderer::~Renderer()
{
	Cleanup();
}

void Renderer::Render()
{
	Clear();

	RenderFunction();
}

void Renderer::Clear() const
{
	glClearColor(DEFAULT_CLEAR_COLOR_R, DEFAULT_CLEAR_COLOR_G, DEFAULT_CLEAR_COLOR_B, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
