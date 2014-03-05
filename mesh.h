#ifndef MESH_H
#define MESH_H

#include "a3.h"


typedef struct Quad {
    VECTOR3D *v1, *v2, *v3, *v4;
} Quad;


typedef struct Blob {
    VECTOR3D position;

    float width;
    float height;
} Blob;


class Mesh
{
    public:

        // Mesh Functions
        void initMesh();
        void drawMesh();

        // Print Functions
        void printBlobs(void);

        // Useful Functions
        VECTOR3D getRandomVertex();
        float getMaxHeight();


    protected:
        
        // Private Blob Functions
        void addBlob(Blob b); // adds a blob to the terrain mesh
        Blob* getLastBlob(); // returns the last blob

        // Private Mesh Functions
        void allocateMesh (int dim); // allocate vertex array and quad array memory
        void initializeMesh(float originX, float originZ, float sideWidth); // construct vertex array and quad array
        void texturizeMesh(); // set up texture mapping for the mesh

        void resetMesh();     // for each mesh vertex, set height (Y value) to 0
        void updateMesh();    // for each mesh vertex, evaluate height contribution from each blob
        void updateNormals(); // for each mesh vertex, update the normal vector
        void displayMesh();   // displays the mesh on the screen

        void computeVertexHeight(VECTOR3D *v); // compute height of the vertex as affected by the blobs

};


#endif