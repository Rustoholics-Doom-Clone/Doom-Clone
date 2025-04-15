#include "raycast.h"

#ifndef MAPH
#define MAPH

typedef struct
{
    int numOfWalls;
    Wall *walls;
    int enemyCount; // Will most definetly be reworked when enemy logic is made
    Vec2 *enemyPos; // Will most definetly be reworked when enemy logic is made
} Map;

#endif