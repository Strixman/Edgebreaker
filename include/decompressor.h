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
    std::tuple<std::vector<Vertex>, std::vector<int>, std::vector<int>> decompress();
private:
    std::queue<Vertex>& _vertices; 
    std::queue<CLERS> _clers;
    const std::vector<Handle>& _H;

    int _T = 0;
    int _N = 2;
    int _A = 0;

    std::vector<Vertex> _G;
    std::vector<int> _V;
    std::vector<int> _O;
    std::vector<int> _M;
    std::vector<int> _U;

private:
    void _decompressConectivity(
        int c
    );
    void _decompressVertices(
        int c
    );
    bool _checkHandle(
        int c
    );
    void _zip(
        int c
    );
    Vertex _decodeDelta(
        int c
    );
};