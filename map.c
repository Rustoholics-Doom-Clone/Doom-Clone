#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "raylib.h"
#include "map.h"
#include "movement.h"

int inFieldOfView(Vec2 playerpos, Vec2 playerdir, float fov, Enemy foe1)
{
    Vec2 diffvec;
    vectorSub(foe1.pos, playerpos, &diffvec);
    normalize(&diffvec);
    if (vectorDot(diffvec, playerdir) < cosf(DEG_TO_RAD(fov / 2)))
    {
        return 0;
    }
    return 1;
}

CollisionData **rayShotEnemies(Vec2 playerpos, Vec2 playerdir, float fov, Wall *wls, int wn, Enemy *enemies, int ec)
{
    CollisionData **result = malloc(sizeof(CollisionData *) * ec);

    for (int i = 0; i < ec; i++)
    {
        result[i] = NULL;
        if (!inFieldOfView(playerpos, playerdir, fov, enemies[i]) || (enemies[i].visibility == INVISIBLE) || (enemies[i].status == DEAD)) // checks if enemy is outside of fov
            continue;

        Vec2 diffvec;
        vectorSub(enemies[i].pos, playerpos, &diffvec);
        float diff = vectorLenght(diffvec);
        normalize(&diffvec);

        int fl = 1;
        for (int j = 0; j < wn; j++)
        {
            CollisionData *temp = checkCollision(wls[j], (Ray3D){playerpos, diffvec}); // checks if a wall is in the way of the enemy
            // printf("Shot ray with direction,%f %f\n", camdir.x, camdir.y);
            if (temp && temp->d < diff)
            {
                fl = 0;
                break;
            }
        }
        if (!fl)
            continue;

        result[i] = malloc(sizeof(CollisionData));

        result[i]->d = diff;
        result[i]->position = enemies[i].pos;
        // result[i]->angle = RAD_TO_DEG(acosf(vectorDot(playerdir, diffvec)));
        result[i]->angle = vectorDot(playerdir, diffvec);
        result[i]->texture = enemies[i].sprite;
    }
    return result;
}

void moveEnemy(Enemy *foe, Vec2 dir, int targetFPS)
{
    // Apply acceleration
    Vec2 acc;
    vectorScale(dir, foe->acceleration / (float)targetFPS, &acc);
    vectorAdd(acc, foe->velocity, &foe->velocity);

    // Cap speed
    float speed = vectorLenght(foe->velocity);
    if (speed > foe->maxSpeed)
        vectorScale(foe->velocity, foe->maxSpeed / speed, &foe->velocity);

    // Apply friction
    float friction = 0.95f; // Tweak to taste
    float frictionPerFrame = powf(friction, 60.0f / (float)targetFPS);
    vectorScale(foe->velocity, frictionPerFrame, &foe->velocity);

    // Move position
    Vec2 ds;
    vectorScale(foe->velocity, 1.0f / (float)targetFPS, &ds);
    vectorAdd(ds, foe->pos, &foe->pos);
}

void updateEnemy(Enemy *foe, Vec2 playerPos, Vec2 playerdir, int *playerHealth, int targetFPS, float fov, Wall *wls, int wn)
{

    if (foe->hp <= 0)
        foe->status = DEAD;
    if (foe->status == DEAD)
        return;

    CollisionData **seePLayer = rayShotEnemies(playerPos, playerdir, fov, wls, wn, foe, 1);

    switch (seePLayer[0] == NULL)
    {
    case 0:
        Vec2 dir;
        vectorSub(playerPos, foe->pos, &dir);
        normalize(&dir);
        moveEnemy(foe, dir, targetFPS);
    case 1:
    }

    freeCollisionData(seePLayer, 1);
}

void updateEnemies(Enemy *Queue, int qSize, Player p1, int targetFPS, float fov, Map mp)
{
    static int currentIndex = 0;

    if (qSize == 0)
        return;

    updateEnemy(Queue + currentIndex, p1.pos, p1.dir, &p1.hp, targetFPS, fov, mp.walls, mp.numOfWalls);
    currentIndex = (currentIndex + 1) % qSize;
}

int saveMap(int numOfWalls, Wall *walls, char *filename)
{
    FILE *mfile = fopen(filename, "w");
    if (!mfile)
        return 0;
    fprintf(mfile, "%d,0\n", numOfWalls);
    for (int i = 0; i < numOfWalls; i++)
        fprintf(mfile, "%f,%f,%f,%f\n", walls[i].start.x, walls[i].start.y, walls[i].stop.x, walls[i].stop.y);
    fclose(mfile);
    return 1;
}

Map *loadMap(char *filename)
{
    FILE *mfile = fopen(filename, "r");
    if (!mfile)
    {
        printf("Could not open file");
        return NULL;
    }
    Map *result = malloc(sizeof(Map));
    if (!result)
    {
        printf("Malloc error");
        return NULL;
    }

    char buffer[50];
    if (!fgets(buffer, sizeof(buffer), mfile))
    {
        printf("Could not read format of file");
        fclose(mfile);
        free(result);
        return NULL;
    }
    int nwalls, nenemy;
    sscanf(buffer, "%d,%d", &nwalls, &nenemy);
    result->numOfWalls = nwalls;
    result->enemyCount = nenemy;

    result->walls = malloc(sizeof(Wall) * nwalls);
    if (!result->walls)
    {
        printf("Malloc error");
        fclose(mfile);
        free(result);
        return NULL;
    }
    // Enemies here. TODO!
    result->enemies = NULL;

    for (int i = 0; i < nwalls && fgets(buffer, sizeof(buffer), mfile); i++)
        sscanf(buffer, "%f,%f,%f,%f", &result->walls[i].start.x, &result->walls[i].start.y, &result->walls[i].stop.x, &result->walls[i].stop.y);

    fclose(mfile);
    return result;
}

void freeMap(Map *m)
{
    if (m->walls)
        free(m->walls);
    if (m->enemies)
        free(m->enemies);
    free(m);
}