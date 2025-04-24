#include "raycast.h"
#include "movement.h"

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
    float attackRadius;
    Texture2D sprite;
    Vec2 pos;
    Vec2 dir;
    Vec2 velocity;
    int hp;
    int id;
    float acceleration;
    float maxSpeed;
} Enemy;

// Checks if enemy is in players field of view
int inFieldOfView(Vec2 playerpos, Vec2 playerdir, float FOV, Enemy foe1);
// Checks if there is a clear line of sight between a player and enemy. If not then Collisiondata* == NULL
CollisionData **rayShotEnemies(Vec2 playerpos, Vec2 playerdir, float FOV, Wall *wls, int wn, Enemy *enemies, int ec);
// Applies a force to an enemy in the dir direction.
void moveEnemy(Enemy *foe, Vec2 dir, int targetFPS);
// Updates an enemy
void updateEnemy(Enemy *foe, Vec2 playerPos, Vec2 playerdir, int *playerHealth, int targetFPS, float fov, Wall *wls, int wn);
// Handles a the queue of enemies to be updated
void updateEnemies(Enemy *Queue, int qSize, Player p1, int targetFPS, float fov, Map mp);

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