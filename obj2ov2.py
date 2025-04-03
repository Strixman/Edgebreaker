import sys
from dataclasses import dataclass
from collections import defaultdict
from typing import List, Tuple, Optional

@dataclass
class Vertex:
    x: float = 0.0
    y: float = 0.0
    z: float = 0.0
    edge: int = -1  # Reference to a half-edge starting at this vertex

@dataclass
class HEEdge:
    v: int = -1          # Target vertex index
    nextHE: int = -1     # Next half-edge in polygon
    invHE: int = -1      # Opposite half-edge
    polygon: int = -1    # Parent polygon index

@dataclass
class HEPolygon:
    firstHE: int = -1    # First half-edge index

class HEMesh:
    def __init__(self):
        self.vertices: List[Vertex] = []
        self.edges: List[HEEdge] = []
        self.polygons: List[HEPolygon] = []
        self.edge_map = defaultdict(lambda: -1)  # (from, to) -> edge index

    def add_vertex(self, x: float, y: float, z: float) -> int:
        self.vertices.append(Vertex(x, y, z))
        return len(self.vertices) - 1

    def add_polygon(self, indices: List[int]) -> None:
        n = len(indices)
        if n < 3:
            return

        polygon_index = len(self.polygons)
        self.polygons.append(HEPolygon())
        new_edges = []

        # Create all edges first
        for i in range(n):
            from_v = indices[i]
            to_v = indices[(i+1)%n]
            
            edge = HEEdge(v=to_v, polygon=polygon_index)
            self.edges.append(edge)
            new_edges.append(len(self.edges)-1)

        # Set up connections and inverses
        for i in range(n):
            from_v = indices[i]
            to_v = indices[(i+1)%n]
            
            # Find inverse edge if it exists
            inv_edge_idx = self.edge_map.get((to_v, from_v), -1)
            if inv_edge_idx != -1:
                self.edges[new_edges[i]].invHE = inv_edge_idx
                self.edges[inv_edge_idx].invHE = new_edges[i]
            
            self.edge_map[(from_v, to_v)] = new_edges[i]

            # Set next pointer
            self.edges[new_edges[i]].nextHE = new_edges[(i+1)%n]

            # Set vertex edge reference if not set
            if self.vertices[from_v].edge == -1:
                self.vertices[from_v].edge = new_edges[i]

        self.polygons[polygon_index].firstHE = new_edges[0]

    def get_vertices(self, poly_idx: int) -> List[int]:
        vertices = []
        he = self.polygons[poly_idx].firstHE
        if he == -1:
            return vertices
        
        start = he
        while True:
            vertices.append(self.edges[he].v)
            he = self.edges[he].nextHE
            if he == start:
                break
        
        # Rotate to match original order
        if vertices:
            vertices = [vertices[-1]] + vertices[:-1]
        return vertices

    def opposite_tris_verts(self, poly_idx: int) -> Tuple[List[int], List[int]]:
        t = [-1, -1, -1]
        v = [-1, -1, -1]
        
        he = self.polygons[poly_idx].firstHE
        for i in range(3):
            inv_he = self.edges[he].invHE
            if inv_he != -1:
                t[i] = self.edges[inv_he].polygon
                # Find the position in the adjacent polygon
                adj_he = self.polygons[t[i]].firstHE
                for j in range(3):
                    if self.edges[adj_he].v == self.edges[he].v:
                        v[i] = j
                        break
                    adj_he = self.edges[adj_he].nextHE
            he = self.edges[he].nextHE
        
        return t, v

def load_obj(filename: str) -> HEMesh:
    mesh = HEMesh()
    with open(filename) as f:
        for line in f:
            parts = line.strip().split()
            if not parts:
                continue
            if parts[0] == 'v':
                mesh.add_vertex(*map(float, parts[1:4]))
            elif parts[0] == 'f':
                verts = []
                for p in parts[1:]:
                    # Handle cases like "1//2" or "1/2/3"
                    v = p.split('/')[0]
                    if not v:
                        continue
                    vert_idx = int(v) - 1  # OBJ files are 1-indexed
                    verts.append(vert_idx)
                
                # Triangulate polygon if it has more than 3 vertices
                for i in range(1, len(verts)-1):
                    mesh.add_polygon([verts[0], verts[i], verts[i+1]])
    return mesh

def save_ov(mesh: HEMesh, filename: str) -> None:
    with open(filename, 'w') as f:
        # Write triangles
        f.write(f"{len(mesh.polygons)}\n")
        for i in range(len(mesh.polygons)):
            t, v = mesh.opposite_tris_verts(i)
            verts = mesh.get_vertices(i)
            for j in range(3):
                if t[j] != -1:
                    f.write(f"{verts[j]} {3 * t[j] + v[j]}\n")
                else:
                    f.write(f"{verts[j]} -1\n")
        
        # Write vertices
        f.write(f"{len(mesh.vertices)}\n")
        for vertex in mesh.vertices:
            f.write(f"{vertex.x} {vertex.y} {vertex.z}\n")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python mesh_converter.py input.obj output.ov")
        sys.exit(1)

    mesh = load_obj(sys.argv[1])
    save_ov(mesh, sys.argv[2])