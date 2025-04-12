#include "reader.h"

std::pair<std::vector<Vertex>, std::vector<Indices>> Reader::read_OBJ(
    const std::string& infile
) {
    std::ifstream in(infile);
    if(!in) throw ReaderException(std::format("Cannot open file {}!", infile));

    std::vector<Vertex> vert;
    std::vector<Indices> tri;

    std::string line, tag;
    while (std::getline(in, line)) {
        std::istringstream ss(line);
        ss >> tag;
        if (tag == "v") {
            Vertex v{};
            ss >> v[0] >> v[1] >> v[2];
            vert.push_back(v);
        } else if (tag == "f") {
            std::vector<int> indices;
            std::string tok;
            while (ss >> tok)
                indices.push_back(std::stoi(tok.substr(0, tok.find('/'))) - 1);

            if (indices.size() < 3) continue;

            for (size_t i = 1; i + 1 < indices.size(); ++i)
                tri.push_back({ indices[0], indices[i], indices[i + 1] });
        }
    }

    return std::make_pair(vert, tri);
}

std::pair<std::vector<Vertex>, std::vector<Indices>> Reader::read_OFF(
    const std::string& infile
) {
    std::ifstream in(infile);
    if(!in) throw ReaderException(std::format("Cannot open file {}!", infile));

    std::vector<Vertex> vert;
    std::vector<Indices> tri;

    std::string header;
    in >> header;
    if (header != "OFF") throw ReaderException("Invalid OFF file");

    int numVerts, numFaces, _;
    in >> numVerts >> numFaces >> _;

    vert.resize(numVerts);
    for (auto& v : vert)
        in >> v[0] >> v[1] >> v[2];

    for (int i = 0; i < numFaces; ++i) {
        int n; in >> n;
        std::vector<int> indices(n);
        for (int& idx : indices) in >> idx;

        for (int j = 1; j + 1 < n; ++j)
            tri.push_back({ indices[0], indices[j], indices[j + 1] });
    }

    return std::make_pair(vert, tri);
}

std::pair<std::vector<Vertex>, std::vector<std::tuple<std::vector<int>, std::vector<int>, std::vector<int>>>> Reader::read_OVX(
    const std::string& infile
) {
    std::ifstream in(infile);
    if(!in) throw ReaderException(std::format("Cannot open file {}!", infile));

    std::vector<Vertex> vert;
    std::vector<std::tuple<std::vector<int>, std::vector<int>, std::vector<int>>> ovx;

    int comp_size;
    in >> comp_size;
    for(int i = 0; i < comp_size; ++i){
        auto& [V, O, dummy] = ovx.emplace_back();
        int vo_size, v, o;
        in >> vo_size;
        for(int j = 0; j < 3 * vo_size; ++j){
            in >> v >> o;
            V.push_back(v);
            O.push_back(o);
        }
        int dummy_size;
        in >> dummy_size;
        dummy.resize(dummy_size);
        for(auto& d : dummy){
            in >> d;
        }
    }
    int vert_size;
    in >> vert_size;
    vert.resize(vert_size);
    for(auto& v : vert){
        in >> v[0] >> v[1] >> v[2];
    }

    return std::make_pair(vert, ovx);
}
