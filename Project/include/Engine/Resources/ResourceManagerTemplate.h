#pragma once
#include <unordered_map>
#include <string>
#include <cstdint>
#include <iostream>
#include <filesystem>

struct SafeHandle {
    uint32_t id = 0;
    uint32_t generation = 0;

    bool operator==(const SafeHandle& other) const;

    bool IsValid() const { return id != 0; }
};

namespace std {
    template<>
    struct hash<SafeHandle> {
        std::size_t operator()(const SafeHandle& handle) const noexcept {
            return std::hash<uint32_t>()(handle.id) ^ (std::hash<uint32_t>()(handle.generation) << 1);
        }
    };
}

class IResource {
    public:
    virtual ~IResource() = default;
	bool alive = false;
};

template<typename T>
concept ResourceConcept = std::is_base_of<IResource, T>::value;

// -------------------------------------------------
template<ResourceConcept ResourceType,typename ResourceInfo>
class IResourcePolicy {
public:
    virtual ~IResourcePolicy() = default;

    // Must create a new instance (GPU or CPU resource)
    virtual ResourceType Create(const std::string& name,const ResourceInfo& resourceInfo) = 0;

    // Must destroy the instance (GPU free, CPU free)
    virtual void Destroy(ResourceType& res) = 0;
};

// concept to check that T is the first template parameter of IResourcePolicy
template<typename ResourceType, typename Policy>
concept ResourcePolicyConcept = std::is_base_of<IResourcePolicy<ResourceType, typename Policy::ResourceInfo>, Policy>::value;

//concept to check that T is the second template parameter of IResourcePolicy
template <typename ResourceInfo, typename Policy>
concept ResourceInfoPolicyConcept = std::is_base_of<IResourcePolicy<typename Policy::ResourceType, ResourceInfo>, Policy>::value;

// -------------------------------------------------

template<ResourceConcept ResourceType, typename Policy>
	requires ResourcePolicyConcept<ResourceType, Policy>
class ResourceManagerTemplate {
public:
    using Handle = SafeHandle;

    ResourceManagerTemplate() : nextID(1) {}

    // -----------------------------
    // Load or retrieve by name
    // -----------------------------
    template<typename ResourceInfo>
		requires ResourceInfoPolicyConcept<ResourceInfo, Policy>
    Handle Load(const std::string& name, const ResourceInfo& resourceInfo) {
        // Already loaded? Return existing handle
        if (nameToHandle.count(name))
            return nameToHandle[name];

        ResourceType res = policy.Create(name, resourceInfo);
        uint32_t id = nextID++;
        uint32_t gen = 1;

        res.alive = true;
        resources[id] = { res, gen };
        nameToHandle[name] = Handle{ id, gen };
        handleToName[id] = name;

        return Handle{ id, gen };
    }

    Handle Register(const std::string& name, ResourceType& resource) {
        // Already loaded? Return existing handle
        if (nameToHandle.count(name))
            return nameToHandle[name];
        uint32_t id = nextID++;
        uint32_t gen = 1;

        resource.alive = true;
        resources[id] = { resource, gen };
        nameToHandle[name] = Handle{ id, gen };
        handleToName[id] = name;
        return Handle{ id, gen };
	}

    // -----------------------------
    // Direct handle-based access
    // -----------------------------
    ResourceType* Get(Handle handle){
        auto it = resources.find(handle.id);
        if (it == resources.end()) return nullptr;
        if (it->second.generation != handle.generation) return nullptr;
        return &it->second.resource;
    }

    const ResourceType* Get(Handle handle) const {
        auto it = resources.find(handle.id);
        if (it == resources.end()) return nullptr;
        if (it->second.generation != handle.generation) return nullptr;
        return &it->second.resource;
    }

    // -----------------------------
    // Name-based access
    // -----------------------------
    ResourceType* Get(const std::string& name) {
        if (!nameToHandle.count(name)) return nullptr;
        return Get(nameToHandle[name]);
    }

    Handle GetHandle(const std::string& name) const {
        auto it = nameToHandle.find(name);
        return (it == nameToHandle.end()) ? Handle{} : it->second;
    }

    bool Exists(const std::string& name) const {
		return nameToHandle.contains(name);
    }

    // -----------------------------
    // Remove resource
    // -----------------------------
    void Remove(const std::string& name) {
        if (!nameToHandle.count(name)) return;
        Remove(nameToHandle[name]);
    }

    void Remove(Handle handle) {
        auto it = resources.find(handle.id);
        if (it == resources.end()) return;

        if (it->second.generation == handle.generation) {
            policy.Destroy(it->second.resource);

            // Remove name mapping
            if (handleToName.count(handle.id))
                nameToHandle.erase(handleToName[handle.id]);

            handleToName.erase(handle.id);
            resources.erase(it);
        }
    }

    bool Alive(Handle handle) const {
        auto it = resources.find(handle.id);
        return it != resources.end() &&
            it->second.generation == handle.generation &&
            it->second.resource.alive;
    }

	// -----------------------------
    // Preload resources
	// -----------------------------
    virtual void PreloadResources(const std::string& resourceDirectory) {
	}

protected:
    struct ResourceSlot {
        ResourceType resource;
        uint32_t generation = 0;
    };

    Policy policy;
    uint32_t nextID;

    std::unordered_map<uint32_t, ResourceSlot> resources;
    std::unordered_map<std::string, Handle> nameToHandle;
    std::unordered_map<uint32_t, std::string> handleToName;
};