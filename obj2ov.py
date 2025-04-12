import sys
import numpy as np
from collections import defaultdict

def split_faces_by_components(vertices, faces):
    # Build an adjacency list from the faces.
    adj = {i: set() for i in range(len(vertices))}
    for face in faces:
        n = len(face)
        for i in range(n):
            a, b = face[i], face[(i + 1) % n]
            adj[a].add(b)
            adj[b].add(a)

    # Compute connected components for vertices.
    visited = [False] * len(vertices)
    vertex_component = {}
    comp_id = 0
    for i in range(len(vertices)):
        if not visited[i]:
            stack = [i]
            while stack:
                v = stack.pop()
                if not visited[v]:
                    visited[v] = True
                    vertex_component[v] = comp_id
                    stack.extend(n for n in adj[v] if not visited[n])
            comp_id += 1

    # Group faces based on the component of their vertices.
    face_groups = {}
    for face in faces:
        # Here we assume each face's vertices belong to the same component.
        cid = vertex_component[face[0]]
        # Optionally, check that all vertices in the face are in the same component.
        if any(vertex_component[v] != cid for v in face):
            raise ValueError("Face spans multiple components.")
        face_groups.setdefault(cid, []).append(face)

    return face_groups



def ov2obj(infile, outfile):
    vertices = []
    triangles = []
    
    # Read input file
    with open(infile, 'r') as f:
        for line in f:
            parts = line.strip().split()
            if not parts:
                continue
            if parts[0] == 'v':
                vertices.append(tuple(map(float, parts[1:4])))
            elif parts[0] == 'f':
                face = [int(p.split('/')[0])-1 for p in parts[1:]]
                if len(face) == 3:
                    triangles.append(face)
                elif len(face) == 4:
                    triangles.append([face[0], face[1], face[2]])
                    triangles.append([face[0], face[2], face[3]])

    components = split_faces_by_components(vertices, triangles)

    
    for comp, triangles in enumerate(components.values()):
        print(len(triangles))
        # Detect boundary edges with correct orientation
        edge_pairs = set()
        for tri in triangles:
            for i in range(3):
                v1 = tri[i]
                v2 = tri[(i+1)%3]
                edge_pairs.add((v1, v2))

        boundary_edges = [(v1, v2) for v1, v2 in edge_pairs if (v2, v1) not in edge_pairs]

        # Build directed adjacency list
        adj = defaultdict(list)
        for v0, v1 in boundary_edges:
            adj[v0].append(v1)

        # Find and close holes with correct winding
        visited = set()
        dummy_indices = []
        vertex_i = len(vertices)

        for start in adj:
            if start in visited:
                continue
            
            # Traverse boundary loop
            current, prev = start, None
            loop = []
            while True:
                loop.append(current)
                visited.add(current)
                neighbors = [n for n in adj[current] if n != prev]
                if not neighbors:
                    break
                next_node = neighbors[0]
                if next_node == loop[0] and len(loop) > 1:
                    break
                prev, current = current, next_node

            if len(loop) < 3:
                continue

            # Reverse loop to maintain outward-facing normals
            loop = loop[::-1]

            # Calculate centroid and create triangles
            centroid = np.mean([vertices[v] for v in loop], axis=0)
            vertices.append(tuple(centroid))
            
            # Create triangles with correct winding order
            new_tris = []
            for i in range(len(loop)):
                a = loop[i]
                b = loop[(i+1)%len(loop)]
                new_tris.append([vertex_i, a, b])
            
            triangles.extend(new_tris)
            dummy_indices.append(vertex_i)
            vertex_i += 1

        print(len(triangles))
        # Generate output data
        num_triangles = len(triangles)
        num_vertices = len(vertices)
        V = [t for tri in triangles for t in tri]
        O = [-1] * (3 * num_triangles)

        edge_dict = {}
        for t_idx, tri in enumerate(triangles):
            for i in range(3):
                c = 3 * t_idx + i
                v1 = tri[(i + 1) % 3]
                v2 = tri[(i - 1) % 3]
                edge = (min(v1, v2), max(v1, v2))

                if edge in edge_dict:
                    c2 = edge_dict[edge]
                    O[c] = c2
                    O[c2] = c
                else:
                    edge_dict[edge] = c

        with open(f"{outfile}_{comp}.ov", 'w') as f:
            f.write(f"{num_triangles}\n")
            for v, t in zip(V, O):
                f.write(f"{v} {t}\n")
            f.write(f"{num_vertices}\n")
            for vertex in vertices:
                f.write(f"{vertex[0]} {vertex[1]} {vertex[2]}\n")

    return dummy_indices

if __name__ == "__main__":
    dummy_indices = ov2obj(sys.argv[1], sys.argv[2])
    print(dummy_indices)