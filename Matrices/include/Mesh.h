#pragma once

#include <cstdint>
#include <vector>

struct Vertex3D {
    float x;
    float y;
    float z;
};

class Mesh {
private:
    std::vector<Vertex3D> m_vertices;
    std::vector<std::uint32_t> m_faces;
};
