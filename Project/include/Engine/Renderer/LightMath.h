#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Engine/Resources/UboDefs.h"
#include <algorithm>

namespace LightMath {
	void GetCascadeSplits(float nearPlane, float farPlane, int cascadeCount, float lambda, fixed_float outSplits[]) {
		float ratio = farPlane / nearPlane;
		for (int i = 0; i < cascadeCount; i++) {
			float p = (i + 1) / static_cast<float>(cascadeCount);
			float log = nearPlane * std::pow(ratio, p);
			float uniform = nearPlane + (farPlane - nearPlane) * p;
			outSplits[i] = lambda * (log - uniform) + uniform;
		}
	}

	glm::vec3 GetLightDirection(LightingUBO lightInfo) {
		if (lightInfo.lightPos.w) return glm::vec3(0.0f); // point light has no direction
		return -glm::normalize(glm::vec3(lightInfo.lightPos));
	}

	// Computes light-space matrices for each cascade
	void ComputeDirectionalLightCascades(
		const glm::vec3& lightDir,
		const glm::mat4& cameraView,
		float fovDegrees,
		float aspectRatio,
		float nearPlane,
		float farPlane,
		int cascadeCount,
		const fixed_float cascadeSplits[],
		glm::mat4 outLightMatrices[]
	)
	{
		// Inverse camera view for frustum corner generation
		glm::mat4 invView = glm::inverse(cameraView);

		// Normalize light direction
		glm::vec3 lightDirection = glm::normalize(lightDir);

		float prevSplitDist = nearPlane;

		for (int cascade = 0; cascade < cascadeCount; cascade++)
		{
			float splitDist = cascadeSplits[cascade];

			// --- 1. Compute frustum corners in view space ---
			float nearDist = prevSplitDist;
			float farDist = splitDist;

			float tanHalfFov = tanf(glm::radians(fovDegrees * 0.5f));
			float nearHeight = nearDist * tanHalfFov;
			float nearWidth = nearHeight * aspectRatio;
			float farHeight = farDist * tanHalfFov;
			float farWidth = farHeight * aspectRatio;

			glm::vec3 frustumCornersVS[8] =
			{
				// near plane
				{ -nearWidth,  nearHeight, -nearDist },
				{  nearWidth,  nearHeight, -nearDist },
				{  nearWidth, -nearHeight, -nearDist },
				{ -nearWidth, -nearHeight, -nearDist },

				// far plane
				{ -farWidth,   farHeight,  -farDist },
				{  farWidth,   farHeight,  -farDist },
				{  farWidth,  -farHeight,  -farDist },
				{ -farWidth,  -farHeight,  -farDist }
			};

			// --- 2. Transform frustum corners to world space ---
			glm::vec3 frustumCornersWS[8];
			for (int i = 0; i < 8; i++)
			{
				glm::vec4 world = invView * glm::vec4(frustumCornersVS[i], 1.0f);
				frustumCornersWS[i] = glm::vec3(world);
			}

			// --- 3. Compute frustum center ---
			glm::vec3 center(0.0f);
			for (int i = 0; i < 8; i++)
				center += frustumCornersWS[i];
			center /= 8.0f;

			// --- 4. Build light view matrix ---
			float lightDistance = 100.0f;
			glm::vec3 lightPos = center - lightDirection * lightDistance;

			glm::mat4 lightView = glm::lookAt(
				lightPos,
				center,
				glm::vec3(0.0f, 1.0f, 0.0f)
			);

			// --- 5. Compute ortho bounds in light space ---
			glm::vec3 minLS(FLT_MAX);
			glm::vec3 maxLS(-FLT_MAX);

			for (int i = 0; i < 8; i++)
			{
				glm::vec3 cornerLS = glm::vec3(lightView * glm::vec4(frustumCornersWS[i], 1.0f));
				minLS = glm::min(minLS, cornerLS);
				maxLS = glm::max(maxLS, cornerLS);
			}

			const float depthPadding = 1000.0f;

			minLS.z -= depthPadding;
			maxLS.z += depthPadding;

			// stabilization
			float shadowMapRes = 2048.0f;
			glm::vec2 texelSize = (glm::vec2(maxLS) - glm::vec2(minLS)) / shadowMapRes;

			minLS.x = floor(minLS.x / texelSize.x) * texelSize.x;
			minLS.y = floor(minLS.y / texelSize.y) * texelSize.y;
			maxLS.x = minLS.x + texelSize.x * shadowMapRes;
			maxLS.y = minLS.y + texelSize.y * shadowMapRes;

			glm::mat4 lightProj = glm::ortho(
				minLS.x, maxLS.x,
				minLS.y, maxLS.y,
				-maxLS.z, -minLS.z
			);

			outLightMatrices[cascade] = lightProj * lightView;

			prevSplitDist = splitDist;
		}
	}

	void ComputePointLightMatrices(
		const glm::vec3& lightPos,
		float nearPlane,
		float farPlane,
		glm::mat4 outMatrices[6]
	)
	{
		glm::mat4 proj = glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, farPlane);

		outMatrices[0] = proj * glm::lookAt(lightPos, lightPos + glm::vec3(1, 0, 0), glm::vec3(0, -1, 0));
		outMatrices[1] = proj * glm::lookAt(lightPos, lightPos + glm::vec3(-1, 0, 0), glm::vec3(0, -1, 0));
		outMatrices[2] = proj * glm::lookAt(lightPos, lightPos + glm::vec3(0, 1, 0), glm::vec3(0, 0, 1));
		outMatrices[3] = proj * glm::lookAt(lightPos, lightPos + glm::vec3(0, -1, 0), glm::vec3(0, 0, -1));
		outMatrices[4] = proj * glm::lookAt(lightPos, lightPos + glm::vec3(0, 0, 1), glm::vec3(0, -1, 0));
		outMatrices[5] = proj * glm::lookAt(lightPos, lightPos + glm::vec3(0, 0, -1), glm::vec3(0, -1, 0));
	}

	float ComputePointLightFarPlane(const glm::vec3& attenuation)
	{
		float c = attenuation.x;
		float b = attenuation.y;
		float a = attenuation.z;

		float maxBrightness = 1.0f;
		float threshold = 0.01f; // 1%

		// Solve quadratic eq: a*d^2 + b*d + (c - maxBrightness/threshold) = 0
		float c2 = c - (maxBrightness / threshold); // drop-off at 1%
		float discriminant = b * b - 4 * a * c2;

		if (a == 0 || discriminant < 0)
			return 25.0f; // fallback

		float d = (-b + sqrt(discriminant)) / (2 * a);
		return std::max(d, 1.0f);
	}
}