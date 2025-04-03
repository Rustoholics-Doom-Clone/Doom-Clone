#ifndef RAYCAST_H
#define RAYCAST_H

typedef struct
{
    float x;
    float y;
} Vec2;

typedef struct
{
    Vec2 position; // where the collision happened
    float d;       // how far away it happened
} CollisionData;

typedef struct
{
    Vec2 start, stop;
} Wall;

typedef struct
{
    Vec2 start, dir;
} Ray;

// Returns info on if and where a ray hits a wall. NULL == Doesn't hit
CollisionData *checkCollision(Wall w1, Ray r1);
// Returns x and y for the equation x*v1 + y*v2 = v3
Vec2 *solveSystem(Vec2 v1, Vec2 v2, Vec2 v3);

#endif