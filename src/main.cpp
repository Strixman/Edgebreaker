#include "converter.h"
#include "reader.h"
#include "writer.h"
#include "compressor.h"
#include "decompressor.h"

#include "types.h"
#include "arg_parser.h"

std::streamsize getFileSize(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    return file.tellg();
}

void ovx(const Args& args){
    std::vector<Vertex> vert;
    std::vector<std::tuple<std::vector<int>, std::vector<int>, std::vector<Dummy>>> _ovx;
    if(args.infile_type == File::Type::OBJ){
        auto [_vert, tri] = Reader::read_OBJ(args.infile);
        vert = _vert;
        _ovx = Converter::toOVX(vert, tri);
    }
    else if(args.infile_type == File::Type::OFF){
        auto [_vert, tri] = Reader::read_OFF(args.infile);
        vert = _vert;
        _ovx = Converter::toOVX(vert, tri);
    }

    Writer::write_OVX(args.outfile, vert, _ovx);

    std::cout << std::format("Converted file {} into {}\n", args.infile, args.outfile);
}

void compress(const Args& args){
    std::vector<Vertex> vert;
    std::vector<std::tuple<std::vector<int>, std::vector<int>, std::vector<Dummy>>> ovx;
    if(args.infile_type == File::Type::OBJ){
        auto [_vert, tri] = Reader::read_OBJ(args.infile);
        vert = _vert;
        ovx = Converter::toOVX(vert, tri);
    }
    else if(args.infile_type == File::Type::OFF){
        auto [_vert, tri] = Reader::read_OFF(args.infile);
        vert = _vert;
        ovx = Converter::toOVX(vert, tri);
    }
    else if(args.infile_type == File::Type::OVX){
        auto [_vert, _ovx] = Reader::read_OVX(args.infile);
        vert = _vert;
        ovx = _ovx;
    }

    std::vector<std::tuple<std::vector<Vertex>, std::pair<int, std::vector<CLERS>>, std::vector<Handle>, std::vector<Dummy>>> compressed;
    int progress = 1;
    for(auto& [V, O, dummy] : ovx){
        std::cout << std::format("Compressing progress: {:.2f}%\n", progress * 100 / (float)ovx.size());
        auto& [vertices, clers, handles, _dummy] = compressed.emplace_back();
        for(auto d : dummy){
            _dummy.push_back(d);
        }

        Compressor c(vert, V, O);
        c.compress(0, vertices, clers, handles, _dummy);
        ++progress;
    }

    if(args.outfile_type == File::Type::BCO){
        Writer::write_Compressed_BIN(args.outfile, compressed);
    }
    else{
        Writer::write_Compressed(args.outfile, compressed);
    }

    auto infile_size = getFileSize(args.infile);
    auto outfile_size = getFileSize(args.outfile);
    std::cout << std::format("Compression ratio: {:.2f}\n", outfile_size / (float)infile_size);
    std::cout << std::format("Relative savings: {:.2f}%\n", ((infile_size - outfile_size) / (float)infile_size) * 100);
    std::cout << std::format("Compressed file {} into {}\n", args.infile, args.outfile);
}

void decompress(const Args& args){
    std::vector<std::tuple<std::queue<Vertex>, std::pair<int, std::vector<CLERS>>, std::vector<Handle>, std::vector<Dummy>>> uncompressed;
    if(args.infile_type == File::Type::BCO){
        uncompressed = Reader::read_Compressed_BIN(args.infile);
    }
    else{
        uncompressed = Reader::read_Compressed(args.infile);
    }

    std::vector<std::tuple<std::vector<Vertex>, std::vector<int>, std::vector<int>, std::vector<Dummy>>> ovx;
    int progress = 1;
    for(auto& [vertices, clers, handles, dummy] : uncompressed){
        std::cout << std::format("Decompressing progress: {:.2f}%\n", progress * 100 / (float)uncompressed.size());
        auto& [vert, V, O, _dummy] = ovx.emplace_back();
        for(auto d : dummy){
            _dummy.push_back(d);
        }

        Decompressor d(vertices, clers, handles);
        d.decompress(vert, V, O);
        ++progress;
    }

    auto [vert, tri] = Converter::fromOVX(ovx);
    if(args.outfile_type == File::Type::OBJ){
        Writer::write_OBJ(args.outfile, vert, tri);
    }
    else if(args.outfile_type == File::Type::OFF){
        Writer::write_OFF(args.outfile, vert, tri);
    }

    std::cout << std::format("Decompressed file {} into {}\n", args.infile, args.outfile);
}

int main(int argc, char* argv[]) {
    auto args = parseArgs(argc, argv);
    if(args.mode == "compress"){
        compress(args);
    }
    else if(args.mode == "decompress"){
        decompress(args);
    }
    else{
        ovx(args);
    }
    return 0;
}