#pragma once

#include <vector>
#include <tuple>
#include <queue>

#include "types.h"

#define N(c) (3 * ((c) / 3) + ((c) + 1) % 3)
#define P(c) (N(N(c)))
#define T(c) ((c) / 3)
#define R(O, c) (O[N(c)])
#define L(O, c) (O[N(N(c))])

class Decompressor{
public:
    Decompressor(
        std::queue<Vertex>& vertices, 
        std::pair<int, std::vector<CLERS>>& clers,
        const std::vector<Handle>& handles
    ); 
public:
    void decompress(
        std::vector<Vertex>& _G,
        std::vector<int>& _V,
        std::vector<int>& _O
    );
private:
    std::queue<Vertex>& _vertices; 
    std::vector<CLERS> _clers;
    const std::vector<Handle>& _H;

    int _T = 0;
    int _N = 2;
    int _A = 0;

    std::vector<int> _M;
    std::vector<int> _U;

private:
    void _decompressConectivity(
        int c,
        std::vector<int>& _V,
        std::vector<int>& _O
    );
    void _decompressVertices(
        int c,
        std::vector<Vertex>& _G,
        std::vector<int>& _V,
        std::vector<int>& _O
    );
    bool _checkHandle(
        int c,
        std::vector<int>& _V,
        std::vector<int>& _O
    );
    void _zip(
        int c,
        std::vector<int>& _V,
        std::vector<int>& _O
    );
    Vertex _decodeDelta(
        int c,
        std::vector<Vertex>& _G,
        std::vector<int>& _V,
        std::vector<int>& _O
    );
};