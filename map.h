#include "raycast.h"
#include "movement.h"
#include <stdio.h>

#ifndef ENEMY
#define ENEMY

// Clamp a value within a range
#define CLAMP(x, lower, upper) ((x) < (lower) ? (lower) : ((x) > (upper) ? (upper) : (x)))

typedef enum Visibility // To see if an enemy should be drawn on the screen or not
{
    VISIBLE,
    INVISIBLE
} Visibility;

typedef enum Status // To see if an enemy is dead or alive
{
    DEAD,
    ALIVE
} Status;

typedef enum EnemyType // To see what properties it should have
{

    MELEE,
    MIDRANGE,
    SNIPER,
    HEALTH,
    AMMO
} EnemyType;

typedef struct Enemy // The enemy
{
    Status status;         // Dead or no
    Visibility visibility; // Visible or no
    float hitRadius;       // How girthy is the enemy
    float attackRadius;    // How far away can he attack
    Texture2D sprite;      // How does he look
    Vec2 pos;              // You're a fart smeller you can figure out some of theese yourself
    Vec2 dir;
    Vec2 velocity;
    int hp;
    int dmg;
    int id;
    int baseCoolDown; // reload time
    int coolDown;     // how far along he is reloading
    float acceleration;
    float maxSpeed;
    int friendlyProjectile; // If he happens to be a friendly flying object
    int type;
} Enemy;

// Shoots a projectile
void shootProjectile(Vec2 pos, Vec2 dir, int dmg, Enemy **projectiles, int *ppointer, int friendly);
// Checks if enemy is in players field of view
int inFieldOfView(Vec2 playerpos, Vec2 playerdir, float FOV, Enemy foe1);
// Checks if there is a clear line of sight between a player and enemy. If not then Collisiondata* == NULL
CollisionData **rayShotEnemies(Player p1, float fov, Map *mp, Enemy *enemies, int ec);
// Shoots from the enemys point of view so it ignores the fov and such
CollisionData **rayShotPlayer(Enemy foe, Player p1, Map *mp);
// Applies a force to an enemy in the dir direction.
void moveEnemy(Enemy *foe, Vec2 dir, int targetFPS, Wall *walls, int wallcount);
// Updates an enemy
void updateEnemy(Enemy *foe, Player p1, int *playerHealth, int *k_pistAmmo, int *pieAmmo, int targetFPS, float fov, Map *mp, int numOfEnemy, Wall *walls, int wallcount);
// Handles a the queue of enemies to be updated
void updateEnemies(Enemy *Queue, int qSize, Player *p1, Weapon *k_pist, Weapon *pie, int targetFPS, float fov, Map *mp, Wall *walls, int wallcount);
// Counts live hostile enemies
int countHostiles(Map *mp);

#endif

#ifndef MAPH
#define MAPH

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