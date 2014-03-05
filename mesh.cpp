#include "mesh.h"
#include "balloon.h"

#define MESH_RESOLUTION 64 // The number of vertices accross the mesh width
#define NUM_BLOBS 20       // Number of random blobs placed in the mesh

// Lighting Properties
GLfloat terrain_ambient[]    = {0.4, 0.4, 0.4, 1.0};
GLfloat terrain_specular[]   = {0.01, 0.01, 0.01, 1.0};
GLfloat terrain_diffuse[]    = {0.5, 0.5, 0.5, 1.0};
GLfloat terrain_shininess[]  = {0.0};

// Global Variables
int resolution;
float maxHeight;

// Data Structures
VECTOR3D **vertices;
VECTOR3D **normals;
Quad *quads;

std::vector<Blob> blobs;

int numVerts;
int numQuads;
int numBlobs;

// Texture Properties
RGBpixmap mesh_pix[2];
GLuint mesh_tex[2];


/**************************************************************************************
 **     Public Mesh Functions
 **
 **************************************************************************************/

///////////////////////////////////////////////////////////////////////////////
//  initMesh - Initialize the mesh

void Mesh::initMesh()
{
    maxHeight = 0;
    
    // initialize mesh and quad arrays
    allocateMesh(MESH_RESOLUTION);
    initializeMesh(0.0f, 0.0f, MESH_RESOLUTION);

    // Randomize the terrain (add blobs)
    Blob b;
    srand(time(NULL));

    for(int i = 0; i < NUM_BLOBS; ++i)
    {
        b.position = VECTOR3D(getRandomVertex());
        b.height = rand() % 8 + 2.0; 
        b.width = ((rand()%20)/100.0f + 0.001f) / (b.height/3.0f);
        addBlob(b);
    }

    // update vertex heights, compute normals, and add textures
    updateMesh();
    updateNormals();
    texturizeMesh();
}

///////////////////////////////////////////////////////////////////////////////
//  drawMesh - Calls the display fuction of the mesh.

void Mesh::drawMesh()
{
    displayMesh();
}

///////////////////////////////////////////////////////////////////////////////
//  printBlobs - Prints properties of all blobs to the command prompt.

void Mesh::printBlobs(void)
{
    for(int i = 0; i < numBlobs; ++i)
    {
        Blob blob = blobs.at(i);
        cout << "x=" << blob.position.x 
             << "\t y=" << blob.position.y 
             << "\t z=" << blob.position.z 
             << "\t h=" << blob.height 
             << "\t w=" << blob.width
             << "\n";
    }
}

///////////////////////////////////////////////////////////////////////////////
//  getRandomVertex - returns a random vertex position in the mesh.

VECTOR3D Mesh::getRandomVertex()
{
    int r = rand() % (resolution-1) +1;
    int c = rand() % (resolution-1) +1;
    
    return vertices[r][c];
}

///////////////////////////////////////////////////////////////////////////////
//  getMaxHeight - returns the highest elevation of the mesh

float Mesh::getMaxHeight()
{
    return maxHeight;
}


/**************************************************************************************
 **     Private Mesh Functions
 **
 **************************************************************************************/


///////////////////////////////////////////////////////////////////////////////
//  allocateMesh - Allocate vertex array, quad array and normals array memory.

void Mesh::allocateMesh (int dim)
{
    resolution = dim;
    numVerts = (dim+1)*(dim+1);
    numQuads = dim*dim;

    vertices = new VECTOR3D*[dim+1];
    normals  = new VECTOR3D*[dim+1];

    for(int i = 0; i <= dim; ++i)
    {
        vertices[i] = new VECTOR3D[dim+1];
        normals [i] = new VECTOR3D[dim+1];
    }

    quads = new Quad[numQuads];
}

///////////////////////////////////////////////////////////////////////////////
//  initializeMesh - Fill the vertex array and quad array.

void Mesh::initializeMesh(float originX, float originZ, float sideWidth)
{
    float cornerX = originX - (sideWidth/2);
    float cornerZ = originZ - (sideWidth/2);
    float delta = sideWidth/(resolution);

    // fill the vertex array
    for(int row = 0; row <= resolution; ++row)
    {
        float zCoord = (cornerZ + (row*delta));

        for(int col = 0; col <= resolution; ++col)
        {
            vertices[row][col] = VECTOR3D((cornerX + (col*delta)), 0.0f, zCoord);
        }
    }

    // fill the quad array
    for(int i = 0; i < numQuads; ++i)
    {
        int row = i/(resolution);
        int col = i%(resolution);

        Quad q;

        q.v1 = &vertices[row  ][col  ];
        q.v2 = &vertices[row  ][col+1];
        q.v3 = &vertices[row+1][col+1];
        q.v4 = &vertices[row+1][col  ];

        quads[i] = q;
    }
}

///////////////////////////////////////////////////////////////////////////////
//  texturizeMesh - Set up the mesh texture mapping and properties

void Mesh::texturizeMesh()
{
    // Setup Texture Mapping
    mesh_pix[0].readBMPFile("textures/ground.bmp");
    glGenTextures(1, mesh_tex);

    // Texture Properties
    glBindTexture(GL_TEXTURE_2D, mesh_tex[0]);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, mesh_pix[0].nCols, mesh_pix[0].nRows, 0, GL_RGB, GL_UNSIGNED_BYTE, mesh_pix[0].pixel);
}
 
///////////////////////////////////////////////////////////////////////////////
//  resetMesh - Reset all vertex height values (set Y values to 0).

void Mesh::resetMesh()
{
    for(int i = 0; i <= resolution; ++i)
        for(int k = 0; k <= resolution; ++k)
            (vertices[i][k]).SetY(0.0f);
}

///////////////////////////////////////////////////////////////////////////////
//  updateMesh - Evaluate the height impact from each blob on the vertices.

void Mesh::updateMesh()
{
    for(int i = 0; i <= resolution; ++i)
    {
        for(int k = 0; k <= resolution; ++k)
        {
            computeVertexHeight(&vertices[i][k]);
            maxHeight = max(maxHeight, vertices[i][k].y);
        }
    }
            
}

///////////////////////////////////////////////////////////////////////////////
//  updateNormals - Evaluate the normals at each vertex for lighting.
 
void Mesh::updateNormals()
{
    // Compute normals for all non-edge vertices
    for(int r = 1; r < resolution; ++r)
    {
        for(int c = 1; c < resolution; ++c)
        {
            VECTOR3D v = vertices[r][c];

            // Cross Products
            VECTOR3D n1 = (vertices[r+1][c+1] - v).CrossProduct(vertices[r-1][c  ] - v);
            VECTOR3D n2 = (vertices[r+1][c  ] - v).CrossProduct(vertices[r+1][c+1] - v);
            VECTOR3D n3 = (vertices[r  ][c-1] - v).CrossProduct(vertices[r+1][c  ] - v);
            VECTOR3D n4 = (vertices[r-1][c  ] - v).CrossProduct(vertices[r  ][c-1] - v);

            // Normalize
            n1.Normalize(); n2.Normalize(); n3.Normalize(); n4.Normalize();

            // Calculate Vertex Normal
            VECTOR3D vn = n1+n2+n3+n4;
            vn.Normalize(); 
            normals[r][c] = vn;
        }
    }

    // Compute normals for all edge (top, bottom, left, right) vertices
    for(int i = 1; i < resolution; ++i)
    {
        VECTOR3D vTop    = vertices[0][i];
        VECTOR3D vBottom = vertices[resolution][i];
        VECTOR3D vLeft   = vertices[i][0];
        VECTOR3D vRight  = vertices[i][resolution];

        // Top Cross Products
        VECTOR3D nTop1    = (vertices[1][i  ] - vTop).CrossProduct(vertices[0][i+1] - vTop);
        VECTOR3D nTop2    = (vertices[0][i-1] - vTop).CrossProduct(vertices[1][i  ] - vTop);

        // Bottom Cross Products
        VECTOR3D nBottom1 = (vertices[resolution-1][i  ] - vBottom).CrossProduct(vertices[resolution  ][i-1] - vBottom);
        VECTOR3D nBottom2 = (vertices[resolution  ][i+1] - vBottom).CrossProduct(vertices[resolution-1][i  ] - vBottom);

        // Left Cross Products
        VECTOR3D nLeft1   = (vertices[i  ][1] - vLeft).CrossProduct(vertices[i-1][0] - vLeft);
        VECTOR3D nLeft2   = (vertices[i+1][0] - vLeft).CrossProduct(vertices[i  ][1] - vLeft);

        // Right Cross Products
        VECTOR3D nRight1  = (vertices[i-1][resolution  ] - vRight).CrossProduct(vertices[i  ][resolution-1] - vRight);
        VECTOR3D nRight2  = (vertices[i  ][resolution-1] - vRight).CrossProduct(vertices[i+1][resolution  ] - vRight);

        // Normalize
        nTop1.Normalize();    nTop2.Normalize();
        nBottom1.Normalize(); nBottom2.Normalize();
        nLeft1.Normalize();   nLeft2.Normalize();
        nRight1.Normalize();  nRight2.Normalize();

        // Calculate Vertex Normals
        VECTOR3D vnTop    = nTop1    + nTop2;      vnTop.Normalize();
        VECTOR3D vnBottom = nBottom1 + nBottom2;   vnBottom.Normalize();
        VECTOR3D vnLeft   = nLeft1   + nLeft2;     vnLeft.Normalize();
        VECTOR3D vnRight  = nRight1  + nRight2;    vnRight.Normalize();

        normals[0][i]          = vnTop;
        normals[resolution][i] = vnBottom;
        normals[i][0]          = vnLeft;
        normals[i][resolution] = vnRight;
    }

    // Compute normals for the 4 corner vertices of the mesh
    VECTOR3D vCorner1 = vertices[0][0];
    VECTOR3D vCorner2 = vertices[0][resolution];
    VECTOR3D vCorner3 = vertices[resolution][0];
    VECTOR3D vCorner4 = vertices[resolution][resolution];

    // Cross Products
    VECTOR3D n1 = (vertices[1][0] - vCorner1).CrossProduct(vertices[0][1] - vCorner1);
    VECTOR3D n2 = (vertices[0][resolution-1] - vCorner2).CrossProduct(vertices[1][resolution] - vCorner2);
    VECTOR3D n3 = (vertices[resolution][1] - vCorner3).CrossProduct(vertices[resolution-1][0] - vCorner3);
    VECTOR3D n4 = (vertices[resolution-1][resolution] - vCorner4).CrossProduct(vertices[resolution][resolution-1] - vCorner4);

    // Normalize
    n1.Normalize(); n2.Normalize(); n3.Normalize(); n4.Normalize();

    // Calculate Vertex Normals
    normals[0][0]                   = n1;
    normals[0][resolution]          = n2;
    normals[resolution][0]          = n3;
    normals[resolution][resolution] = n4;

}

///////////////////////////////////////////////////////////////////////////////
//  drawMesh - draw the mesh of quads (called from the main display function).

void Mesh::displayMesh()
{
    // Specify material parameters for the mesh 
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, terrain_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, terrain_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, terrain_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, terrain_shininess);

    // Set the color of the mesh
    glColor3f(0.8, 0.8, 0.8);
    glBindTexture(GL_TEXTURE_2D, mesh_tex[0]);

    // Draw all quads
    for(int i = 0; i < numQuads; ++i)
    {
        // Get Current Quad
        Quad q = quads[i];

        // Retrieve the Normals for this Quad
        int row = i/(resolution);
        int col = i%(resolution);

        VECTOR3D *n1 = &normals[row  ][col  ];
        VECTOR3D *n2 = &normals[row  ][col+1];
        VECTOR3D *n3 = &normals[row+1][col+1];
        VECTOR3D *n4 = &normals[row+1][col  ];

        // Draw the Quad 
        glBegin(GL_QUADS);
          glTexCoord2f(0.0, 0.0);
          glNormal3f(  n1->GetX(),   n1->GetY(),   n1->GetZ());
          glVertex3f(q.v1->GetX(), q.v1->GetY(), q.v1->GetZ());
          glTexCoord2f(0.0, 1.0);
          glNormal3f(  n2->GetX(),   n2->GetY(),   n2->GetZ());
	      glVertex3f(q.v2->GetX(), q.v2->GetY(), q.v2->GetZ());
          glTexCoord2f(1.0, 1.0);
          glNormal3f(  n3->GetX(),   n3->GetY(),   n3->GetZ());
	      glVertex3f(q.v3->GetX(), q.v3->GetY(), q.v3->GetZ());
          glTexCoord2f(1.0, 0.0);
          glNormal3f(  n4->GetX(),   n4->GetY(),   n4->GetZ());
	      glVertex3f(q.v4->GetX(), q.v4->GetY(), q.v4->GetZ());
	      glNormal3f(0,1,0);
        glEnd();
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}

///////////////////////////////////////////////////////////////////////////////
//  addBlob - Adds a new blob to the terrain mesh

void Mesh::addBlob(Blob b)
{
    blobs.push_back(b);
    numBlobs++;
}

///////////////////////////////////////////////////////////////////////////////
//  getLastBlob - returns the last blob

Blob* Mesh::getLastBlob()
{
    return &blobs.back();
}

///////////////////////////////////////////////////////////////////////////////
//  computeVertexHeight - compute height of the vertex as affected by the blobs.

void Mesh::computeVertexHeight(VECTOR3D *v)
{
    float sigmaHeight = 0;
    float distanceSquared;
    Blob blob;
 
    // Calculate the height change from each blob using the Gaussian function
    for(int i = 0; i < numBlobs; ++i)
    {
        blob = blobs.at(i);
        distanceSquared = (*v-blob.position).GetQuaddLength();
        sigmaHeight += blob.height * exp((-blob.width)*distanceSquared);
    }
    
    // Adjust the height of the vertex
    v->SetY(v->GetY() + sigmaHeight);
}
