#include <stdlib.h>
#include <math.h>
#include "movement.h"



/*TODO: 
Add rotation by mouse (?)
*/

void wishMoveForward(Player *player) {
    Vec2 old_wish = player->wishDir;
    Vec2 v1 = player->dir;
    Vec2 res = VECINIT;
    vectorAdd(old_wish, v1, &res);
    player->wishDir = res;
};
void wishMoveRight(Player *player) {
    Vec2 old_wish = player->wishDir;
    Vec2 v1 = player->dir;
    rotate(&v1, PI/2);
    Vec2 res = VECINIT;
    vectorAdd(old_wish, v1, &res);
    player->wishDir = res;
};
void wishMoveLeft(Player *player) {
    Vec2 old_wish = player->wishDir;
    Vec2 v1 = player->dir;
    rotate(&v1, PI/2);
    vectorScale(v1, -1, &v1);
    Vec2 res = VECINIT;
    vectorAdd(old_wish, v1, &res);
    player->wishDir = res;
};
void wishMoveBack(Player *player) {
    Vec2 old_wish = player->wishDir;
    Vec2 v1 = player->dir;
    vectorScale(v1, -1, &v1);
    Vec2 res = VECINIT;
    vectorAdd(old_wish, v1, &res);
    player->wishDir = res;
};

void rotateRight(Player *player) {
    rotate(&player->dir, ROTSPEED);
};
void rotateLeft(Player *player) {
    rotate(&player->dir, -ROTSPEED);
};

bool onSegment(Vec2 p, Vec2 q, Vec2 r) 
{ 
    if (q.x <= fmaxf(p.x, r.x) && q.x >= fminf(p.x, r.x) && 
        q.y <= fmaxf(p.y, r.y) && q.y >= fminf(p.y, r.y)) 
       return true; 
  
    return false; 
} 

int orientation(Vec2 p, Vec2 q, Vec2 r) {
    int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

    if (val == 0) return 0;

    return (val > 0)? 1: 2;
}

//Check if two lines intersect
bool intersect(Vec2 p1, Vec2 q1, Vec2 p2, Vec2 q2) {


    int o1 = orientation(p1, q1, p2); 
    int o2 = orientation(p1, q1, q2); 
    int o3 = orientation(p2, q2, p1); 
    int o4 = orientation(p2, q2, q1);

    if (o1 != o2 && o3 != o4) 
    return true;

    if (o1 == 0 && onSegment(p1, p2, q1)) return true; 
    if (o2 == 0 && onSegment(p1, q2, q1)) return true;  
    if (o3 == 0 && onSegment(p2, p1, q2)) return true; 
    if (o4 == 0 && onSegment(p2, q1, q2)) return true; 

    return false;
}

bool vecCompare(Vec2 v1, Vec2 v2) {
    if (v1.x == v2.x && v1.y == v2.y) {
        return true;
    }
    return false;
}
void executeMovement(Player *player, Wall *walls, int wallcount) {
    Vec2 old_vel = player->vel;
    Vec2 old_pos = player->pos;
    Vec2 wish_dir = player->wishDir;
    Vec2 res = VECINIT;
    Vec2 new_vel = VECINIT;
    if (vectorLenght(wish_dir) != 0.0) {
        normalize(&wish_dir);
    }
    float accel_speed = MAXSPEED*0.1/60;
    vectorScale(wish_dir, accel_speed, &wish_dir);
    vectorAdd(old_vel, wish_dir, &new_vel);
    vectorScale(new_vel, 0.9, &new_vel);
    if (vectorLenght(new_vel) < 0.1) {
        new_vel = VECINIT;
    }
    vectorAdd(old_pos, new_vel, &res);
    int i = 0;
    while (i < wallcount) {
        if (intersect(old_pos, res, walls[i].start, walls[i].stop)) {
            Vec2 pos_res = VECINIT;
            Vec2 wall_res = VECINIT;
            Vec2 new_pos = VECINIT;
            vectorSub(old_pos, res, &pos_res);
            vectorSub(walls[i].start, walls[i].stop, &wall_res);
            float dot = vectorDot(pos_res, wall_res);
            float len = vectorLenght(wall_res);
            float k = dot/(len*len);
            vectorScale(wall_res, k, &new_pos);
            if (vectorLenght(new_pos) < 0.5) {
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

void healPlayer(Player *player, int heal) {
    int old_hp = player->hp;
    player->hp = MIN(MAXHP, old_hp+heal);
}

void addAmmo(Player *player, int ammo) {
    int old_ammo = player->ammo;
    player->ammo = MIN(MAXAMMO, old_ammo+ammo);
}


