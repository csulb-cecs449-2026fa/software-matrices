#include <gtest/gtest.h>

#include <cmath>
#include <numbers>
#include <set>
#include <string_view>
#include <vector>

#include "matrices.h"
#include "raster_test_helpers.h"
#include "triangles.h"

namespace {

constexpr float tolerance{ 0.0001f };

void expectMatrixNear(
    std::string_view context,
    const glm::mat4& actual,
    const glm::mat4& expected) {
    SCOPED_TRACE(context);
    for (glm::length_t row{ 0 }; row < 4; ++row) {
        for (glm::length_t column{ 0 }; column < 4; ++column) {
            EXPECT_NEAR(
                actual[column][row],
                expected[column][row],
                tolerance)
                << "matrix row " << row + 1 << ", column " << column + 1;
        }
    }
}

void expectVectorNear(
    std::string_view context,
    const glm::vec4& actual,
    const glm::vec4& expected) {
    SCOPED_TRACE(context);
    EXPECT_NEAR(actual.x, expected.x, tolerance) << "x component";
    EXPECT_NEAR(actual.y, expected.y, tolerance) << "y component";
    EXPECT_NEAR(actual.z, expected.z, tolerance) << "z component";
    EXPECT_NEAR(actual.w, expected.w, tolerance) << "w component";
}

void expectVectorNear(
    std::string_view context,
    const glm::vec3& actual,
    const glm::vec3& expected) {
    SCOPED_TRACE(context);
    EXPECT_NEAR(actual.x, expected.x, tolerance) << "x component";
    EXPECT_NEAR(actual.y, expected.y, tolerance) << "y component";
    EXPECT_NEAR(actual.z, expected.z, tolerance) << "z component";
}

glm::vec3 perspectiveDivide(const glm::vec4& clip) {
    return glm::vec3{ clip } / clip.w;
}

TEST(BuildModelMatrix, AppliesScaleThenTranslation) {
    // With no rotation, the matrix should scale the local coordinates before
    // translating them into world space.
    const glm::mat4 model{
        buildModelMatrix(
            glm::vec3{ 5.0f, -2.0f, 1.0f },
            glm::vec3{ 0.0f, 0.0f, 0.0f },
            glm::vec3{ 2.0f, 3.0f, 4.0f })
    };
    // GLM constructor values are listed one column at a time.
    const glm::mat4 expectedMatrix{
        2.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 3.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 4.0f, 0.0f,
        5.0f, -2.0f, 1.0f, 1.0f
    };
    const glm::vec4 transformed{
        model * glm::vec4{ 1.0f, 2.0f, 3.0f, 1.0f }
    };
    const glm::vec4 expected{ 7.0f, 4.0f, 13.0f, 1.0f };

    expectMatrixNear("model matrix", model, expectedMatrix);
    expectVectorNear("scaled and translated vertex", transformed, expected);
}

TEST(BuildModelMatrix, AppliesYawPitchAndRollInLectureOrder) {
    // Ninety-degree rotations around all three axes make the expected result
    // easy to calculate while still detecting a different multiplication order.
    const float quarterTurn{ std::numbers::pi_v<float> / 2.0f };
    const glm::mat4 model{
        buildModelMatrix(
            glm::vec3{ 5.0f, -4.0f, 1.0f },
            glm::vec3{ quarterTurn, quarterTurn, quarterTurn },
            glm::vec3{ 2.0f, 3.0f, 4.0f })
    };
    const glm::vec4 transformed{
        model * glm::vec4{ 1.0f, 2.0f, 3.0f, 1.0f }
    };
    const glm::vec4 expected{ 3.0f, 8.0f, 7.0f, 1.0f };

    expectVectorNear("fully transformed vertex", transformed, expected);
}

TEST(BuildViewMatrix, PlacesTheCameraAtTheViewSpaceOrigin) {
    // The assignment camera should become the origin, its look-at point should
    // lie directly down the negative z-axis, and world +x should remain right.
    const glm::vec3 cameraPosition{ 0.0f, 3.0f, 0.0f };
    const glm::vec3 lookingAt{ 0.0f, 0.0f, -5.0f };
    const glm::mat4 view{ buildViewMatrix(cameraPosition, lookingAt) };
    const float distance{ std::sqrt(34.0f) };
    const glm::mat4 expectedMatrix{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 5.0f / distance, 3.0f / distance, 0.0f,
        0.0f, -3.0f / distance, 5.0f / distance, 0.0f,
        0.0f, -15.0f / distance, -9.0f / distance, 1.0f
    };

    const glm::vec4 transformedCamera{
        view * glm::vec4{ cameraPosition, 1.0f }
    };
    const glm::vec4 transformedTarget{
        view * glm::vec4{ lookingAt, 1.0f }
    };
    const glm::vec4 transformedRight{
        view * glm::vec4{ cameraPosition + glm::vec3{ 1.0f, 0.0f, 0.0f }, 1.0f }
    };

    expectMatrixNear("view matrix", view, expectedMatrix);
    expectVectorNear(
        "camera position",
        transformedCamera,
        glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f });
    expectVectorNear(
        "look-at point",
        transformedTarget,
        glm::vec4{ 0.0f, 0.0f, -distance, 1.0f });
    expectVectorNear(
        "point to the camera's right",
        transformedRight,
        glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f });
}

TEST(BuildPerspectiveMatrix, MapsTheFrustumToNdcBounds) {
    // A near-plane corner maps to the corresponding NDC corner, while the
    // center of the far plane maps to the far NDC depth bound.
    constexpr float nearPlane{ 1.0f };
    constexpr float farPlane{ 11.0f };
    constexpr float top{ 2.0f };
    constexpr float right{ 4.0f };
    const glm::mat4 projection{
        buildPerspectiveMatrix(nearPlane, farPlane, top, right)
    };
    const glm::mat4 expectedMatrix{
        0.25f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, -1.2f, -1.0f,
        0.0f, 0.0f, -2.2f, 0.0f
    };

    const glm::vec3 nearCorner{
        perspectiveDivide(
            projection * glm::vec4{ right, top, -nearPlane, 1.0f })
    };
    const glm::vec3 farCenter{
        perspectiveDivide(
            projection * glm::vec4{ 0.0f, 0.0f, -farPlane, 1.0f })
    };

    expectMatrixNear("perspective projection matrix", projection, expectedMatrix);
    expectVectorNear(
        "near top-right corner",
        nearCorner,
        glm::vec3{ 1.0f, 1.0f, -1.0f });
    expectVectorNear(
        "far-plane center",
        farCenter,
        glm::vec3{ 0.0f, 0.0f, 1.0f });
}

TEST(BuildOrthographicMatrix, MapsTheBoxToNdcBounds) {
    // Asymmetric bounds ensure that the projection includes the required
    // offsets rather than only scaling each coordinate.
    constexpr float nearPlane{ 2.0f };
    constexpr float farPlane{ 10.0f };
    constexpr float left{ -4.0f };
    constexpr float right{ 6.0f };
    constexpr float bottom{ -3.0f };
    constexpr float top{ 5.0f };
    const glm::mat4 projection{
        buildOrthographicMatrix(
            nearPlane, farPlane, left, right, bottom, top)
    };
    const glm::mat4 expectedMatrix{
        0.2f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.25f, 0.0f, 0.0f,
        0.0f, 0.0f, -0.25f, 0.0f,
        -0.2f, -0.25f, -1.5f, 1.0f
    };

    const glm::vec4 nearCorner{
        projection * glm::vec4{ left, bottom, -nearPlane, 1.0f }
    };
    const glm::vec4 farCorner{
        projection * glm::vec4{ right, top, -farPlane, 1.0f }
    };
    const glm::vec4 center{
        projection * glm::vec4{ 1.0f, 1.0f, -6.0f, 1.0f }
    };

    expectMatrixNear("orthographic projection matrix", projection, expectedMatrix);
    expectVectorNear(
        "near left-bottom corner",
        nearCorner,
        glm::vec4{ -1.0f, -1.0f, -1.0f, 1.0f });
    expectVectorNear(
        "far right-top corner",
        farCorner,
        glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
    expectVectorNear(
        "box center",
        center,
        glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f });
}

TEST(DrawMesh, AppliesModelViewProjectionForOrthographicProjection) {
    // The transforms do not commute: translating in model space and then
    // rotating in view space gives different pixels if M, V, and P are
    // multiplied in any order other than projection * view * model.
    const Pixel background{};
    const Pixel edgeColor{ 255, 255, 255, 255 };
    Framebuffer framebuffer{ 20, 20, background };
    const std::vector<Vertex3D> vertices{
        { 0.0f, 0.0f, 0.0f },
        { 0.5f, 0.0f, 0.0f },
        { 0.0f, 0.5f, 0.0f }
    };
    const std::vector<std::uint32_t> faces{ 0, 1, 2 };

    const glm::mat4 model{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.0f, -3.0f, 1.0f
    };
    const glm::mat4 view{
        0.0f, 1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    const glm::mat4 orthographic{
        0.4f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.4f, 0.0f, 0.0f,
        0.0f, 0.0f, -0.2f, 0.0f,
        0.0f, 0.0f, -1.2f, 1.0f
    };

    drawMesh(
        framebuffer,
        model,
        view,
        orthographic,
        vertices,
        faces,
        edgeColor);

    Framebuffer expectedFramebuffer{ 20, 20, background };
    drawTriangle(
        expectedFramebuffer,
        glm::ivec2{ 10, 8 },
        glm::ivec2{ 10, 6 },
        glm::ivec2{ 8, 8 },
        edgeColor);
    const std::set<PixelPosition> expectedPositions{
        nonBackgroundPositions(expectedFramebuffer, background)
    };

    EXPECT_EQ(
        nonBackgroundPositions(framebuffer, background),
        expectedPositions);
}

TEST(DrawMesh, PerformsPerspectiveDivideBeforeScreenMapping) {
    // Each vertex reaches a different view-space depth. Dividing clip x and y
    // by w must therefore shrink the farther vertex more before rasterization.
    const Pixel background{};
    const Pixel edgeColor{ 255, 255, 255, 255 };
    Framebuffer framebuffer{ 20, 20, background };
    const std::vector<Vertex3D> vertices{
        { -0.6f, -0.6f, 0.0f },
        { 0.6f, -0.6f, 0.0f },
        { 0.0f, 1.0f, -2.0f }
    };
    const std::vector<std::uint32_t> faces{ 0, 1, 2 };

    const glm::mat4 model{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, -1.0f, 1.0f
    };
    const glm::mat4 view{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, -2.0f, 1.0f
    };
    const glm::mat4 perspective{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, -11.0f / 9.0f, -1.0f,
        0.0f, 0.0f, -20.0f / 9.0f, 0.0f
    };

    drawMesh(
        framebuffer,
        model,
        view,
        perspective,
        vertices,
        faces,
        edgeColor);

    Framebuffer expectedFramebuffer{ 20, 20, background };
    drawTriangle(
        expectedFramebuffer,
        glm::ivec2{ 8, 12 },
        glm::ivec2{ 12, 12 },
        glm::ivec2{ 10, 8 },
        edgeColor);
    const std::set<PixelPosition> expectedPositions{
        nonBackgroundPositions(expectedFramebuffer, background)
    };

    EXPECT_EQ(
        nonBackgroundPositions(framebuffer, background),
        expectedPositions);
}

TEST(ClipToScreen, MapsNdcCenterToFramebufferCenter) {
    // NDC (0, 0) is the center even though screen y increases downward.
    const Framebuffer framebuffer{ 100, 50 };
    const glm::ivec2 screen{
        clipToScreen(framebuffer, glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f })
    };
    const glm::ivec2 expected{ 50, 25 };

    EXPECT_EQ(screen, expected);
}

} // namespace
