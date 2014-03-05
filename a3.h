#ifndef A3_H
#define A3_H

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <iostream>
#include <array>
#include <cmath>
#include <vector>

#include "VECTOR3D.h"
#include "RGBpixmap.h"

using namespace std;

typedef struct Target
{
    VECTOR3D position;
    float meshHeight;

    bool hit;
    bool moving;

    int ticksBeforeMoving;
    float maxHeight;
    float size;
    float delta;
} Target;

#endif