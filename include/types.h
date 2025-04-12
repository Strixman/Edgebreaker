#pragma once
#include <array>

struct File {
    enum class EXT {
        OBJ,
        OFF,
        OVX
    };
    enum class Type {
        BIN,
        ASCII
    };
};

using Vertex = std::array<float,3>;

using Indices = std::array<int, 3>;

using Edge = std::array<int, 2>;