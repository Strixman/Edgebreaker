#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <format>
#include <sstream>
#include <string>

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
    static std::pair<std::vector<Vertex>, std::vector<std::tuple<std::vector<int>, std::vector<int>, std::vector<int>>>> read_OVX(
        const std::string& infile
    );
};