#pragma once
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Generic template for setting uniform values.
template<typename T>
struct UniformSetter {
    static void Apply(GLint location, const T& value) {
        static_assert(sizeof(T) == 0,
            "UniformSetter<T> has no specialization for this type.");
    }
};

// --- Specializations for common types ---

// --- float ---
template<>
struct UniformSetter<float> {
    static void Apply(GLint loc, const float& v) {
        glUniform1f(loc, v);
    }
};

// --- int / samplers ---
template<>
struct UniformSetter<int> {
    static void Apply(GLint loc, const int& v) {
        glUniform1i(loc, v);
    }
};

// --- unsigned int ---
template<>
struct UniformSetter<unsigned int> {
    static void Apply(GLint loc, const unsigned int& v) {
        glUniform1ui(loc, v);
    }
};

// --- vec2 ---
template<>
struct UniformSetter<glm::vec2> {
    static void Apply(GLint loc, const glm::vec2& v) {
        glUniform2fv(loc, 1, glm::value_ptr(v));
    }
};

// --- vec3 ---
template<>
struct UniformSetter<glm::vec3> {
    static void Apply(GLint loc, const glm::vec3& v) {
        glUniform3fv(loc, 1, glm::value_ptr(v));
    }
};

// --- vec4 ---
template<>
struct UniformSetter<glm::vec4> {
    static void Apply(GLint loc, const glm::vec4& v) {
        glUniform4fv(loc, 1, glm::value_ptr(v));
    }
};

// --- mat4 ---
template<>
struct UniformSetter<glm::mat4> {
    static void Apply(GLint loc, const glm::mat4& m) {
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(m));
    }
};
