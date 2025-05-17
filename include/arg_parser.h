#pragma once
#include <iostream>
#include <string>
#include <unordered_map>

#include "types.h"

struct Args {
    std::string mode;
    std::string infile;
    std::string outfile;
    File::Type infile_type;
    File::Type outfile_type;
};

void printUsage(const std::string& programName);
void printUsageCompress(const std::string& programName);
void printUsageDecompress(const std::string& programName);
void printUsageOVX(const std::string& programName);

File::Type findFileType(const std::string& fileName);
Args parseArgs(int argc, char* argv[]);
