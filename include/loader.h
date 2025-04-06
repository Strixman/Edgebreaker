#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <format>
#include <sstream>
#include <string>

#include "types.h"

class LoaderException: public std::exception {
public:
    explicit LoaderException(const std::string& msg) : message(msg) {}
public:
    const char* what() const noexcept override {
        return message.c_str();
    }
private:
    std::string message;
};

class Loader {
public:
    enum class EXT {
        OBJ,
        OFF,
    };
    static std::pair<std::vector<Vertex>, std::vector<Indices>> load(
        const std::string& infile,
        const EXT ext
    );

    static std::pair<std::vector<Vertex>, std::vector<Indices>> load_OBJ(
        const std::string& infile
    );

    static std::pair<std::vector<Vertex>, std::vector<Indices>> load_OFF(
        const std::string& infile
    );
};