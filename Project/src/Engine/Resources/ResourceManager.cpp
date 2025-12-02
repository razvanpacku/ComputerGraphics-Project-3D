#include "Engine/Resources/ResourceManager.h"

ResourceManager& ResourceManager::Get() {
	static ResourceManager instance;
	return instance;
}
