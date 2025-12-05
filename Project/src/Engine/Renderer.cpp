#include "Engine/Renderer.h"

#include "Engine/App.h"
#include "Engine/Window.h"
#include "Engine/Resources/ResourceManager.h"

//glm for transformations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/type_aligned.hpp>

#include <iostream>

// temporary variables, functions and objects for testing

GLuint
VaoId,
VboId;

ShaderManager::Handle shaderHandle;
TextureManager::Handle textureHandle;
MaterialManager::Handle materialHandle;
MeshManager::Handle meshHandle;

struct alignas(16) Matrices {
	glm::aligned_mat4 model;
	glm::aligned_mat4 view;
	glm::aligned_mat4 projection;
};

//hardcoded camera related variables
#include "Engine/InputManager.h"
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, -3.0f);
float cameraSpeed = 2.5f;
float sensitivity = 0.1f;
float yaw = 90.0f, pitch = 0.0f;
glm::vec3 front;
glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 right;
glm::vec3 up;

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

void ProcessCameraMovement(Camera_Movement dir, float deltaTime) {
	float velocity = cameraSpeed * deltaTime;
	glm::vec3 direction(0.0f);

    if (dir == FORWARD)  direction += front;
    if (dir == BACKWARD) direction -= front;
    if (dir == LEFT)     direction -= right;
    if (dir == RIGHT)    direction += right;
    if (dir == UP)       direction += up;
    if (dir == DOWN)     direction -= up;

	direction = glm::normalize(direction);
	cameraPos += direction * velocity;
}

void UpdateCameraVectors() {
	glm::vec3 newFront;
	newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	newFront.y = sin(glm::radians(pitch));
	newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(newFront);
	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}

glm::mat4 GetViewMatrix() {
	return glm::lookAt(cameraPos, cameraPos + front, up);
}

#define REN_DELTA_TIME() ((float)App::Get().DeltaTime())

float angle = 0.0f;

void Renderer::CreateVBO(void)
{

	struct Vertex {
		glm::vec4 position;
		glm::vec2 texCoords;
		glm::vec3 normal;
	};

	/*
	std::vector<Vertex> vertices = {
		{ glm::vec4(0.0f,  1.0f, 0.0f, 1.0f),		glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),	glm::vec2(0.0f, 1.0f) },
		{ glm::vec4(0.866025f, -0.5f, 0.0f, 1.0f),	glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),	glm::vec2(1.0f, 0.0f) },
		{ glm::vec4(-0.866025f, -0.5f, 0.0f, 1.0f),	glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),	glm::vec2(0.0f, 0.0f) },
	};
	std::vector<GLuint> indices = { 0, 1, 2 };
	*/

	//square
	/*
	std::vector<Vertex> vertices = {
		{ glm::vec4(-0.5f,  0.5f, 0.0f, 1.0f),		glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),	glm::vec2(0.0f, 1.0f) },
		{ glm::vec4(0.5f,  0.5f, 0.0f, 1.0f),		glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),	glm::vec2(1.0f, 1.0f) },
		{ glm::vec4(0.5f, -0.5f, 0.0f, 1.0f),		glm::vec4(0.0f, 0.0f, 1.0f, 1.0f),	glm::vec2(1.0f, 0.0f) },
		{ glm::vec4(-0.5f, -0.5f, 0.0f, 1.0f),	glm::vec4(1.0f, 1.0f, 0.0f, 1.0f),	glm::vec2(0.0f, 0.0f) },
	};
	std::vector<uint32_t> indices = {
		0, 1, 2,
		2, 3, 0
	};
	*/
	//cube
	std::vector<Vertex> vertices = {
		// Front face
		{ glm::vec4(-0.5f, -0.5f,  0.5f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)},
		{ glm::vec4( 0.5f, -0.5f,  0.5f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f) },
		{ glm::vec4( 0.5f,  0.5f,  0.5f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f) },
		{ glm::vec4(-0.5f,  0.5f,  0.5f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f) },
		// Back face
		{ glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f) },
		{ glm::vec4( 0.5f, -0.5f, -0.5f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f) },
		{ glm::vec4( 0.5f,  0.5f, -0.5f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f) },
		{ glm::vec4(-0.5f,  0.5f, -0.5f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f) },
		// Right face
		{ glm::vec4(0.5f, -0.5f, -0.5f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f) },
		{ glm::vec4(0.5f, -0.5f,  0.5f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f) },
		{ glm::vec4(0.5f,  0.5f,  0.5f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f) },
		{ glm::vec4(0.5f,  0.5f, -0.5f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f) },
		// Left face
		{ glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f) },
		{ glm::vec4(-0.5f, -0.5f,  0.5f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f) },
		{ glm::vec4(-0.5f,  0.5f,  0.5f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f) },
		{ glm::vec4(-0.5f,  0.5f, -0.5f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(-1.0f, 0.0f, 0.0f) },
		// Top face
		{ glm::vec4(-0.5f,  0.5f,  0.5f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f) },
		{ glm::vec4( 0.5f,  0.5f,  0.5f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f) },
		{ glm::vec4( 0.5f,  0.5f, -0.5f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f) },
		{ glm::vec4(-0.5f,  0.5f, -0.5f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)},
		// Bottom face
		{ glm::vec4(-0.5f, -0.5f,  0.5f, 1.0f), glm::vec2(1.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f) },
		{ glm::vec4( 0.5f, -0.5f,  0.5f, 1.0f), glm::vec2(0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f) },
		{ glm::vec4( 0.5f, -0.5f, -0.5f, 1.0f), glm::vec2(0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f) },
		{ glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f), glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f) }};
	std::vector<uint32_t> indices = {
		// Front face
		0, 1, 2, 2, 3, 0,
		// Back face
		4, 5, 6, 6, 7, 4,
		// Right face
		8, 9,10,10,11, 8,
		// Left face
	   12,13,14,14,15,12,
		// Top face
	   16,17,18,18,19,16,
		// Bottom face
	   20,21,22,22,23,20
	};

	std::vector<uint8_t> vertexBytes(
		reinterpret_cast<uint8_t*>(vertices.data()),
		reinterpret_cast<uint8_t*>(vertices.data()) +
		vertices.size() * sizeof(Vertex)
	);
	std::vector<VertexAttribute> attributes = {
		{ 0, 4, GL_FLOAT, offsetof(Vertex, position), GL_FALSE },
		{ 1, 2, GL_FLOAT, offsetof(Vertex, texCoords), GL_FALSE },
		{ 2, 3, GL_FLOAT, offsetof(Vertex, normal), GL_FALSE },
	};

	meshHandle = _rm.meshes.Load(
		"Cube",
		MeshResoruceInfo{
			vertexBytes,
			indices,
			attributes,
			sizeof(Vertex)
		});
}
void Renderer::DestroyVBO(void)
{
	_rm.meshes.Remove(meshHandle);
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

	materialHandle = _rm.materials.Load(
		"SimpleMaterial",
		MaterialResourceInfo{
			"SimpleShader"
		});

	auto* material = _rm.materials.Get(materialHandle);
	material->SetTexture("Texture", textureHandle);

	struct alignas(16) TestBlock
	{
		glm::aligned_mat4 first;
		glm::aligned_vec3 testColor[2];
	};

	TestBlock testBlockData = { glm::mat4(1.0f), {glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)} };
	auto writer = material->GetLocalUboWriter("Test");
	writer->SetBlock(testBlockData);
	writer->Upload();
}
void Renderer::RenderFunction(void)
{
	auto& _rm = ResourceManager::Get();
	angle += 90.f * App::Get().DeltaTime();

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
	glm::mat4 projection = glm::infinitePerspective(glm::radians(90.0f), (float)ratio, 0.1f);
	glm::mat4 view = GetViewMatrix();
	glm::mat4 model = glm::rotate(
		glm::mat4(1.0f),
		glm::radians(angle),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);

	Matrices matrices = {
		model,
		view,
		projection
	};

	auto* matricesWriter = _rm.ubos.GetUboWriter("Matrices");
	matricesWriter->SetBlock(matrices);
	matricesWriter->Upload();
	auto* material = _rm.materials.Get(materialHandle);
	material->SetUniform("viewPos", cameraPos);
	material->Apply();

	//Drawing function
	auto* mesh = _rm.meshes.Get(meshHandle);
	mesh->Bind();
	glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);

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

	// temporary hardcoded camera movement
	UpdateCameraVectors();

	auto& _im = InputManager::Get();

	_im.BindMouseDelta([](double dx, double dy) {
		auto& window = AppAttorney::GetWindow(App::Get());
		if( window.GetMouseMode() != MouseMode::Disabled )
			return;

		if (dx != 0 || dy != 0) {
			yaw += (float)dx * sensitivity;
			pitch -= (float)dy * sensitivity;
			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;
		}
		UpdateCameraVectors();
		});

	_im.BindKey(GLFW_KEY_W, InputEventType::Held, []() {
		ProcessCameraMovement(FORWARD, REN_DELTA_TIME());
		});

	_im.BindKey(GLFW_KEY_S, InputEventType::Held, []() {
		ProcessCameraMovement(BACKWARD, REN_DELTA_TIME());
		});

	_im.BindKey(GLFW_KEY_A, InputEventType::Held, []() {
		ProcessCameraMovement(LEFT, REN_DELTA_TIME());
		});

	_im.BindKey(GLFW_KEY_D, InputEventType::Held, []() {
		ProcessCameraMovement(RIGHT, REN_DELTA_TIME());
		});

	_im.BindKey(GLFW_KEY_SPACE, InputEventType::Held, []() {
		ProcessCameraMovement(UP, REN_DELTA_TIME());
		});

	_im.BindKey(GLFW_KEY_LEFT_SHIFT, InputEventType::Held, []() {
		ProcessCameraMovement(DOWN, REN_DELTA_TIME());
		});


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
    shader->Set(uniform, unit);
}
