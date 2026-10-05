#include "matrices.h"

#include <cstddef>

#include <glm/ext.hpp>

#include "triangles.h"

glm::mat4 buildModelMatrix(
    const glm::vec3& position,
    const glm::vec3& orientation,
    const glm::vec3& scale) {
    glm::mat4 model{ 1 };

    return model;
}

glm::mat4 buildViewMatrix(
    const glm::vec3& cameraPosition,
    const glm::vec3& lookingAt) {
    // TODO: Construct and return the inverse camera matrix.
    return glm::mat4{ 1 };
}

glm::mat4 buildPerspectiveMatrix(
    float nearPlane,
    float farPlane,
    float top,
    float right) {
    // TODO: Construct and return the perspective projection matrix.
    return glm::mat4{ 1 };
}

glm::mat4 buildOrthographicMatrix(
    float nearPlane,
    float farPlane,
    float left,
    float right,
    float bottom,
    float top) {
    // TODO: Construct and return the orthographic projection matrix.
    return glm::mat4{ 1 };
}

glm::ivec2 clipToScreen(
    const Framebuffer& framebuffer,
    const glm::vec4& clip) {
    const std::int32_t xs{
        static_cast<std::int32_t>(framebuffer.width() * (clip.x + 1) / 2.0)
    };
    const std::int32_t ys{
        static_cast<std::int32_t>(
            framebuffer.height() - framebuffer.height() * (clip.y + 1) / 2.0)
    };
    return glm::ivec2{ xs, ys };
}

void drawMesh(
    Framebuffer& framebuffer,
    const glm::mat4& modelMatrix,
    const glm::mat4& viewMatrix,
    const glm::mat4& projectionMatrix,
    const std::vector<Vertex3D>& vertices,
    const std::vector<std::uint32_t>& faces,
    Pixel color) {
    // TODO: Construct the MVP matrix. Multiplication order matters.
    glm::mat4 mvp{};

    for (std::size_t i{ 0 }; i < faces.size(); i += 3) {
        const auto& localA{ vertices[faces[i]] };
        const auto& localB{ vertices[faces[i + 1]] };
        const auto& localC{ vertices[faces[i + 2]] };

        // TODO: Transform each local-space vertex to clip space with the MVP
        // matrix, including the perspective divide.
        glm::vec4 clipA{ localA.x, localA.y, localA.z, 1.0f };
        glm::vec4 clipB{ localB.x, localB.y, localB.z, 1.0f };
        glm::vec4 clipC{ localC.x, localC.y, localC.z, 1.0f };

        const auto screenA{ clipToScreen(framebuffer, clipA) };
        const auto screenB{ clipToScreen(framebuffer, clipB) };
        const auto screenC{ clipToScreen(framebuffer, clipC) };

        drawTriangle(framebuffer, screenA, screenB, screenC, color);
    }
}
