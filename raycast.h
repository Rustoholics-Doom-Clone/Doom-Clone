#ifndef PICONST
#define PICONST
#endif

#ifndef VECTOR2_H
#define VECTOR2_H

typedef struct
{
    float x;
    float y;
} Vec2;

#define DEG_TO_RAD(deg) ((deg) * (PI / 180.0))

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
// Solves the system [v1,v2]*result = v3 for result. Returns 1 if there is a single solution otherwise 0
int solveSystem(Vec2 v1, Vec2 v2, Vec2 v3, Vec2 *result);

#endif

#ifndef WALLSTRUCT
#define WALLSTRUCT

typedef struct
{
    Vec2 start, stop; // Both ends of the wall
} Wall;

#endif

#ifndef RAYCAST_H
#define RAYCAST_H

typedef struct
{
    Vec2 position; // where the collision happened
    float d;       // how far away it happened
    float angle;   // used when firing multiple rays to determine the angle from the source.
} CollisionData;

typedef struct
{
    Vec2 start, dir; 
} Ray3D;


// Returns info on if and where a ray hits a wall. NULL == Doesn't hit, Remember to free the result
CollisionData *checkCollision(Wall w1, Ray3D r1);
// Shots rn rays accros the fov. Returns a list of collisiondata for the closest collisions of each ray. nw is the number of walls
CollisionData **multiRayShot(Vec2 campos, Vec2 camdir, float fov, int wn, Wall *walls, int rn);
// Frees an array of collisiondata pointers of length n, handles entries that are null as well.
void freeCollisionData(CollisionData **a, int n);
// Takes an array of collisiondata of length n and returns an array of floats of lenght n that is the collision distance scaled.
float *wallHeightArray(CollisionData **a, int n, float fov, int width);
#endif

typedef struct Map Map;
// Works like checkCollision but checks against all walls in m and returns the closest.
CollisionData *mapCollision(Map *m, Ray3D r1);
// Works like multiRayShot but with a map instead of a manual list of walls.
CollisionData **mapMultiRayShot(Ray3D cam, float fov, int rn, Map *m);
