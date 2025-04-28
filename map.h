#include "raycast.h"
#include "movement.h"
#include <stdio.h>

#ifndef WALLSTRUCT
#define WALLSTRUCT

typedef struct
{
    Vec2 start, stop; // Both ends of the wall
} Wall;

#endif

#ifndef ENEMY
#define ENEMY

#define CLAMP(x, lower, upper) ((x) < (lower) ? (lower) : ((x) > (upper) ? (upper) : (x)))

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
    float attackRadius;
    Texture2D sprite;
    Vec2 pos;
    Vec2 dir;
    Vec2 velocity;
    int hp;
    int id;
    int baseCoolDown;
    int coolDown;
    float acceleration;
    float maxSpeed;
} Enemy;

// Checks if enemy is in players field of view
int inFieldOfView(Vec2 playerpos, Vec2 playerdir, float FOV, Enemy foe1);
// Checks if there is a clear line of sight between a player and enemy. If not then Collisiondata* == NULL
CollisionData **rayShotEnemies(Player p1, float fov, Map *mp, Enemy *enemies, int ec);
// Applies a force to an enemy in the dir direction.
void moveEnemy(Enemy *foe, Vec2 dir, int targetFPS);
// Updates an enemy
void updateEnemy(Enemy *foe, Player p1, int *playerHealth, int targetFPS, float fov, Map *mp, int numOfEnemy);
// Handles a the queue of enemies to be updated
void updateEnemies(Enemy *Queue, int qSize, Player *p1, int targetFPS, float fov, Map *mp);

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
// Opens a new map file
FILE *newMap(const char *filename);
// Adds a shape to the map file. Makes the shapes out of vertices.
int addShape(FILE *map, Vec2 *corners, const char *texture, int cornercount, int closed);
// Adds an enemy to the map file.
int addEnemy(FILE *map, Vec2 pos, int id, float acceleration, float maxSpeed, const char *sprite);
// saves an array of walls as a map.
int saveMap(int numOfWalls, Wall *walls, char *filename);
// reads a map from a file.
Map *loadMap(char *filename);
// Frees a Map
void freeMap(Map *m);

#endif