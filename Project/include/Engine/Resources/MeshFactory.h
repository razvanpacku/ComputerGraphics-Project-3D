#pragma once
#include "MeshManager.h"

namespace MeshFactory
{
	extern std::vector<VertexAttribute> standardAttributes;

	// --- Generation Functions ---
	Mesh CreateCube(float size = 1.0f);
	Mesh CreateQuad(float size = 1.0f, uint16_t resolution = 1, uint8_t type = 0);
	Mesh CreateUVSphere(float radius = 0.5f, uint16_t sectorCount = 36, uint16_t stackCount = 18);

	Mesh CreateBoundingBox(const BoundingBox& bbox = BoundingBox{ glm::vec3(-0.5f), glm::vec3(0.5f) });

	// -------------------------------------------------------------------------
	// function for obtaining a generated mesh along with its name to be preloaded
	std::vector<std::pair<std::string, Mesh>> ObtainPrimitiveMeshes();
}

