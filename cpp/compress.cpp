#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace std;

enum class MeshType { MANIFOLD, TPATCH };
enum class FileFormat { BINARY, ASCII };

struct Coord3D {
    float x, y, z;
    Coord3D() : x(0), y(0), z(0) {}
    Coord3D(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};
using Vertex = Coord3D;
using Vector = Coord3D;

namespace {
    constexpr size_t MAX_SIZE = 256;

    string sOVTable;
    MeshType eMeshType = MeshType::MANIFOLD;
    FileFormat eFileFormat = FileFormat::ASCII;
    int nStartCorner = 0;

    ofstream fclers;
    ofstream fvertices;
    ofstream fhandles;

    vector<int> O;
    vector<int> V;
    vector<Vertex> G;
    vector<Vertex> G_est;

    int T = 0;
    int N = 0;
    vector<int> M;
    vector<int> U;
}

void PrintErrorAndQuit(const string& sErrorString) {
    cerr << sErrorString << endl;
    exit(EXIT_FAILURE);
}

void ProcessArguments(int argc, char* argv[]) {
    if (argc != 5) {
        PrintErrorAndQuit("Wrong number of arguments.\n\nUsage: EBCompression OVTable MeshType FileFormat\n");
    }

    sOVTable = argv[1];
    string sMeshType = argv[2];
    nStartCorner = stoi(argv[3]);
    string sFileFormat = argv[4];

    if (sMeshType == "MANIFOLD") {
        eMeshType = MeshType::MANIFOLD;
        cout << "MeshType - MANIFOLD\n";
    } else if (sMeshType == "TPATCH") {
        eMeshType = MeshType::TPATCH;
        cout << "MeshType - TPATCH\n";
    } else {
        PrintErrorAndQuit("Not supported mesh type\n");
    }

    if (sFileFormat == "BINARY") {
        eFileFormat = FileFormat::BINARY;
        cout << "FileFormat - BINARY\n";
    } else if (sFileFormat == "ASCII") {
        eFileFormat = FileFormat::ASCII;
        cout << "FileFormat - ASCII\n";
    } else {
        PrintErrorAndQuit("Not supported file format\n");
    }
}

void OpenOutputFiles() {
    fclers.open("clers.txt");
    if (!fclers) PrintErrorAndQuit("Cannot open clers file\n");

    fvertices.open("vertices.txt");
    if (!fvertices) PrintErrorAndQuit("Cannot open vertices file\n");

    fhandles.open("handles.txt");
    if (!fhandles) PrintErrorAndQuit("Cannot open handles file\n");
}

void ProcessInputFile(const string& sFileName) {
    ifstream pInFile(sFileName);
    if (!pInFile) PrintErrorAndQuit("Cannot open input file\n");

    int nNumOfTriangles;
    pInFile >> nNumOfTriangles;
    if (!pInFile) PrintErrorAndQuit("Error reading number of triangles\n");

    V.resize(nNumOfTriangles * 3);
    O.resize(nNumOfTriangles * 3);

    for (int i = 0; i < nNumOfTriangles; ++i) {
        for (int j = 0; j < 3; ++j) {
            int idx = i * 3 + j;
            pInFile >> V[idx] >> O[idx];
            if (!pInFile) PrintErrorAndQuit("Error reading V or O table\n");
        }
    }

    int nNumOfVertices;
    pInFile >> nNumOfVertices;
    if (!pInFile) PrintErrorAndQuit("Error reading number of vertices\n");

    G.resize(nNumOfVertices);
    G_est.resize(nNumOfVertices);

    for (int i = 0; i < nNumOfVertices; ++i) {
        pInFile >> G[i].x >> G[i].y >> G[i].z;
        if (!pInFile) PrintErrorAndQuit("Error reading vertex coordinates\n");
    }

    M.resize(nNumOfVertices, 0);
    U.resize(nNumOfTriangles, 0);
}

inline int NextEdge(int edge) { return 3 * (edge / 3) + (edge + 1) % 3; }
inline int PrevEdge(int edge) { return NextEdge(NextEdge(edge)); }
inline int E2T(int edge) { return edge / 3; }
int RightTri(int c) { return O[NextEdge(c)]; }
int LeftTri(int c) { return O[NextEdge(NextEdge(c))]; }

void EncodeDelta(int c);
void Compress(int c);
void CheckHandle(int c);

void initCompression(int c, MeshType eMeshType) {
    T = 0;
    c = PrevEdge(c);

    EncodeDelta(NextEdge(c));
    if (eMeshType == MeshType::MANIFOLD) M[V[NextEdge(c)]] = 1;

    EncodeDelta(c);
    M[V[c]] = 1;

    EncodeDelta(PrevEdge(c));
    M[V[PrevEdge(c)]] = 1;

    U[E2T(c)] = 1;

    int a = O[c];
    int count = 1;
    while (a != PrevEdge(O[PrevEdge(c)])) {
        count++;
        U[E2T(a)] = 1;
        T++;
        EncodeDelta(a);
        M[V[a]] = 1;
        a = O[NextEdge(a)];
    }

    U[E2T(a)] = 1;
    T++;
    count++;

    if (eMeshType == MeshType::MANIFOLD && eFileFormat == FileFormat::ASCII) {
        fclers << "MANIFOLD\n";
    } else if (eMeshType == MeshType::TPATCH && eFileFormat == FileFormat::ASCII) {
        fclers << "TPATCH\n";
    }

    if (eFileFormat == FileFormat::ASCII) {
        fclers << count << '\n';
    }

    Compress(O[PrevEdge(a)]);
}

void Compress(int c) {
    do {
        U[E2T(c)] = 1;
        T++;
        CheckHandle(c);

        if (M[V[c]] == 0) {
            fclers << "C\n";
            EncodeDelta(c);
            M[V[c]] = 1;
            c = RightTri(c);
        } else if (U[E2T(RightTri(c))] > 0) {
            if (U[E2T(LeftTri(c))] > 0) {
                fclers << "E\n";
                return;
            } else {
                fclers << "R\n";
                c = LeftTri(c);
            }
        } else if (U[E2T(LeftTri(c))] > 0) {
            fclers << "L\n";
            c = RightTri(c);
        } else {
            U[E2T(c)] = T * 3 + 2;
            fclers << "S\n";
            Compress(RightTri(c));
            c = LeftTri(c);
            if (U[E2T(c)] > 0) return;
        }
    } while (true);
}

void CheckHandle(int c) {
    if (U[E2T(O[NextEdge(c)])] > 1) {
        fhandles << U[E2T(O[NextEdge(c)])] << ' ' << T * 3 + 1 << '\n';
    }
    if (U[E2T(O[PrevEdge(c)])] > 1) {
        fhandles << U[E2T(O[PrevEdge(c)])] << ' ' << T * 3 + 2 << '\n';
    }
}

void EncodeNoPrediction(int c) {
    fvertices << G[V[c]].x << ' ' << G[V[c]].y << ' ' << G[V[c]].z << '\n';
}

void EncodeDelta(int c) {
    EncodeNoPrediction(c);
}

int main(int argc, char* argv[]) {
    ProcessArguments(argc, argv);
    OpenOutputFiles();
    ProcessInputFile(sOVTable);
    initCompression(nStartCorner, eMeshType);
    return 0;
}