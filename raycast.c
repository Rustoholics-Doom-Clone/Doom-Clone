#include "raylib.h"
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

CollisionData *checkCollision(Wall w1, Ray3D r1)
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
    data->angle = NAN;

    Vec2 temp = VECINIT;
    vectorScale(r1.dir, result.x, &temp);
    vectorAdd(r1.start, temp, &data->position);

    return data;
}
int solveSystem(Vec2 v1, Vec2 v2, Vec2 v3, Vec2 *result)
{

    float det = v1.x * v2.y - v2.x * v1.y;

    if (det == 0.0f)
    {
        printf("Parallel or singular system\n");
        return 0;
    }

    // Cramer's Rule
    result->x = (v3.x * v2.y - v2.x * v3.y) / det;
    result->y = (v1.x * v3.y - v3.x * v1.y) / det;

    return 1;
}

CollisionData **multiRayShot(Vec2 campos, Vec2 camdir, float fov, int wn, Wall *walls, int rn)
{
    float step = fov / (rn - 1);
    float start = (-1.0 * fov) / 2;

    normalize(&camdir);

    CollisionData **result = malloc(rn * sizeof(CollisionData *));
    if (!result)
    {
        printf("Malloc error in multiRayShot");
        return NULL;
    }

    rotate(&camdir, DEG_TO_RAD(start));
    for (int i = 0; i < rn; i++)
    {
        result[i] = NULL; // Initialize to NULL

        for (int j = 0; j < wn; j++)
        {
            CollisionData *temp = checkCollision(walls[j], (Ray3D){campos, camdir});
            printf("Shot ray with direction,%f %f\n", camdir.x, camdir.y);
            if (temp && (!result[i] || result[i]->d > temp->d))
            {
                if (!result[i])
                    result[i] = malloc(sizeof(CollisionData));

                if (result[i])
                    *result[i] = *temp;
            }
        }
        result[i]->angle = start - i * step;
        rotate(&camdir, DEG_TO_RAD(step));
    }
    rotate(&camdir, DEG_TO_RAD(start));
    return result;
}

void freeCollisionData(CollisionData **a, int n)
{
    for (int i = 0; i < n; i++)
    {
        if (a[i])
            free(a[i]);
    }
    free(a);
}
