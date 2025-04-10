#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "movement.h"



/*TODO: 
Stop at walls
Add rotation by mouse (?)
*/

void moveForward(Player *player) {
    float old_x = player->pos.x;
    float old_y = player->pos.y;
    player->pos.x = old_x += player->dir.x;
    player->pos.y = old_y += player->dir.y;
};
void moveRight(Player *player) {
    float old_x = player->pos.x;
    float old_y = player->pos.y;
    Vec2 dir = player->dir;
    rotate(&dir, PI/2);
    player->pos.x = old_x += dir.x;
    player->pos.y = old_y += dir.y;
};
void moveLeft(Player *player) {
    float old_x = player->pos.x;
    float old_y = player->pos.y;
    Vec2 dir = player->dir;
    rotate(&dir, -PI/2);
    player->pos.x = old_x += dir.x;
    player->pos.y = old_y += dir.y;
};
void moveBack(Player *player) {
    float old_x = player->pos.x;
    float old_y = player->pos.y;
    Vec2 dir = player->dir;
    rotate(&dir, PI);
    player->pos.x = old_x += dir.x;
    player->pos.y = old_y += dir.y;
};

void rotateRight(Player *player) {
    rotate(&player->dir, ROTSPEED);
};
void rotateLeft(Player *player) {
    rotate(&player->dir, -ROTSPEED);
}
