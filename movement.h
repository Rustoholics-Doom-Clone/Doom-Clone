#include "raycast.h"

//how long a step should be
#define STEP (Vec2){1.0, 0.0}

#ifndef PI
#define PI 3.14159265358979323846
#endif
//How fast character rotates
#define ROTSPEED PI/120

#ifndef MOVEMENT_H
#define MOVEMENT_H

typedef struct
{
    Vec2 pos;
    Vec2 dir;
} Player;

#define PLAYERINIT (Player){VECINIT, (Vec2){1.0, 0.0}}

//move character forwards, requires current position and angle in radians
void moveForward(Player *player);
//move character right
void moveRight(Player *player);
//move character left
void moveLeft(Player *player);
//move character back
void moveBack(Player *player);
//rotates character right by pi/120 rad
void rotateRight(Player *player);
//rotates character left by pi/120 rad
void rotateLeft(Player *player);

#endif