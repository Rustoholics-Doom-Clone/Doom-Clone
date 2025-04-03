#ifndef VECTOR2_H
#define VECTOR2_H

typedef struct
{
    float x;
    float y;
} Vec2;

// Stores v1-v2 in result
void vectorSub(Vec2 v1, Vec2 v2, Vec2 *result);
// Stores v1 + v2 in result
void vectorAdd(Vec2 v1, Vec2 v2, Vec2 *result);
// Stores the scalar product of v1 * k in result
void vectorScale(Vec2 v1, float k, Vec2 *result);
// returns the dot product of v1 and v2
float vectorDot(Vec2 v1, Vec2 v2);
// normalize vector
void normalize(Vec2 *v1);

// Returns x and y for the equation x*v1 + y*v2 = v3
void solveSystem(Vec2 v1, Vec2 v2, Vec2 v3, Vec2 result);

#endif

#ifndef RAYCAST_H
#define RAYCAST_H

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

#endif