#pragma once
#include <glad/glad.h>
#include <optional>

#include "ResourceManagerTemplate.h"

struct Texture : public IResource {
	Texture() = default;
	~Texture() override = default;

	bool LoadFromFile(const std::string& path, bool generateMipmaps = true);
	void Destroy();

	void Bind(GLuint unit = 0) const;

	GLuint id = 0;
	uint16_t width = 0;
	uint16_t height = 0;
	uint8_t channels = 0;
};

struct TextureResourceInfo {
	std::string path;
	bool generateMipmaps = true;
};

class TexturePolicy : public IResourcePolicy<Texture, TextureResourceInfo> {
public:
	using ResourceInfo = TextureResourceInfo;
	using ResourceType = Texture;

	Texture Create(const std::string& name, const TextureResourceInfo& resourceInfo) override;
	void Destroy(Texture& res) override;
};

class TextureManager : public ResourceManagerTemplate<Texture, TexturePolicy> {
public:
	using TextureHandle = Handle;

	TextureManager();
	~TextureManager();

	bool Bind(const TextureHandle& h);
	bool Bind(const std::string& name);

	void Unbind(const TextureHandle& h);
	void Unbind(const std::string& name);

	int GetBoundUnit(const TextureHandle& h) const;
	int GetBoundUnit(const std::string& name) const;

	void UnbindAll();
private:
	int FindFreeUnit();
	void BindToUnit(Texture* texture, int unit);
	void UnbindFromUnit(int unit);

	// OpenGL texture unit tracking:
	//   index = texture unit
	//   value = optional handle stored there
	std::vector<std::optional<TextureHandle>> unitToHandle;

	// Reverse lookup: handle -> texture unit
	std::unordered_map<TextureHandle, int> handleToUnit;

	int maxUnits = 0;
	int nextUnitToBind = 0;
};