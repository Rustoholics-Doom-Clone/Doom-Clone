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
    CollisionData **result = malloc(sizeof(CollisionData *) * ec); // allocate memory for the data

    for (int i = 0; i < ec; i++)
    {
        result[i] = NULL;
        if (!inFieldOfView(p1.pos, p1.dir, fov, enemies[i]) || (enemies[i].visibility == INVISIBLE)) // checks if enemy is outside of fov or dead or invisible
            continue;

        // make a normalized vector pointing towards the enemy from the player
        Vec2 diffvec;
        vectorSub(enemies[i].pos, p1.pos, &diffvec);
        float diff = vectorLenght(diffvec);
        normalize(&diffvec);

        result[i] = malloc(sizeof(CollisionData)); // allocate memory for this collision

        result[i]->d = diff;
        result[i]->position = enemies[i].pos;
        result[i]->angle = vectorDot(p1.dir, diffvec); // well be using the cos of the angle later and since both of the vectors are normalized this is the cos of the angle
        result[i]->texture = enemies[i].sprite;
        result[i]->textureOffset = NAN;
    }
    return result;
}

CollisionData **rayShotPlayer(Enemy foe, Player p1, Map *mp)
{
    CollisionData **result = malloc(sizeof(CollisionData *)); // allocate memory for the data

    result[0] = NULL;

    // make a normalized vector pointing towards the enemy from the player
    Vec2 diffvec;
    vectorSub(foe.pos, p1.pos, &diffvec);
    float diff = vectorLenght(diffvec);
    normalize(&diffvec);

    int fl = 1; // this is a flag to see wether or not there was a wall closer to the player that the enemy
    for (int j = 0; j < mp->numOfWalls; j++)
    {
        CollisionData *temp = checkCollision(mp->walls[j], (Ray3D){p1.pos, diffvec}); // checks if a wall is in the way of the enemy
        // printf("Shot ray with direction,%f %f\n", camdir.x, camdir.y);
        if (temp && temp->d < diff) // If there is a collision with a wall and the collision is closer than the distance between the player and enemy
        {
            fl = 0; // flag is false
            break;  // break loop early
        }
    }
    if (!fl)
    {
        free(result);
        return NULL;
    }

    result[0] = malloc(sizeof(CollisionData)); // allocate memory for this collision

    result[0]->d = diff;
    result[0]->position = foe.pos;
    result[0]->angle = RAD_TO_DEG(acosf(vectorDot(p1.dir, diffvec)));
    result[0]->texture = foe.sprite;

    return result;
}

void moveEnemy(Enemy *foe, Vec2 dir, int targetFPS, Wall *walls, int wallcount)
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
    Vec2 res;
    vectorScale(foe->velocity, 1.0f / (float)targetFPS, &ds);
    vectorAdd(ds, foe->pos, &res);

    Vec2 old_pos = foe->pos;
    int i = 0;
    while (i < wallcount)
    {
        if (intersect(old_pos, res, walls[i].start, walls[i].stop))
        {
            Vec2 pos_res = VECINIT;
            Vec2 wall_res = VECINIT;
            Vec2 new_pos = VECINIT;
            vectorSub(old_pos, res, &pos_res);
            vectorSub(walls[i].start, walls[i].stop, &wall_res);
            float dot = vectorDot(pos_res, wall_res);
            float len = vectorLenght(wall_res);
            float k = dot / (len * len);
            vectorScale(wall_res, k, &new_pos);
            if (vectorLenght(new_pos) < 0.5)
            {
                res = old_pos;
                break;
            }
            vectorSub(old_pos, new_pos, &res);
            i = 0;
        }
        else
        {
            i += 1;
        }
    }
    foe->pos = res;
}

void updateEnemy(Enemy *foe, Player p1, int *playerHealth, int *k_pistAmmo, int *pieAmmo, int targetFPS, float fov, Map *mp, int numOfEnemy, Wall *walls, int wallcount)
{
    if (foe->status == DEAD)
        return;
    if (foe->hp <= 0)
    { // Check if enemy is or should be dead
        foe->status = DEAD;
        if (foe->type == 3 | foe->type == 4)
        {
            foe->visibility = INVISIBLE;
            return;
        }
        foe->sprite = LoadTexture("Sprites/Nollekorttransp.png");
        return;
    }

    CollisionData **seePLayer = rayShotPlayer(*foe, p1, mp); // shoots a ray at the player to see if there is line of sight.

    switch (seePLayer == NULL)
    {
    case 0: // If there is line of sight

        Vec2 dir;
        vectorSub(p1.pos, foe->pos, &dir);
        float dist = vectorLenght(dir);
        normalize(&dir);

        if (dist <= foe->attackRadius) // If player is within attackRadius
        {
            foe->velocity = VECINIT; // Stop!
            if (foe->coolDown <= 0)
            {

                switch (foe->type)
                {
                case 0:
                    *playerHealth -= foe->dmg;
                    break;
                case 3:
                    *playerHealth -= foe->dmg;
                    foe->hp = 0;
                    break;
                case 4:
                    *k_pistAmmo += 10;
                    *pieAmmo += 2;
                    foe->hp = 0;
                    break;

                default:
                    shootProjectile(foe->pos, dir, foe->dmg, mp->projectiles, &mp->ppointer, 0);

                    break;
                }

                foe->coolDown = foe->baseCoolDown / numOfEnemy;
            }
            else
            {
                foe->coolDown -= 1;
            }

            break;
            // early return
        }

        foe->dir = dir;                                        // Turn toward player
        moveEnemy(foe, foe->dir, targetFPS, walls, wallcount); // Walk forward
        break;

    case 1:                       // No line of sight
        int choice = rand() % 10; // Roll the dice
        switch (choice)
        {
        case 0:
            moveEnemy(foe, foe->dir, targetFPS, walls, wallcount); // Move forward
            break;
        case 1:
            moveEnemy(foe, foe->dir, targetFPS, walls, wallcount); // Move forward
            break;
        case 2:
            rotate(&foe->dir, DEG_TO_RAD(10.0)); // turn left
            break;
        case 3:
            rotate(&foe->dir, DEG_TO_RAD(-10.0)); // turn right
            break;
        default:
            break;
        }
        break;
    }

    freeCollisionData(seePLayer, 1);
}

void updateEnemies(Enemy *Queue, int qSize, Player *p1, Weapon *k_pist, Weapon *pie, int targetFPS, float fov, Map *mp, Wall *walls, int wallcount)
{
    static int currentIndex = 0; // Index is saved between calls

    if (qSize == 0) // if no enemies return
        return;

    updateEnemy(Queue + currentIndex, *p1, &p1->hp, &k_pist->ammo, &pie->ammo, targetFPS, fov, mp, qSize, walls, wallcount); // update the enemy at index
    currentIndex = (currentIndex + 1) % qSize;                                                                               // move index
}

FILE *newMap(const char *filename)
{
    return fopen(filename, "w");
}

int addShape(FILE *map, Vec2 *corners, const char *texture, int cornercount, int closed)
{
    if (!map)                                   // if the file isn't opened properly
        return 0;                               // Fail
    for (int i = 0; i < (cornercount - 1); i++) // Move through all corners in order and create walls as you go along
    {
        fprintf(map, "%f,%f,%f,%f,%s\n", corners[i].x, corners[i].y, corners[i + 1].x, corners[i + 1].y, texture);
    }
    if (closed) // if the shape is closed make a wall between the start and end
    {
        fprintf(map, "%f,%f,%f,%f,%s\n", corners[cornercount - 1].x, corners[cornercount - 1].y, corners[0].x, corners[0].y, texture);
    }
    return 1; // Success
}

int addEnemy(FILE *map, Vec2 pos, int id, EnemyType type)
{

    if (!map)                                              // if the file isn't opened properly
        return 0;                                          // Fail
    fprintf(map, "%f,%f,%d,%d\n", pos.x, pos.y, id, type); // write enemy properties to file
    return 1;                                              // success
}

int saveMap(int numOfWalls, Wall *walls, char *filename) // kindof redundant right now. Don't use
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

Map *loadMap(const char *filename)
{
    // Opening file
    FILE *mfile = fopen(filename, "r");
    if (!mfile)
    {
        printf("Could not open file");
        return NULL;
    }
    // Allocating memory
    Map *result = malloc(sizeof(Map));
    if (!result)
    {
        printf("Malloc error");
        return NULL;
    }

    char buffer[128];
    // Read the format line of the file. I.E the first line
    if (!fgets(buffer, sizeof(buffer), mfile))
    {
        printf("Could not read format of file");
        fclose(mfile);
        free(result);
        return NULL;
    }
    int nwalls, nenemy;
    sscanf(buffer, "%d,%d", &nwalls, &nenemy); // decipher the format
    result->numOfWalls = nwalls;
    result->enemyCount = nenemy;

    result->walls = malloc(sizeof(Wall) * nwalls); // Allocate memory for walls according to format
    if (!result->walls)
    {
        printf("Malloc error");
        fclose(mfile);
        free(result);
        return NULL;
    }
    if (nenemy)
    {
        result->enemies = malloc(sizeof(Enemy) * nenemy); // Allocate memory for enemies according to format
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
    // read the first lines as walls. Loads coordinates and textures
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
    // Read the remaining lines as enemies. Loads attributes and sets som default attributes and also loads sprite.
    for (int i = 0; i < nenemy && fgets(buffer, sizeof(buffer), mfile) && nenemy; i++)
    {
        int type;
        sscanf(buffer, "%f,%f,%d,%d", &result->enemies[i].pos.x, &result->enemies[i].pos.y, &result->enemies[i].id, &type);

        switch (type)
        {
        case 0: // Creates a melee enemy
            result->enemies[i].sprite = LoadTexture("Sprites/MeleeNollantransp.png");
            result->enemies[i].attackRadius = 40.0;
            result->enemies[i].dmg = 3;
            result->enemies[i].hp = 70;
            result->enemies[i].baseCoolDown = 30;
            result->enemies[i].acceleration = 300;
            result->enemies[i].maxSpeed = 1200;
            result->enemies[i].type = 0;

            break;
        case 1: // Creates a midrange enemy
            result->enemies[i].sprite = LoadTexture("Sprites/MidrangeNollantransp.png");
            result->enemies[i].attackRadius = 330.0;
            result->enemies[i].dmg = 20;
            result->enemies[i].hp = 100;
            result->enemies[i].baseCoolDown = 240;
            result->enemies[i].acceleration = 100;
            result->enemies[i].maxSpeed = 400;
            result->enemies[i].type = 1;

            break;
        case 2: // Creates a long range enemy
            result->enemies[i].sprite = LoadTexture("Sprites/LongRangeNollan.png");
            result->enemies[i].attackRadius = 600.0;
            result->enemies[i].dmg = 35;
            result->enemies[i].hp = 50;
            result->enemies[i].baseCoolDown = 300;
            result->enemies[i].acceleration = 100;
            result->enemies[i].maxSpeed = 400;
            result->enemies[i].type = 2;

            break;
        case 3: // Creates a health pickup
            result->enemies[i].sprite = LoadTexture("Sprites/Health.png");
            result->enemies[i].attackRadius = 50.0;
            result->enemies[i].dmg = -20;
            result->enemies[i].hp = INT_MAX;
            result->enemies[i].baseCoolDown = 0;
            result->enemies[i].acceleration = 0;
            result->enemies[i].maxSpeed = 0;
            result->enemies[i].type = 3;

            break;
        case 4: // Creates a ammo pickup
            result->enemies[i].sprite = LoadTexture("Sprites/Ammo.png");
            result->enemies[i].attackRadius = 50.0;
            result->enemies[i].dmg = 0;
            result->enemies[i].hp = INT_MAX;
            result->enemies[i].baseCoolDown = 0;
            result->enemies[i].acceleration = 0;
            result->enemies[i].maxSpeed = 0;
            result->enemies[i].type = 4;

            break;
        default:
            printf("Invalid enemy type\n");
            break;
        }
        // Common for all enemies
        result->enemies[i].coolDown = 0;
        result->enemies[i].status = ALIVE;
        result->enemies[i].visibility = VISIBLE;
        result->enemies[i].velocity = VECINIT;
        result->enemies[i].dir = (Vec2){0.0, 1.0};
        result->enemies[i].hitRadius = (result->enemies[i].sprite.width * 16) / 64;
        result->enemies[i].acceleration *= nenemy;
        result->enemies[i].maxSpeed *= nenemy;
        result->enemies[i].friendlyProjectile = -1;
    }

    result->projectiles = malloc(MAXPROJECTILES * sizeof(Enemy *));
    for (int i = 0; i < MAXPROJECTILES; i++)
        result->projectiles[i] = NULL;
    result->ppointer = 0;
    fclose(mfile); // close file
    return result; // return map
}

void freeMap(Map *m)
{
    // If memory allocated -> free memory allocated.
    if (m->walls)
        free(m->walls);
    if (m->enemies)
        free(m->enemies);
    if (m->projectiles)
    {
        for (int i = 0; i < MAXPROJECTILES; i++)
        {
            if (m->projectiles[i])
            {
                free(m->projectiles[i]);
            }
        }
        free(m->projectiles);
    }

    free(m);
}