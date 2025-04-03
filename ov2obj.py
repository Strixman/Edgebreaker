import sys

def read_ov(ov_filename):
    """Reads an OVTable file and extracts vertices and triangles."""
    vertices = []
    faces = []
    opposite_corners = []

    with open(ov_filename, 'r') as f:
        # Read number of triangles
        num_triangles = int(f.readline().strip())
        for _ in range(num_triangles):
            v1, o1 = map(int, f.readline().split())
            v2, o2 = map(int, f.readline().split())
            v3, o3 = map(int, f.readline().split())
            faces.append((v1, v2, v3))
            opposite_corners.extend([o1, o2, o3])

        # Read number of vertices
        num_vertices = int(f.readline().strip())
        for _ in range(num_vertices):
            x, y, z = map(float, f.readline().split())
            vertices.append((x, y, z))

    return vertices, faces, opposite_corners

def write_obj(obj_filename, vertices, faces):
    """Writes the OBJ format to a file."""
    with open(obj_filename, 'w') as f:
        # Write vertex coordinates
        for x, y, z in vertices:
            f.write(f"v {x} {y} {z}\n")

        # Write faces (triangles)
        for v1, v2, v3 in faces:
            f.write(f"f {v1 + 1} {v2 + 1} {v3 + 1}\n")

def ov_to_obj(ov_filename, obj_filename):
    """Converts OVTable format to OBJ format."""
    vertices, faces, opposite_corners = read_ov(ov_filename)
    write_obj(obj_filename, vertices, faces)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python ov_to_obj.py input.ov output.obj")
        sys.exit(1)

    ov_to_obj(sys.argv[1], sys.argv[2])
