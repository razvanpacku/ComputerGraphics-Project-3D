#include "Engine/Renderer.h"

#include "Engine/App.h"
#include "Engine/Window.h"
#include "Engine/Resources/ResourceManager.h"

//glm for transformations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

// temporary functions

GLuint
VaoId,
VboId;

ShaderManager::Handle shaderHandle;
TextureManager::Handle textureHandle;

float angle = 0.0f;

void Renderer::CreateVBO(void)
{
	// vertices 
	GLfloat Vertices[] = {
		//Positions								Colors						TextureCoords
		0.0f,  1.0f, 0.0f, 1.0f,				1.0f, 0.0f, 0.0f, 1.0f,		0.0f, 1.0f,
		0.866025f, -0.5f, 0.0f, 1.0f,			0.0f, 1.0f, 0.0f, 1.0f,		1.0f, 0.0f,
		-0.866025f, -0.5f, 0.0f, 1.0f,			0.0f, 0.0f, 1.0f, 1.0f,		0.0f, 0.0f,
	};

	const GLsizei floatsPerVertex = 10; // 4 position + 4 color + 2 texture coords
	const GLsizei stride = floatsPerVertex * sizeof(GLfloat);

	// create VBO and upload interleaved data
	glGenBuffers(1, &VboId);
	glBindBuffer(GL_ARRAY_BUFFER, VboId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	// create and bind VAO
	glGenVertexArrays(1, &VaoId);
	glBindVertexArray(VaoId);
	// attribute 0  = position (4 floats)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, stride, (void*)0);

	// attribute 1  = color (4 floats)
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(4*sizeof(GLfloat)));

	// attribute 2  = texture coords (2 floats)
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(GLfloat)));
}
void Renderer::DestroyVBO(void)
{
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VboId);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VaoId);
}

void Renderer::CreateShaders(void)
{
	auto& _rm = ResourceManager::Get();
	shaderHandle = _rm.shaders.Load(
		"SimpleShader",
		ShaderResourceInfo{
			"resources/shaders/example/example.vert",
			"resources/shaders/example/example.frag"
		});
	
	_rm.shaders.UseShader(shaderHandle);
}
void Renderer::DestroyShaders(void)
{
	auto& _rm = ResourceManager::Get();
	_rm.shaders.Remove(shaderHandle);
}

void Renderer::Initialize(void)
{
	auto& _rm = ResourceManager::Get();
	CreateVBO();
	CreateShaders();

	textureHandle = _rm.textures.Load(
		"ExampleTexture",
		TextureResourceInfo{
			"resources/textures/dev.png",
			true
		});
}
void Renderer::RenderFunction(void)
{
	auto& _rm = ResourceManager::Get();
	angle += 1.f * App::Get().DeltaTime();

	auto& win = AppAttorney::GetWindow(App::Get());
	double ratio = 1.0;
	if(win.IsFullscreen())
	{
		ratio = (double)win.GetMonitorWidth() / (double)win.GetMonitorHeight();
	}
	else
	{
		ratio = (double)win.GetWidth() / (double)win.GetHeight();
	}

	glm::mat4 view = glm::ortho(-ratio, ratio, -1., 1.);
	
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 mat = view * rotation;

	auto shader = _rm.shaders.Get(shaderHandle);
	shader->SetUniform("rotation", mat);
	//Binding texture to uniform
	BindTextureToUniform("Texture", _rm.textures.GetHandle("ExampleTexture"), shader);
	//Drawing function
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glFlush();
}

void Renderer::Cleanup(void)
{
	DestroyShaders();
	DestroyVBO();
}

Renderer::Renderer(App& app) : app(app), _rm(ResourceManager::Get())
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

void Renderer::BindTextureToUniform(const char* uniform, TextureManager::Handle h, Shader* shader)
{
	if (!h.IsValid() || shader == nullptr) return;

    _rm.textures.Bind(h);
    int unit = _rm.textures.GetBoundUnit(h);
    shader->SetUniform(uniform, unit);
}
