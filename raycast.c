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

void rotate(Vec2 *v1, float rad)
{
    float old_x = v1->x;
    float old_y = v1->y;

    v1->x = old_x * cosf(rad) - old_y * sinf(rad);
    v1->y = old_x * sinf(rad) + old_y * cosf(rad);

    normalize(v1);
}

CollisionData *checkCollision(Wall w1, Ray r1)
{
    Vec2 odelta = VECINIT;
    Vec2 result = VECINIT;
    Vec2 wdir = VECINIT;

    normalize(&r1.dir);
    vectorSub(w1.stop, w1.start, &wdir);
    vectorSub(w1.start, r1.start, &odelta);

    if (!solveSystem(r1.dir, wdir, odelta, &result))
    {
        return NULL;
    }
    if (result.x < 0.0 || result.y > 0.0 || result.y < -1.0)
    {
        return NULL;
    }

    CollisionData *data = malloc(sizeof(CollisionData));
    data->d = result.x;

    Vec2 temp = VECINIT;
    vectorScale(r1.dir, result.x, &temp);
    vectorAdd(r1.start, temp, &data->position);

    return data;
}
int solveSystem(Vec2 v1, Vec2 v2, Vec2 v3, Vec2 *result)
{
    if (v1.x == 0.0f)
    {
        if (v2.y == 0.0f || v1.y == 0.0f)
        {
            printf("Parallel");
            return 0;
        }

        printf("ZERO DIVISION");
        return 0;
    }
    float denom;
    denom = v2.y - ((v1.y / v1.x) * v2.x);
    // printf("Denom %f ", denom);
    if (denom == 0.0f)
    {
        printf("Parallel");
        return 0;
    }
    float c = (v3.y - (v1.y * (v3.x / v1.x))) / denom;
    // printf("C %f ", c);
    result->y = c;
    result->x = ((v3.x / v1.x) - c * (v2.x / v1.x));
    return 1;
}
}
