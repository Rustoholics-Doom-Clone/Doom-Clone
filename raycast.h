#ifndef VECTOR2_H
#define VECTOR2_H

typedef struct
{
    float x;
    float y;
} Vec2;

#define VECINIT (Vec2){0.0, 0.0}

// Stores v1-v2 in result
void vectorSub(Vec2 v1, Vec2 v2, Vec2 *result);
// Stores v1 + v2 in result
void vectorAdd(Vec2 v1, Vec2 v2, Vec2 *result);
// Stores the scalar product of v1 * k in result
void vectorScale(Vec2 v1, float k, Vec2 *result);
// returns the dot product of v1 and v2
float vectorDot(Vec2 v1, Vec2 v2);
// returns vectorlenght
float vectorLenght(Vec2 v1);
// normalize vector
void normalize(Vec2 *v1);
// Rotates v1 around the origin counter clockwise looking from positive z
void rotate(Vec2 *v1, float rad);

// Returns x and y for the equation x*v1 + y*v2 = v3. The answer is saved as a vector in result
void solveSystem(Vec2 v1, Vec2 v2, Vec2 v3, Vec2 *result);

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
    Vec2 start, stop; // Both ends of the wall
} Wall;

typedef struct
{
    Vec2 start, dir; // The origin of the ray and the direction. Remember to normalize the direction
} Ray;

// Returns info on if and where a ray hits a wall. NULL == Doesn't hit, Remember to free the result
CollisionData *checkCollision(Wall w1, Ray r1);

#endif