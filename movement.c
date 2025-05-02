#include <stdlib.h>
#include <math.h>
#include "movement.h"
#include "map.h"

/*TODO:
Add rotation by mouse (?)
*/

void wishMoveForward(Player *player)
{
    Vec2 old_wish = player->wishDir;
    Vec2 v1 = player->dir;
    Vec2 res = VECINIT;
    vectorAdd(old_wish, v1, &res);
    player->wishDir = res;
};
void wishMoveRight(Player *player)
{
    Vec2 old_wish = player->wishDir;
    Vec2 v1 = player->dir;
    rotate(&v1, PI / 2);
    Vec2 res = VECINIT;
    vectorAdd(old_wish, v1, &res);
    player->wishDir = res;
};
void wishMoveLeft(Player *player)
{
    Vec2 old_wish = player->wishDir;
    Vec2 v1 = player->dir;
    rotate(&v1, PI / 2);
    vectorScale(v1, -1, &v1);
    Vec2 res = VECINIT;
    vectorAdd(old_wish, v1, &res);
    player->wishDir = res;
};
void wishMoveBack(Player *player)
{
    Vec2 old_wish = player->wishDir;
    Vec2 v1 = player->dir;
    vectorScale(v1, -1, &v1);
    Vec2 res = VECINIT;
    vectorAdd(old_wish, v1, &res);
    player->wishDir = res;
};

void rotateRight(Player *player)
{
    rotate(&player->dir, ROTSPEED);
};
void rotateLeft(Player *player)
{
    rotate(&player->dir, -ROTSPEED);
};

bool onSegment(Vec2 p, Vec2 q, Vec2 r)
{
    if (q.x <= fmaxf(p.x, r.x) && q.x >= fminf(p.x, r.x) &&
        q.y <= fmaxf(p.y, r.y) && q.y >= fminf(p.y, r.y))
        return true;

    return false;
}

int orientation(Vec2 p, Vec2 q, Vec2 r)
{
    int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

    if (val == 0)
        return 0;

    return (val > 0) ? 1 : 2;
}

// Check if two lines intersect
bool intersect(Vec2 p1, Vec2 q1, Vec2 p2, Vec2 q2)
{

    int o1 = orientation(p1, q1, p2);
    int o2 = orientation(p1, q1, q2);
    int o3 = orientation(p2, q2, p1);
    int o4 = orientation(p2, q2, q1);

    if (o1 != o2 && o3 != o4)
        return true;

    if (o1 == 0 && onSegment(p1, p2, q1))
        return true;
    if (o2 == 0 && onSegment(p1, q2, q1))
        return true;
    if (o3 == 0 && onSegment(p2, p1, q2))
        return true;
    if (o4 == 0 && onSegment(p2, q1, q2))
        return true;

    return false;
}

float lenFromPointToLine(Vec2 vec, Line line)
{
    Vec2 ap = VECINIT;
    vectorSub(line.a, vec, &ap);
    float apn = vectorDot(ap, line.n);
    Vec2 apnv = VECINIT;
    vectorScale(line.n, apn, &apnv);
    Vec2 dist = VECINIT;
    vectorSub(ap, apnv, &dist);
    float len = vectorLenght(dist);

    return len;
};

Line vecsToLine(Vec2 v1, Vec2 v2)
{
    Vec2 n = VECINIT;
    Vec2 a = VECINIT;
<<<<<<< HEAD
    if (v2.x - v1.x != 0.0) {
        n = (Vec2){1.0, (v2.y - v1.y) / (v2.x - v1.x)};
        a = (Vec2){0, v1.y - n.y * v1.x};
        normalize(&n);
    } else {
=======
    if (v2.x - v1.x != 0.0)
    {
        n = (Vec2){1.0, (v2.y - v1.y) / (v2.x - v1.x)};
        a = (Vec2){0, v1.y - n.y * v1.x};
        normalize(&n);
    }
    else
    {
>>>>>>> origin/main
        n = (Vec2){0.0, 1.0};
        a = (Vec2){0.0, 0.0};
    }

    return (Line){n, a};
};

void executeMovement(Player *player, Wall *walls, int wallcount)
{
    Vec2 old_vel = player->vel;
    Vec2 old_pos = player->pos;
    Vec2 wish_dir = player->wishDir;
    Vec2 res = VECINIT;
    Vec2 new_vel = VECINIT;
    if (vectorLenght(wish_dir) != 0.0)
    {
        normalize(&wish_dir);
    }
    float accel_speed = MAXSPEED * 0.1 / 60;
    vectorScale(wish_dir, accel_speed, &wish_dir);
    vectorAdd(old_vel, wish_dir, &new_vel);
    vectorScale(new_vel, 0.9, &new_vel);
    if (vectorLenght(new_vel) < 0.1)
    {
        new_vel = VECINIT;
    }
    vectorAdd(old_pos, new_vel, &res);
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

    player->wishDir = VECINIT;
    player->vel = new_vel;
    player->pos = res;
}

void healPlayer(Player *player, int heal)
{
    int old_hp = player->hp;
    player->hp = MIN(MAXHP, old_hp + heal);
}

void addAmmo(Player *player, int ammo)
{
    int old_ammo = player->ammo;
    player->ammo = MIN(MAXAMMO, old_ammo + ammo);
}

CollisionData **rayShotProjectile(Player p1, float fov, Map *mp, Enemy **projectiles)
{
    CollisionData **result = malloc(sizeof(CollisionData *) * MAXPROJECTILES); // allocate memory for the data

    for (int i = 0; i < MAXPROJECTILES; i++)
    {
        result[i] = NULL;

        if (!projectiles[i] || !inFieldOfView(p1.pos, p1.dir, fov, *projectiles[i]) || (projectiles[i]->visibility == INVISIBLE)) // checks if enemy is outside of fov or dead or invisible
            continue;

        // make a normalized vector pointing towards the enemy from the player
        Vec2 diffvec;
        vectorSub(projectiles[i]->pos, p1.pos, &diffvec);
        float diff = vectorLenght(diffvec);
        normalize(&diffvec);

        // This is commented out since there is now z-sorting on enemies and projectiles and thus no need to raycast.
        /*
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
        if (!fl)      // if flag is false
            continue; // check next enemy
        */
        result[i] = malloc(sizeof(CollisionData)); // allocate memory for this collision

        result[i]->d = diff;
        result[i]->position = projectiles[i]->pos;
        // result[i]->angle = RAD_TO_DEG(acosf(vectorDot(playerdir, diffvec)));
        result[i]->angle = vectorDot(p1.dir, diffvec); // well be using the cos of the angle later and since both of the vectors are normalized this is the cos of the angle
        result[i]->texture = projectiles[i]->sprite;
        result[i]->textureOffset = NAN;
    }
    return result;
}

void shootEnemy(Player *player, Enemy *enemy, Wall *walls, int wallcount, int dmg)
{

    Vec2 player_look = VECINIT;
    vectorAdd(player->pos, player->dir, &player_look);

    int nbwall = 0;
    for (int j = 0; j < wallcount; j++)
    {
        if (!intersect(player->pos, enemy->pos, walls[j].start, walls[j].stop))
        {
            nbwall++;
        }
    }
    if (nbwall == wallcount)
    {
        if (lenFromPointToLine(enemy->pos, vecsToLine(player->pos, player_look)) < enemy->hitRadius)
        {
            enemy->hp -= dmg;
        }
    }
}

void shootProjectile(Weapon *wpn, Player *player)
{
    Enemy *proj = malloc(sizeof(Enemy));
    if (!proj)
        return;
    // Make an enemy object
    proj->sprite = LoadTexture("Sprites/Projectiles/projectilespritetransp.png");
    proj->acceleration = 2000.0 * MAXPROJECTILES;
    proj->attackRadius = proj->sprite.width / 2;
    proj->baseCoolDown = 0;
    proj->coolDown = 0;
    proj->dir = player->dir;
    proj->dmg = wpn->dmg;
    proj->hitRadius = 0;
    proj->hp = 1;
    proj->id = -1;
    proj->maxSpeed = 4000.0 * MAXPROJECTILES;
    proj->pos = player->pos;
    Vec2 offset;
    vectorScale(player->dir, 20.0, &offset);
    vectorAdd(proj->pos, offset, &proj->pos);
    proj->status = ALIVE;
    proj->velocity = VECINIT;
    proj->visibility = VISIBLE;

    // Try to slot in the object somewhere
    int fl = 1;
    for (int i = 0; i < MAXPROJECTILES; i++)
    {
        if (!wpn->projectiles[i])
        {
            wpn->projectiles[i] = proj;
            fl = 0;
            break;
        }
    }
    if (fl)
    {
        free(wpn->projectiles[wpn->ppointer]);
        wpn->projectiles[wpn->ppointer] = proj;
        wpn->ppointer = (wpn->ppointer + 1) % MAXPROJECTILES;
    }
}

void attackEnemy(Weapon *wpn, Player *player, Map *mp)
{
    switch (wpn->type)
    {
    case FIST: // Basically hitscan but with a range
        for (int i = 0; i < mp->enemyCount; i++)
        {
            Vec2 diffvec;
            vectorSub(mp->enemies[i].pos, player->pos, &diffvec);
            float ds = vectorLenght(diffvec);
            if ((ds - 30.0) < mp->enemies[i].hitRadius)
                shootEnemy(player, mp->enemies + i, mp->walls, mp->numOfWalls, wpn->dmg);
        }
        break;

    case HITSCAN: // Basically a laser beam
        for (int i = 0; i < mp->enemyCount; i++)
        {
            shootEnemy(player, mp->enemies + i, mp->walls, mp->numOfWalls, wpn->dmg);
        }
        break;

    case PROJECTILE:
        shootProjectile(wpn, player); // Shoot a projectile
        break;
    default:
        break;
    }
    wpn->currentCooldown = wpn->baseCooldown; // Reset fire cooldown
    wpn->ammo--;                              // lower ammo
}

int updateProjectile(Enemy *projectile, Player player, Enemy *enemies, int ec)
{
    Vec2 diffvec;
    for (int i = 0; i < ec; i++)
    {
        vectorSub(projectile->pos, enemies[i].pos, &diffvec);
        if (vectorLenght(diffvec) <= (projectile->attackRadius + enemies[i].hitRadius))
        {
            enemies[i].hp -= projectile->dmg;
            return 1; // Signal to updateProjectiles that it should free and NULL it
        }
    }

    moveEnemy(projectile, projectile->dir, 60);       // Move the projectile
    vectorSub(projectile->pos, player.pos, &diffvec); // Check if the projectile is too far away from the player
    if (vectorLenght(diffvec) >= 2000)
    {
        return 1; // Signal to updateProjectiles that it should free and NULL it
    }

    return 0;
}

void updateProjectiles(Enemy **projectiles, Player player, Enemy *enemies, int ec, Weapon *wpn)
{

    if (projectiles[wpn->ppointer]) // if the current queue slot contains anything
    {
        if (updateProjectile(projectiles[wpn->ppointer], player, enemies, ec)) // update the projectile and check if it should be removed
        {
            free(projectiles[wpn->ppointer]);  // deallocate projectile
            projectiles[wpn->ppointer] = NULL; // set spot to null
        }
    }

    wpn->ppointer = (wpn->ppointer + 1) % MAXPROJECTILES; // move over one spot in the queue
    return;
}

Weapon *getWeapons(int width, int height)
{
    Weapon *wps = malloc(sizeof(Weapon) * 3);
    if (!wps)
        return NULL;

    // Make the fist weapon
    wps[0].normalSprite = LoadTexture("Sprites/Weapons/Fist1transp.png");
    wps[0].shootingSprite = LoadTexture("Sprites/Weapons/Fist2transp.png");
    wps[0].baseCooldown = 15;
    wps[0].currentCooldown = 0;
    wps[0].screenPos = (Vec2){width * 0.5, 0};
    wps[0].normalScale = (Vec2){0.6 * width / 800.0, 0.6 * width / 800.0};
    wps[0].shootingScale = (Vec2){0.8 * width / 800.0, 0.8 * width / 800.0};
    wps[0].ppointer = 0;
    wps[0].projectiles = NULL;
    wps[0].type = FIST;
    wps[0].ammo = INT_MAX;
    wps[0].dmg = 30;

    // Make the smg
    wps[1].normalSprite = LoadTexture("Sprites/Weapons/kpisttransp.png");
    wps[1].shootingSprite = LoadTexture("Sprites/Weapons/kpist2transp.png");
    wps[1].baseCooldown = 15;
    wps[1].currentCooldown = 0;
    wps[1].screenPos = (Vec2){width / 5.0, 0};
    wps[1].normalScale = (Vec2){0.8 * width / 800.0, 0.8 * width / 800.0};
    wps[1].shootingScale = (Vec2){0.8 * width / 800.0, 0.8 * width / 800.0};
    wps[1].ppointer = 0;
    wps[1].projectiles = NULL;
    wps[1].type = HITSCAN;
    wps[1].ammo = 120;
    wps[1].dmg = 20;

    // Make the pie
    wps[2].normalSprite = LoadTexture("Sprites/Weapons/Projectile1transp.png");
    wps[2].shootingSprite = LoadTexture("Sprites/Weapons/Fist2transp.png");
    wps[2].baseCooldown = 15;
    wps[2].currentCooldown = 0;
    wps[2].screenPos = (Vec2){width * 0.5, 0};
    wps[2].normalScale = (Vec2){0.8 * width / 800.0, 0.8 * width / 800.0};
    wps[2].shootingScale = (Vec2){0.8 * width / 800.0, 0.8 * width / 800.0};
    wps[2].ppointer = 0;
    wps[2].projectiles = malloc(sizeof(Enemy *) * MAXPROJECTILES);
    for (int i = 0; i < MAXPROJECTILES; i++)
        wps[2].projectiles[i] = NULL; // initialize to NULL

    wps[2].type = PROJECTILE;
    wps[2].ammo = 10;
    wps[2].dmg = 100;

    return wps;
}
