#include "Engine/Renderer/Renderer.h"

#include "Engine/App.h"
#include "Engine/Window.h"
#include "Engine/Resources/ResourceManager.h"
#include "Engine/InputManager.h"

#include "Engine/Resources/UboDefs.h"
#include "Engine/Renderer/LightMath.h"

#include "Engine/Renderer/RenderableProvider/MeshRenderableProvider.h"
#include "Engine/Renderer/BatchBuilder.h"
#include "Engine/Renderer/Culling/Frustum.h"

#include "Engine/DataStructures/TransformFunctions.h"

// temporary camera controller until entity system is done to fetch cameraController from camera entity
#include "Engine/Controllers/FlyingCameraController.h"

#include <iostream>
#include <algorithm>

Renderer::Renderer(App& app) : app(app), _rm(ResourceManager::Get())
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(DEFAULT_CLEAR_COLOR_R, DEFAULT_CLEAR_COLOR_G, DEFAULT_CLEAR_COLOR_B, 1.0f);

	auto& _im = InputManager::Get();

	_im.BindKey(GLFW_KEY_B, InputEventType::Pressed, [this]() {
		this->showBoundingBoxes = !this->showBoundingBoxes;
		});

	LightMath::GetCascadeSplits(nearPlane, farPlane, 6, 1, cascadeSplits);
}

Renderer::~Renderer()
{
}

void Renderer::UpdateLighting(LightingUBO* light)
{
	if(light)
		renderLight = light;
	auto& _rm = ResourceManager::Get();
	auto* lightingWriter = _rm.ubos.GetUboWriter("Lighting");
	lightingWriter->SetBlock(*renderLight);
	lightingWriter->Upload();
}

void Renderer::Render()
{

	Clear();

	UpdateCameraUBOs();

	RenderFrame();
	ClearQueue();
	//glFlush();
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

void Renderer::RenderFrame() {
	DrawShadowPass();
	DrawMainPass();
}

// =================================================
// Submit functions
// =================================================
void Renderer::Submit(const Renderable& r)
{
	renderQueue.Push(r);

	if(showBoundingBoxes && r.hasBounds)
	{
		MeshRenderableProvider meshProvider;
		meshProvider.meshHandle = _rm.meshes.GetHandle("primitive/bounding_box");
		meshProvider.materialHandle = _rm.materials.GetHandle("boundingBox");
		glm::mat4 modelMatrix = r.modelMatrix;
		BoundingBox transformedAABB = TransformAABB(r.aabb, modelMatrix);

		Transform t;
		t.position = (transformedAABB.min + transformedAABB.max) * 0.5f;
		t.scale = transformedAABB.max - transformedAABB.min;
		meshProvider.modelMatrix = t.GetModelMatrix();

		std::vector<Renderable> boxRenders;
		meshProvider.GenerateRenderables(boxRenders);
		renderQueue.Push(boxRenders);
	}
}

void Renderer::Submit(const std::vector<Renderable>& rs)
{
	renderQueue.Push(rs);

	if(showBoundingBoxes)
	{
		std::vector<Renderable> boxRenders;
		MeshRenderableProvider meshProvider;
		meshProvider.meshHandle = _rm.meshes.GetHandle("primitive/bounding_box");
		meshProvider.materialHandle = _rm.materials.GetHandle("boundingBox");
		for (const auto& r : rs)
		{
			if(!r.hasBounds) continue;
			glm::mat4 modelMatrix = r.modelMatrix;
			BoundingBox transformedAABB = TransformAABB(r.aabb, modelMatrix);

			Transform t;
			t.position = (transformedAABB.min + transformedAABB.max) * 0.5f;
			t.scale = transformedAABB.max - transformedAABB.min;
			meshProvider.modelMatrix = t.GetModelMatrix();

			meshProvider.GenerateRenderables(boxRenders);
		}
		renderQueue.Push(boxRenders);
	}
}

// 
// Draw passes
// =================================================

void Renderer::DrawShadowPass()
{
	auto batchedShadowCasters = BatchBuilder::Build(renderQueue.GetShadowCasters());
	ShadowUBO shadowData;
	shadowData.lightPos = renderLight->lightPos;
	shadowData.cascadedSplits[0] = LightMath::ComputePointLightFarPlane(glm::vec3(renderLight->attenuationFactor));
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
		glm::vec3 lightDir = LightMath::GetLightDirection(*renderLight);
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
	//update sortDistance for transparent items
	for(auto& renderable : transparentList)
	{
		glm::vec3 camPos = renderCamera->GetPosition();
		glm::vec3 objPos = TransformFunctions::DecomposePosition(renderable.item.modelMatrix);
		renderable.item.sortDistance = glm::length(camPos - objPos);
	}
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

		if (layer == RenderLayer::Transparent)
		{
			glDepthMask(GL_FALSE);
		}

		for (const auto& submission : submissions) {
			DrawSubmission(submission);
		}

		if (layer == RenderLayer::Transparent)
		{
			glDepthMask(GL_TRUE);
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