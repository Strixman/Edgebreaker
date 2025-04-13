#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <format>
#include <sstream>
#include <string>
#include <queue>

#include "types.h"

class ReaderException: public std::exception {
public:
    explicit ReaderException(const std::string& msg) : message(msg) {}
public:
    const char* what() const noexcept override {
        return message.c_str();
    }
private:
    std::string message;
};

class Reader {
public:
    static std::pair<std::vector<Vertex>, std::vector<Indices>> read_OBJ(
        const std::string& infile
    );

    static std::pair<std::vector<Vertex>, std::vector<Indices>> read_OFF(
        const std::string& infile
    );
    static std::pair<std::vector<Vertex>, std::vector<std::tuple<std::vector<int>, std::vector<int>, std::vector<Dummy>>>> read_OVX(
        const std::string& infile
    );
    static std::vector<std::tuple<std::queue<Vertex>, std::pair<int, std::vector<CLERS>>, std::vector<Handle>, std::vector<Dummy>>> read_Compressed(
        const std::string& infile
    );
};