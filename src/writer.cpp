#include "writer.h"

void Writer::write_OVX(
    const std::string& outfile,
    const std::vector<Vertex>& vert,
    const std::vector<std::tuple<std::vector<int>, std::vector<int>, std::vector<int>>> &ovx
) {
    std::ofstream out(outfile);
    if(!out) throw WriterException(std::format("Cannot write to file {}!", outfile));

    //out << ovx.size() << "\n";
    for(auto& [V, O, dummy] : ovx){
        out << (O.size() / 3) << "\n";
        for(int i = 0; i < O.size(); ++i){
            out << V[i] << " " << O[i] << "\n";
        }
        //out << dummy.size() << "\n";
        //for(auto d : dummy){
        //    out << d << "\n";
        //}
    }
    out << vert.size() << "\n";
    for(auto& v : vert){
        out << v[0] << " " << v[1] << " " << v[2] << "\n";
    }
}

void Writer::write_Compressed(
    const std::string &outfile, 
    const std::vector<std::tuple<std::vector<Vertex>, std::pair<int, std::vector<CLERS>>, std::vector<Handle>, std::vector<int>>> &compressed
) {
    std::ofstream out(outfile);
    if(!out) throw WriterException(std::format("Cannot write to file {}!", outfile));

    out << compressed.size() << "\n";
    for(auto& [vertices, clers, handles, dummy] : compressed){
        out << vertices.size() << "\n";
        for(auto& v : vertices){
            out << v[0] << " " << v[1] << " " << v[2] << "\n";
        }
        out << clers.second.size() << "\n";
        out << clers.first << "\n";
        for(auto c : clers.second){
            switch (c)
            {
            case CLERS::C:
                out << "C";
                break;
            case CLERS::L:
                out << "L";
                break;
            case CLERS::E:
                out << "E ";
                break;
            case CLERS::R:
                out << "R";
                break;
            case CLERS::S:
                out << "S";
                break;
            }
            out << "\n";
        }
        out << handles.size() << "\n";
        for(auto& h : handles){
            out << h[0] << " " << h[1] << "\n";
        }
        out << dummy.size() << "\n";
        for(auto d : dummy){
            out << d << "\n";
        }
    }
}
