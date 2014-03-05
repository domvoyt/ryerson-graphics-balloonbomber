#include "a3.h"
#include "mesh.h"
#include "balloon.h"

// Program constants (can be modified to adjust a few default properties)
#define PI 3.14159265358979323846 // Math Constant PI 

#define CAMERA_RADIUS 64.0f                 // The default camera distance from origin 
#define CAMERA_LOOKAT 0.0f,0.0f,0.0f        // The default camera look-at point

#define NUM_TARGETS 10     // The number of targets to shoot


// Basic Function Definitions
void init(int w, int h);
void reshape(int w, int h);

// Display Functions
void display();
void drawBomb();
void drawTargets();

// Target Functions
void moveTargets(int);
void findNearbyTargets();
void checkCollisions();

// Event Handlers Function Definitions
//void mouseButtonHandler(int button, int state, int x, int y);
//void mouseMotionHandler(int x, int y);
void keyboardHandler(unsigned char key, int x, int y);
void specialKeyHandler(int key, int x, int y);


// Lighting Properties
GLfloat light_position0[] = {50.0,20.0,20.0,1.0};
GLfloat light_position1[] = {0.0,0.0,0.0,1.0};
GLfloat light_diffuse[]   = {1.0, 1.0, 1.0, 1.0};
GLfloat light_specular[]  = {0.0, 0.0, 0.0, 1.0};

// Global Variables
Mesh mesh;
Balloon balloon;
Target *targets;

int targetsLeft;

// Constants
const float target_size = 1.0f;
const float bomb_radius = 0.5f;

// Texture Mapping
RGBpixmap pix1[10];
GLuint textureId;

// Bomb Properties
VECTOR3D bombPosition;
bool activeBomb;
std::vector<int> nearbyTargets;

// Camera Properties
VECTOR3D cameraPos;
float cameraRadius;
int cameraInclination;
int cameraAzimuth;

// Flags
bool wireframe;
bool texture;
bool viewTargets;
bool balloonCamera;


int main(int argc, char **argv)
{
    glutInit(&argc,argv); 
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH); 

    GLint gsw = glutGet(GLUT_SCREEN_WIDTH);
    GLint gsh = glutGet(GLUT_SCREEN_HEIGHT);
    GLint windowSize = gsh-100;

    glutInitWindowPosition((gsw-windowSize)/2, 20);
    glutInitWindowSize(windowSize, windowSize);
    glutCreateWindow ("CPS511 A3: Hot-Air Balloon Bomber"); 

    glutDisplayFunc(display);
    //glutMouseFunc(mouseButtonHandler);
    //glutMotionFunc(mouseMotionHandler);
    glutKeyboardFunc(keyboardHandler);
    glutSpecialFunc(specialKeyHandler);

    init(windowSize, windowSize); // Calls the init function
    glutMainLoop(); // Enters the GLUT event processing loop.

    return 0;
}


/////////////////////////////////////////////////////////////////////////////////////
//       The init function - used to initialize program properties and variables

void init(int w, int h) 
{ 
    glClearColor(0.75, 0.9, 0.9, 1.0);

    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70.0,1.0,0.1,1000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Setup lighting properties
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
    glShadeModel(GL_SMOOTH);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_COLOR_MATERIAL);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    // Initialize Globals
    cameraRadius = CAMERA_RADIUS;
    cameraPos = VECTOR3D(0.0f, 0.0f, 0.0f);
    cameraAzimuth = 60;
    cameraInclination = 45;

    // Initialize Flags
    wireframe = false;
    texture = true;
    balloonCamera = false;
    activeBomb = false;

    // Initialize Objects
    mesh.initMesh();
    balloon.initBalloon(mesh.getMaxHeight());

    // Initialize targets
    srand( time(NULL) );

    targets = new Target[NUM_TARGETS];
    targetsLeft = NUM_TARGETS;

    for(int i = 0; i < NUM_TARGETS; ++i)
    {
        Target t;
        t.position = mesh.getRandomVertex();
        t.size = target_size;
        t.meshHeight = t.position.y - t.size*2;
        t.moving = false;
        t.hit = false;
        t.ticksBeforeMoving = rand() % 120 +1;

        targets[i] = t;
    }

    cout << "There are " << NUM_TARGETS << " targets. Shoot them down!" << "\n";
    glutTimerFunc(25, moveTargets, 0);
} 

/////////////////////////////////////////////////////////////////////////////////////
//       Called at initialization and whenever user resizes the window 

void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(70.0,1.0,0.1,1000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/////////////////////////////////////////////////////////////////////////////////////
//       The display callback function - called by OpenGL

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Set up the Viewing Transformation (V matrix)
    if(balloonCamera)
    {
        gluLookAt(balloon.position.x, balloon.getBaseHeight(), balloon.position.z, 
                  balloon.position.x, -1000.0f, balloon.position.z-0.000001f, 0.0,1.0,0.0);
    }
    else
    {
        cameraPos.SetX(cameraRadius * sin(cameraInclination*PI/180) * cos(cameraAzimuth*PI/180));
        cameraPos.SetY(cameraRadius * cos(cameraInclination*PI/180));
        cameraPos.SetZ(cameraRadius * sin(cameraInclination*PI/180) * sin(cameraAzimuth*PI/180));
        
        gluLookAt(cameraPos.GetX(),cameraPos.GetY(),cameraPos.GetZ(), CAMERA_LOOKAT, 0.0,1.0,0.0);
    }
  
    // Set polygon rasterization mode (wireframe or fill)
    if(wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Call draw functions/methods
    mesh.drawMesh();
    balloon.drawBalloon();

    drawBomb();
    drawTargets();

    glutSwapBuffers();
}

/////////////////////////////////////////////////////////////////////////////////////
//       drawBomb - draws the falling bomb

void drawBomb()
{
    if(activeBomb)
    {
        // Set the color of the bomb
        glColor3f(0.1, 0.1, 0.1);
        
        // Draw the bomb
        glPushMatrix();
        glTranslatef(bombPosition.x, bombPosition.y, bombPosition.z);
        glutSolidSphere(bomb_radius,8,8);
        glPopMatrix();

        // Change bomb height
        if(bombPosition.y <= 0.0f)
        {
            activeBomb = false;
            nearbyTargets.clear();
        }
        else
        {
            bombPosition.y -= 0.1f;

            // if there are nearby targets, check for collisions
            if(!nearbyTargets.empty())
                checkCollisions();
        }

        glutPostRedisplay();
    }
}

/////////////////////////////////////////////////////////////////////////////////////
//       drawTargets - draws the moving targets

void drawTargets()
{
    glColor3f(1.0, 0.1, 0.1);

    for(int i = 0; i < NUM_TARGETS; ++i)
    {
        Target t = targets[i];

        if((t.moving) || (viewTargets && !t.hit))
        {
            glPushMatrix();
            glTranslatef(t.position.x, t.position.y, t.position.z);
            glutSolidCube(target_size);
            glPopMatrix();
        }
    }
}

/**************************************************************************************
 **     Target Functions
 **
 **************************************************************************************/

/////////////////////////////////////////////////////////////////////////////////////
//       moveTargets - animates the targets

void moveTargets(int)
{
    for(int i = 0; i < NUM_TARGETS; ++i)
    {
        Target *t = &targets[i];
        
        if(!t->hit && !viewTargets)
        {
            // If currently moving
            if(t->moving)
            {
                // Moving Up
                if(t->delta > 0.0 && t->position.y < (t->meshHeight + t->maxHeight))
                {
                    t->position.y += t->delta;
                }
                // Max Height Reached
                else if(t->delta > 0.0)
                {
                    t->delta *= -1.0;
                }
                // Moving Down
                else if(t->position.y > (t->meshHeight-(t->size*2)))
                {
                    t->position.y += t->delta;
                }
                // Done moving
                else
                {
                    t->moving = false;
                    t->ticksBeforeMoving = rand() % 400 +1;
                }
            }
            // if currently waiting to move
            else
            {
                t->ticksBeforeMoving--;

                if(t->ticksBeforeMoving == 0)
                {
                    t->moving = true;
                    t->position.y = t->meshHeight - t->size;
                    t->maxHeight = rand() % 10 +1.0;
                    t->delta = 0.05f;
                }
            }
        }
    }

    glutTimerFunc(25, moveTargets, 0);
    glutPostRedisplay();
}

/////////////////////////////////////////////////////////////////////////////////////
//       findNearbyTargets - finds all the targets that might be hit by the falling bomb

void findNearbyTargets()
{
    for(int i = 0; i < NUM_TARGETS; ++i)
    {
        Target t = targets[i];

        if(!t.hit)
        {
            // Compute distance vector between target and bomb
            VECTOR3D v = t.position-bombPosition;

            // Check if the target might be hit by bomb
            if(abs(v.x) < 1.0f && abs(v.z) < 1.0f)
            {
                nearbyTargets.push_back(i);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////
//       targetHit - checks for collisions between the bomb and nearby targets

void checkCollisions()
{
    bool collision = false;

    for(int i = 0; i < nearbyTargets.size(); ++i)
    {
        Target *t = &targets[ nearbyTargets[i] ];
        VECTOR3D v = (*t).position-bombPosition;

        bool isAboveGround = ( (t->position.y) > (t->meshHeight-(t->size/2)) );

        // If target is above ground and the bomb is touching it, record collision
        if(isAboveGround && abs(v.x) < 1.0f && abs(v.y) < 1.0f && abs(v.z) < 1.0f)
        {
            t->hit = true;
            t->moving = false;

            collision = true;
            targetsLeft -= 1;
            cout << "Target Hit! " << targetsLeft << " left.\n";
        }
    }

    // Clear the nearby targets array if collision was detected
    if(collision)
    {
        nearbyTargets.clear();
        bombPosition.y = 0.0f;     
    }

    // Check if there are targets left
    if(targetsLeft == 0)
    {
        cout << "All the targets are down! You win!\n";
    }
}


/**************************************************************************************
 **     Event Handler Functions
 **
 **************************************************************************************/


////////////////////////////////////////////////////////////////////////////////////////
//   keyboard callback function: this function is called by OpenGL when the user 
//                               presses a key on the keyboard

void keyboardHandler(unsigned char key, int a, int b)
{
    switch(key)
    {
        // Quit Program: 'Esc'
        case 27:  
            exit(0);
            break;

        // Drop Bomb
        case ' ':
            if(!activeBomb)
            {
                activeBomb = true;
                bombPosition = balloon.position;
                bombPosition.y = balloon.getBaseHeight();
                findNearbyTargets();
            }
            break;

        // Camera Move Up 
        case 'w':
        case 'W':
            if(cameraInclination > 5)
                cameraInclination = (cameraInclination - 5);
            break;

        // Camera Move Down
        case 's':
        case 'S':
            if(cameraInclination < 175)
                cameraInclination = (cameraInclination + 5);
            break;

        // Camera Move Clockwise
        case 'a':
        case 'A':
            cameraAzimuth = (cameraAzimuth + 5) %360;
            break;

        // Camera Move Anti-Clockwise
        case 'd':
        case 'D':
            cameraAzimuth = (cameraAzimuth - 5) %360;
            break;

        // Camera Zoom Out
        case 'q':
        case 'Q':
            cameraRadius += 0.5f;
            break;

        // Camera Zoom In
        case 'e':
        case 'E':
            cameraRadius -= 0.5f;
            break;

        // Print the properties of all existing blobs
        case 'b':
        case 'B':
            mesh.printBlobs();
            break;
    }

    glutPostRedisplay();
}

////////////////////////////////////////////////////////////////////////////////////////
//   special key callback function: this function is called by OpenGL when the user 
//                                  presses a special key on the keyboard (ie. Function keys)

void specialKeyHandler(int key, int x, int y)
{
    switch(key)
    {
        // Move balloon in the negative z direction
        case GLUT_KEY_UP:
            balloon.position.z -= 0.125f;
            break;

        // Move balloon in the positive z direction
        case GLUT_KEY_DOWN:
            balloon.position.z += 0.125f;
            break;

        // Move balloon in the negative x direction
        case GLUT_KEY_LEFT:
            balloon.position.x -= 0.125f;
            break;

        // Move balloon in the positive x direction
        case GLUT_KEY_RIGHT:
            balloon.position.x += 0.125f;
            break;

        // Toggle camera mode
        case GLUT_KEY_F1:
            balloonCamera = !balloonCamera;
            break;

        // Toggle rendering mode
        case GLUT_KEY_F5:
            wireframe = !wireframe;
            break;

        // Toggle view targets mode
        case GLUT_KEY_F6:
            viewTargets = !viewTargets;
            break;
	}

    glutPostRedisplay();
}
