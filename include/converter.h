#pragma once

#include <iostream>
#include <vector>
#include <tuple>

#include "types.h"

class Converter {
public:
    static std::vector<std::tuple<std::vector<int>, std::vector<int>, std::vector<int>>> toOVX(
        std::vector<Vertex>& vert, 
        std::vector<Indices>& tri
    );
    static std::pair<std::vector<Vertex>, std::vector<Indices>> fromOVX(
        const std::vector<int>& O, 
        const std::vector<int>& V, 
        const std::vector<int>& dummy
    );
};