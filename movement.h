#include "raycast.h"
#include "raylib.h"

// how long a step should be
#define STEP (Vec2){1.0, 0.0}
#define MAXHP 100
#define MAXAMMO 100
#define STARTPOS (Vec2){0.0, 0.0}
#define MAXSPEED 320
#define SHOOTDELAY 30
#define MAXPROJECTILES 10

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
} Line;

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
    int ammo;
    int shoot_cd;
} Player;

#define PLAYERINIT (Player){STARTPOS, (Vec2){0.0, 1.0}, VECINIT, VECINIT, MAXHP, MAXAMMO, 0}

// move character forwards, requires current position and angle in radians
void wishMoveForward(Player *player);
// move character right
void wishMoveRight(Player *player);
// move character left
void wishMoveLeft(Player *player);
// move character back
void wishMoveBack(Player *player);
// rotates character right by ROTSPEED rad
void rotateRight(Player *player);
// rotates character left by ROTSPEED rad
void rotateLeft(Player *player);
// executes current movement in current wishDir
void executeMovement(Player *player, Wall *walls, int wallCount);
// Shoots an enemy if they are within line of sight and close enough to the crosshair
void shootEnemy(Player *player, Enemy *enemy, Wall *walls, int wallcount, int dmg);
// Makes sure player health doesn't go over max health
void healPlayer(Player *player, int heal);
// Makes sure player ammo doesn't go over max ammo
void addAmmo(Player *player, int ammo);
// Works like rayShotEnemy but with extra safeguards since **projectiles might be empty
CollisionData **rayShotProjectile(Player p1, float fov, Map *mp, Enemy **projectiles);
// Creates a projectile that is facing the same direction as the player
void shootProjectile(Weapon *wpn, Player *player);
// Moves the projectile and checks if it has hit an enemy
int updateProjectile(Enemy *projectile, Player player, Enemy *enemies, int ec);
// Goes through all the projectiles and updates them
void updateProjectiles(Enemy **projectiles, Player player, Enemy *enemies, int ec, Weapon *wpn);
// Attacks depending on which weapon you're holding
void attackEnemy(Weapon *wpn, Player *player, Map *mp);
// Initializes all the weapons
Weapon *getWeapons(int width, int height);

#endif