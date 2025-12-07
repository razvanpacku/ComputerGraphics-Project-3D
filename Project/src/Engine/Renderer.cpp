#include "Engine/Renderer.h"

#include "Engine/App.h"
#include "Engine/Window.h"
#include "Engine/Resources/ResourceManager.h"

//glm for transformations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/type_aligned.hpp>

#include "Engine/Resources/UboDefs.h"

#include <iostream>

// temporary variables, functions and objects for testing

TextureManager::Handle textureHandle;
MaterialManager::Handle materialHandle;

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

void Renderer::Initialize(void)
{
	auto& _rm = ResourceManager::Get();

	auto writer = _rm.ubos.GetUboWriter("Lighting");
	writer->SetBlock((LightingUBO{
		glm::aligned_vec4(1.0f, 1.0f, 1.0f, 0.0f),
		glm::fixed_vec3(1.0f, 1.0f, 1.0f),
		0.25f,
		glm::fixed_vec3(1.0f, 0.09f, 0.032f)
		}));
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
	) * glm::scale(glm::mat4(1.0f),
		glm::vec3(0.2f)
	);


	MatricesUBO matrices = {
		model,
		view,
		projection
	};

	auto* matricesWriter = _rm.ubos.GetUboWriter("Matrices");
	matricesWriter->SetBlock(matrices);
	matricesWriter->Upload();

	auto* cameraWriter = _rm.ubos.GetUboWriter("Camera");
	cameraWriter->SetBlock(CameraUBO{
		cameraPos
		});
	cameraWriter->Upload();

	//Drawing function
	auto* modell = _rm.models.Get("rocket");
	for(auto& meshEntry : modell->meshEntries)
	{
		auto* material = _rm.materials.Get(meshEntry.material);
		material->Apply();

		auto* mesh = _rm.meshes.Get(meshEntry.mesh);
		mesh->Bind();
		glDrawElements(GL_TRIANGLES, mesh->indexCount, GL_UNSIGNED_INT, 0);
	}

	glFlush();
}

void Renderer::Cleanup(void)
{
}

Renderer::Renderer(App& app) : app(app), _rm(ResourceManager::Get())
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

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
