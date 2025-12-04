#pragma once
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstring>
#include <algorithm>

#include <iostream>

#include "ShaderReflection.h"
#include "Ubo.h"

template<typename T>
concept GlmType = requires(T v) {
    glm::value_ptr(v);
};

// Utility class to write data into UBOs
class UboWriter
{
public:
	UboWriter(const Ubo& uboInfo);

    template<typename T>
    bool Set(const std::string& memberName, const T& value, int arrayIndex = 0) {
        auto it = std::find_if(
            ubo.fields.begin(),
            ubo.fields.end(),
            [&memberName](const UniformBlockFieldInfo& field) { return field.name == memberName; }
        );
        if (it == ubo.fields.end())
            return false;

        const UniformBlockFieldInfo& m = *it;

		// clamp arrayIndex
		arrayIndex = std::clamp(arrayIndex, 0, m.size - 1);

		size_t offset = m.offset + arrayIndex * m.arrayStride;

        memcpy(data.data() + offset, &value, sizeof(T));
        return true;
    }

	// Specialization for glm types
	template<GlmType T>
    bool Set(const std::string& memberName, const T& value, int arrayIndex = 0) {
        auto it = std::find_if(
            ubo.fields.begin(),
            ubo.fields.end(),
            [&memberName](const UniformBlockFieldInfo& field) { return field.name == memberName; }
        );
        if (it == ubo.fields.end())
            return false;

        const UniformBlockFieldInfo& m = *it;

        // clamp arrayIndex
        arrayIndex = std::clamp(arrayIndex, 0, m.size - 1);

        size_t offset = m.offset + arrayIndex * m.arrayStride;

        memcpy(data.data() + offset, glm::value_ptr(value), sizeof(T));
        return true;
    }

    // Raw setter for custom sizes
    template <typename T>
    bool SetRaw(const std::string& memberName, const void* valuePtr, size_t size, int arrayIndex = 0) {
        auto it = std::find_if(
            ubo.fields.begin(),
            ubo.fields.end(),
            [&memberName](const UniformBlockFieldInfo& field) { return field.name == memberName; }
        );
        if (it == ubo.fields.end())
            return false;
        const UniformBlockFieldInfo& m = *it;
        // clamp arrayIndex
        arrayIndex = std::clamp(arrayIndex, 0, m.size - 1);
        size_t offset = m.offset + arrayIndex * m.arrayStride;
        if (size > static_cast<size_t>(m.arrayStride))
            return false; // value too large for the member
        memcpy(data.data() + offset, valuePtr, size);
        return true;
	}

    // Setter for entire block
	template <typename T>
    bool SetBlock(const T& block) {
        static_assert(std::is_trivially_copyable_v<T>, "Block must be trivially copyable");
        if (sizeof(T) != static_cast<size_t>(ubo.dataSize)) {
			// doesn't match std140 size
            return false;
        }
        memcpy(data.data(), &block, sizeof(T));
        return true;
	}

    void Upload() const;
private:
	const Ubo& ubo;
	std::vector<uint8_t> data;
};

