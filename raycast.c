#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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
float vectorDot(Vec2 v1, Vec2 v2)
{
    return (v1.x * v2.x + v1.y * v2.y);
}

float vectorLenght(Vec2 v1)
{
    return sqrtf(vectorDot(v1, v1));
}

void normalize(Vec2 *v1)
{
    vectorScale(*v1, 1 / vectorLenght(*v1), v1);
}

CollisionData *checkCollision(Wall w1, Ray r1)
{
}
void solveSystem(Vec2 v1, Vec2 v2, Vec2 v3, Vec2 *result)
{
    if (v1.x == 0.0f)
    {
        printf("ZERO DIVISION");
        return;
    }
    float vectorLenght(Vec2 v1);
    float denom;
    denom = v2.y - ((v1.y / v1.x) * v2.x);
    printf("Denom %f ", denom);
    if (denom == 0.0f)
    {
        printf("ZERO DIVISION");
        return;
    }
    float c = (v3.y - (v1.y * (v3.x / v1.x))) / denom;
    printf("C %f ", c);
    result->y = c;
    result->x = ((v3.x / v1.x) - c * (v2.x / v1.x));
}
