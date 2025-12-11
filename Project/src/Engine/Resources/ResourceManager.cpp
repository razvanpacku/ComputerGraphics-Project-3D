#include "Engine/Resources/ResourceManager.h"

#include "Engine/Resources/MeshFactory.h"

ResourceManager& ResourceManager::Get() {
	static ResourceManager instance;
	return instance;
}

void ResourceManager::PreloadResources(const std::string& resourceDirectory) {
	std::cout << "Loading shaders:" << std::endl;
	shaders.PreloadResources(resourceDirectory);
	std::cout << "Loading textures:" << std::endl;
	textures.PreloadResources(resourceDirectory);

	std::cout << "Loading primitive meshes:" << std::endl;
	auto primitiveMeshes = MeshFactory::ObtainPrimitiveMeshes();
	for (const auto& [name, mesh] : primitiveMeshes) {
		std::cout << "  Loading primitive mesh: " << name << "\n";
		meshes.Register(name, const_cast<Mesh&>(mesh));
	}

	std::cout << "Loading materials:" << std::endl;
	materials.PreloadResources(resourceDirectory);
	std::cout << "Loading models:" << std::endl;
	models.PreloadResources(resourceDirectory);
}