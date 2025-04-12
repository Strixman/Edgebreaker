#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <format>
#include <sstream>
#include <string>

#include "types.h"

class WriterException: public std::exception {
    public:
        explicit WriterException(const std::string& msg) : message(msg) {}
    public:
        const char* what() const noexcept override {
            return message.c_str();
        }
    private:
        std::string message;
    };
    

class Writer {
public:
    static void write_OVX(
        const std::string& outfile,
        const std::vector<Vertex>& vert,
        const std::vector<std::tuple<std::vector<int>, std::vector<int>, std::vector<int>>>& ovx
    );
    static void write_Compressed(
        const std::string& outfile,
        const std::vector<std::tuple<std::vector<Vertex>, std::pair<int, std::vector<CLERS>>, std::vector<Handle>, std::vector<int>>>& compressed
    );
};