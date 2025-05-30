#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <format>
#include <sstream>
#include <string>
#include <iomanip> 

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
        const std::vector<std::tuple<std::vector<int>, std::vector<int>, std::vector<Dummy>>>& ovx
    );
    static void write_Compressed(
        const std::string& outfile,
        const std::vector<std::tuple<std::vector<Vertex>, std::pair<int, std::vector<CLERS>>, std::vector<Handle>, std::vector<Dummy>>>& compressed
    );
    static void write_Compressed_BIN(
        const std::string& outfile,
        const std::vector<std::tuple<std::vector<Vertex>, std::pair<int, std::vector<CLERS>>, std::vector<Handle>, std::vector<Dummy>>>& compressed
    );
    static void write_OBJ(
        const std::string& outfile,
        const std::vector<Vertex>& vert,
        const std::vector<Indices>& tri
    );
    static void write_OFF(
        const std::string& outfile,
        const std::vector<Vertex>& vert,
        const std::vector<Indices>& tri
    );
};