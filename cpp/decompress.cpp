#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

struct Coord3D {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    Coord3D() = default;
    Coord3D(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

using Vertex = Coord3D;
using Vector = Coord3D;

enum class MeshType { MANIFOLD, TPATCH };
enum class FileFormat { BINARY, ASCII };

// Global variables (could be encapsulated in a class in a more refined version)
static std::string sInputDirectory;
static std::string sOutFileName;
static MeshType eMeshType = MeshType::MANIFOLD;
static FileFormat eFileFormat = FileFormat::ASCII;

static std::ifstream fclers;
static std::ifstream fvertices;
static std::ifstream fhandles;

static int nNumOfTriangles;
static int nNumOfVertices;

static std::vector<int> O;
static std::vector<int> V;
static std::vector<Vertex> G;
static std::vector<int> U;
static std::vector<int> M;

static int T = 0;
static int N = 0;
static int I = 0;
static int A = 0;
static std::vector<int> H;
static std::vector<char> C;
static std::vector<Vertex> G_in;

// Function declarations
void ClearMemoryAndFiles();
void PrintErrorAndQuit(const std::string& sErrorString);
void ProcessArguments(int argc, char* argv[]);
void OpenInputFiles();
void WriteVOTableFile(const std::string& sFileName);
void InitDecompressionModule();
void DecompressConnectivity(int c);
bool CheckHandle(int c);
void Zip(int c);
void DecompressVertices(int c);
Vertex DecodeDelta(int c);

int NextEdge(int edge) { return (3 * (edge / 3) + (edge + 1) % 3); }
int PrevEdge(int edge) { return NextEdge(NextEdge(edge)); }
int RightTri(int c) { return O[NextEdge(c)]; }
int LeftTri(int c) { return O[NextEdge(NextEdge(c))]; }
int E2T(int edge) { return (edge / 3); }

void ClearMemoryAndFiles() {
    if (fclers.is_open()) fclers.close();
    if (fvertices.is_open()) fvertices.close();
    if (fhandles.is_open()) fhandles.close();

    O.clear();
    V.clear();
    G.clear();
    U.clear();
    M.clear();
    H.clear();
    C.clear();
    G_in.clear();
}

void PrintErrorAndQuit(const std::string& sErrorString) {
    std::cerr << sErrorString << std::endl;
    ClearMemoryAndFiles();
    exit(EXIT_FAILURE);
}

void ProcessArguments(int argc, char* argv[]) {
    if (argc != 4) {
        PrintErrorAndQuit("Wrong number of arguments.\n\n"
                          "Usage: EBDecompression InputFileDir OutFileName FileFormat\n");
    }

    sInputDirectory = argv[1];
    sOutFileName = argv[2];
    std::string sFileFormat = argv[3];

    if (sFileFormat == "BINARY") {
        eFileFormat = FileFormat::BINARY;
        std::cout << "FileFormat - BINARY\n";
    } else if (sFileFormat == "ASCII") {
        eFileFormat = FileFormat::ASCII;
        std::cout << "FileFormat - ASCII\n";
    } else {
        PrintErrorAndQuit("Not supported file format\n");
    }
}

void OpenInputFiles() {
    std::string clersPath = sInputDirectory + "\\clers.txt";
    fclers.open(clersPath);
    if (!fclers) PrintErrorAndQuit("Cannot open clers file\n");

    std::string verticesPath = sInputDirectory + "\\vertices.txt";
    fvertices.open(verticesPath);
    if (!fvertices) PrintErrorAndQuit("Cannot open vertices file\n");

    std::string handlesPath = sInputDirectory + "\\handles.txt";
    fhandles.open(handlesPath);
    if (!fhandles) PrintErrorAndQuit("Cannot open handles file\n");
}

void WriteVOTableFile(const std::string& sFileName) {
    std::ofstream pOutFile(sFileName);
    if (!pOutFile) PrintErrorAndQuit("Cannot open output file\n");

    pOutFile << nNumOfTriangles << '\n';
    for (int i = 0; i < nNumOfTriangles; i++) {
        pOutFile << V[i * 3] << ' ' << O[i * 3] << '\n'
                 << V[i * 3 + 1] << ' ' << O[i * 3 + 1] << '\n'
                 << V[i * 3 + 2] << ' ' << O[i * 3 + 2] << '\n';
    }

    pOutFile << G.size() << '\n';
    for (const auto& vertex : G) {
        pOutFile << vertex.x << ' ' << vertex.y << ' ' << vertex.z << '\n';
    }
}

void ReadInputVertices() {
    Vertex vTemp;
    while (fvertices >> vTemp.x >> vTemp.y >> vTemp.z) {
        G_in.push_back(vTemp);
    }
}

void ReadHandlesPairs() {
    int nTemp;
    while (fhandles >> nTemp) {
        H.push_back(nTemp);
    }
}

void ReadMeshType() {
    std::string sMeshType;
    fclers >> sMeshType;
    
    if (sMeshType == "MANIFOLD") {
        eMeshType = MeshType::MANIFOLD;
        std::cout << "MeshType - MANIFOLD\n";
    } else if (sMeshType == "TPATCH") {
        eMeshType = MeshType::TPATCH;
        std::cout << "MeshType - TPATCH\n";
    } else {
        PrintErrorAndQuit("Not supported mesh type\n");
    }
}

void ReadNumTriOnFirstVertex() {
    fclers >> I;
}

void ReadClearsAndUppendClersForFirstVertexTri() {
    for (int i = 0; i < (I - 2); ++i) {
        C.push_back('C');
    }
    C.push_back('R');

    std::string line;
    while (std::getline(fclers, line)) {
        if (!line.empty()) {
            C.push_back(line[0]);
        }
    }
}

void InitDecompressionModule() {
    nNumOfTriangles = C.size() + 1;
    nNumOfVertices = G_in.size();

    O.resize(3 * nNumOfTriangles, -3);
    V.resize(3 * nNumOfTriangles, 0);

    V[0] = 0; V[1] = 2; V[2] = 1;
    O[0] = -1; O[1] = -1;

    U.resize(nNumOfTriangles, 0);
    M.resize(nNumOfVertices, 0);
    G.resize(nNumOfVertices);
}

void DecompressConnectivity(int c) {
    do {
        T++;
        O[c] = 3 * T;
        O[3 * T] = c;
        V[3 * T + 1] = V[PrevEdge(c)];
        V[3 * T + 2] = V[NextEdge(c)];
        c = NextEdge(O[c]);

        switch (C[T - 1]) {
            case 'C':
                O[NextEdge(c)] = -1;
                V[3 * T] = ++N;
                break;
            case 'L':
                O[NextEdge(c)] = -2;
                if (!CheckHandle(NextEdge(c))) Zip(NextEdge(c));
                break;
            case 'R':
                O[c] = -2;
                CheckHandle(c);
                c = NextEdge(c);
                break;
            case 'S':
                DecompressConnectivity(c);
                c = NextEdge(c);
                if (O[c] >= 0) return;
                break;
            case 'E':
                O[c] = -2;
                O[NextEdge(c)] = -2;
                CheckHandle(c);
                if (!CheckHandle(NextEdge(c))) Zip(NextEdge(c));
                return;
        }
    } while (true);
}

bool CheckHandle(int c) {
    if (A >= H.size() || c != H[A + 1]) return false;
    
    O[c] = H[A];
    O[H[A]] = c;

    int a = PrevEdge(c);
    while (O[a] >= 0 && a != H[A]) a = PrevEdge(O[a]);
    if (O[a] == -2) Zip(a);

    a = PrevEdge(O[c]);
    while (O[a] >= 0 && a != c) a = PrevEdge(O[a]);
    if (O[a] == -2) Zip(a);

    A += 2;
    return true;
}

void Zip(int c) {
    int b = NextEdge(c);
    while (O[b] >= 0 && O[b] != c) b = NextEdge(O[b]);
    if (O[b] != -1) return;

    O[c] = b;
    O[b] = c;
    int a = NextEdge(c);
    V[NextEdge(a)] = V[NextEdge(b)];
    while (O[a] >= 0 && a != b) {
        a = NextEdge(O[a]);
        V[NextEdge(a)] = V[NextEdge(b)];
    }

    c = PrevEdge(c);
    while (O[c] >= 0 && c != b) c = PrevEdge(O[c]);
    if (O[c] == -2) Zip(c);
}

void DecompressVertices(int c) {
    do {
        U[E2T(c)] = 1;
        if (M[V[c]] == 0) {
            N++;
            G[N] = DecodeDelta(c);
            M[V[c]] = 1;
            c = RightTri(c);
        } else if (U[E2T(RightTri(c))] == 1) {
            if (U[E2T(LeftTri(c))] == 1) {
                return;
            } else {
                c = LeftTri(c);
            }
        } else if (U[E2T(LeftTri(c))] == 1) {
            c = RightTri(c);
        } else {
            DecompressVertices(RightTri(c));
            c = LeftTri(c);
            if (U[E2T(c)] > 0) return;
        }
    } while (true);
}

Vertex DecodeDelta(int c) {
    return G_in[V[c]];
}

int main(int argc, char* argv[]) {
    try {
        ProcessArguments(argc, argv);
        OpenInputFiles();

        ReadInputVertices();
        ReadHandlesPairs();
        ReadMeshType();
        ReadNumTriOnFirstVertex();
        ReadClearsAndUppendClersForFirstVertexTri();

        InitDecompressionModule();

        T = 0; N = 2; A = 0;
        DecompressConnectivity(2);

        G[0] = DecodeDelta(0);
        if (eMeshType == MeshType::MANIFOLD) M[0] = 1;
        G[1] = DecodeDelta(2);
        M[1] = 1;
        G[2] = DecodeDelta(1);
        M[2] = 1;
        N = 2;
        U[0] = 1;
        DecompressVertices(O[2]);

        WriteVOTableFile(sOutFileName);
        ClearMemoryAndFiles();
    } catch (const std::exception& e) {
        PrintErrorAndQuit(e.what());
    }
    return 0;
}