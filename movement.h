#include "raycast.h"
#include "raylib.h"

//how long a step should be
#define STEP (Vec2){1.0, 0.0}
#define MAXHP 100
#define MAXAMMO 100
#define STARTPOS (Vec2){160.0, 160.0}

//How fast character rotates
#define ROTSPEED PI/120

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MOVEMENT_H
#define MOVEMENT_H



typedef struct
{
    Vec2 pos;
    Vec2 dir;
    int hp;
    int ammo;
} Player;

#define PLAYERINIT (Player){STARTPOS, (Vec2){1.0, 0.0}, MAXHP, MAXAMMO}

typedef struct
{
    Vec2 n;
    Vec2 a;
} Line;

#define LINEINIT (Line){VECINIT, VECINIT}


//move character forwards, requires current position and angle in radians
void moveForward(Player *player);
//move character right
void moveRight(Player *player);
//move character left
void moveLeft(Player *player);
//move character back
void moveBack(Player *player);
//rotates character right by ROTSPEED rad
void rotateRight(Player *player);
//rotates character left by ROTSPEED rad
void rotateLeft(Player *player);
//Checks if there is collision between player and wall, and pushes away player if so
void collideWithWall(Player *player, Wall wall);
//Makes sure player health doesn't go over max health
void healPlayer(Player *player, int heal);
//Makes sure player ammo doesn't go over max ammo
void addAmmo(Player *player, int ammo);

#endif