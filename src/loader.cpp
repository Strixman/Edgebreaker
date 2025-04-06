#include "loader.h"

std::pair<std::vector<Vertex>, std::vector<Indices>> Loader::load(
    const std::string& infile,
    const EXT ext
) {
    switch (ext)
    {
    case EXT::OBJ: 
        return load_OBJ(infile);
    case EXT::OFF:
        return load_OFF(infile);
    default:
        throw LoaderException("Invalid file type!");
    }
}

std::pair<std::vector<Vertex>, std::vector<Indices>> Loader::load_OBJ(
    const std::string& infile
) {
    std::ifstream in(infile);
    if(!in) throw LoaderException(std::format("Cannot open file {}!", infile));

    std::vector<Vertex> vert;
    std::vector<Indices> tri;

    std::string line, tag;
    while (std::getline(in, line)) {
        std::istringstream ss(line);
        ss >> tag;
        if (tag == "v") {
            Vertex v;
            ss >> v.x >> v.y >> v.z;
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

std::pair<std::vector<Vertex>, std::vector<Indices>> Loader::load_OFF(
    const std::string& infile
) {
    std::ifstream in(infile);
    if(!in) throw LoaderException(std::format("Cannot open file {}!", infile));

    std::vector<Vertex> vert;
    std::vector<Indices> tri;

    std::string header;
    in >> header;
    if (header != "OFF") throw LoaderException("Invalid OFF file");

    int numVerts, numFaces, _;
    in >> numVerts >> numFaces >> _;

    vert.resize(numVerts);
    for (auto& v : vert)
        in >> v.x >> v.y >> v.z;

    for (int i = 0; i < numFaces; ++i) {
        int n; in >> n;
        std::vector<int> indices(n);
        for (int& idx : indices) in >> idx;

        for (int j = 1; j + 1 < n; ++j)
            tri.push_back({ indices[0], indices[j], indices[j + 1] });
    }

    return std::make_pair(vert, tri);
}