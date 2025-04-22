#include "raycast.h"

#ifndef WALLSTRUCT
#define WALLSTRUCT

typedef struct
{
    Vec2 start, stop; // Both ends of the wall
} Wall;

#endif

#ifndef ENEMY
#define ENEMY

typedef enum Visibility
{
    VISIBLE,
    INVISIBLE
} Visibility;

typedef enum Status
{
    DEAD,
    ALIVE
} Status;

typedef struct Enemy
{
    Status status;
    Visibility visibility;
    float hitRadius;
    char *sprite;
    Vec2 pos;
    Vec2 dir;
    int hp;
    int id;
} Enemy;

// Checks if enemy is in players field of view
int inFieldOfView(Vec2 playerpos, Vec2 playerdir, float FOV, Enemy foe1);
// Checks if there is a clear line of sight between a player and enemy. If not then Collisiondata* == NULL
CollisionData **rayShotEnemies(Vec2 playerpos, Vec2 playerdir, float FOV, Wall *wls, int wn, Enemy *enemies, int ec);

#endif

#ifndef MAPH
#define MAPH

typedef struct Map
{
    int numOfWalls;
    Wall *walls;
    int enemyCount;
    Enemy *enemies;
} Map;

// saves an array of walls as a map.
int saveMap(int numOfWalls, Wall *walls, char *filename);
// reads a map from a file.
Map *loadMap(char *filename);
// Frees a Map
void freeMap(Map *m);

#endif