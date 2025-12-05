#include "Engine/Resources/MeshManager.h"

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