#include "Engine/DataStructures/Transform.h"

// =================================================
// Transform
// =================================================
glm::mat4 Transform::GetModelMatrix() const {
	glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 rotationMatrix = glm::toMat4(rotation);
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
	return translationMatrix * rotationMatrix * scaleMatrix;
}

glm::mat4 Transform::GetGUIModelMatrix(glm::vec2 relativePosition, glm::vec2 relativeSize, glm::vec2 anchor, float screenWidth, float screenHeight) const {
	Transform t = *this;
	anchor -= glm::vec2(0.5f, 0.5f); // adjust anchor to be centered around (0,0)

	// calculate position in screen space (apply relativePosition first, then the transform.position as pixel offset)
    float posPxX = relativePosition.x * screenWidth + t.position.x;
    float posPxY = relativePosition.y * screenHeight + t.position.y;

    // same for scale
    float sizePxX = relativeSize.x * screenWidth + t.scale.x;
    float sizePxY = relativeSize.y * screenHeight + t.scale.y;

    posPxX -= (anchor.x) * sizePxX;
    posPxY -= (anchor.y) * sizePxY;

    t.position.x = posPxX / screenWidth;
    t.position.y = posPxY / screenHeight;

    t.scale.x = sizePxX / screenWidth;
    t.scale.y = sizePxY / screenHeight;


    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), t.position);

    float aspect = screenWidth / screenHeight;

    glm::mat4 aspectFix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, aspect, 1.0f));
    glm::mat4 invAspectFix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f / aspect, 1.0f));
	// make rotation be centered in the anchor point
    glm::mat4 rotationMatrix =  glm::translate(glm::mat4(1.0f), glm::vec3(anchor.x * t.scale.x, anchor.y * t.scale.y, 0.0f)) *
                                aspectFix * 
                                glm::toMat4(t.rotation) *
                                invAspectFix *
		                        glm::translate(glm::mat4(1.0f), glm::vec3(-anchor.x * t.scale.x, -anchor.y * t.scale.y, 0.0f));
	glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), t.scale);
	return translationMatrix * rotationMatrix * scaleMatrix;
}