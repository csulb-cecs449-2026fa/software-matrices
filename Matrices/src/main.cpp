#include <cstdint>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <numbers>
#include <optional>
#include <string>
#include <vector>

#include <SFML/Graphics.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glm/ext.hpp>

#include "matrices.h"

namespace {

constexpr std::size_t verticesPerFace{ 3 };

void fromAssimpMesh(
    const aiMesh* mesh,
    std::vector<Vertex3D>& vertices,
    std::vector<std::uint32_t>& faces) {
    for (std::size_t i{ 0 }; i < mesh->mNumVertices; ++i) {
        vertices.push_back(Vertex3D{
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        });
    }

    faces.reserve(mesh->mNumFaces * verticesPerFace);
    for (std::size_t i{ 0 }; i < mesh->mNumFaces; ++i) {
        faces.push_back(mesh->mFaces[i].mIndices[0]);
        faces.push_back(mesh->mFaces[i].mIndices[1]);
        faces.push_back(mesh->mFaces[i].mIndices[2]);
    }
}

void assimpLoad(
    const std::string& path,
    std::vector<Vertex3D>& vertices,
    std::vector<std::uint32_t>& faces) {
    Assimp::Importer importer{};
    const aiScene* scene{
        importer.ReadFile(path, aiProcessPreset_TargetRealtime_MaxQuality)
    };

    if (scene == nullptr) {
        std::cout << "ASSIMP ERROR" << importer.GetErrorString() << std::endl;
        std::exit(1);
    }

    fromAssimpMesh(scene->mMeshes[0], vertices, faces);
}

} // namespace

int main() {
    sf::RenderWindow window{
        sf::VideoMode::getFullscreenModes().at(0),
        "SFML Demo"
    };
    const auto windowSize{ window.getSize() };
    Framebuffer framebuffer{
        static_cast<int>(windowSize.x),
        static_cast<int>(windowSize.y)
    };
    sf::Texture framebufferTexture{ windowSize };
    framebufferTexture.setSmooth(false);
    sf::Sprite framebufferSprite{ framebufferTexture };

    std::vector<Vertex3D> bunnyVertices{};
    std::vector<std::uint32_t> bunnyFaces{};
    assimpLoad("models/bunny.obj", bunnyVertices, bunnyFaces);

    const glm::vec3 bunnyPosition{ 0.0f, -1.0f, -2.5f };
    glm::vec3 bunnyOrientation{ 0.0f, 0.0f, 0.0f };
    const glm::vec3 bunnyScale{ 9.0f, 9.0f, 9.0f };

    const float fovy{ 60.0f };
    const float ratio{ static_cast<float>(windowSize.x) / windowSize.y };
    const float near{ 0.1f };
    const float far{ 100.0f };
    const float top{
        near * std::tan((fovy * std::numbers::pi_v<float> / 180.0f) / 2)
    };
    const float bottom{ -top };
    const float right{ top * ratio };
    const float left{ -right };

    const glm::vec3 cameraPosition{ 0.0f, 3.0f, 0.0f };
    const glm::vec3 lookingAt{ 0.0f, 0.0f, -5.0f };

    while (window.isOpen()) {
        while (const std::optional event{ window.pollEvent() }) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        bunnyOrientation.y += 0.001f;

        const glm::mat4 bunnyModelMatrix{
            buildModelMatrix(bunnyPosition, bunnyOrientation, bunnyScale)
        };
        const glm::mat4 viewMatrix{
            buildViewMatrix(cameraPosition, lookingAt)
        };
        const glm::mat4 projectionMatrix{
            buildPerspectiveMatrix(near, far, top, right)
        };

        framebuffer.clear(Pixel{});
        drawMesh(
            framebuffer,
            bunnyModelMatrix,
            viewMatrix,
            projectionMatrix,
            bunnyVertices,
            bunnyFaces,
            Pixel{ 255, 255, 255, 255 });

        framebufferTexture.update(
            reinterpret_cast<const std::uint8_t*>(framebuffer.data().data()));
        window.clear(sf::Color::Black);
        window.draw(framebufferSprite);
        window.display();
    }

    return 0;
}
