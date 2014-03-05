#include "balloon.h"

// Lighting Properties
GLfloat balloon_ambient[]    = {0.53, 0.54, 0.53, 1.0};
GLfloat balloon_specular[]   = {0.01, 0.01, 0.0, 1.0};
GLfloat balloon_diffuse[]   = {0.5, 0.5, 0.5, 1.0};
GLfloat balloon_shininess[]  = {0.0};

// Texture Properties
RGBpixmap balloon_pix[3];
GLuint balloon_tex[3];


///////////////////////////////////////////////////////////////////////////////
//  initBalloon - Initialize the balloon.

void Balloon::initBalloon(float terrainHeight)
{
    position = VECTOR3D(0.0f, terrainHeight+10.0, 0.0f);

    GLfloat planes[] = {0.0, 0.0, 0.3, 0.0};
    GLfloat planet[] = {0.0, 0.3, 0.0, 0.0};

    // Setup Texture Mapping
    balloon_pix[0].readBMPFile("textures/balloon_top.bmp");
    balloon_pix[1].readBMPFile("textures/balloon_rope.bmp");
    balloon_pix[2].readBMPFile("textures/balloon_basket.bmp");

    glGenTextures(3, balloon_tex);

    // Texture Properties
    for(int i = 0; i < 3; ++i)
    {
        glBindTexture(GL_TEXTURE_2D, balloon_tex[i]);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glTexGenfv(GL_S, GL_OBJECT_PLANE, planes);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
        glTexGenfv(GL_T, GL_OBJECT_PLANE, planet);
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, balloon_pix[i].nCols, balloon_pix[i].nRows, 0, GL_RGB, GL_UNSIGNED_BYTE, balloon_pix[i].pixel);
    }
}

///////////////////////////////////////////////////////////////////////////////
//  getBaseHeight - returns the height to the base of the balloon.

float Balloon::getBaseHeight()
{
    return position.y - 6.25f;
}

///////////////////////////////////////////////////////////////////////////////
//  drawBalloon - draws the balloon.

void Balloon::drawBalloon()
{
    // Specify material parameters for the balloon
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, balloon_ambient);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, balloon_specular);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, balloon_diffuse);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, balloon_shininess);

    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);

    // Set up a Quadratic Object
    GLUquadricObj *qobj = gluNewQuadric();
    gluQuadricDrawStyle(qobj,GLU_FILL);
    gluQuadricTexture(qobj, GL_TRUE);
    gluQuadricNormals(qobj,GLU_SMOOTH);

    // Set the color of the balloon
    //glColor3f(0.125, 0.80, 0.125);
    glColor3f(0.8, 0.8, 0.8);

    // Draw the Balloon
    glBindTexture(GL_TEXTURE_2D, balloon_tex[0]);

    glPushMatrix();
    glTranslatef(position.x, position.y, position.z);
    
    glPushMatrix();
    glScalef(0.9,1.0,0.9);
    gluSphere(qobj, 3.0, 20, 20);
    glPopMatrix();

    glTranslatef(0.0,-4.0,0.0);

    // Draw bottom part of the Balloon
    glPushMatrix();
    glRotatef(-90.0,1.0,0.0,0.0);
    gluCylinder(qobj,0.5,1.99,2.0,32,10);
    glPopMatrix();

    // Draw the Ropes
    glBindTexture(GL_TEXTURE_2D, balloon_tex[1]);

    glPushMatrix();
    glTranslatef(0.35,0.0,0.35);
    glRotatef(80.0,1.0,0.0,0.0);
    glRotatef(10.0,0.0,1.0,0.0);
    gluCylinder(qobj,0.05,0.05,1.3,32,10);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.35,0.0,0.35);
    glRotatef(80.0,1.0,0.0,0.0);
    glRotatef(-10.0,0.0,1.0,0.0);
    gluCylinder(qobj,0.05,0.05,1.3,32,10);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.35,0.0,-0.35);
    glRotatef(100.0,1.0,0.0,0.0);
    glRotatef(-10.0,0.0,1.0,0.0);
    gluCylinder(qobj,0.05,0.05,1.3,32,10);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.35,0.0,-0.35);
    glRotatef(100.0,1.0,0.0,0.0);
    glRotatef(10.0,0.0,1.0,0.0);
    gluCylinder(qobj,0.05,0.05,1.3,32,10);
    glPopMatrix();

    glTranslatef(0.0,-2.25,0.0);

    // Draw Basket
    glBindTexture(GL_TEXTURE_2D, balloon_tex[2]);
    glPushMatrix();
    glRotatef(-90.0,1.0,0.0,0.0);
    gluCylinder(qobj,0.6,0.85,1.0,32,10);
    gluDisk(qobj, 0.0, 0.6, 32, 1);
    glPopMatrix();

    glPopMatrix();

    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glBindTexture(GL_TEXTURE_2D, 0);
}
