#include "GameObjects.h"



Camera::Camera(float aspectRatio, float fov)
    : projectionMatrix(createProjectionMatrix(aspectRatio, fov))
{
}

glm::mat4 Camera::getProjectionView() const
{
    return projectionMatrix * createViewMartix(position, rotation);
}

void Paddle::update(float dt)
{
    position += velocity * dt;
    velocity *= 0.96;
    roomCollide(*this, [](float& v) { v = 0; });
    aabb.update(*this);
}

void Ball::update(float dt)
{
    position += velocity * dt;
    roomCollide(*this, [](float& v) { v *= -1; });
    aabb.update(*this);
}

void Ball::bounceOffPaddle(const Paddle& paddle)
{
    glm::vec2 paddleCenter = {paddle.position.x + Paddle::WIDTH / 2,
                              paddle.position.y + Paddle::HEIGHT / 2};
    glm::vec2 ballCenter = {position.x + Ball::WIDTH / 2, position.y + Ball::HEIGHT / 2};
    auto offset = paddleCenter - ballCenter;
    velocity.x = -offset.x * 5;
    velocity.y = -offset.y * 5;
}
