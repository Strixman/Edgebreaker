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

std::pair<std::vector<Vertex>, std::vector<std::tuple<std::vector<int>, std::vector<int>, std::vector<Dummy>>>> Reader::read_OVX(
    const std::string& infile
) {
    std::ifstream in(infile);
    if(!in) throw ReaderException(std::format("Cannot open file {}!", infile));

    std::vector<Vertex> vert;
    std::vector<std::tuple<std::vector<int>, std::vector<int>, std::vector<Dummy>>> ovx;

    int comp_size;
    in >> comp_size;
    for(int i = 0; i < comp_size; ++i){
        auto& [V, O, dummy] = ovx.emplace_back();

        int vo_size, v, o;
        in >> vo_size;
        for(int j = 0; j < 3 * vo_size; ++j){
            in >> v >> o;
            V.push_back(v); //TODO remove
            O.push_back(o);
        }
        int dummy_size;
        in >> dummy_size;
        dummy.resize(dummy_size);
        for(auto& d : dummy){
            in >> d.first;
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

std::vector<std::tuple<std::queue<Vertex>, std::pair<int, std::vector<CLERS>>, std::vector<Handle>, std::vector<Dummy>>> Reader::read_Compressed(
    const std::string &infile
) {
    std::ifstream in(infile);
    if(!in) throw ReaderException(std::format("Cannot open file {}!", infile));

    std::vector<std::tuple<std::queue<Vertex>, std::pair<int, std::vector<CLERS>>, std::vector<Handle>, std::vector<Dummy>>> decompressed;

    int comp_size;
    in >> comp_size;
    for(int i = 0;  i < comp_size; ++i){
        auto& [vertices, clers, handles, dummy] = decompressed.emplace_back();

        int vertices_size;
        in >> vertices_size;
        for(int j = 0; j < vertices_size; ++j){
            Vertex& v = vertices.emplace();
            in >> v[0] >> v[1] >> v[2];
        }
        int clers_size;
        in >> clers_size >> clers.first;
        for(int j = 0; j < clers_size; ++j){
            char ch;
            in >> ch;
            switch (ch)
            {
            case 'C':
                clers.second.push_back(CLERS::C);
                break;
            case 'L':
                clers.second.push_back(CLERS::L);
                break;
            case 'E':
                clers.second.push_back(CLERS::E);
                break;
            case 'R':
                clers.second.push_back(CLERS::R);
                break;
            case 'S':
                clers.second.push_back(CLERS::S);
                break;
            }
        }
        int handles_size, a, b;
        in >> handles_size;
        for(int j = 0; j < handles_size; ++j){
            Handle& h = handles.emplace_back();
            in >> h[0] >> h[1];
        }
        int dummy_size, d;
        in >> dummy_size;
        for(int j = 0; j < dummy_size; ++j){
            in >> d;
            dummy.push_back({d, {}});
        }
    }

    return decompressed;
}



std::vector<std::tuple<std::queue<Vertex>, std::pair<int, std::vector<CLERS>>, std::vector<Handle>, std::vector<Dummy>>> Reader::read_Compressed_BIN(
    const std::string &infile
) {
    std::ifstream in(infile, std::ios::binary);
    if (!in) throw ReaderException(std::format("Cannot open file {}!", infile));

    std::vector<std::tuple<std::queue<Vertex>, std::pair<int, std::vector<CLERS>>, std::vector<Handle>, std::vector<Dummy>>> decompressed;

    size_t comp_size;
    in.read(reinterpret_cast<char*>(&comp_size), sizeof(comp_size));

    for (size_t i = 0; i < comp_size; ++i) {
        auto& [vertices, clers, handles, dummy] = decompressed.emplace_back();

        // Read vertices count and each Vertex (assuming 3 numbers per vertex)
        size_t vertices_size;
        in.read(reinterpret_cast<char*>(&vertices_size), sizeof(vertices_size));
        for (size_t j = 0; j < vertices_size; ++j) {
            Vertex& v = vertices.emplace();
            in.read(reinterpret_cast<char*>(&v[0]), sizeof(v[0]));
            in.read(reinterpret_cast<char*>(&v[1]), sizeof(v[1]));
            in.read(reinterpret_cast<char*>(&v[2]), sizeof(v[2]));
        }

        // Read CLERS count and the integer part (clers.first)
        size_t clers_enum_size;
        in.read(reinterpret_cast<char*>(&clers_enum_size), sizeof(clers_enum_size));
        in.read(reinterpret_cast<char*>(&clers.first), sizeof(clers.first));
        for (size_t j = 0; j < clers_enum_size; ++j) {
            char ch;
            in.read(reinterpret_cast<char*>(&ch), sizeof(ch));
            switch (ch) {
                case 'C':
                    clers.second.push_back(CLERS::C);
                    break;
                case 'L':
                    clers.second.push_back(CLERS::L);
                    break;
                case 'E':
                    clers.second.push_back(CLERS::E);
                    break;
                case 'R':
                    clers.second.push_back(CLERS::R);
                    break;
                case 'S':
                    clers.second.push_back(CLERS::S);
                    break;
            }
        }

        // Read handles count and each handle (each assumed to have 2 elements)
        size_t handles_size;
        in.read(reinterpret_cast<char*>(&handles_size), sizeof(handles_size));
        for (size_t j = 0; j < handles_size; ++j) {
            Handle& h = handles.emplace_back();
            in.read(reinterpret_cast<char*>(&h[0]), sizeof(h[0]));
            in.read(reinterpret_cast<char*>(&h[1]), sizeof(h[1]));
        }

        // Read dummy count and each dummy (only using dummy.first; the second element is kept empty)
        size_t dummy_size;
        in.read(reinterpret_cast<char*>(&dummy_size), sizeof(dummy_size));
        for (size_t j = 0; j < dummy_size; ++j) {
            int d;
            in.read(reinterpret_cast<char*>(&d), sizeof(d));
            dummy.push_back({d, {}}); // Adjust according to Dummy's structure.
        }
    }
    return decompressed;
}