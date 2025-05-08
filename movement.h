#include "raycast.h"
#include "raylib.h"

// player maxhp
#define MAXHP 100
#define STARTPOS (Vec2){0.0, 0.0}
#define MAXSPEED 320
#define MAXPROJECTILES 30

// How fast character rotates
#define ROTSPEED PI / 120

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MOVEMENT_H
#define MOVEMENT_H

typedef struct
{
    Vec2 n;
    Vec2 a;
} Line; //n = dir, a = offset

typedef struct Enemy Enemy;

typedef enum WeaponType
{
    FIST,
    HITSCAN,
    PROJECTILE
} WeaponType;

typedef struct
{
    WeaponType type;
    Texture2D normalSprite;
    Texture2D shootingSprite;
    int baseCooldown;
    int currentCooldown;
    Vec2 screenPos;
    Vec2 normalScale;
    Vec2 shootingScale;
    Enemy **projectiles;
    int ppointer;
    int dmg;
    int ammo;
} Weapon;

typedef struct
{
    Vec2 pos;
    Vec2 dir;
    Vec2 vel;
    Vec2 wishDir;
    int hp;
} Player;

#define PLAYERINIT (Player){STARTPOS, (Vec2){0.0, 1.0}, VECINIT, VECINIT, MAXHP}

// add forwards to the wish vector
void wishMoveForward(Player *player);
// add right to the wish vector
void wishMoveRight(Player *player);
// add left to the wish vector
void wishMoveLeft(Player *player);
// add backwards to the wish vector
void wishMoveBack(Player *player);
// rotates character right by ROTSPEED rad
void rotateRight(Player *player);
// rotates character left by ROTSPEED rad
void rotateLeft(Player *player);
// Tries to match current velocity with the players wishdir
void executeMovement(Player *player, Wall *walls, int wallCount);
// Shoots an enemy if they are within line of sight and close enough to the crosshair
void shootEnemy(Player *player, Enemy *enemy, Wall *walls, int wallcount, int dmg);
// Works like rayShotEnemy but with extra safeguards since **projectiles might be empty
CollisionData **rayShotProjectile(Player p1, float fov, Map *mp, Enemy **projectiles);
// Creates a projectile that is facing the same direction as the player
void shootProjectile(Vec2 pos, Vec2 dir, int dmg, Enemy **projectiles, int *ppointer, int friendly);
// Moves the projectile and checks if it has hit an enemy
int updateProjectile(Enemy *projectile, Player *player, Enemy *enemies, int ec);
// Goes through all the projectiles and updates them
void updateProjectiles(Enemy **projectiles, Player *player, Enemy *enemies, int ec, Weapon *wpn, int *ppointer);
// Attacks depending on which weapon you're holding
void attackEnemy(Weapon *wpn, Player *player, Map *mp);
// Initializes all the weapons
Weapon *getWeapons(int width, int height, Enemy **projectiles);
// Get if there is intersection between line p1q1 and p2q2
bool intersect(Vec2 p1, Vec2 q1, Vec2 p2, Vec2 q2);

#endif