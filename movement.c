#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "movement.h"

//how long a step should be
#define STEP (Vec2){1.0, 0.0}

#define PI 3.14159265358979323846
//How fast character rotates
#define ROTSPEED PI/120

/*TODO: 
Stop at walls
Add rotation by mouse (?)
*/


//returns a vec2 rotated by rad radians
Vec2 rotate(Vec2 step, float rad) {
    float x = cosf(rad)*step.x-sinf(rad)*step.y;
    float y = sinf(rad)*step.x+cosf(rad)*step.y;
    return (Vec2){x, y};
};

Vec2 moveForward(Vec2 pos, float rad) {
    Vec2 dir = VECINIT;
    dir = rotate(STEP, rad);
    float x = pos.x += dir.x;
    float y = pos.y += dir.y;
    return (Vec2){x, y};
};
Vec2 moveRight(Vec2 pos, float rad) {
    Vec2 dir = VECINIT;
    dir = rotate(STEP, rad+PI/2);
    float x = pos.x += dir.x;
    float y = pos.y += dir.y;
    return (Vec2){x, y};
};
Vec2 moveLeft(Vec2 pos, float rad) {
    Vec2 dir = VECINIT;
    dir = rotate(STEP, rad-PI/2);
    float x = pos.x += dir.x;
    float y = pos.y += dir.y;
    return (Vec2){x, y};
};
Vec2 moveBack(Vec2 pos, float rad) {
    Vec2 dir = VECINIT;
    dir = rotate(STEP, rad+PI);
    float x = pos.x += dir.x;
    float y = pos.y += dir.y;
    return (Vec2){x, y};
};

float rotateRight(float angle) {
    return angle += ROTSPEED;
};
float rotateLeft(float angle) {
    return angle -= ROTSPEED;
}
