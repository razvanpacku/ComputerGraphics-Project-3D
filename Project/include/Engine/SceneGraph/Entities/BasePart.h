#pragma once
#include "TransformEntity.h"
#include "RenderEntity.h"

enum class BasePartShape
{
	CUBE,
	SPHERE,
	QUAD,
	QUAD_SINGLE_FACE,
	QUAD_DOUBLE_SIDED
};

// ================================================================
// BasePart
//
// Represents a prmitive shape part in the scene.
// ================================================================
class BasePart : public RenderEntity, public TransformEntity
{
public:
	BasePart(const BasePartShape& shape = BasePartShape::CUBE, const std::string& material = "flat", const std::string& name = "BasePart");

	BasePartShape GetShape() const { return shape; }
	void SetShape(BasePartShape newShape);

	std::string GetMaterial() const { return materialName; }
	void SetMaterial(const std::string& newMaterial);
private:
	void ProvideRenderables(std::vector<Renderable>& outRenderables) override;
	void UpdateTransform(const glm::mat4& newTransform) override;

	BasePartShape shape;
	std::string materialName;
};

