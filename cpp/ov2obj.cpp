#include <iostream>
#include <fstream>
using namespace std;

void readOV(int& numpts, int& numtri,
            float **x, float **y, float **z, int **v1, int **v2, int **v3, string s);
void writeOBJ(int numpts, int numtri,
             float *x, float *y, float *z, int *v1, int *v2, int *v3, string s);

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        cerr << "Usage: ov2obj input.ov output.obj" << endl;
        return 1;
    }
    
    int numpts, numtri;
    int *v1, *v2, *v3;  // Triangle vertex indices (0-based)
    float *x, *y, *z;    // Vertex coordinates
    
    readOV(numpts, numtri, &x, &y, &z, &v1, &v2, &v3, argv[1]);
    writeOBJ(numpts, numtri, x, y, z, v1, v2, v3, argv[2]);

    // Cleanup
    delete[] x; delete[] y; delete[] z;
    delete[] v1; delete[] v2; delete[] v3;
    
    return 0;
}

void readOV(int& numpts, int& numtri,
           float **x, float **y, float **z, 
           int **v1, int **v2, int **v3, string s)
{
    ifstream infile(s);
    infile >> numtri;
    
    // Allocate triangle arrays
    *v1 = new int[numtri];
    *v2 = new int[numtri];
    *v3 = new int[numtri];
    
    int tmp;
    for (int i = 0; i < numtri; i++) {
        infile >> (*v1)[i] >> tmp;  // Read vertex index and ignore adjacency
        infile >> (*v2)[i] >> tmp;
        infile >> (*v3)[i] >> tmp;
    }
    
    // Read vertices
    infile >> numpts;
    *x = new float[numpts];
    *y = new float[numpts];
    *z = new float[numpts];
    for (int i = 0; i < numpts; i++) {
        infile >> (*x)[i] >> (*y)[i] >> (*z)[i];
    }
    
    infile.close();
}

void writeOBJ(int numpts, int numtri,
             float *x, float *y, float *z,
             int *v1, int *v2, int *v3, string s)
{
    ofstream outfile(s);
    
    // Write vertices (v x y z)
    for (int i = 0; i < numpts; i++) {
        outfile << "v " << x[i] << " " << y[i] << " " << z[i] << endl;
    }
    
    // Write faces (1-based indices)
    for (int i = 0; i < numtri; i++) {
        outfile << "f " << v1[i]+1 << " " 
                        << v2[i]+1 << " " 
                        << v3[i]+1 << endl;
    }
    
    outfile.close();
}