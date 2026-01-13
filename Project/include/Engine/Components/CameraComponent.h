#pragma once
#include <cstdint>

enum class CameraType : uint8_t
{
	FLYING_CAMERA = 0,
	ORBIT_CAMERA = 1,
};

struct CameraComponent
{
	CameraType type = CameraType::FLYING_CAMERA;
	bool dirty = false;
};
