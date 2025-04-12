#include "converter.h"
#include "reader.h"
#include "writer.h"
#include "compressor.h"
#include "decompressor.h"

void compress(const std::string& infile, const std::string& outfile){
    auto [vert, tri] = Reader::read_OBJ(infile);
    auto ovx = Converter::toOVX(vert, tri);

    std::vector<std::tuple<std::vector<Vertex>, std::pair<int, std::vector<CLERS>>, std::vector<Handle>, std::vector<int>>> compressed;
    for(auto& [V, O, dummy] : ovx){
        auto& [vertices, clers, handles, _dummy] = compressed.emplace_back();
        for(auto d : dummy){
            _dummy.push_back(d);
        }

        Compressor c(vert, V, O);
        c.compress(0, vertices, clers, handles);
    }

    Writer::write_Compressed(outfile, compressed);
}

int main(int argc, char* argv[]) {
    compress(argv[1], "out.txt");

    std::vector<std::tuple<std::vector<int>, std::vector<int>, std::vector<int>>> ovx;
    auto uncompressed = Reader::read_Compressed("out.txt");
    for(auto& [vertices, clers, handles, dummy] : uncompressed){
        Decompressor d(vertices, clers, handles);
        auto [vert, V, O] = d.decompress();
        ovx.push_back(std::make_tuple(V, O, dummy));
        Writer::write_OVX("out.ovx", vert, {std::make_tuple(V, O, dummy)});
    }

    return 0;
}