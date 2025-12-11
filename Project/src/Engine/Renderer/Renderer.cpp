#include "Engine/Renderer/Renderer.h"

#include "Engine/App.h"
#include "Engine/Window.h"
#include "Engine/Resources/ResourceManager.h"

//glm for transformations
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/type_aligned.hpp>

#include "Engine/Resources/UboDefs.h"

#include "Engine/Renderer/RenderableProvider//ModelRenderableProvider.h"
#include "Engine/Renderer/BatchBuilder.h"

#include <iostream>
#include <algorithm>

// temporary variables, functions and objects for testing
std::vector<Renderable> tempRenderables;

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
		0.05f,
		glm::fixed_vec3(1.0f, 0.09f, 0.032f)
		}));
	writer->Upload();

	auto* modell = _rm.models.Get("asteroid");
	Transform t = {
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::quat(glm::vec3(0.0f, glm::radians(angle), 0.0f)),
		glm::vec3(1.f)
	};
	ModelRenderableProvider modelProvider;
	modelProvider.model = modell;
	modelProvider.transform = t;
	modelProvider.GenerateRenderables(tempRenderables);

	// this model will generate only one renderable
	// fill tempRenderables with copies of it for testing, with different positions
	for (int i = 0; i < 1000; i++) {
		Renderable r = tempRenderables[0];
		//r.transform.position = glm::vec3((i % 10) * 2 - 9.0f, -1.0f, (i / 10) * 2 - 9.0f);

		//give each ateroid a random position similar to a asteroid ring
		float angle = (float)(rand() % 360);
		float distance = 5.0f + (float)(rand() % 1000) / 100.0f; // between 5.0 and 15.0
		r.transform.position = glm::vec3(
			cos(glm::radians(angle)) * distance,
			((float)(rand() % 200) / 100.0f) - 1.0f, // between -1.0 and 1.0
			sin(glm::radians(angle)) * distance
		);

		// random rotation
		r.transform.rotation = glm::quat(glm::vec3(
			glm::radians((float)(rand() % 360)),
			glm::radians((float)(rand() % 360)),
			glm::radians((float)(rand() % 360))
		));

		// give each asteroid slight random scaling variation
		r.transform.scale = glm::vec3(0.1f + (float)(rand() % 100) / 500.0f);

		if(i) tempRenderables.push_back(r);
		else tempRenderables[0] = r; // first one is already in the vector
	}

	modelProvider.model = _rm.models.Get("rocket");
	modelProvider.transform = {
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::quat(glm::vec3(0.0f, 0.0f, 0.0f)),
		glm::vec3(0.2f)
	};
	modelProvider.GenerateRenderables(tempRenderables);
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
	/*
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
	*/

	auto* cameraWriter = _rm.ubos.GetUboWriter("Camera");
	cameraWriter->SetBlock(CameraUBO{
		cameraPos,
		view,
		projection
		});
	cameraWriter->Upload();

	/*
	auto* modell = _rm.models.Get("rocket");
	Transform t = {
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::quat(glm::vec3(0.0f, glm::radians(angle), 0.0f)),
		glm::vec3(0.2f)
	};
	ModelRenderableProvider modelProvider;
	modelProvider.model = modell;
	modelProvider.transform = t;
	std::vector<Renderable> renderables;
	modelProvider.GenerateRenderables(renderables);

	renderQueue.Push(renderables);
	*/
	for(size_t i = 0; i < tempRenderables.size()-1; i++)
	{
		auto& r = tempRenderables[i];
		// make the ring of asteroids slowly rotate, based on their distance from center
		float distance = glm::length(glm::vec2(r.transform.position.x, r.transform.position.z));
		float rotationSpeed = 100.0f / distance; // closer asteroids rotate faster
		// update position based on rotation around Y axis
		float currentAngle = glm::degrees(atan2(r.transform.position.z, r.transform.position.x));
		currentAngle += rotationSpeed * App::Get().DeltaTime();
		r.transform.position.x = cos(glm::radians(currentAngle)) * distance;
		r.transform.position.z = sin(glm::radians(currentAngle)) * distance;
		
	}

	renderQueue.Push(tempRenderables);
	RenderFrame();

	ClearQueue();

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

// =================================================
// New functions
// =================================================

void Renderer::RenderFrame() {
	//TODO: Frustum culling

	auto transparentList = renderQueue.GetSortedLayer(RenderLayer::Transparent);
	//sort back to front using renderable::sortDistance
	std::stable_sort(transparentList.begin(), transparentList.end(),
		[](const RenderSubmission& a, const RenderSubmission& b) {
			return a.item.sortDistance > b.item.sortDistance;
		});

	
	auto batchedOpaque = BatchBuilder::Build(renderQueue.GetSortedLayer(RenderLayer::Opaque));
	auto batchedTransparent = BatchBuilder::Build(transparentList);
	auto batchedGUI = BatchBuilder::Build(renderQueue.GetSortedLayer(RenderLayer::GUI));

	DrawList(batchedOpaque);
	DrawList(batchedTransparent);
	DrawList(batchedGUI);
	/*
	DrawList(renderQueue.GetSortedLayer(RenderLayer::Opaque));
	DrawList(transparentList);
	DrawList(renderQueue.GetSortedLayer(RenderLayer::GUI));
	*/
}

void Renderer::DrawList(const std::vector<RenderSubmission>& submissions)
{
	for (const auto& submission : submissions) {
		DrawSubmission(submission);
	}
}

void Renderer::DrawSubmission(const RenderSubmission& submission)
{
	Mesh* mesh = nullptr;
	if(submission.item.mesh)
		mesh = submission.item.mesh;
	else 
		mesh = _rm.meshes.Get(submission.item.meshHandle);
	if (!mesh) return;
	auto* material = _rm.materials.Get(submission.item.materialHandle);
	if (!material) return;

	material->Apply(&glState);

	if ((submission.item.meshHandle.IsValid() && glState.currentMesh != submission.item.meshHandle) ||
		(glState.currentDynamicMesh != submission.item.mesh)) {
			mesh->Bind();
			glState.currentMesh = submission.item.meshHandle;
			glState.currentDynamicMesh = submission.item.mesh;
	}

	GLenum primitive = submission.item.primitive != 0 ? submission.item.primitive : mesh->primitive;
	if (submission.item.instanceData)
	{
		if(mesh->isInstancingEnabled() == false)
		{
			mesh->EnableInstancing(true);
		}
		mesh->UploadInstancedData(submission.item.instanceData->modelMatrices.data(), submission.item.instanceData->count);
		glDrawElementsInstanced(primitive, mesh->indexCount, GL_UNSIGNED_INT, 0, submission.item.instanceData->count);
	}
	else {
		glDrawElements(primitive, mesh->indexCount, GL_UNSIGNED_INT, 0);
	}
}
