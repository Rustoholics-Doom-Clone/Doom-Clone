#include "raycast.h"

#ifndef WALLSTRUCT
#define WALLSTRUCT

typedef struct
{
    Vec2 start, stop; // Both ends of the wall
} Wall;

#endif

#ifndef MAPH
#define MAPH

typedef struct Map
{
    int numOfWalls;
    Wall *walls;
    int enemyCount; // Will most definetly be reworked when enemy logic is made
    Vec2 *enemyPos; // Will most definetly be reworked when enemy logic is made
} Map;

// saves an array of walls as a map.
int saveMap(int numOfWalls, Wall *walls, char *filename);
// reads a map from a file.
Map *loadMap(char *filename);

#endif