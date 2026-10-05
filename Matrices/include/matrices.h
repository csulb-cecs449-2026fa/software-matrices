#pragma once

#include <cstdint>
#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "Vertex3D.h"
#include "framebuffer.h"

glm::mat4 buildModelMatrix(
    const glm::vec3& position,
    const glm::vec3& orientation,
    const glm::vec3& scale);

glm::mat4 buildViewMatrix(
    const glm::vec3& cameraPosition,
    const glm::vec3& lookingAt);

glm::mat4 buildPerspectiveMatrix(
    float nearPlane,
    float farPlane,
    float top,
    float right);

glm::mat4 buildOrthographicMatrix(
    float nearPlane,
    float farPlane,
    float left,
    float right,
    float bottom,
    float top);

glm::ivec2 clipToScreen(
    const Framebuffer& framebuffer,
    const glm::vec4& clip);

void drawMesh(
    Framebuffer& framebuffer,
    const glm::mat4& modelMatrix,
    const glm::mat4& viewMatrix,
    const glm::mat4& projectionMatrix,
    const std::vector<Vertex3D>& vertices,
    const std::vector<std::uint32_t>& faces,
    Pixel color);
