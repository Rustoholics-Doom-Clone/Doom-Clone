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

#endif