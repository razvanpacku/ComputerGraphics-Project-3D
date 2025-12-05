#pragma once
#include <glad/glad.h>

#include "ResourceManagerTemplate.h"

//forward declaration
class MeshManager;

struct VertexAttribute {
	GLuint index;			// attribute loctation in shader
	GLint size;				// number of components
	GLenum type;			// data type
	uint32_t offset;		// offset in bytes from the beginning of the vertex
	bool normalized;
};

struct Mesh : IResource {
	GLuint vao;
	GLuint vbo;				
	GLuint ebo;

	std::vector<uint8_t> vertexData;
	std::vector<uint32_t> indices;

	std::vector<VertexAttribute> attributes;

	GLenum primitive = GL_TRIANGLES;

	uint32_t vertexStride = 0;
	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;

	void Bind() const;

private:
	static MeshManager* _mm;

	friend class MeshManager;
};

struct MeshResoruceInfo {
	std::vector<uint8_t> vertexData;
	std::vector<uint32_t> indices;
	std::vector<VertexAttribute> attributes;
	uint32_t stride;
};

class MeshPolicy : public IResourcePolicy<Mesh, MeshResoruceInfo> {
public:
	using ResourceType = Mesh;
	using ResourceInfo = MeshResoruceInfo;

	Mesh Create(const std::string& name, const MeshResoruceInfo& resourceInfo) override;
	void Destroy(Mesh& res) override;
private:
	void Upload(Mesh* mesh);
};

class MeshManager : public ResourceManagerTemplate<Mesh, MeshPolicy>
{
public:
	using MeshHandle = Handle;

	MeshManager();

	// activate the mesh
	void UseMesh(const MeshHandle& h);
	void UseMesh(const Mesh& mesh);
	void UseMesh(const std::string& name);

	GLuint currentVAO = 0;
};

