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

enum class CLERS {
    C, L, E, R, S
};

using Indices = std::array<int, 3>;

using Edge = std::array<int, 2>;

using Handle = std::array<int, 2>;

using Vertex = std::array<float,3>;
inline Vertex operator+(const Vertex& a, const Vertex& b){
    return { a[0] + b[0], a[1] + b[1], a[2] + b[2] };
}

inline Vertex operator-(const Vertex& a, const Vertex& b){
    return { a[0] - b[0], a[1] - b[1], a[2] - b[2] };
}

inline Vertex operator*(float a, const Vertex& b){
    return { a * b[0], a * b[1], a * b[2] };
}

inline Vertex operator/(const Vertex& b, float a){
    return { a / b[0], a / b[1], a / b[2] };
}

using Dummy = std::pair<int, Vertex>;