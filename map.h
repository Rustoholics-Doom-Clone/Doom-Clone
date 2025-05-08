#include "raycast.h"
#include "movement.h"
#include <stdio.h>

#ifndef MAPH
#define MAPH

typedef enum EnemyType EnemyType;

typedef struct Wall
{
    Vec2 start, stop;  // Both ends of the wall
    Texture2D texture; // The texture of the wall
} Wall;

typedef struct Map
{
    int numOfWalls;
    Wall *walls;
    int enemyCount;
    Enemy *enemies;
    Enemy **projectiles;
    int ppointer;
} Map;
// Opens a new map file
FILE *newMap(const char *filename);
// Adds a shape to the map file. Makes the shapes out of vertices.
int addShape(FILE *map, Vec2 *corners, const char *texture, int cornercount, int closed);
// Adds an enemy to the map file.
int addEnemy(FILE *map, Vec2 pos, int id, EnemyType type);
// reads a map from a file.
Map *loadMap(const char *filename);
// Frees a Map
void freeMap(Map *m);

#endif