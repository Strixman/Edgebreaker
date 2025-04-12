#include "compressor.h"

Compressor::Compressor(
    const std::vector<Vertex> &vert,
    const std::vector<int> &V, 
    const std::vector<int> &O
) : _G(vert), _V(V), _O(O), _M(vert.size(), 0), _U(V.size() / 3, 0), _D(vert.size(), {0, 0, 0}) { }

void Compressor::compress(
    int c,
    std::vector<Vertex>& vertices,
    std::pair<int, std::vector<CLERS>>& clers,
    std::vector<Handle>& handles
) {
    _reset();

    _encodeDelta(N(c), vertices);
    _M[_V[N(c)]] = 1;
    _encodeDelta(c, vertices);
    _M[_V[c]] = 1;
    _encodeDelta(P(c), vertices);
    _M[_V[P(c)]] = 1;
    _U[T(c)] = 1;

    int count = 1;
    int a = _O[c];
    while(a != P(_O[P(c)])){
        _U[T(a)] = 1;
        ++_T;
        ++count;
        _encodeDelta(a, vertices);
        _M[_V[a]] = 1;
        a = _O[N(a)];
    }
    _U[T(a)] = 1;
    ++_T;
    ++count;
    clers.first = count;

    _compress(_O[P(a)], vertices, clers, handles);
}

void Compressor::_compress(
    int c,
    std::vector<Vertex>& vertices,
    std::pair<int, std::vector<CLERS>>& clers,
    std::vector<Handle>& handles
) {
    for(;;){
        _U[T(c)] = 1;
        ++_T;

        if(_U[T(_O[N(c)])] > 1) {
            handles.push_back({_U[T(_O[N(c)])], _T * 3 + 1});
        }
        if(_U[T(_O[P(c)])] > 1) {
            handles.push_back({_U[T(_O[P(c)])], _T * 3 + 2});
        }

        if(_M[_V[c]] == 0) {
            _encodeDelta(c, vertices);
            _M[_V[c]] = 1;

            clers.second.push_back(CLERS::C);
            c = R(_O, c);
        }
        else {
            if(_U[T(R(_O, c))] > 0) {
                if(_U[T(L(_O, c))] > 0) {
                    clers.second.push_back(CLERS::E);
                    return;
                }
                else {
                    clers.second.push_back(CLERS::R);
                    c = L(_O, c);
                }
            }
            else {
                if(_U[T(L(_O, c))] > 0) {
                    clers.second.push_back(CLERS::L);
                    c = R(_O, c);
                }
                else {
                    clers.second.push_back(CLERS::S);
                    _U[T(c)] = _T * 3 + 2;
                    
                    _compress(R(_O, c), vertices, clers, handles);

                    c = L(_O, c);

                    if(_U[T(c)] > 0) {
                        return;
                    }
                }
            }
        }
    }
}

void Compressor::_encodeDelta(
    int c,
    std::vector<Vertex>& vertices
) {
    Vertex pred, delta;
    if (_M[_V[_O[c]]] > 0 && _M[_V[P(c)]] > 0) {
        // Case 1: a, b, d known (a + b - d)
        pred = _D[_V[N(c)]] + _D[_V[P(c)]] - _D[_V[_O[c]]];
        delta = _G[_V[c]] - pred;
    } else if (_M[_V[_O[c]]] > 0) {
        // Case 2: a and d known (2a - d)
        pred = 2 * _D[_V[N(c)]] - _D[_V[_O[c]]];
        delta = _G[_V[c]] - pred;
    } else if (_M[_V[N(c)]] > 0 && _M[_V[P(c)]] > 0) {
        // Case 3: a and b known ((a + b)/2)
        pred = (_D[_V[N(c)]] + _D[_V[P(c)]]) / 2.0;
        delta = _G[_V[c]] - pred;
    } else if (_M[_V[N(c)]] > 0) {
        // Case 4: a known (a)
        pred = _D[_V[N(c)]];
        delta = _G[_V[c]] - pred;
    } else if (_M[_V[P(c)]] > 0) {
        // Case 5: b known (b)
        pred = _D[_V[P(c)]];
        delta = _G[_V[c]] - pred;
    } else {
        // Case 6: nothing known (0)
        pred = Vertex({0, 0, 0});
        delta = _G[_V[c]] - pred;
    }

    _D[_V[c]] = delta + pred;
    vertices.push_back(delta);
}

void Compressor::_reset() {
    std::fill(_M.begin(), _M.end(), 0);
    std::fill(_U.begin(), _U.end(), 0);
    std::fill(_D.begin(), _D.end(), Vertex({0, 0, 0}));

    _T = 0;
}
