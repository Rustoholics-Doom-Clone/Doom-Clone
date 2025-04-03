#include <stdio.h>
#include <stdlib.h>
#include "raycast.h"

void vectorSub(Vec2 v1, Vec2 v2, Vec2 *result)
{
    result->x = v1.x - v2.x;
    result->y = v1.y - v2.y;
}

void vectorAdd(Vec2 v1, Vec2 v2, Vec2 *result)
{
    result->x = v1.x + v2.x;
    result->y = v1.y + v2.y;
}

void vectorScale(Vec2 v1, float k, Vec2 *result)
{
    result->x = v1.x * k;
    result->y = v1.y * k;
}
float vectorDot(Vec2 v1, Vec2 v2) {}

CollisionData *checkCollision(Wall w1, Ray r1)
{
}
Vec2 *solveSystem(Vec2 v1, Vec2 v2, Vec2 v3)
{
}
