#include "Maths.h"

glm::mat4 createModelMatrix(const glm::vec3& pos, const glm::vec3& rot)
{
    glm::mat4 matrix{1.0f};

    matrix = glm::rotate(matrix, glm::radians(rot.x), {1, 0, 0});
    matrix = glm::rotate(matrix, glm::radians(rot.y), {0, 1, 0});
    matrix = glm::rotate(matrix, glm::radians(rot.z), {0, 0, 1});

    matrix = glm::translate(matrix, pos);

    return matrix;
}

glm::mat4 createViewMartix(const glm::vec3& pos, const glm::vec3& rot)
{
    glm::mat4 matrix{1.0f};

    matrix = glm::rotate(matrix, glm::radians(rot.x), {1, 0, 0});
    matrix = glm::rotate(matrix, glm::radians(rot.y), {0, 1, 0});
    matrix = glm::rotate(matrix, glm::radians(rot.z), {0, 0, 1});

    matrix = glm::translate(matrix, -pos);

    return matrix;
}

glm::mat4 createProjectionMatrix(float aspectRatio, float fov)
{
    return glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 256.0f);
}
