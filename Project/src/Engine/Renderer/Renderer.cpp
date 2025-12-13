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
#include "Engine/Renderer/RenderableProvider//MeshRenderableProvider.h"
#include "Engine/Renderer/BatchBuilder.h"
#include "Engine/Renderer/Culling/Frustum.h"

#include <iostream>
#include <algorithm>

// temporary variables, functions and objects for testing
std::vector<Renderable> tempRenderables;
std::vector<Renderable> boundingBoxes;
bool showBoundingBoxes = false;
#define ASTEROID_COUNT 1000
#define INVERSE_LIGHT_INTENSITY 0.05f

LightingUBO light = LightingUBO{
		glm::aligned_vec4(1.0f, 1.0f, 1.0f, 0.0f),
		glm::fixed_vec3(1.0f, 1.0f, 1.0f),
		0.05f,
		glm::fixed_vec3(1.0f, 0.09f * INVERSE_LIGHT_INTENSITY, 0.032f * INVERSE_LIGHT_INTENSITY)
};

//hardcoded camera related variables
#include "Engine/InputManager.h"
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, -3.0f);
float cameraSpeed = 2.5f;
float sensitivity = 0.1f;
float yaw = 90.0f, pitch = 0.0f;
float nearPlane = 0.1f, farPlane = 10000.0f;
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

glm::mat4 GetPerspectiveMatrix() {
	auto& win = AppAttorney::GetWindow(App::Get());
	return glm::infinitePerspective(glm::radians(90.0f), win.GetAspectRatio(), nearPlane);
}

glm::mat4 GetViewMatrix() {
	return glm::lookAt(cameraPos, cameraPos + front, up);
}

// light related functions

void GetCascadeSplits(float nearPlane, float farPlane, int cascadeCount, float lambda, fixed_float outSplits[]) {
	float ratio = farPlane / nearPlane;
	for (int i = 0; i < cascadeCount; i++) {
		float p = (i + 1) / static_cast<float>(cascadeCount);
		float log = nearPlane * std::pow(ratio, p);
		float uniform = nearPlane + (farPlane - nearPlane) * p;
		outSplits[i] = lambda * (log - uniform) + uniform;
	}
}

fixed_float cascadeSplits[6];

glm::vec3 GetLightDirection(LightingUBO lightInfo) {
	if (lightInfo.lightPos.w) return glm::vec3(0.0f); // point light has no direction
	return -glm::normalize(glm::vec3(lightInfo.lightPos));
}

glm::mat4 ComputeDirectionalLightMatrix(const glm::vec3& lightDir) {

	//TODO: Choose proper center and ortho bounds based on scene contents
	glm::vec3 center = cameraPos;
	glm::vec3 pos = center - lightDir * 25.0f; // Push the camera "behind" the scene

	glm::mat4 view = glm::lookAt(pos, center, glm::vec3(0.0f, 1.0f, 0.0f));

	// Ortho bounds (tweak depending on scene)
	float size = 20.0f;
	glm::mat4 proj = glm::ortho(-size, size, -size, size, 1.0f, 100.0f);

	return proj * view;
}

// Computes light-space matrices for each cascade
void ComputeDirectionalLightCascades(
	const glm::vec3& lightDir,
	const glm::mat4& cameraView,
	float fovDegrees,
	float aspectRatio,
	float nearPlane,
	float farPlane,
	int cascadeCount,
	const fixed_float cascadeSplits[],
	glm::mat4 outLightMatrices[]
)
{
	// Inverse camera view for frustum corner generation
	glm::mat4 invView = glm::inverse(cameraView);

	// Normalize light direction
	glm::vec3 lightDirection = glm::normalize(lightDir);

	float prevSplitDist = nearPlane;

	for (int cascade = 0; cascade < cascadeCount; cascade++)
	{
		float splitDist = cascadeSplits[cascade];

		// --- 1. Compute frustum corners in view space ---
		float nearDist = prevSplitDist;
		float farDist = splitDist;

		float tanHalfFov = tanf(glm::radians(fovDegrees * 0.5f));
		float nearHeight = nearDist * tanHalfFov;
		float nearWidth = nearHeight * aspectRatio;
		float farHeight = farDist * tanHalfFov;
		float farWidth = farHeight * aspectRatio;

		glm::vec3 frustumCornersVS[8] =
		{
			// near plane
			{ -nearWidth,  nearHeight, -nearDist },
			{  nearWidth,  nearHeight, -nearDist },
			{  nearWidth, -nearHeight, -nearDist },
			{ -nearWidth, -nearHeight, -nearDist },

			// far plane
			{ -farWidth,   farHeight,  -farDist },
			{  farWidth,   farHeight,  -farDist },
			{  farWidth,  -farHeight,  -farDist },
			{ -farWidth,  -farHeight,  -farDist }
		};

		// --- 2. Transform frustum corners to world space ---
		glm::vec3 frustumCornersWS[8];
		for (int i = 0; i < 8; i++)
		{
			glm::vec4 world = invView * glm::vec4(frustumCornersVS[i], 1.0f);
			frustumCornersWS[i] = glm::vec3(world);
		}

		// --- 3. Compute frustum center ---
		glm::vec3 center(0.0f);
		for (int i = 0; i < 8; i++)
			center += frustumCornersWS[i];
		center /= 8.0f;

		// --- 4. Build light view matrix ---
		float lightDistance = 100.0f;
		glm::vec3 lightPos = center - lightDirection * lightDistance;

		glm::mat4 lightView = glm::lookAt(
			lightPos,
			center,
			glm::vec3(0.0f, 1.0f, 0.0f)
		);

		// --- 5. Compute ortho bounds in light space ---
		glm::vec3 minLS(FLT_MAX);
		glm::vec3 maxLS(-FLT_MAX);

		for (int i = 0; i < 8; i++)
		{
			glm::vec3 cornerLS = glm::vec3(lightView * glm::vec4(frustumCornersWS[i], 1.0f));
			minLS = glm::min(minLS, cornerLS);
			maxLS = glm::max(maxLS, cornerLS);
		}

		const float depthPadding = 1000.0f;

		minLS.z -= depthPadding;
		maxLS.z += depthPadding;

		// stabilization
		float shadowMapRes = 2048.0f;
		glm::vec2 texelSize = (glm::vec2(maxLS) - glm::vec2(minLS)) / shadowMapRes;

		minLS.x = floor(minLS.x / texelSize.x) * texelSize.x;
		minLS.y = floor(minLS.y / texelSize.y) * texelSize.y;
		maxLS.x = minLS.x + texelSize.x * shadowMapRes;
		maxLS.y = minLS.y + texelSize.y * shadowMapRes;

		glm::mat4 lightProj = glm::ortho(
			minLS.x, maxLS.x,
			minLS.y, maxLS.y,
			-maxLS.z, -minLS.z
		);

		outLightMatrices[cascade] = lightProj * lightView;

		prevSplitDist = splitDist;
	}
}

void ComputePointLightMatrices(
	const glm::vec3& lightPos,
	float nearPlane,
	float farPlane,
	glm::mat4 outMatrices[6]
)
{
	glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, farPlane);

	outMatrices[0] = proj * glm::lookAt(lightPos, lightPos + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
	outMatrices[1] = proj * glm::lookAt(lightPos, lightPos + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
	outMatrices[2] = proj * glm::lookAt(lightPos, lightPos + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
	outMatrices[3] = proj * glm::lookAt(lightPos, lightPos + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
	outMatrices[4] = proj * glm::lookAt(lightPos, lightPos + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
	outMatrices[5] = proj * glm::lookAt(lightPos, lightPos + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
}

float ComputePointLightFarPlane(const glm::vec3& attenuation)
{
	float c = attenuation.x;
	float b = attenuation.y;
	float a = attenuation.z;

	float maxBrightness = 1.0f;
	float threshold = 0.01f; // 1%

	// Solve quadratic eq: a*d^2 + b*d + (c - maxBrightness/threshold) = 0
	float c2 = c - (maxBrightness / threshold); // drop-off at 1%
	float discriminant = b * b - 4 * a * c2;

	if (a == 0 || discriminant < 0)
		return 25.0f; // fallback

	float d = (-b + sqrt(discriminant)) / (2 * a);
	return std::max(d, 1.0f);
}

#define REN_DELTA_TIME() ((float)App::Get().DeltaTime())

float angle = 0.0f;

void Renderer::Initialize(void)
{
	auto& _rm = ResourceManager::Get();

	auto writer = _rm.ubos.GetUboWriter("Lighting");
	writer->SetBlock(light);
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
	for (int i = 0; i < ASTEROID_COUNT; i++) {
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
	{
		MeshRenderableProvider meshProvider;
		meshProvider.meshHandle = _rm.meshes.GetHandle("primitive/quad");
		meshProvider.materialHandle = _rm.materials.GetHandle("matte");
		meshProvider.transform = {
			glm::vec3(-20.0f, 0.0f, 0.0f),
			glm::quat(glm::vec3(0.0f, glm::radians(-90.0f), 0.0f)),
			glm::vec3(100.0f, 100.0f, 1.0f)
		};
		meshProvider.GenerateRenderables(tempRenderables);
	}
}
void Renderer::RenderFunction(void)
{
	auto& win = AppAttorney::GetWindow(App::Get());
	// update Camera ubo
	glm::mat4 projection = GetPerspectiveMatrix();
	glm::mat4 view = GetViewMatrix();
	auto* cameraWriter = _rm.ubos.GetUboWriter("Camera");
	cameraWriter->SetBlock(CameraUBO{
		cameraPos,
		view,
		projection
		});
	cameraWriter->Upload();

	Frustum frustrum(projection * view);
	renderQueue.SetViewFrustum(frustrum);

	auto& _rm = ResourceManager::Get();
	angle += 90.f * App::Get().DeltaTime();

	for(size_t i = 0; i < ASTEROID_COUNT; i++)
	{
		auto& r = tempRenderables[i];
		// make the ring of asteroids slowly rotate, based on their distance from center
		float distance = glm::length(glm::vec2(r.transform.position.x, r.transform.position.z));
		float rotationSpeed = 100.0f / (distance * distance); // closer asteroids rotate faster
		// update position based on rotation around Y axis
		float currentAngle = glm::degrees(atan2(r.transform.position.z, r.transform.position.x));
		float deltaAngle = rotationSpeed * App::Get().DeltaTime();
		currentAngle += deltaAngle;
		r.transform.position.x = cos(glm::radians(currentAngle)) * distance;
		r.transform.position.z = sin(glm::radians(currentAngle)) * distance;

		// rotate each asteroid on y axis, matching ring rotation speed
		glm::quat deltaQuat = glm::quat(glm::vec3(0.0f, glm::radians(-deltaAngle), 0.0f));
		r.transform.rotation = deltaQuat * r.transform.rotation;
		
	}

	// in boundingBoxes generate bounding boxes using the bounding_box mesh for each renderable
	if(showBoundingBoxes){
		boundingBoxes.clear();
		MeshRenderableProvider meshProvider;
		meshProvider.meshHandle = _rm.meshes.GetHandle("primitive/bounding_box");
		meshProvider.materialHandle = _rm.materials.GetHandle("boundingBox");
		for (size_t i = 0; i < tempRenderables.size(); i++) {
			auto& r = tempRenderables[i];
			if (!r.hasBounds) continue;

			glm::mat4 modelMatrix = r.transform.GetModelMatrix();
			BoundingBox transformedAABB = TransformAABB(r.aabb, modelMatrix);

			meshProvider.transform.position = (transformedAABB.min + transformedAABB.max) * 0.5f;

			meshProvider.transform.scale = transformedAABB.max - transformedAABB.min;

			meshProvider.GenerateRenderables(boundingBoxes);
		}
		renderQueue.Push(boundingBoxes);
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

	_im.BindMouseScroll([](double xoffset, double yoffset) {
		double scale = glm::pow(1.1, yoffset);
		cameraSpeed *= (float)scale;
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

	// --------------------------------
	_im.BindKey(GLFW_KEY_B, InputEventType::Pressed, []() {
		showBoundingBoxes = !showBoundingBoxes;
		});

	// --------------------------------

	GetCascadeSplits(nearPlane, farPlane, 6, 1, cascadeSplits);

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
	// === Shadow Pass ===
	auto batchedShadowCasters = BatchBuilder::Build(renderQueue.GetShadowCasters());
	ShadowUBO shadowData;
	shadowData.lightPos = light.lightPos;
	shadowData.cascadedSplits[0] = ComputePointLightFarPlane(glm::vec3(light.attenuationFactor));
	std::string shaderName;
	bool isPointLight = false;
	if(light.lightPos.w) // point light
	{
		isPointLight = true;
		ComputePointLightMatrices(
			glm::vec3(light.lightPos),
			0.1f,
			shadowData.cascadedSplits[0],
			shadowData.lightSpaceMatrix
		);
		auto* shadowWriter = _rm.ubos.GetUboWriter("Shadow");
		shadowWriter->SetBlock(shadowData);
		shadowWriter->Upload();

		pointShadowFBO.BindForWriting();
		shaderName = "pointShadow";
	}
	else // directional light
	{
		glm::vec3 lightDir = GetLightDirection(light);
		auto& win = AppAttorney::GetWindow(App::Get());

		ComputeDirectionalLightCascades(
			lightDir,
			GetViewMatrix(),
			90.0f,
			win.GetAspectRatio(),
			nearPlane,
			farPlane,
			6,
			cascadeSplits,
			shadowData.lightSpaceMatrix
		);
		memcpy(shadowData.cascadedSplits, cascadeSplits, sizeof(fixed_float) * 6);
		auto* shadowWriter = _rm.ubos.GetUboWriter("Shadow");
		shadowWriter->SetBlock(shadowData);
		shadowWriter->Upload();

		dirShadowFBO.BindForWriting();
		shaderName = "dirShadow";
	}
	_rm.shaders.UseShader(shaderName, &glState);

	for (auto& submission : batchedShadowCasters)
	{
		DrawShadowSubmission(submission);
	}


	ShadowFramebuffer::Unbind(glState);
	auto& win = AppAttorney::GetWindow(App::Get());
	win.ResetViewport();
	// === Main Pass ===
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

	//check and set face culling
	if(glState.cullBackfaces != submission.item.cullBackfaces)
	{
		glState.cullBackfaces = submission.item.cullBackfaces;
		if(glState.cullBackfaces)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
	}

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

void Renderer::DrawShadowSubmission(const RenderSubmission& submission)
{
	Mesh* mesh = nullptr;
	if (submission.item.mesh)
		mesh = submission.item.mesh;
	else
		mesh = _rm.meshes.Get(submission.item.meshHandle);
	if (!mesh) return;

	//face culling is always enabled for shadow pass
	// no material application needed for shadow pass

	if ((submission.item.meshHandle.IsValid() && glState.currentMesh != submission.item.meshHandle) ||
		(glState.currentDynamicMesh != submission.item.mesh)) {
		mesh->Bind();
		glState.currentMesh = submission.item.meshHandle;
		glState.currentDynamicMesh = submission.item.mesh;
	}

	GLenum primitive = submission.item.primitive != 0 ? submission.item.primitive : mesh->primitive;
	if (submission.item.instanceData)
	{
		if (mesh->isInstancingEnabled() == false)
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
