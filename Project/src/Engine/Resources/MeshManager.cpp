#include "Engine/Resources/MeshManager.h"

#include <glm/glm.hpp>
#include "Engine/Renderer/Renderable.h"

// ==========================================
// Mesh
// ==========================================
MeshManager* Mesh::_mm = nullptr;

void Mesh::Bind() const
{
	if(vao != Mesh::_mm->currentVAO) {
        Mesh::_mm->currentVAO = vao;
        glBindVertexArray(vao);
	}
}

void Mesh::EnableInstancing(bool vaoAlreadyBound)
{
    if (instanceVBO != 0) return; // already enabled
    if(!vaoAlreadyBound) glBindVertexArray(vao);

    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    // Allocate buffer
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

	GLuint attributeIndexStart = 12; // starting attribute index for instance matrix
	const GLsizei stride = sizeof(glm::mat4) + (isGuiMesh ? sizeof(glm::vec4) : 0);
	uintptr_t offset = 0;

    if(isGuiMesh) {
        // UV offset attribute
        GLuint uvAttribIndex = attributeIndexStart-1;
        glEnableVertexAttribArray(uvAttribIndex);
        glVertexAttribPointer(uvAttribIndex, 4, GL_FLOAT, GL_FALSE,
            stride,
            0);
        glVertexAttribDivisor(uvAttribIndex, 1); // advance per instance
        offset += sizeof(glm::vec4);;
	}

    for (GLuint i = 0; i < 4; i++) {
        GLuint attribIndex = attributeIndexStart + i;

        glEnableVertexAttribArray(attribIndex);
        glVertexAttribPointer(attribIndex, 4, GL_FLOAT, GL_FALSE,
            stride,
            reinterpret_cast<const void*>(offset + sizeof(glm::vec4) * i));
        glVertexAttribDivisor(attribIndex, 1); // advance per instance
    }

    if (!vaoAlreadyBound) glBindVertexArray(0);
}

void Mesh::UploadInstancedData(const void* data, size_t count)
{
    if (instanceVBO == 0) return; // instancing not enabled
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    // upload only the matrix data, GUI meshes have uv offsets at the beginning
    if (!isGuiMesh) {
        glBufferData(GL_ARRAY_BUFFER,
            sizeof(glm::mat4) * count,
            data,
            GL_DYNAMIC_DRAW);
    }
    else {
		//alocate size for uv offsets + matrices
        glBufferData(GL_ARRAY_BUFFER,
            (sizeof(glm::vec4) + sizeof(glm::mat4)) * count,
            nullptr,
			GL_DYNAMIC_DRAW);

        glBufferSubData(GL_ARRAY_BUFFER,
            sizeof(glm::vec4) * count,
            sizeof(glm::mat4) * count,
			data);
    }
}

void Mesh::UploadInstanceDataGUI(const InstanceDataGUI* data)
{
    if (instanceVBO == 0 || !isGuiMesh) return;

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER,
        data->guiData.size() * sizeof(GUIData),
        data->guiData.data(),
        GL_DYNAMIC_DRAW);
}

// ==========================================
// MeshPolicy
// ==========================================
Mesh MeshPolicy::Create(const std::string& name, const MeshResoruceInfo& resourceInfo)
{
	Mesh mesh;

	mesh.vertexData = resourceInfo.vertexData;
	mesh.indices = resourceInfo.indices;
	mesh.attributes = resourceInfo.attributes;
	mesh.vertexStride = resourceInfo.stride;
	mesh.vertexCount = resourceInfo.vertexData.size() / resourceInfo.stride;
	mesh.indexCount = resourceInfo.indices.size();
	mesh.boundingBox = resourceInfo.boundingBox;
	mesh.cullBackfaces = resourceInfo.cullBackfaces;


	Upload(&mesh);
	mesh.alive = true;
	return mesh;
}

void MeshPolicy::Destroy(Mesh& res)
{
	if (!res.alive) return;
	glDeleteBuffers(1, &res.vbo);
	glDeleteBuffers(1, &res.ebo);
	glDeleteVertexArrays(1, &res.vao);
    if (res.instanceVBO != 0) {
        glDeleteBuffers(1, &res.instanceVBO);
        res.instanceVBO = 0;
	}
	res.alive = false;
}

void MeshPolicy::Upload(Mesh* mesh)
{
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);

    glBindVertexArray(mesh->vao);

    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER,
        mesh->vertexData.size(),
        mesh->vertexData.data(),
        GL_STATIC_DRAW);

    // EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        mesh->indices.size() * sizeof(uint32_t),
        mesh->indices.data(),
        GL_STATIC_DRAW);

    // Attributes
    for (const auto& attr : mesh->attributes)
    {
        glEnableVertexAttribArray(attr.index);
        glVertexAttribPointer(
            attr.index,
            attr.size,
            attr.type,
            attr.normalized,
            mesh->vertexStride,
            reinterpret_cast<const void*>(attr.offset)
        );
    }

    glBindVertexArray(0);
}

// ==========================================
// MeshManager
// ==========================================
MeshManager::MeshManager()
{
    Mesh::_mm = this;
}

void MeshManager::UseMesh(const MeshHandle& h)
{
    Mesh* mesh = Get(h);
    if (mesh && mesh->vao != currentVAO) {
		currentVAO = mesh->vao;
		mesh->Bind();
    }
}

void MeshManager::UseMesh(const Mesh& mesh)
{
    if (mesh.vao != currentVAO) {
        currentVAO = mesh.vao;
        mesh.Bind();
    }
}

void MeshManager::UseMesh(const std::string& name)
{
    Mesh* mesh = Get(name);
    if (mesh && mesh->vao != currentVAO) {
        currentVAO = mesh->vao;
        mesh->Bind();
    }
}