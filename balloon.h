#ifndef BALLOON_H
#define BALLOON_H

#include "a3.h"


class Balloon
{
    public:
        
        VECTOR3D position;
        float baseHeight;

        Balloon() { }

        void initBalloon(float);
        void drawBalloon();
        float getBaseHeight();
};

#endif