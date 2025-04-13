#include "converter.h"
#include "reader.h"
#include "writer.h"
#include "compressor.h"
#include "decompressor.h"

std::streamsize getFileSize(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    return file.tellg();
}

void compress(const std::string& infile, const std::string& outfile){
    auto [vert, tri] = Reader::read_OBJ(infile);
    auto ovx = Converter::toOVX(vert, tri);

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

    Writer::write_Compressed_BIN(outfile, compressed);

    auto infile_size = getFileSize(infile);
    auto outfile_size = getFileSize(outfile);
    std::cout << std::format("Compression ratio: {:.2f}\n", outfile_size / (float)infile_size);
    std::cout << std::format("Relative savings: {:.2f}%\n", ((infile_size - outfile_size) / (float)infile_size) * 100);
    std::cout << std::format("Compressed file {} into {}\n", infile, outfile);
}

void decompress(const std::string& infile, const std::string& outfile){
    auto uncompressed = Reader::read_Compressed_BIN(infile);

    std::vector<std::tuple<std::vector<Vertex>, std::vector<int>, std::vector<int>, std::vector<int>>> ovx;
    int progress = 1;
    for(auto& [vertices, clers, handles, dummy] : uncompressed){
        std::cout << std::format("Decompressing progress: {:.2f}%\n", progress * 100 / (float)uncompressed.size());
        auto& [vert, V, O, _dummy] = ovx.emplace_back();
        for(auto d : dummy){
            _dummy.push_back(d.first);
        }

        Decompressor d(vertices, clers, handles);
        d.decompress(vert, V, O);
        ++progress;
    }

    auto [vert, tri] = Converter::fromOVX(ovx);
    Writer::write_OBJ(outfile, vert, tri);

    std::cout << std::format("Decompressed file {} into {}\n", infile, outfile);
}

int main(int argc, char* argv[]) {
    compress(argv[1], "out.bin");

    decompress("out.bin", "out.obj");

    return 0;
}