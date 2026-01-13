#include "Engine/Resources/ResourceManagerTemplate.h"

bool SafeHandle::operator==(const SafeHandle& other) const {
    return id == other.id && generation == other.generation;
}
