#pragma once

#include <iostream>
#include <vector>
#include <tuple>

#include "types.h"

class Converter {
public:
    static std::vector<std::tuple<std::vector<int>, std::vector<int>, std::vector<Dummy>>> toOVX(
        std::vector<Vertex>& vert, 
        std::vector<Indices>& tri
    );
    static std::pair<std::vector<Vertex>, std::vector<Indices>> fromOVX(
        const std::vector<std::tuple<std::vector<Vertex>, std::vector<int>, std::vector<int>, std::vector<Dummy>>>& ovx
    );
};