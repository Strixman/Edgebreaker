#include "converter.h"
#include "reader.h"
#include "writer.h"
#include "compressor.h"
#include "decompressor.h"

void compress(const std::string& infile, const std::string& outfile){
    auto [vert, tri] = Reader::read_OBJ(infile);
    auto ovx = Converter::toOVX(vert, tri);

    //auto [vert, ovx] = Reader::read_OVX(infile);

    std::vector<std::tuple<std::vector<Vertex>, std::pair<int, std::vector<CLERS>>, std::vector<Handle>, std::vector<Dummy>>> compressed;
    for(auto& [V, O, dummy] : ovx){
        auto& [vertices, clers, handles, _dummy] = compressed.emplace_back();
        for(auto d : dummy){
            _dummy.push_back(d);
        }

        Compressor c(vert, V, O);
        c.compress(0, vertices, clers, handles, _dummy);
    }

    Writer::write_Compressed(outfile, compressed);

    std::cout << std::format("Compressed file {} into {}\n", infile, outfile);
}

void decompress(const std::string& infile, const std::string& outfile){
    auto uncompressed = Reader::read_Compressed(infile);

    std::vector<std::tuple<std::vector<Vertex>, std::vector<int>, std::vector<int>, std::vector<int>>> ovx;
    for(auto& [vertices, clers, handles, dummy] : uncompressed){
        auto& [vert, V, O, _dummy] = ovx.emplace_back();
        for(auto d : dummy){
            _dummy.push_back(d.first);
        }

        Decompressor d(vertices, clers, handles);
        d.decompress(vert, V, O);
    }

    auto [vert, tri] = Converter::fromOVX(ovx);
    Writer::write_OBJ(outfile, vert, tri);

    std::cout << std::format("Decompressed file {} into {}\n", infile, outfile);
}

int main(int argc, char* argv[]) {
    //compress(argv[1], "out.txt");

    decompress("out.txt", "out.obj");

    return 0;
}