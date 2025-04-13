#pragma once

#include <vector>
#include <tuple>

#include "types.h"

#define N(c) (3 * ((c) / 3) + ((c) + 1) % 3)
#define P(c) (N(N(c)))
#define T(c) ((c) / 3)
#define R(O, c) (O[N(c)])
#define L(O, c) (O[N(N(c))])

class Compressor{
public:
    Compressor(
        const std::vector<Vertex>& vert,
        const std::vector<int>& V,
        const std::vector<int>& O
    ); 
public:
    void compress(
        int c,
        std::vector<Vertex>& vertices,
        std::pair<int, std::vector<CLERS>>& clers,
        std::vector<Handle>& handles,
        std::vector<Dummy>& dummy
    );
private:
    int _T = 0;
    std::vector<int> _M;
    std::vector<int> _U;   
    std::vector<Vertex> _D;

    const std::vector<Vertex>& _G;
    const std::vector<int>& _V;
    const std::vector<int>& _O;
private:
    void _compress(
        int c,
        std::vector<Vertex>& vertices,
        std::pair<int, std::vector<CLERS>>& clers,
        std::vector<Handle>& handles,
        std::vector<Dummy>& dummy
    );
    void _reset();
    void _encodeDelta(
        int c,
        std::vector<Vertex>& vertices,
        std::vector<Dummy>& dummy
    );
};