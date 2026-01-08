#pragma once
#include "RenderEntity.h"
#include "TransformEntity.h"

// ================================================================
// ModelEntity
//
// An entity that represents a 3D model in the scene.
// ================================================================
class ModelEntity : public RenderEntity, public TransformEntity
{
public:
	ModelEntity(const std::string& model = "", const std::string& name = "Model");

	const std::string& GetModel() const { return modelName; }
	void SetModel(const std::string& model);
protected:
	void ProvideRenderables(std::vector<Renderable>& outRenderables) override;
	void UpdateTransform(const glm::mat4& newTransform) override;
private:

	std::string modelName;
};

