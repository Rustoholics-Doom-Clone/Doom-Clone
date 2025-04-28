#include "raycast.h"
#include "raylib.h"

// how long a step should be
#define STEP (Vec2){1.0, 0.0}
#define MAXHP 100
#define MAXAMMO 100
#define STARTPOS (Vec2){0.0, 0.0}
#define MAXSPEED 320
#define SHOOTDELAY 30

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
void shootEnemy(Player *player, Enemy *Queue, Wall *walls, int wallcount);
// Makes sure player health doesn't go over max health
void healPlayer(Player *player, int heal);
// Makes sure player ammo doesn't go over max ammo
void addAmmo(Player *player, int ammo);

#endif