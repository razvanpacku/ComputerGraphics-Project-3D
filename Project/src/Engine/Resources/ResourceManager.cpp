#include "Engine/Resources/ResourceManager.h"

ResourceManager& ResourceManager::Get() {
	static ResourceManager instance;
	return instance;
}

void ResourceManager::PreloadResources(const std::string& resourceDirectory) {
	std::cout << "Loading shaders:" << std::endl;
	shaders.PreloadResources(resourceDirectory);
	std::cout << "Loading textures:" << std::endl;
	textures.PreloadResources(resourceDirectory);
	std::cout << "Loading materials:" << std::endl;
	materials.PreloadResources(resourceDirectory);
	std::cout << "Loading models:" << std::endl;
	models.PreloadResources(resourceDirectory);
}