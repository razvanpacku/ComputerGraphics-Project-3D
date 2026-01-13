#include "Engine/Resources/MeshFactory.h"

namespace MeshFactory {
	MeshPolicy mp;

	struct Vertex {
		glm::vec4 position;
		glm::vec2 uv;
		glm::vec3 normal;
	};

	std::vector<VertexAttribute> standardAttributes = {
		{ 0, 4, GL_FLOAT, offsetof(Vertex, position), GL_FALSE },
		{ 1, 2, GL_FLOAT, offsetof(Vertex, uv), GL_FALSE },
		{ 2, 3, GL_FLOAT, offsetof(Vertex, normal), GL_FALSE },
	};

	enum QuadType : uint8_t {
		NO_BACKFACE_CULLING = 0,
		SINGLE_SIDED = 1,
		DOUBLE_SIDED = 2
	};

	const float PI = 3.14159265359f;

	Mesh CreateCube(float size) {
		float h = size / 2.0f;
		std::vector<Vertex> vertices = {
			// top face
			{{-h,  h, -h, 1.0f}, {0.0f, 1.0f}, { 0.0f,  1.0f,  0.0f}},
			{{-h,  h,  h, 1.0f}, {0.0f, 0.0f}, { 0.0f,  1.0f,  0.0f}},
			{{ h,  h, -h, 1.0f}, {1.0f, 1.0f}, { 0.0f,  1.0f,  0.0f}},
			{{ h,  h,  h, 1.0f}, {1.0f, 0.0f}, { 0.0f,  1.0f,  0.0f}},
			// bottom face
			{{-h, -h, -h, 1.0f}, {0.0f, 0.0f}, { 0.0f, -1.0f,  0.0f}},
			{{ h, -h, -h, 1.0f}, {1.0f, 0.0f}, { 0.0f, -1.0f,  0.0f}},
			{{-h, -h,  h, 1.0f}, {0.0f, 1.0f}, { 0.0f, -1.0f,  0.0f}},
			{{ h, -h,  h, 1.0f}, {1.0f, 1.0f}, { 0.0f, -1.0f,  0.0f}},
			// front face
			{{-h, -h,  h, 1.0f}, {0.0f, 0.0f}, { 0.0f,  0.0f,  1.0f}},
			{{ h, -h,  h, 1.0f}, {1.0f, 0.0f}, { 0.0f,  0.0f,  1.0f}},
			{{-h,  h,  h, 1.0f}, {0.0f, 1.0f}, { 0.0f,  0.0f,  1.0f}},
			{{ h,  h,  h, 1.0f}, {1.0f, 1.0f}, { 0.0f,  0.0f,  1.0f}},
			// back face
			{{-h, -h, -h, 1.0f}, {1.0f, 0.0f}, { 0.0f,  0.0f, -1.0f}},
			{{-h,  h, -h, 1.0f}, {1.0f, 1.0f}, { 0.0f,  0.0f, -1.0f}},
			{{ h, -h, -h, 1.0f}, {0.0f, 0.0f}, { 0.0f,  0.0f, -1.0f}},
			{{ h,  h, -h, 1.0f}, {0.0f, 1.0f}, { 0.0f,  0.0f, -1.0f}},
			// right face
			{{ h, -h, -h, 1.0f}, {1.0f, 0.0f}, { 1.0f,  0.0f,  0.0f}},
			{{ h,  h, -h, 1.0f}, {1.0f, 1.0f}, { 1.0f,  0.0f,  0.0f}},
			{{ h, -h,  h, 1.0f}, {0.0f, 0.0f}, { 1.0f,  0.0f,  0.0f}},
			{{ h,  h,  h, 1.0f}, {0.0f, 1.0f}, { 1.0f,  0.0f,  0.0f}},
			// left face
			{{-h, -h, -h, 1.0f}, {0.0f, 0.0f}, {-1.0f,  0.0f,  0.0f}},
			{{-h, -h,  h, 1.0f}, {1.0f, 0.0f}, {-1.0f,  0.0f,  0.0f}},
			{{-h,  h, -h, 1.0f}, {0.0f, 1.0f}, {-1.0f,  0.0f,  0.0f}},
			{{-h,  h,  h, 1.0f}, {1.0f, 1.0f}, {-1.0f,  0.0f,  0.0f}},
		};
		std::vector<uint32_t> indices = {
			0, 1, 2,
			2, 1, 3,
			4, 5, 6,
			6, 5, 7,
			8, 9, 10,
			10, 9, 11,
			12, 13, 14,
			14, 13, 15,
			16, 17, 18,
			18, 17, 19,
			20, 21, 22,
			22, 21, 23
		};

		std::vector<uint8_t> vertexData(vertices.size() * sizeof(Vertex));
		memcpy(vertexData.data(), vertices.data(), vertexData.size());
		Mesh mesh = mp.Create(
			"",
			MeshResoruceInfo{
				vertexData,
				indices,
				standardAttributes,
				sizeof(Vertex),
				BoundingBox{ glm::vec3(-h, -h, -h), glm::vec3(h, h, h) },
				true
			});
		return mesh;
	}

	Mesh CreateQuad(float size, uint16_t resolution, uint8_t type) {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		for (uint16_t y = 0; y <= resolution; ++y) {
			for (uint16_t x = 0; x <= resolution; ++x) {
				float xf = (static_cast<float>(x) / resolution - 0.5f) * size;
				float yf = (static_cast<float>(y) / resolution - 0.5f) * size;
				Vertex vert;
				vert.position = glm::vec4(xf, yf , 0.0f, 1.0f);
				vert.uv = glm::vec2(static_cast<float>(x) / resolution, static_cast<float>(y) / resolution);
				vert.normal = glm::vec3(0.0f, 0.0f, 1.0f);
				vertices.push_back(vert);
				
				// if double sided, add back face vertex with the same position and uv but inverted normal
				if (type == DOUBLE_SIDED) {
					vert.normal = -vert.normal;
					vertices.push_back(vert);
				}
			}
		}
		for (uint16_t y = 0; y < resolution; ++y) {
			for (uint16_t x = 0; x < resolution; ++x) {
				if (type != DOUBLE_SIDED) {
					uint32_t topLeft = y * (resolution + 1) + x;
					uint32_t topRight = topLeft + 1;
					uint32_t bottomLeft = (y + 1) * (resolution + 1) + x;
					uint32_t bottomRight = bottomLeft + 1;
					indices.push_back(topLeft);
					indices.push_back(topRight);
					indices.push_back(bottomLeft);
					indices.push_back(topRight);
					indices.push_back(bottomRight);
					indices.push_back(bottomLeft);
				}
				else {
					uint32_t topLeftFront = (y * (resolution + 1) + x) * 2;
					uint32_t topRightFront = topLeftFront + 2;
					uint32_t bottomLeftFront = ((y + 1) * (resolution + 1) + x) * 2;
					uint32_t bottomRightFront = bottomLeftFront + 2;
					// front face
					indices.push_back(topLeftFront);
					indices.push_back(topRightFront);
					indices.push_back(bottomLeftFront);
					indices.push_back(topRightFront);
					indices.push_back(bottomRightFront);
					indices.push_back(bottomLeftFront);
					// back face
					uint32_t topLeftBack = topLeftFront + 1;
					uint32_t topRightBack = topRightFront + 1;
					uint32_t bottomLeftBack = bottomLeftFront + 1;
					uint32_t bottomRightBack = bottomRightFront + 1;
					indices.push_back(topRightBack);
					indices.push_back(topLeftBack);
					indices.push_back(bottomLeftBack);
					indices.push_back(bottomRightBack);
					indices.push_back(topRightBack);
					indices.push_back(bottomLeftBack);
				}
			}
		}
		std::vector<uint8_t> vertexData(vertices.size() * sizeof(Vertex));
		memcpy(vertexData.data(), vertices.data(), vertexData.size());
		Mesh mesh = mp.Create(
			"",
			MeshResoruceInfo{
				vertexData,
				indices,
				standardAttributes,
				sizeof(Vertex),
				BoundingBox{ glm::vec3(-size / 2.0f, -size / 2.0f, 0.0f), glm::vec3(size / 2.0f, size / 2.0f, 0.0f) },
				type != NO_BACKFACE_CULLING
			});
		return mesh;
	}

	Mesh CreateUVSphere(float radius, uint16_t sectorCount, uint16_t stackCount)
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;


		for (uint16_t i = 0; i <= stackCount; ++i) {
			float stackPercent = (float)i / stackCount;           // [0,1]
			float stackAngle = PI / 2.0f - stackPercent * PI;     // from +pi/2 to -pi/2

			float xz = radius * cosf(stackAngle);
			float y = radius * sinf(stackAngle);

			for (uint16_t j = 0; j <= sectorCount; ++j) {
				float sectorPercent = (float)j / sectorCount;     // [0,1]
				float sectorAngle = sectorPercent * 2.0f * PI;

				float x = xz * cosf(sectorAngle);
				float z = xz * sinf(sectorAngle);

				Vertex v{};
				v.position = glm::vec4(x, y, z, 1.0f);

				// Normal is position normalized
				glm::vec3 n = glm::normalize(glm::vec3(x, y, z));
				v.normal = n;

				// UV coordinates
				v.uv = glm::vec2(1.0f - sectorPercent, 1.0f - stackPercent);


				vertices.push_back(v);
			}
		}

		// ----- Generate indices -----
		for (uint16_t i = 0; i < stackCount; ++i) {
			uint32_t k1 = i * (sectorCount + 1);
			uint32_t k2 = k1 + sectorCount + 1;

			for (uint16_t j = 0; j < sectorCount; ++j, ++k1, ++k2) {
				if (i != 0) {
					indices.push_back(k1);
					indices.push_back(k1 + 1);
					indices.push_back(k2);
				}
				if (i != (stackCount - 1)) {
					indices.push_back(k1 + 1);
					indices.push_back(k2 + 1);
					indices.push_back(k2);
				}
			}
		}

		std::vector<uint8_t> vertexData(vertices.size() * sizeof(Vertex));
		memcpy(vertexData.data(), vertices.data(), vertexData.size());

		Mesh mesh = mp.Create(
			"",
			MeshResoruceInfo{
				vertexData,
				indices,
				standardAttributes,
				sizeof(Vertex),
				BoundingBox{
					glm::vec3(-radius, -radius, -radius),
					glm::vec3(radius,  radius,  radius)
				},
				true
			}
		);
		return mesh;
	}

	Mesh CreateBoundingBox(const BoundingBox& bbo) {
		struct VertexBB {
			glm::vec4 position;
		};

		std::vector<VertexAttribute> bbAttributes = {
			{ 0, 4, GL_FLOAT, offsetof(VertexBB, position), GL_FALSE },
		};

		std::vector<VertexBB> vertices = {
			// 8 corners of the bounding box
			{{bbo.min.x, bbo.min.y, bbo.min.z, 1.0f}},
			{{bbo.max.x, bbo.min.y, bbo.min.z, 1.0f}},
			{{bbo.max.x, bbo.max.y, bbo.min.z, 1.0f}},
			{{bbo.min.x, bbo.max.y, bbo.min.z, 1.0f}},
			{{bbo.min.x, bbo.min.y, bbo.max.z, 1.0f}},
			{{bbo.max.x, bbo.min.y, bbo.max.z, 1.0f}},
			{{bbo.max.x, bbo.max.y, bbo.max.z, 1.0f}},
			{{bbo.min.x, bbo.max.y, bbo.max.z, 1.0f}},
		};
		std::vector<uint32_t> indices = {
			0, 1, 1, 2, 2, 3, 3, 0, // bottom face
			4, 5, 5, 6, 6, 7, 7, 4, // top face
			0, 4, 1, 5, 2, 6, 3, 7  // vertical edges
		};

		std::vector<uint8_t> vertexData(vertices.size() * sizeof(VertexBB));
		memcpy(vertexData.data(), vertices.data(), vertexData.size());
		Mesh mesh = mp.Create(
			"",
			MeshResoruceInfo{
				vertexData,
				indices,
				bbAttributes,
				sizeof(VertexBB),
				bbo,
				false
			});
		mesh.primitive = GL_LINES;
		return mesh;
	}

	// -------------------------------------------------------------------------

	std::vector<std::pair<std::string, Mesh>> ObtainPrimitiveMeshes() {
		std::string prefix = "primitive/";
		std::vector<std::pair<std::string, Mesh>> meshes;
		meshes.emplace_back(prefix + "cube", CreateCube(1.0f));
		meshes.emplace_back(prefix + "quad", CreateQuad(1.0f, 1, SINGLE_SIDED));
		meshes.emplace_back(prefix + "quad_doubleSided", CreateQuad(1.0f, 1, DOUBLE_SIDED));
		meshes.emplace_back(prefix + "quad_backface", CreateQuad(1.0f, 1, NO_BACKFACE_CULLING));
		meshes.emplace_back(prefix + "uv_sphere", CreateUVSphere(0.5f, 72, 36));

		meshes.emplace_back(prefix + "bounding_box", CreateBoundingBox(BoundingBox{ glm::vec3(-0.5f), glm::vec3(0.5f) }));
		return meshes;
	}
}
