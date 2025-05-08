#include "enemy.h"
#include <math.h>
#include <stdlib.h>
#include "map.h"

int inFieldOfView(Vec2 playerpos, Vec2 playerdir, float fov, Enemy foe1)
{
    Vec2 toEnemy;
    vectorSub(foe1.pos, playerpos, &toEnemy); // Draw a line between player and enemy

    float distance = vectorLenght(toEnemy); // How far away is the enemy
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
        if (temp && temp->d < diff)                                                   // If there is a collision with a wall and the collision is closer than the distance between the player and enemy
        {
            fl = 0; // flag is false
            break;  // break loop early
        }
    }
    if (!fl) // there is a wall in the way, Abandon ship!
    {
        free(result);
        return NULL;
    }

    result[0] = malloc(sizeof(CollisionData)); // allocate memory for this collision
    // Save the important stuff
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
    while (i < wallcount) // go through each wall, (Not literally that would hurt)
    {
        if (intersect(old_pos, res, walls[i].start, walls[i].stop)) // If broken nose (Walked into the Wall)
        {
            // Lots of linear algebra to try and end up on the right side of the wall
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
    if (foe->status == DEAD) // You're dead, skip your turn
        return;
    if (foe->hp <= 0)
    { // Check if enemy should be dead
        foe->status = DEAD;
        if (foe->type == 3 | foe->type == 4)
        {
            foe->visibility = INVISIBLE; // Health packs and ammo are invisible when dead
            return;
        }
        foe->sprite = LoadTexture("Sprites/Nollekorttransp.png"); // Nollan becomes nollekort.
        return;
    }

    CollisionData **seePLayer = rayShotPlayer(*foe, p1, mp); // shoots a ray at the player to see if there is line of sight.

    switch (seePLayer == NULL)
    {
    case 0: // If there is line of sight

        // Draw a line towards the player
        Vec2 dir;
        vectorSub(p1.pos, foe->pos, &dir);
        float dist = vectorLenght(dir);
        normalize(&dir);

        if (dist <= foe->attackRadius) // If player is within attackRadius
        {
            foe->velocity = VECINIT; // Stop!
            if (foe->coolDown <= 0)  // if not reloading
            {

                switch (foe->type) // Attack the player depending on which type of enemy
                {
                case 0:
                    *playerHealth -= foe->dmg;
                    break;
                case 3:
                    *playerHealth -= foe->dmg;
                    foe->hp = 0;
                    break;
                case 4:
                    *k_pistAmmo += 20;
                    *pieAmmo += 3;
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

int countHostiles(Map *mp)
{
    int result = 0;
    for (int i = 0; i < mp->enemyCount; i++)
    {
        if (mp->enemies[i].status == ALIVE && mp->enemies[i].type < 3) // All alive enemies that are not health- or ammo-packs are counted
        {
            result++;
        }
    }
    return result;
}