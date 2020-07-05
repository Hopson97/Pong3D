#pragma once

#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 createModelMatrix(const glm::vec3& pos, const glm::vec3& rot);
glm::mat4 createViewMartix(const glm::vec3& pos, const glm::vec3& rot);
glm::mat4 createProjectionMatrix(float aspectRatio, float fov);