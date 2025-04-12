#include "decompressor.h"
#include <iostream>

Decompressor::Decompressor(
    std::queue<Vertex> &vertices, 
    std::pair<int, std::vector<CLERS>> &clers, 
    const std::vector<Handle> &handles
) : _vertices(vertices), 
    _H(handles),
    _G(vertices.size(), {0, 0, 0}),
    _M(vertices.size(), 0)
{
    for(int i = 0; i < clers.first - 2; ++i){
        _clers.push(CLERS::C);
    }
    _clers.push(CLERS::R);
    for(auto c : clers.second){
        _clers.push(c);
    }

    int num_clers = _clers.size();
    int num_tri = num_clers + 1;

    _V.resize(3 * num_tri, 0);
    _O.resize(3 * num_tri, -3);
    _U.resize(num_tri, 0);

    _T = 0;
    _N = 2;
    _A = 0;

    _V[1] = 2;
    _V[2] = 1;
    _O[0] = -1;
    _O[1] = -1;
}

std::tuple<std::vector<Vertex>, std::vector<int>, std::vector<int>> Decompressor::decompress() {
    _decompressConectivity(2);
    _G[0] = _decodeDelta(0);
    _M[0] = 1;
    _G[1] = _decodeDelta(2);
    _M[1] = 1;
    _G[2] = _decodeDelta(1);
    _M[2] = 1;
    
    _N = 2;
    _U[0] = 1;
    _decompressVertices(_O[2]);

    return std::make_tuple(_G, _V, _O);
}

void Decompressor::_decompressConectivity(int c) {
    for(;;){
        ++_T;
        _O[c] = 3 * _T;
        _O[3 * _T] = c;
        _V[3 * _T + 1] = _V[P(c)];
        _V[3 * _T + 2] = _V[N(c)];
        c = N(_O[c]);

        CLERS cs = _clers.front();
        _clers.pop();
        switch (cs)
        {
        case CLERS::C:
            _O[N(c)] = -1;
            _V[3 * _T] = ++_N;
            break;
        case CLERS::L:
            _O[N(c)] = -2;
            if(!_checkHandle(N(c))){
                _zip(N(c));
            }
            break;  
        case CLERS::R:
            _O[c] = -2;
            _checkHandle(c);
            c = N(c);
            break;
        case CLERS::S:
            _decompressConectivity(c);
            c = N(c);
            if(_O[c] >= 0){
                return;
            }
            break;
        case CLERS::E:
            _O[c] = -2;
            _O[N(c)] = -2;
            _checkHandle(c);
            if(!_checkHandle(N(c))){
                _zip(N(c));
            }
            return;
        }
    }
}

void Decompressor::_decompressVertices(
    int c
) {
    for(;;){
        _U[T(c)] = 1;
        if(_M[_V[c]] == 0){
            ++_N;
            _G[_N] = _decodeDelta(c);
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
            _decompressVertices(R(_O, c));
            c = L(_O, c);
            if(_U[T(c)] > 0){
                return;
            }
        }
    }
}

bool Decompressor::_checkHandle(
    int c
) {
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
            _zip(a);
        }
        a = P(_O[a]);
        while(_O[a] >= 0 && a != c){
            a = P(_O[a]);
        }
        if(_O[a] == -2){
            _zip(a);
        }
        ++_A;
        return true;
    }
}

void Decompressor::_zip(
    int c
) {
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
        _zip(c);
    }
}

Vertex Decompressor::_decodeDelta(
    int c
) {
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