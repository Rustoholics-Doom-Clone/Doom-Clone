#include <stdlib.h>
#include <math.h>
#include "movement.h"



/*TODO: 
Add rotation by mouse (?)
Add acceleration based movement (?)
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
};

void healPlayer(Player *player, int heal) {
    int old_hp = player->hp;
    player->hp = MIN(MAXHP, old_hp+heal);
}

void addAmmo(Player *player, int ammo) {
    int old_ammo = player->ammo;
    player->ammo = MIN(MAXAMMO, old_ammo+ammo);
}


//Converts the points of a wall into a line in vector form x = a + tn
Line wallToLine(Wall wall) {
    Vec2 n = (Vec2){1.0, (wall.stop.y-wall.start.y) / (wall.stop.x-wall.stop.y)};
    Vec2 a = (Vec2){0, wall.start.y - n.x * wall.start.y};
    normalize(&n);
    return (Line){n, a};
};

//Gets the shortest vector between a point and a line
Vec2 vecFromPointToLine(Vec2 vec, Line line) {
    Vec2 ap = VECINIT;
    vectorSub(line.a, vec, &ap);
    float apn = vectorDot(ap, line.n);
    Vec2 apnv = VECINIT;
    vectorScale(line.n, apn, &apnv);
    Vec2 dist = VECINIT;
    vectorSub(ap, apnv, &dist);
    return dist;
};

//Checks if the player is colliding with a wall
void collideWithWall(Player *player, Wall wall) {
    Vec2 pos = player->pos;
    Line wallLine = wallToLine(wall);
    Vec2 dist = vecFromPointToLine(pos, wallLine);
    Vec2 invDist = VECINIT;
    vectorScale(dist, -1.0, &invDist);

    float maxY = fmaxf(wall.start.y, wall.stop.y)+1.0;
    float minY = fminf(wall.start.y, wall.stop.y)-1.0;
    float maxX = fmaxf(wall.start.x, wall.stop.x)+1.0;
    float minX = fminf(wall.start.x, wall.stop.x)-1.0;

    if (vectorLenght(dist) < vectorLenght(STEP) &&
     pos.x > minX && pos.x < maxX &&
      pos.y > minY && pos.y < maxY) 
    {
        player->pos.x = pos.x += invDist.x;
        player->pos.y = pos.y += invDist.y;
    }

};
