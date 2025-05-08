#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "raycast.h"
#include "map.h"

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
    vectorSub(w1.stop, w1.start, &wdir);    // get wall direction vector
    vectorSub(w1.start, r1.start, &odelta); // get difference between ray start and wall start

    if (!solveSystem(r1.dir, wdir, odelta, &result)) // Solve for a collision between the walls direction and the ray
    {
        return NULL;
    }
    if (result.x < 0.0 || result.y > 0.0 || result.y < -1.0) // if the wall is in negative ray direction or the collision is outside of the walls actual lenght
    {
        return NULL;
    }

    // Create colliisonData
    CollisionData *data = malloc(sizeof(CollisionData));
    data->d = result.x;         // Distance of collision
    data->angle = NAN;          // Not applicable in the single ray case
    data->texture = w1.texture; // Get texture now since it would be impossible to do later

    // Get the position of the collision
    Vec2 temp = VECINIT;
    vectorScale(r1.dir, result.x, &temp);
    vectorAdd(r1.start, temp, &data->position);

    // Calculate the texture offset and repeating
    Vec2 wallVec;
    vectorSub(w1.stop, w1.start, &wallVec);
    float wallLength = vectorLenght(wallVec);

    Vec2 hitVec;
    vectorSub(data->position, w1.start, &hitVec);

    float hitDist = vectorDot(hitVec, wallVec) / wallLength;

    float textureOffset = hitDist / wallLength;

    float repeats = wallLength / TILE_SIZE;
    textureOffset *= repeats;
    textureOffset = fmodf(textureOffset, 1.0f);
    if (textureOffset < 0)
        textureOffset += 1.0f;

    data->textureOffset = textureOffset;

    return data;
}
int solveSystem(Vec2 v1, Vec2 v2, Vec2 v3, Vec2 *result)
{

    float det = v1.x * v2.y - v2.x * v1.y;

    if (det == 0.0f)
    {
        return 0;
    }

    // Cramer's Rule for a system with two unknowns
    result->x = (v3.x * v2.y - v2.x * v3.y) / det;
    result->y = (v1.x * v3.y - v3.x * v1.y) / det;

    return 1;
}

CollisionData **multiRayShot(Vec2 campos, Vec2 camdir, float fov, int wn, Wall *walls, int rn)
{
    float step = fov / (rn - 1);    // Devide the FOV into equal slices
    float start = (-1.0 * fov) / 2; // Start at one side of the FOV

    normalize(&camdir);

    CollisionData **result = malloc(rn * sizeof(CollisionData *));
    if (!result)
    {
        printf("Malloc error in multiRayShot");
        return NULL;
    }

    rotate(&camdir, DEG_TO_RAD(start)); // Rotate to start
    for (int i = 0; i < rn; i++)        // Every ray
    {
        result[i] = NULL; // Initialize to NULL

        for (int j = 0; j < wn; j++) // Every Wall
        {
            CollisionData *temp = checkCollision(walls[j], (Ray3D){campos, camdir}); // See if ray collides with wall
            if (temp && (!result[i] || result[i]->d > temp->d))                      // If there is a valid result and either there is no valid result yet or the distance is shorter
            {
                if (!result[i]) // If there is no result yet
                    result[i] = malloc(sizeof(CollisionData));

                if (result[i])          // If the result is allocated
                    *result[i] = *temp; // save the data
            }
        }
        if (result[i]) // If data was found
        {
            result[i]->angle = start + i * step; // Give the data an angle relative to the player direction
            result[i]->id = i;                   // Give the data an id
        }
        rotate(&camdir, DEG_TO_RAD(step)); // Rotate camera
    }
    rotate(&camdir, DEG_TO_RAD(start)); // Rotate camera back to original direction
    return result;
}

void freeCollisionData(CollisionData **a, int n)
{
    if (a)
    {
        for (int i = 0; i < n; i++)
        {
            if (a[i])
                free(a[i]);
        }
        free(a);
    }
}
