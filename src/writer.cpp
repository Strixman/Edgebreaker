#include "writer.h"

void Writer::write_OVX(
    const std::string& outfile,
    const std::vector<Vertex>& vert,
    const std::vector<std::tuple<std::vector<int>, std::vector<int>, std::vector<int>>> &ovx
) {
    std::ofstream out(outfile);
    if(!out) throw WriterException(std::format("Cannot write to file {}!", outfile));

    out << ovx.size() << "\n";
    for(auto& [V, O, dummy] : ovx){
        out << (O.size() / 3) << "\n";
        for(int i = 0; i < O.size(); ++i){
            out << V[i] << " " << O[i] << "\n";
        }
        out << dummy.size() << "\n";
        for(auto d : dummy){
            out << d << "\n";
        }
    }
    out << vert.size() << "\n";
    for(auto& v : vert){
        out << v[0] << " " << v[1] << " " << v[2] << "\n";
    }
}