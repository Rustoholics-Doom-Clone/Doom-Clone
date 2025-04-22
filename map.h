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