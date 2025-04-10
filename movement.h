#include "raycast.h"

#ifndef MOVEMENT_H
#define MOVEMENT_H


//move character forwards, requires current position and angle in radians
Vec2 moveForward(Vec2 pos, float rad);
//move character right
Vec2 moveRight(Vec2 pos, float rad);
//move character left
Vec2 moveLeft(Vec2 pos, float rad);
//move character back
Vec2 moveBack(Vec2 pos, float rad);
//rotates character right by pi/120 rad
float rotateRight(float angle);
//rotates character left by pi/120 rad
float rotateLeft(float angle);

#endif