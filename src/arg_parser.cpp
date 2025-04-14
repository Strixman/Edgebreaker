#include <iostream>
#include <string>
#include <unordered_map>

struct Args {
    std::string mode;
    std::string infile;
    std::string outfile;
    File::Type infile_type;
    File::Type outfile_type;
};

void printUsage(const std::string& programName) {
    std::cerr << "Usage: " << programName << " <compress|decompress|ovx> "
              << "<input_file> <output_file> ";
}
void printUsageCompress(const std::string& programName) {
    std::cerr << "Usage: " << programName << " compress "
                << "<input_file.[obj|off|ovx]> <output_file.[bco|co]> ";  
}
void printUsageDecompress(const std::string& programName) {
    std::cerr << "Usage: " << programName << " decompress "
                << "<input_file.[bco|co]> <output_file.[obj|off|ovx]> ";  
}
void printUsageOVX(const std::string& programName) {
    std::cerr << "Usage: " << programName << " ovx "
                << "<input_file.[obj|off]> <output_file.[ovx]> ";  
}

File::Type findFileType(const std::string& fileName){
    std::string::size_type pos = fileName.find_last_of('.');
    if (pos != std::string::npos) {
        std::string ext = fileName.substr(pos + 1);
        if(ext == "obj") return File::Type::OBJ;
        else if(ext == "off") return File::Type::OFF;
        else if(ext == "ovx") return File::Type::OVX;
        else if(ext == "bco") return File::Type::BCO;
        else if(ext == "co") return File::Type::CO;
    }
    return File::Type::UNKNOWN;
}

Args parseArgs(int argc, char* argv[]) {
    if (argc < 4) {
        printUsage(argv[0]);
        exit(EXIT_FAILURE);
    }

    Args args;
    args.mode = argv[1];
    args.infile = argv[2];
    args.outfile = argv[3];
    args.infile_type = findFileType(args.infile);
    args.outfile_type = findFileType(args.outfile);

    if (args.mode == "compress") {
        if((args.infile_type != File::Type::OBJ && args.infile_type != File::Type::OFF && args.infile_type != File::Type::OVX) ||
            (args.outfile_type != File::Type::BCO && args.outfile_type != File::Type::CO)
        ){
            printUsageCompress(argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    else if(args.mode == "decompress"){
        if((args.infile_type != File::Type::BCO && args.infile_type != File::Type::CO) || (args.outfile_type != File::Type::OVX &&
            args.outfile_type != File::Type::OBJ && args.outfile_type != File::Type::OFF)
        ){
            printUsageDecompress(argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    else if(args.mode == "ovx"){
        if((args.infile_type != File::Type::OBJ && args.infile_type != File::Type::OFF) ||
            (args.outfile_type != File::Type::OVX)
        ){
            printUsageOVX(argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    else{
        printUsage(argv[0]);
        exit(EXIT_FAILURE);
    }

    return args;
}
