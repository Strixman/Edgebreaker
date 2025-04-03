import sys
from collections import defaultdict
from math import isclose

# -------------------------------------------------------------------
# Data types (similar to EBdatatypes.h)
# -------------------------------------------------------------------
class Coord3D:
    def __init__(self, x=0.0, y=0.0, z=0.0):
        self.x = x
        self.y = y
        self.z = z

Vertex = Coord3D
Vector = Coord3D

MAX_SIZE = 256

# Mesh type and file format enums (not used explicitly in the code below)
MANIFOLD, TPATCH = range(2)
BINARY, ASKII = range(2)

# -------------------------------------------------------------------
# Half-edge data structures (similar to HEPolMesh.h)
# -------------------------------------------------------------------
class HEvertex:
    def __init__(self, x=None, y=None, z=None, p=None):
        if p is not None:
            self.pt = p
        else:
            self.pt = Vertex(x, y, z)
        self.edge = -1  # reference to some half-edge starting at this vertex

class HEedge:
    def __init__(self):
        self.v = -1          # vertex to which this HE is incident
        self.nextHE = -1     # next HE of current polygon
        self.invHE = -1      # opposite edge of adjacent polygon
        self.polygon = -1    # reference to the polygon

class HEpolygon:
    def __init__(self):
        self.firstHE = -1    # first HE of the polygon

class HEmesh:
    def __init__(self):
        self.vertexlist = []  # list of HEvertex
        self.edgelist = []    # list of HEedge
        self.polylist = []    # list of HEpolygon
        # temporary table: for each vertex, a list of incident half-edges indices
        self._tmptable = []

    def addVertex(self, *args):
        """Add vertex either from a Vertex object or x,y,z coordinates."""
        if len(args) == 1 and isinstance(args[0], Vertex):
            v = args[0]
            self.vertexlist.append(HEvertex(p=v))
        elif len(args) == 3:
            self.vertexlist.append(HEvertex(x=args[0], y=args[1], z=args[2]))
        else:
            raise ValueError("addVertex requires either a Vertex or three coordinates.")
        self._tmptable.append([])

    def addPolygon(self, indices):
        """
        indices: list of vertex indices (assumed to form a closed polygon).
        The polygon is triangulated (or kept as polygon) following the half-edge setup.
        """
        n = len(indices)
        if n < 3:
            return

        p = HEpolygon()
        prev = -1

        for i in range(n):
            e = HEedge()
            e.v = indices[(i + 1) % n]
            # record this half-edge in the temp table for the current vertex
            self._tmptable[indices[i]].append(len(self.edgelist))

            if self.vertexlist[indices[i]].edge == -1:
                self.vertexlist[indices[i]].edge = len(self.edgelist)

            # try to find the inverse half-edge
            inv = -1
            k = indices[(i + 1) % n]
            for ce in self._tmptable[k]:
                if self.edgelist[ce].v == indices[i]:
                    inv = ce
                    break
            if inv != -1:
                self.edgelist[inv].invHE = len(self.edgelist)
                e.invHE = inv

            if p.firstHE == -1:
                p.firstHE = len(self.edgelist)
            else:
                self.edgelist[prev].nextHE = len(self.edgelist)

            e.polygon = len(self.polylist)
            prev = len(self.edgelist)
            self.edgelist.append(e)
        # complete the loop for the polygon
        self.edgelist[prev].nextHE = p.firstHE
        self.polylist.append(p)

    def size(self):
        """Return tuple (numpts, numpol)."""
        return len(self.vertexlist), len(self.polylist)

    def vertexNeighbours(self, vtx_no):
        neighvtx = []
        incidentHE = self.vertexlist[vtx_no].edge
        if incidentHE == -1:
            return -1, neighvtx

        neighvtx.append(self.edgelist[incidentHE].v)
        # follow the half-edge around the vertex
        currentHE = self.edgelist[self.edgelist[incidentHE].invHE].nextHE if self.edgelist[incidentHE].invHE != -1 else -1

        while currentHE >= 0 and currentHE != incidentHE:
            neighvtx.append(self.edgelist[currentHE].v)
            inv = self.edgelist[currentHE].invHE
            if inv == -1:
                currentHE = -1
            else:
                currentHE = self.edgelist[inv].nextHE
        return (len(neighvtx) if currentHE != -1 else -1), neighvtx

    def adjacentPolygons(self, pol_no):
        neighpol = []
        firstedge = self.polylist[pol_no].firstHE
        if firstedge == -1:
            return neighpol

        inv = self.edgelist[firstedge].invHE
        if inv != -1:
            neighpol.append(self.edgelist[inv].polygon)
        currentedge = self.edgelist[firstedge].nextHE

        while currentedge >= 0 and currentedge != firstedge:
            inv = self.edgelist[currentedge].invHE
            if inv != -1:
                neighpol.append(self.edgelist[inv].polygon)
            currentedge = self.edgelist[currentedge].nextHE
        return neighpol

    def getVertices(self, pol_no):
        verts = []
        firstedge = self.polylist[pol_no].firstHE
        if firstedge == -1:
            return verts

        verts.append(self.edgelist[firstedge].v)
        currentedge = self.edgelist[firstedge].nextHE

        while currentedge >= 0 and currentedge != firstedge:
            verts.append(self.edgelist[currentedge].v)
            currentedge = self.edgelist[currentedge].nextHE
        # perform rotation: move last element to the front
        if verts:
            verts = [verts[-1]] + verts[:-1]
        return verts

    def oppositeVertices(self, tri_no):
        """Compute opposite vertices for a triangle.
           Returns a tuple (edge, o1, o2, o3) where o1, o2, o3 are computed as in the C++ code.
        """
        edge = self.polylist[tri_no].firstHE
        # chain of half-edges as in C++ code
        o1 = self.edgelist[self.edgelist[self.edgelist[self.edgelist[edge].nextHE].invHE].nextHE].v

        edge = self.edgelist[edge].nextHE
        o2 = self.edgelist[self.edgelist[self.edgelist[self.edgelist[edge].nextHE].invHE].nextHE].v

        edge = self.edgelist[edge].nextHE
        o3 = self.edgelist[self.edgelist[self.edgelist[self.edgelist[edge].nextHE].invHE].nextHE].v

        return edge, o1, o2, o3

    def oppositeTrisVerts(self, tri_no):
        """
        For the given triangle, compute arrays t and v.
        Returns two lists, t and v, each of length 3.
        """
        t = [None] * 3
        v = [None] * 3

        edge = self.polylist[tri_no].firstHE
        invedge = self.edgelist[edge].invHE
        t[2] = self.edgelist[invedge].polygon
        vtx = self.getVertices(self.edgelist[invedge].polygon)
        if vtx:
            if vtx[0] == self.edgelist[invedge].v:
                v[2] = 1
            elif len(vtx) > 1 and vtx[1] == self.edgelist[invedge].v:
                v[2] = 2
            else:
                v[2] = 0

        edge = self.edgelist[edge].nextHE
        invedge = self.edgelist[edge].invHE
        t[0] = self.edgelist[invedge].polygon
        vtx = self.getVertices(self.edgelist[invedge].polygon)
        if vtx:
            if vtx[0] == self.edgelist[invedge].v:
                v[0] = 1
            elif len(vtx) > 1 and vtx[1] == self.edgelist[invedge].v:
                v[0] = 2
            else:
                v[0] = 0

        edge = self.edgelist[edge].nextHE
        invedge = self.edgelist[edge].invHE
        t[1] = self.edgelist[invedge].polygon
        vtx = self.getVertices(self.edgelist[invedge].polygon)
        if vtx:
            if vtx[0] == self.edgelist[invedge].v:
                v[1] = 1
            elif len(vtx) > 1 and vtx[1] == self.edgelist[invedge].v:
                v[1] = 2
            else:
                v[1] = 0

        return t, v

    def getBoundaries(self):
        """Find boundary vertices and loop extents.
           Returns a tuple (num_boundaries, bVertices, loopExtents).
        """
        bVertices = []
        loopExtents = []
        visited = [False] * len(self.edgelist)
        for i in range(len(self.edgelist)):
            if not visited[i] and self.edgelist[i].invHE == -1:
                self._growBoundary(i, visited, bVertices, loopExtents)
            visited[i] = True
        # number of boundaries is half the size of loopExtents list
        return len(loopExtents) // 2, bVertices, loopExtents

    def _growBoundary(self, startedge, visited, bVertices, loopExtents):
        loopExtents.append(len(bVertices))
        currentHE = startedge
        while True:
            bVertices.append(self.edgelist[currentHE].v)
            e = currentHE
            while True:
                e = self.edgelist[e].nextHE
                visited[e] = True
                if self.edgelist[e].invHE == -1:
                    currentHE = e
                    break
                e = self.edgelist[e].invHE
                visited[e] = True
            if currentHE == startedge:
                break
        loopExtents.append(len(bVertices))

    def triangulateLoops(self, bVertices, loopExtents):
        """
        For each boundary loop, compute a dummy vertex as the centroid and add triangles.
        Returns a tuple (dummyTriCount, dummyVtxIndices).
        """
        dummyTriCount = 0
        dummyVtxIndices = []
        numloops = len(loopExtents) // 2
        for i in range(numloops):
            frm = loopExtents[2 * i]
            to = loopExtents[2 * i + 1]
            mean = Vertex(0.0, 0.0, 0.0)
            count = to - frm
            for j in range(frm, to):
                vtx = self.vertexlist[bVertices[j]].pt
                mean.x += vtx.x
                mean.y += vtx.y
                mean.z += vtx.z
            mean.x /= count
            mean.y /= count
            mean.z /= count

            dummyIndex = len(self.vertexlist)
            dummyVtxIndices.append(dummyIndex)
            self.addVertex(mean)

            # Triangulate the boundary loop.
            # Note: The original C++ code uses a ternary operator;
            # here we assume the condition always uses j+2.
            for j in range((to - frm) - 1):
                # Create triangle: [dummyIndex, second vertex, third vertex]
                v_triangle = [
                    dummyIndex,
                    bVertices[to - (j + 1)],
                    bVertices[to - (j + 2)]
                ]
                self.addPolygon(v_triangle)
                dummyTriCount += 1
        return dummyTriCount, dummyVtxIndices

    def clearTable(self):
        for vec in self._tmptable:
            vec.clear()
        self._tmptable.clear()

# -------------------------------------------------------------------
# Functions for loading and saving (similar to obj2ov.C)
# -------------------------------------------------------------------
def loadOBJ(hem, filename):
    try:
        with open(filename, 'r') as infile:
            for line in infile:
                line = line.strip()
                if not line or line.startswith('#'):
                    continue
                parts = line.split()
                if not parts:
                    continue
                type_token = parts[0]
                if type_token == "v":
                    # vertex line
                    try:
                        x, y, z = map(float, parts[1:4])
                        hem.addVertex(x, y, z)
                    except Exception as e:
                        print("Error parsing vertex:", e, file=sys.stderr)
                elif type_token == "f":
                    indices = []
                    for token in parts[1:]:
                        # remove texture/normal info if present
                        if '/' in token:
                            token = token.split('/')[0]
                        try:
                            idx = int(token)
                        except Exception as e:
                            print("Invalid face index:", token, file=sys.stderr)
                            continue
                        current_vertex_count = len(hem.vertexlist)
                        if idx < 0:
                            idx = current_vertex_count + idx
                        else:
                            idx = idx - 1
                        if idx < 0 or idx >= current_vertex_count:
                            print("Invalid vertex index in face:", token, file=sys.stderr)
                            continue
                        indices.append(idx)
                    n = len(indices)
                    if n < 3:
                        continue
                    # triangulate the face (fan triangulation)
                    for i in range(1, n - 1):
                        tri = [indices[0], indices[i], indices[i + 1]]
                        hem.addPolygon(tri)
    except Exception as e:
        print("Error reading file:", e, file=sys.stderr)

def saveOVTable(hem, filename):
    try:
        with open(filename, 'w') as ofile:
            numpts, numtri = hem.size()
            ofile.write(str(numtri) + "\n")
            for i in range(numtri):
                vtxs = hem.getVertices(i)
                t, o = hem.oppositeTrisVerts(i)
                # Write three lines per triangle
                ofile.write(f"{vtxs[0]} {3 * t[0] + o[0]}\n")
                ofile.write(f"{vtxs[1]} {3 * t[1] + o[1]}\n")
                ofile.write(f"{vtxs[2]} {3 * t[2] + o[2]}\n")
            ofile.write(str(numpts) + "\n")
            for i in range(numpts):
                v = hem.vertexlist[i].pt
                ofile.write(f"{v.x} {v.y} {v.z}\n")
    except Exception as e:
        print("Error writing file:", e, file=sys.stderr)

# -------------------------------------------------------------------
# Main entry point (similar to main function in C++)
# -------------------------------------------------------------------
def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} input.obj output.ov", file=sys.stderr)
        sys.exit(1)
    hem = HEmesh()
    loadOBJ(hem, sys.argv[1])

    bvert_count, bvert, loopextents = hem.getBoundaries()
    dummy_tri_count, dummy_vtx_indices = hem.triangulateLoops(bvert, loopextents)
    # dpts is len(dummy_vtx_indices) if needed

    saveOVTable(hem, sys.argv[2])
    sys.exit(0)

if __name__ == "__main__":
    main()
