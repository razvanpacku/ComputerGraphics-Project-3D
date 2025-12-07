#pragma once
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstring>
#include <algorithm>

#include <iostream>
#include <span>

#include "ShaderReflection.h"
#include "Ubo.h"

// Utility class to write data into UBOs
class UboWriter
{
public:
	UboWriter(const UniformBlockInfo* uboInfo);

    template<typename T>
    bool Set(const std::string& memberName, const T& value, int arrayIndex = 0) {
        auto it = std::find_if(
            ubo->fields.begin(),
            ubo->fields.end(),
            [&memberName](const UniformBlockFieldInfo& field) { return field.name == memberName; }
        );
        if (it == ubo->fields.end())
            return false;

        const UniformBlockFieldInfo& m = *it;

		// clamp arrayIndex
		arrayIndex = std::clamp(arrayIndex, 0, m.size - 1);

		size_t offset = m.offset + arrayIndex * m.arrayStride;

        memcpy(data.data() + offset, &value, sizeof(T));
        return true;
    }

    template <typename T>
    bool SetArray(const std::string& memberName, std::span<const T> values)
    {
        auto it = std::find_if(
            ubo->fields.begin(),
            ubo->fields.end(),
            [&](const UniformBlockFieldInfo& f) { return f.name == memberName; }
        );
        if (it == ubo->fields.end())
            return false;

        const UniformBlockFieldInfo& m = *it;

        // array must not exceed the declared size
        size_t writeCount = std::min(values.size(), static_cast<size_t>(m.size));

        // write each element using arrayStride
        for (size_t i = 0; i < writeCount; i++)
        {
            size_t offset = m.offset + i * m.arrayStride;
            memcpy(data.data() + offset, &values[i], sizeof(T));
        }
        return true;
    }

    template <typename T>
    bool SetArray(const std::string& memberName, const std::vector<T>& values) {
        return SetArray(memberName, std::span<const T>(values.begin(), values.size()));
    }

    template <typename T>
    bool SetArray(const std::string& memberName, std::initializer_list<T> values)
    {
        return SetArray(memberName, std::span<const T>(values.begin(), values.size()));
    }

	// Specialization for glm types
	template<GlmType T>
    bool Set(const std::string& memberName, const T& value, int arrayIndex = 0) {
        auto it = std::find_if(
            ubo->fields.begin(),
            ubo->fields.end(),
            [&memberName](const UniformBlockFieldInfo& field) { return field.name == memberName; }
        );
        if (it == ubo->fields.end())
            return false;

        const UniformBlockFieldInfo& m = *it;

        // clamp arrayIndex
        arrayIndex = std::clamp(arrayIndex, 0, m.size - 1);

        size_t offset = m.offset + arrayIndex * m.arrayStride;

        memcpy(data.data() + offset, glm::value_ptr(value), sizeof(T));
        return true;
    }

    template <GlmType T>
    bool SetArray(const std::string& memberName, std::span<const T> values)
    {
        auto it = std::find_if(
            ubo->fields.begin(),
            ubo->fields.end(),
            [&](const UniformBlockFieldInfo& f) { return f.name == memberName; }
        );
        if (it == ubo->fields.end())
            return false;

        const UniformBlockFieldInfo& m = *it;

        size_t writeCount = std::min(values.size(), static_cast<size_t>(m.size));
        size_t elementSize = sizeof(T);

        for (size_t i = 0; i < writeCount; i++)
        {
            size_t offset = m.offset + i * m.arrayStride;
            memcpy(data.data() + offset, glm::value_ptr(values[i]), elementSize);
        }
        return true;
    }

    template <GlmType T>
    bool SetArray(const std::string& memberName, const std::vector<T>& values) {
        return SetArray(memberName, std::span<const T>(values.begin(), values.size()));
    }

    // initializer_list overload
    template <GlmType T>
    bool SetArray(const std::string& memberName, std::initializer_list<T> values)
    {
        return SetArray(memberName, std::span<const T>(values.begin(), values.size()));
    }

    // Raw setter for custom sizes
    template <typename T>
    bool SetRaw(const std::string& memberName, const void* valuePtr, size_t size, int arrayIndex = 0) {
        auto it = std::find_if(
            ubo->fields.begin(),
            ubo->fields.end(),
            [&memberName](const UniformBlockFieldInfo& field) { return field.name == memberName; }
        );
        if (it == ubo->fields.end())
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
        if (sizeof(T) != static_cast<size_t>(ubo->dataSize)) {
			// doesn't match std140 size
            return false;
        }
        memcpy(data.data(), &block, sizeof(T));
        return true;
	}

	const UniformBlockInfo* GetUboInfo() const { return ubo; }

	void PrintDebugInfo() const;

    void Upload() const;
private:
	const UniformBlockInfo* ubo;
	std::vector<uint8_t> data;
};

