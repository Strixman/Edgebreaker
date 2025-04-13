#include "decompressor.h"
#include <iostream>

Decompressor::Decompressor(
    std::queue<Vertex> &vertices, 
    std::pair<int, std::vector<CLERS>> &clers, 
    const std::vector<Handle> &handles
) : _vertices(vertices), 
    _H(handles),
    _M(vertices.size(), 0)
{
    for(int i = 0; i < clers.first - 2; ++i){
        _clers.push_back(CLERS::C);
    }
    _clers.push_back(CLERS::R);
    for(auto c : clers.second){
        _clers.push_back(c);
    }

    _U.resize((_clers.size() + 1), 0);
}

void Decompressor::decompress(
    std::vector<Vertex>& _G,
    std::vector<int>& _V,
    std::vector<int>& _O
) {
    _G.resize(_vertices.size(), {0, 0, 0});
    _V.resize(3 * (_clers.size() + 1), 0);
    _O.resize(3 * (_clers.size() + 1), -3);

    _T = 0;
    _N = 2;
    _A = 0;

    _V[0] = 0;
    _V[1] = 2;
    _V[2] = 1;
    _O[0] = -1;
    _O[1] = -1;

    _decompressConectivity(2, _V, _O);

    _G[0] = _decodeDelta(0, _G, _V, _O);
    _M[0] = 1;
    _G[1] = _decodeDelta(2, _G, _V, _O);
    _M[1] = 1;
    _G[2] = _decodeDelta(1, _G, _V, _O);
    _M[2] = 1;
 
    _N = 2;
    _U[0] = 1;

    _decompressVertices(_O[2], _G, _V, _O);
}

void Decompressor::_decompressConectivity(
    int c,
    std::vector<int>& _V,
    std::vector<int>& _O
) {
    std::cout << "dc(" << c << "),";
    for(;;){
        ++_T;
        _O[c] = 3 * _T;
        _O[3 * _T] = c;
        _V[3 * _T + 1] = _V[P(c)];
        _V[3 * _T + 2] = _V[N(c)];
        c = N(_O[c]);

        switch (_clers[_T - 1])
        {
        case CLERS::C:
            _O[N(c)] = -1;
            ++_N;
            _V[3 * _T] = _N;
            break;
        case CLERS::L:
            _O[N(c)] = -2;
            if(!_checkHandle(N(c), _V, _O)){
                _zip(N(c), _V, _O);
            }
            break;  
        case CLERS::R:
            _O[c] = -2;
            _checkHandle(c, _V, _O);
            c = N(c);
            break;
        case CLERS::S:
            _decompressConectivity(c, _V, _O);
            c = N(c);
            if(_O[c] >= 0){
                return;
            }
            break;
        case CLERS::E:
            _O[c] = -2;
            _O[N(c)] = -2;
            _checkHandle(c, _V, _O);
            if(!_checkHandle(N(c), _V, _O)){
                _zip(N(c), _V, _O);
            }
            return;
        }
    }
}

void Decompressor::_decompressVertices(
    int c,
    std::vector<Vertex>& _G,
    std::vector<int>& _V,
    std::vector<int>& _O
) {
    std::cout << "dv(" << c << "),";
    for(;;){
        _U[T(c)] = 1;
        if(_M[_V[c]] == 0){
            ++_N;
            _G[_N] = _decodeDelta(c, _G, _V, _O);
            _M[_V[c]] = 1;
            c = R(_O, c);
        }
        else if(_U[T(R(_O, c))] == 1){
            if(_U[T(L(_O, c))] == 1){
                return;
            }
            else{
                c = L(_O, c);
            }
        }
        else if(_U[T(L(_O, c))] == 1) {
            c = R(_O, c);
        }
        else{
            _decompressVertices(R(_O, c), _G, _V, _O);
            c = L(_O, c);
            if(_U[T(c)] > 0){
                return;
            }
        }
    }
}

bool Decompressor::_checkHandle(
    int c,
    std::vector<int>& _V,
    std::vector<int>& _O
) {
    std::cout << "ch(" << c << "),";
    if(_A >= _H.size() || c != _H[_A][1]){
        return false;
    }
    else{
        _O[c] = _H[_A][0];
        _O[_H[_A][0]] = c;

        int a = P(c);
        while(_O[a] >= 0 && a != _H[_A][0]){
            a = P(_O[a]);
        }
        if(_O[a] == -2){
            _zip(a, _V, _O);
        }

        a = P(_O[c]);
        while(_O[a] >= 0 && a != c){          
            a = P(_O[a]);
        }
        if(_O[a] == -2){
            _zip(a, _V, _O);
        }

        ++_A;
        return true;
    }
}

void Decompressor::_zip(
    int c,
    std::vector<int>& _V,
    std::vector<int>& _O
) {
    std::cout << "z(" << c << "),";
    int b = N(c);
    while(_O[b] >= 0 && _O[b] != c){
        b = N(_O[b]);
    }
    if(_O[b] != -1){
        return;
    }

    _O[c] = b;
    _O[b] = c;
    int a = N(c);
    _V[N(a)] = _V[N(b)];

    while(_O[a] >= 0 && a != b) {
        a = N(_O[a]);
        _V[N(a)] = _V[N(b)];
    }

    c = P(c);
    while(_O[c] >= 0 && c != b){
        c = P(_O[c]);
    }
    if(_O[c] == -2){
        _zip(c, _V, _O);
    }
}

Vertex Decompressor::_decodeDelta(
    int c,
    std::vector<Vertex>& _G,
    std::vector<int>& _V,
    std::vector<int>& _O
) {
    std::cout << "d(" << c << "),";
    return {0, 0, 0};

    Vertex delta = _vertices.front();
    _vertices.pop();

    if (_M[_V[_O[c]]] > 0 && _M[_V[P(c)]] > 0) {
        // Case 1: a, b, d known (a + b - d)
        return delta + (_G[_V[N(c)]] + _G[_V[P(c)]] - _G[_V[_O[c]]]);
    } else if (_M[_V[_O[c]]] > 0) {
        // Case 2: a and d known (2a - d)
        return delta + (2 * _G[_V[N(c)]] - _G[_V[_O[c]]]);
    } else if (_M[_V[N(c)]] > 0 && _M[_V[P(c)]] > 0) {
        // Case 3: a and b known ((a + b)/2)
        return delta + ((_G[_V[N(c)]] + _G[_V[P(c)]]) / 2.0f);
    } else if (_M[_V[N(c)]] > 0) {
        // Case 4: a known (a)
        return delta + _G[_V[N(c)]];
    } else if (_M[_V[P(c)]] > 0) {
        // Case 5: b known (b)
        return delta + _G[_V[P(c)]];
    }

    // Case 6: nothing known
    return delta;
}