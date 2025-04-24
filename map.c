#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "raylib.h"
#include "map.h"
#include "movement.h"
#include "raycast.h"

int inFieldOfView(Vec2 playerpos, Vec2 playerdir, float fov, Enemy foe1)
{
    Vec2 toEnemy;
    vectorSub(foe1.pos, playerpos, &toEnemy);

    float distance = vectorLenght(toEnemy);
    if (distance < 0.001f)
        return 1; // Player is on top of the enemy — it's visible

    // Angle offset from enemy center to its edge (projected angle)
    float spriteHalfAngle = atanf(foe1.sprite.width / (2.0f * distance));

    // Normalize direction to enemy
    Vec2 toEnemyNorm = toEnemy;
    normalize(&toEnemyNorm);

    // Angle between player direction and vector to enemy center
    float centerDot = vectorDot(toEnemyNorm, playerdir);
    float angleToCenter = acosf(CLAMP(centerDot, -1.0f, 1.0f)); // Ensure safe acos

    // Check if **any** part of the sprite is within the FOV
    return angleToCenter - spriteHalfAngle <= DEG_TO_RAD(fov / 2.0f);
}

CollisionData **rayShotEnemies(Player p1, float fov, Map *mp, Enemy *enemies, int ec)
{
    CollisionData **result = malloc(sizeof(CollisionData *) * ec);

    for (int i = 0; i < ec; i++)
    {
        result[i] = NULL;
        if (!inFieldOfView(p1.pos, p1.dir, fov, enemies[i]) || (enemies[i].visibility == INVISIBLE) || (enemies[i].status == DEAD)) // checks if enemy is outside of fov
            continue;

        Vec2 diffvec;
        vectorSub(enemies[i].pos, p1.pos, &diffvec);
        float diff = vectorLenght(diffvec);
        normalize(&diffvec);

        int fl = 1;
        for (int j = 0; j < mp->numOfWalls; j++)
        {
            CollisionData *temp = checkCollision(mp->walls[j], (Ray3D){p1.pos, diffvec}); // checks if a wall is in the way of the enemy
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
        result[i]->angle = vectorDot(p1.dir, diffvec);
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

void updateEnemy(Enemy *foe, Player p1, int *playerHealth, int targetFPS, float fov, Map *mp)
{

    if (foe->hp <= 0)
        foe->status = DEAD;
    if (foe->status == DEAD)
        return;

    Vec2 dir;
    vectorSub(p1.pos, foe->pos, &dir);

    if (vectorLenght(dir) <= foe->attackRadius)
    {
        foe->velocity = VECINIT;
        *playerHealth -= 1;
        return;
    }

    CollisionData **seePLayer = rayShotEnemies(p1, fov, mp, foe, 1);

    switch (seePLayer[0] == NULL)
    {
    case 0:

        normalize(&dir);
        foe->dir = dir;
        moveEnemy(foe, foe->dir, targetFPS);
        break;

    case 1:
        int choice = rand() % 4;
        switch (choice)
        {
        case 0:
            moveEnemy(foe, foe->dir, targetFPS);
            break;
        case 1:
            moveEnemy(foe, foe->dir, targetFPS);
            break;
        case 2:
            rotate(&foe->dir, DEG_TO_RAD(10.0));
            break;
        case 3:
            rotate(&foe->dir, DEG_TO_RAD(-10.0));
            break;
        default:
            break;
        }
        break;
    }

    freeCollisionData(seePLayer, 1);
}

void updateEnemies(Enemy *Queue, int qSize, Player *p1, int targetFPS, float fov, Map *mp)
{
    static int currentIndex = 0;

    if (qSize == 0)
        return;

    updateEnemy(Queue + currentIndex, *p1, &p1->hp, targetFPS, fov, mp);
    currentIndex = (currentIndex + 1) % qSize;
}

FILE *newMap(const char *filename)
{
    return fopen(filename, "w");
}

int addShape(FILE *map, Vec2 *corners, const char *texture, int cornercount, int closed)
{
    if (!map)
        return 0;
    for (int i = 0; i < (cornercount - 1); i++)
    {
        fprintf(map, "%f,%f,%f,%f,%s\n", corners[i].x, corners[i].y, corners[i + 1].x, corners[i + 1].y, texture);
    }
    if (closed)
    {
        fprintf(map, "%f,%f,%f,%f,%s\n", corners[cornercount - 1].x, corners[cornercount - 1].y, corners[0].x, corners[0].y, texture);
    }
    return 1;
}

int addEnemy(FILE *map, Vec2 pos, int id, float acceleration, float maxSpeed, const char *sprite)
{

    if (!map)
        return 0;
    fprintf(map, "%f,%f,%d,%f,%f,%s\n", pos.x, pos.y, id, acceleration, maxSpeed, sprite);
    return 1;
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

    char buffer[128];
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
    if (nenemy)
    {
        result->enemies = malloc(sizeof(Enemy) * nenemy);
        if (!result->enemies)
        {
            printf("Malloc error");
            fclose(mfile);
            free(result->walls);
            free(result);
            return NULL;
        }
    }
    else
    {
        result->enemies = NULL;
    }

    for (int i = 0; i < nwalls && fgets(buffer, sizeof(buffer), mfile); i++)
    {
        char textbuff[64];

        sscanf(buffer, "%f,%f,%f,%f,%63s", &result->walls[i].start.x, &result->walls[i].start.y, &result->walls[i].stop.x, &result->walls[i].stop.y, textbuff);
        result->walls[i].texture = LoadTexture(textbuff);
        if (result->walls[i].texture.id == 0)
        {
            printf("Failed to load texture %s \n", textbuff);
        }
    }
    for (int i = 0; i < nenemy && fgets(buffer, sizeof(buffer), mfile) && nenemy; i++)
    {
        char textbuff[64];
        sscanf(buffer, "%f,%f,%d,%f,%f,%63s", &result->enemies[i].pos.x, &result->enemies[i].pos.y, &result->enemies[i].id, &result->enemies[i].acceleration, &result->enemies[i].maxSpeed, textbuff);
        result->enemies[i].sprite = LoadTexture(textbuff);
        if (result->enemies[i].sprite.id == 0)
        {
            printf("Failed to load texture %s \n", textbuff);
        }
        result->enemies[i].attackRadius = 50.0f;
        result->enemies[i].dir = (Vec2){1.0, 0.0};
        result->enemies[i].hitRadius = 50.0f;
        result->enemies[i].hp = 100;
        result->enemies[i].status = ALIVE;
        result->enemies[i].velocity = VECINIT;
        result->enemies[i].visibility = VISIBLE;
    }

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