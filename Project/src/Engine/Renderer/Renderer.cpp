#include "Engine/Renderer/Renderer.h"

#include "Engine/App.h"
#include "Engine/Window.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/InputManager.h"

#include "Engine/Resources/UboDefs.h"
#include "Engine/Renderer/LightMath.h"

#include "Engine/Renderer/RenderableProvider//ModelRenderableProvider.h"
#include "Engine/Renderer/RenderableProvider/MeshRenderableProvider.h"
#include "Engine/Renderer/RenderableProvider/GUIRenderableProvider.h"
#include "Engine/Renderer/RenderableProvider/TextRenderableProcider.h"
#include "Engine/Renderer/BatchBuilder.h"
#include "Engine/Renderer/Culling/Frustum.h"

// temporary camera controller until entity system is done to fetch cameraController from camera entity
#include "Engine/Controllers/FlyingCameraController.h"

#include <iostream>
#include <algorithm>

// temporary variables, functions and objects for testing
std::vector<Renderable> tempRenderables;
std::vector<Renderable> boundingBoxes;
std::vector<Renderable> fpsText;
#define ASTEROID_COUNT 1000
float angle = 0.0f;
// light related functions

Renderer::~Renderer()
{
	Cleanup();
}

void Renderer::Initialize(void)
{
	auto& _rm = ResourceManager::Get();

	renderLight.lightPos = glm::vec4(1.f, 1.f, 1.f, 0.f);
	auto writer = _rm.ubos.GetUboWriter("Lighting");
	writer->SetBlock(renderLight);
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
			glm::quat(glm::vec3(0.0f, glm::radians(90.0f), 0.0f)),
			glm::vec3(100.0f, 100.0f, 1.0f)
		};
		meshProvider.GenerateRenderables(tempRenderables);
	}
	/*
	{
		GUIRederableProvider guiProvider;
		guiProvider.materialHandle = _rm.materials.GetHandle("guiBase");
		guiProvider.textureHandle = _rm.textures.GetHandle("dev2.png");
		guiProvider.transform = {
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::quat(glm::vec3(0.0f, 0.0f, glm::radians(45.f))),
			glm::vec3(256.f, 256.f, 1.0f)
		};
		guiProvider.relativeSize = glm::vec4(0.0f);
		guiProvider.relativePosition = glm::vec2(0.0f, 1.0f);
		guiProvider.anchorPoint = glm::vec2(0.0f, 1.0f);
		guiProvider.GenerateRenderables(tempRenderables);
	}
	*/
}

void Renderer::Render()
{
	Clear();

	UpdateCameraUBOs();
	GetRenderables();
	RenderFrame();
	ClearQueue();
	glFlush();
}

void Renderer::Clear() const
{
	glClearColor(DEFAULT_CLEAR_COLOR_R, DEFAULT_CLEAR_COLOR_G, DEFAULT_CLEAR_COLOR_B, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::UpdateCameraUBOs() {
	auto& win = AppAttorney::GetWindow(App::Get());
	// update Camera ubo
	glm::mat4 projection = GetPerspectiveMatrix();
	glm::mat4 view = GetViewMatrix();
	auto* cameraWriter = _rm.ubos.GetUboWriter("Camera");
	cameraWriter->SetBlock(CameraUBO{
		renderCamera->GetPosition(),
		view,
		projection
		});
	cameraWriter->Upload();
	glm::mat4 guiView = GetGUIViewMatrix();
	auto* guiCameraWriter = _rm.ubos.GetUboWriter("GUICamera");
	guiCameraWriter->SetBlock(GUICameraUBO{
		guiView
		});
	guiCameraWriter->Upload();

	Frustum frustrum(projection * view);
	renderQueue.SetViewFrustum(frustrum);
}

void Renderer::GetRenderables() {
	auto& _rm = ResourceManager::Get();
	angle += 90.f * App::Get().DeltaTime();

	for (size_t i = 0; i < ASTEROID_COUNT; i++)
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

	{
		fpsText.clear();
		int fps = static_cast<int>(1.0f / App::Get().DeltaTime());
		TextRenderableProvier textProvider;
		std::string text = "FPS: " + std::to_string(fps);
		textProvider.text = text;
		textProvider.pixelScale = glm::vec2(128.f, 16.f);
		textProvider.rotation = glm::quat(glm::vec3(0.f));
		textProvider.anchorPoint = glm::vec2(0.5f, 1.0f);
		textProvider.relativePosition = glm::vec2(0.5f, 1.0f);
		textProvider.GenerateRenderables(fpsText);
		renderQueue.Push(fpsText);
	}

	// in boundingBoxes generate bounding boxes using the bounding_box mesh for each renderable
	if (showBoundingBoxes) {
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
}

void Renderer::RenderFrame() {
	DrawShadowPass();
	DrawMainPass();
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

	renderCamera = new FlyingCameraController({ 3.0f, 0.0f, 0.0f }, 180.0f, 0.0f);

	auto& _im = InputManager::Get();

	_im.BindKey(GLFW_KEY_B, InputEventType::Pressed, [this]() {
		this->showBoundingBoxes = !this->showBoundingBoxes;
		});

	LightMath::GetCascadeSplits(nearPlane, farPlane, 6, 1, cascadeSplits);

	Initialize();
}

// =================================================
// Draw passes
// =================================================

void Renderer::DrawShadowPass()
{
	auto batchedShadowCasters = BatchBuilder::Build(renderQueue.GetShadowCasters());
	ShadowUBO shadowData;
	shadowData.lightPos = renderLight.lightPos;
	shadowData.cascadedSplits[0] = LightMath::ComputePointLightFarPlane(glm::vec3(renderLight.attenuationFactor));
	std::string shaderName;
	bool isPointLight = false;
	if (shadowData.lightPos.w) // point light
	{
		isPointLight = true;
		LightMath::ComputePointLightMatrices(
			glm::vec3(shadowData.lightPos),
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
		glm::vec3 lightDir = LightMath::GetLightDirection(renderLight);
		auto& win = AppAttorney::GetWindow(App::Get());

		LightMath::ComputeDirectionalLightCascades(
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
}

void Renderer::DrawMainPass()
{
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

// =================================================
// Draw functions
// =================================================

void Renderer::DrawList(const std::vector<RenderSubmission>& submissions)
{
	// all submissions should share the same layer
	RenderLayer layer = submissions.empty() ? RenderLayer::Opaque : submissions[0].item.layer;

	if (layer == RenderLayer::GUI) {
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glDisable(GL_CULL_FACE);

		for (const auto& submission : submissions) {
			DrawGUISubmission(submission);
		}

		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glEnable(GL_CULL_FACE);
	}
	else {
		for (const auto& submission : submissions) {
			DrawSubmission(submission);
		}
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
		mesh->UploadInstancedData(dynamic_cast<InstanceData*>(submission.item.instanceData)->modelMatrices.data(), submission.item.instanceData->count);
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
		mesh->UploadInstancedData(dynamic_cast<InstanceData*>(submission.item.instanceData)->modelMatrices.data(), submission.item.instanceData->count);
		glDrawElementsInstanced(primitive, mesh->indexCount, GL_UNSIGNED_INT, 0, submission.item.instanceData->count);
	}
	else {
		glDrawElements(primitive, mesh->indexCount, GL_UNSIGNED_INT, 0);
	}
}

void Renderer::DrawGUISubmission(const RenderSubmission& submission)
{
	Mesh* mesh = nullptr;
	if (!submission.item.meshHandle.IsValid()) {
		mesh = _rm.meshes.Get("primitive/quad");
	}
	else {
		mesh = _rm.meshes.Get(submission.item.meshHandle);
	}
	if (!mesh) return;
	auto* material = _rm.materials.Get(submission.item.materialHandle);
	if (!material) return;

	//override texture if a textureHandle is provided
	if (submission.item.textureHandle.IsValid()) {
		material->SetTexture("tex", submission.item.textureHandle);
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
		if (mesh->isInstancingEnabled() == false)
		{
			mesh->EnableInstancing(true);
		}
		mesh->UploadInstanceDataGUI(dynamic_cast<InstanceDataGUI*>(submission.item.instanceData));
		glDrawElementsInstanced(primitive, mesh->indexCount, GL_UNSIGNED_INT, 0, submission.item.instanceData->count);
	}
	else {
		glDrawElements(primitive, mesh->indexCount, GL_UNSIGNED_INT, 0);
	}
}