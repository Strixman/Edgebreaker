#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
using namespace std;

// EBdatatypes.h content
#ifndef _EB_DATA_TYPES_H_
#define _EB_DATA_TYPES_H_

struct EBTriangle {
    int v[3];
};

struct Coord3D {
    float x;
    float y;
    float z;

    Coord3D() { x = y = z = 0.0; }
    Coord3D(float xx, float yy, float zz) { x = xx; y = yy; z = zz; }
};

typedef Coord3D Vertex;
typedef Coord3D Vector;

#define MAX_SIZE 256

enum MeshType { MANIFOLD, TPATCH };
enum FileFormat { BINARY, ASKII };

#endif // _EB_DATA_TYPES_H_

// HEPolMesh.h content
#ifndef _HALF_EDGE_POL_MESH_H_
#define _HALF_EDGE_POL_MESH_H_

class HEedge;
class HEpolygon;

struct HEvertex {
    HEvertex(float x, float y, float z) {
        pt.x = x; pt.y = y; pt.z = z;
        edge = -1;
    }
    HEvertex(Vertex p) {
        pt = p;
        edge = -1;
    }
    Vertex pt;
    int edge; // Reference to some HE starting from this vertex
};

class HEedge {
public:
    HEedge() { v = nextHE = invHE = polygon = -1; }
    int v;          // Vertex to which this HE is incident
    int nextHE;     // Next HE of current polygon
    int invHE;      // Opposite edge of adjacent polygon
    int polygon;    // Reference to the polygon
};

class HEpolygon {
public:
    HEpolygon() { firstHE = -1; }
    int firstHE;    // First HE of the polygon
};

class HEmesh {
public:
    HEmesh() {}
    ~HEmesh();

    void addVertex(Vertex v);
    void addVertex(float x, float y, float z);
    void addPolygon(int n, int* indices);

    void size(int& numpts, int& numpol);
    int vertexNeighbours(int vtx_no, vector<int>& neighvtx);
    void adjacentPolygons(int pol_no, vector<int>& neighpol);
    void getVertices(int pol_no, vector<int>& verts);
    int oppositeVertices(int tri_no, int& o1, int& o2, int& o3);
    void oppositeTrisVerts(int tri_no, int* t, int* v);
    int getBoundaries(vector<int>& bVertices, vector<int>& loopExtents);
    int triangulateLoops(vector<int>& bVertices, vector<int>& loopExtents, vector<int>& dummyVtxIndices);
    void clearTable();

    vector<HEvertex>  vertexlist;
    vector<HEedge>    edgelist;
    vector<HEpolygon> polylist;

private:
    vector<vector<int>> _tmptable;
    void _growBoundary(int edg, vector<bool>& visited, vector<int>& bVertices, vector<int>& loopExtents);
};

#endif // _HALF_EDGE_POL_MESH_H_

// HEPolMesh.C method implementations
HEmesh::~HEmesh() {
    vertexlist.clear();
    edgelist.clear();
    polylist.clear();
    clearTable();
}

void HEmesh::addVertex(Vertex v) {
    vertexlist.push_back(HEvertex(v));
    _tmptable.push_back(vector<int>());
}

void HEmesh::addVertex(float x, float y, float z) {
    vertexlist.push_back(HEvertex(x, y, z));
    _tmptable.push_back(vector<int>());
}

void HEmesh::addPolygon(int n, int* indices) {
    HEpolygon p;
    int prev = -1;

    for (int i = 0; i < n; i++) {
        HEedge e;
        e.v = indices[(i+1)%n];
        _tmptable[indices[i]].push_back(edgelist.size());

        if (vertexlist[indices[i]].edge == -1)
            vertexlist[indices[i]].edge = edgelist.size();

        int inv = -1;
        int k = indices[(i+1)%n];
        for (int j = 0; j < _tmptable[k].size(); j++) {
            int ce = _tmptable[k][j];
            if (edgelist[ce].v == indices[i]) {
                inv = ce;
                break;
            }
        }
        if (inv != -1) {
            edgelist[inv].invHE = edgelist.size();
            e.invHE = inv;
        }

        if (p.firstHE == -1)
            p.firstHE = edgelist.size();
        else
            edgelist[prev].nextHE = edgelist.size();

        e.polygon = polylist.size();
        prev = edgelist.size();
        edgelist.push_back(e);
    }
    edgelist[prev].nextHE = p.firstHE;
    polylist.push_back(p);
}

void HEmesh::size(int& numpts, int& numpol) {
    numpts = vertexlist.size();
    numpol = polylist.size();
}

int HEmesh::vertexNeighbours(int vtx_no, vector<int>& neighvtx) {
    neighvtx.clear();
    int incidentHE = vertexlist[vtx_no].edge;
    if (incidentHE == -1) return -1;

    neighvtx.push_back(edgelist[incidentHE].v);
    int currentHE = edgelist[edgelist[incidentHE].invHE].nextHE;

    while (currentHE >= 0 && currentHE != incidentHE) {
        neighvtx.push_back(edgelist[currentHE].v);
        currentHE = edgelist[edgelist[currentHE].invHE].nextHE;
    }
    return (currentHE == -1) ? -1 : neighvtx.size();
}

void HEmesh::adjacentPolygons(int pol_no, vector<int>& neighpol) {
    neighpol.clear();
    int firstedge = polylist[pol_no].firstHE;
    if (firstedge == -1) return;

    neighpol.push_back(edgelist[edgelist[firstedge].invHE].polygon);
    int currentedge = edgelist[firstedge].nextHE;

    while (currentedge >= 0 && currentedge != firstedge) {
        neighpol.push_back(edgelist[edgelist[currentedge].invHE].polygon);
        currentedge = edgelist[currentedge].nextHE;
    }
}

void HEmesh::getVertices(int pol_no, vector<int>& verts) {
    verts.clear();
    int firstedge = polylist[pol_no].firstHE;
    if (firstedge == -1) return;

    verts.push_back(edgelist[firstedge].v);
    int currentedge = edgelist[firstedge].nextHE;

    while (currentedge >= 0 && currentedge != firstedge) {
        verts.push_back(edgelist[currentedge].v);
        currentedge = edgelist[currentedge].nextHE;
    }
    rotate(verts.begin(), verts.begin() + (verts.size() - 1), verts.end());
}

int HEmesh::oppositeVertices(int tri_no, int& o1, int& o2, int& o3) {
    int edge = polylist[tri_no].firstHE;
    o1 = edgelist[edgelist[edgelist[edgelist[edge].nextHE].invHE].nextHE].v;

    edge = edgelist[edge].nextHE;
    o2 = edgelist[edgelist[edgelist[edgelist[edge].nextHE].invHE].nextHE].v;

    edge = edgelist[edge].nextHE;
    o3 = edgelist[edgelist[edgelist[edgelist[edge].nextHE].invHE].nextHE].v;

    return edge;
}

void HEmesh::oppositeTrisVerts(int tri_no, int* t, int* v) {
    int edge = polylist[tri_no].firstHE;
    int invedge = edgelist[edge].invHE;
    t[2] = edgelist[invedge].polygon;
    vector<int> vtx;
    getVertices(edgelist[invedge].polygon, vtx);

    v[2] = (vtx[0] == edgelist[invedge].v) ? 1 :
           (vtx[1] == edgelist[invedge].v) ? 2 : 0;

    edge = edgelist[edge].nextHE;
    invedge = edgelist[edge].invHE;
    t[0] = edgelist[invedge].polygon;
    getVertices(edgelist[invedge].polygon, vtx);
    v[0] = (vtx[0] == edgelist[invedge].v) ? 1 :
           (vtx[1] == edgelist[invedge].v) ? 2 : 0;

    edge = edgelist[edge].nextHE;
    invedge = edgelist[edge].invHE;
    t[1] = edgelist[invedge].polygon;
    getVertices(edgelist[invedge].polygon, vtx);
    v[1] = (vtx[0] == edgelist[invedge].v) ? 1 :
           (vtx[1] == edgelist[invedge].v) ? 2 : 0;
}

int HEmesh::getBoundaries(vector<int>& bVertices, vector<int>& loopExtents) {
    vector<bool> visited(edgelist.size(), false);
    for (int i = 0; i < edgelist.size(); i++) {
        if (!visited[i] && edgelist[i].invHE == -1)
            _growBoundary(i, visited, bVertices, loopExtents);
        visited[i] = true;
    }
    return loopExtents.size() / 2;
}

void HEmesh::_growBoundary(int startedge, vector<bool>& visited,
                           vector<int>& bVertices, vector<int>& loopExtents) {
    loopExtents.push_back(bVertices.size());
    int currentHE = startedge;
    do {
        bVertices.push_back(edgelist[currentHE].v);
        int e = currentHE;
        while (true) {
            e = edgelist[e].nextHE;
            visited[e] = true;
            if (edgelist[e].invHE == -1) {
                currentHE = e;
                break;
            }
            e = edgelist[e].invHE;
            visited[e] = true;
        }
    } while (currentHE != startedge);
    loopExtents.push_back(bVertices.size());
}

int HEmesh::triangulateLoops(vector<int>& bVertices, vector<int>& loopExtents,
                             vector<int>& dummyVtxIndices) {
    int dummyTriCount = 0;
    int numloops = loopExtents.size() / 2;
    for (int i = 0; i < numloops; i++) {
        int from = loopExtents[2*i], to = loopExtents[2*i+1];
        Vertex mean;
        for (int j = from; j < to; j++) {
            Vertex vtx = vertexlist[bVertices[j]].pt;
            mean.x += vtx.x; mean.y += vtx.y; mean.z += vtx.z;
        }
        mean.x /= (to-from); mean.y /= (to-from); mean.z /= (to-from);

        int dummyIndex = vertexlist.size();
        dummyVtxIndices.push_back(dummyIndex);
        addVertex(mean);

        for (int j = 0; j < (to-from); j++) {
            int v[3] = { dummyIndex, bVertices[to - (j+1)], bVertices[to - (j+2 >= from ? j+2 : to - 1)] };
            addPolygon(3, v);
            dummyTriCount++;
        }
    }
    return dummyTriCount;
}

void HEmesh::clearTable() {
    for (auto& vec : _tmptable) vec.clear();
    _tmptable.clear();
}

// off2ov.C functions
void loadOFF(HEmesh& hem, string s) {
    ifstream infile(s);
    string str;
    infile >> str;

    int numpts, numtri, tmp;
    infile >> numpts >> numtri >> tmp;

    for (int i = 0; i < numpts; i++) {
        Vertex p;
        infile >> p.x >> p.y >> p.z;
        hem.addVertex(p);
    }

    for (int i = 0; i < numtri; i++) {
        int v[3], jnk;
        infile >> jnk >> v[0] >> v[1] >> v[2];
        hem.addPolygon(3, v);
    }
    infile.close();
}

void saveOVTable(HEmesh& hem, string s) {
    ofstream ofile(s);
    int numpts, numtri;
    hem.size(numpts, numtri);
    ofile << numtri << endl;

    for (int i = 0; i < numtri; i++) {
        vector<int> vtxs;
        hem.getVertices(i, vtxs);
        int t[3], o[3];
        hem.oppositeTrisVerts(i, t, o);
        ofile << vtxs[0] << " " << (3*t[0] + o[0]) << endl;
        ofile << vtxs[1] << " " << (3*t[1] + o[1]) << endl;
        ofile << vtxs[2] << " " << (3*t[2] + o[2]) << endl;
    }

    ofile << numpts << endl;
    for (int i = 0; i < numpts; i++) {
        Vertex v = hem.vertexlist[i].pt;
        ofile << v.x << " " << v.y << " " << v.z << endl;
    }
    ofile.close();
}

int main(int argc, char** argv) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " input.off output.ov\n";
        return 1;
    }
    HEmesh hem;
    loadOFF(hem, argv[1]);

    vector<int> bvert, loopextents;
    int numloops = hem.getBoundaries(bvert, loopextents);

    vector<int> dummy;
    int dtris = hem.triangulateLoops(bvert, loopextents, dummy);
    int dpts = dummy.size();

    saveOVTable(hem, argv[2]);
    return 0;
}