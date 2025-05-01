#include "raylib.h"
#include "raycast.h"
#include "movement.h"
#include "map.h"
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define TILE_SIZE 64
#define MAP_WIDTH 10
#define MAP_HEIGHT 10
#define MAX_WALLS 1024
#define NUM_RAYS 200
#define FOV 60.0f



void draw3DView(CollisionData **hits, int rayCount, Texture2D floorTexture, Texture2D roofTexture)
{
    for (int i = 0; i < rayCount; i++)
    {
        if (!hits[i])
            continue;

        float dist = hits[i]->d;
        float corrected = dist * cosf(DEG_TO_RAD(hits[i]->angle));    // Correct fisheye effect
        float wallHeight = ((TILE_SIZE * SCREEN_HEIGHT) / corrected); // Wall height based on screen size

        Texture2D texture = hits[i]->texture;

        float sliceWidth = (float)SCREEN_WIDTH / NUM_RAYS;
        float wallTop = (SCREEN_HEIGHT / 2.0f) - (wallHeight / 2.0f);
        float wallBottom = wallTop + wallHeight;

       // Compute roof rect
       Rectangle srcRoof = {
        0, 0,
        roofTexture.width, roofTexture.height
        };

        Rectangle destRoof = {
            i * sliceWidth,     // X
            0,                  // Y (top of screen)
            sliceWidth,         // Width
            wallTop             // Height (from top to start of wall)
        };
        

        // Draw a piece of roof texture stretched to fit
    DrawTexturePro(roofTexture, srcRoof, destRoof, (Vector2){0, 0}, 0.0f, WHITE);

        // --- Draw walls ---
        float texX = hits[i]->textureOffset * texture.width;
        // Source rectangle: a vertical slice of the wall texture
        Rectangle source = {
            texX,
            0,
            1,
            (float)texture.height};

        // Destination rectangle: the scaled vertical slice on screen
        Rectangle destination = {
            i * sliceWidth, // X on screen
            (SCREEN_HEIGHT / 2.0f) - (wallHeight / 2.0f),
            sliceWidth, // stretches pixels in source retangel to slicewith
            wallHeight};

        DrawTexturePro(texture, source, destination, (Vector2){0, 0}, 0.0f, WHITE);

       // Compute floor rect
        Rectangle srcFloor = {
            0, 0,
            floorTexture.width, floorTexture.height
        };

        Rectangle destFloor = {
            i * sliceWidth,          // X position on screen
            wallBottom,              // Y position (below wall)
            sliceWidth,              // Width on screen (same as wall slice width)
            SCREEN_HEIGHT - wallBottom // Height from wall bottom to bottom of screen
        };

        // Draw a piece of floor texture stretched to fit
        DrawTexturePro(floorTexture, srcFloor, destFloor, (Vector2){0, 0}, 0.0f, WHITE);
    }
    
}

int compareEnemyDistance(const void *a, const void *b)
{

    CollisionData *f1 = *(CollisionData **)a;
    CollisionData *f2 = *(CollisionData **)b;
    if (!f1 || !f2)
        return -1;

    float cmp = f1->d - f2->d;
    if (cmp == 0.0)
        return 0;
    if (cmp < 0.0)
        return 1;
    return -1;
}

void drawEnemies(Player p1, CollisionData **enemyColl, int enemyCount)
{
    qsort(enemyColl, enemyCount, sizeof(CollisionData *), compareEnemyDistance);

    Vec2 plane = {
        -p1.dir.y * tanf(DEG_TO_RAD(FOV / 2)),
        p1.dir.x * tanf(DEG_TO_RAD(FOV / 2))};

    for (int i = 0; i < enemyCount; i++)
    {
        if (!enemyColl[i])
            continue;

        Vec2 enemyPos = enemyColl[i]->position;

        // Vector from player to enemy
        float dx = enemyPos.x - p1.pos.x;
        float dy = enemyPos.y - p1.pos.y;

        // Inverse camera transform
        float invDet = 1.0f / (plane.x * p1.dir.y - p1.dir.x * plane.y);

        float transformX = invDet * (p1.dir.y * dx - p1.dir.x * dy);
        float transformY = invDet * (-plane.y * dx + plane.x * dy);

        if (transformY <= 0)
            continue; // Enemy is behind the player

        // Projected X position on screen
        float enemyScreenX = (SCREEN_WIDTH / 2) * (1 + transformX / transformY);

        Texture2D sprite = enemyColl[i]->texture;

        // Preserve sprite aspect ratio
        float aspectRatio = (float)sprite.width / (float)sprite.height;

        float dist = enemyColl[i]->d;
        float corrected = dist * enemyColl[i]->angle;               // Correct fisheye effect
        float wallHeight = (TILE_SIZE * SCREEN_HEIGHT) / corrected; // Wall height based on screen size

        // Sprite height scaling factor
        float spritesScale = 24.0;
        float spriteHeight = spritesScale * (SCREEN_HEIGHT / transformY) * 1.8f; // 1.8 = tune to taste
        float spriteWidth = spriteHeight * aspectRatio;

        Rectangle src = {
            0, 0,
            (float)sprite.width,
            (float)sprite.height};

        Rectangle dest = {
            enemyScreenX - spriteWidth / 2,
            SCREEN_HEIGHT / 2 + wallHeight / 2 - spriteHeight,
            spriteWidth,
            spriteHeight};

        DrawTexturePro(sprite, src, dest, (Vector2){0, 0}, 0.0f, WHITE);
    }
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Raycasting in raylib");
    SetTargetFPS(60);
    srand(time(NULL));

    Player player = PLAYERINIT;

    Map *mp = loadMap("testmap1.csv");
    Texture2D floorTexture = LoadTexture("Sprites/Ground.png");
    Texture2D roofTexture = LoadTexture("Sprites/Sky.png");

    while (!WindowShouldClose())
    {
        if (player.shoot_cd > 0)
        {
            player.shoot_cd--;
        }

        if (IsKeyDown(KEY_RIGHT))
        {
            rotateRight(&player);
        }

        if (IsKeyDown(KEY_LEFT))
        {
            rotateLeft(&player);
        }
        if (IsKeyDown('W'))
        {
            wishMoveForward(&player);
        }

        if (IsKeyDown('A'))
        {
            wishMoveLeft(&player);
        }

        if (IsKeyDown('S'))
        {
            wishMoveBack(&player);
        }

        if (IsKeyDown('D'))
        {
            wishMoveRight(&player);
        }

        if (IsKeyDown(KEY_SPACE) && player.shoot_cd == 0 && player.ammo > 0)
        {
            for (int i = 0; i < mp->enemyCount; i++)
            {
                shootEnemy(&player, mp->enemies + i, mp->walls, mp->numOfWalls);
            }
            player.shoot_cd = SHOOTDELAY;
            player.ammo--;
        }

        executeMovement(&player, mp->walls, mp->numOfWalls);

        CollisionData **hits = multiRayShot(player.pos, player.dir, FOV, mp->numOfWalls, mp->walls, NUM_RAYS);

        CollisionData **enemyData = rayShotEnemies(player, FOV, mp, mp->enemies, mp->enemyCount);

        BeginDrawing();
        ClearBackground(BLACK);

        draw3DView(hits, NUM_RAYS, floorTexture, roofTexture);
        drawEnemies(player, enemyData, mp->enemyCount);

        updateEnemies(mp->enemies, mp->enemyCount, &player, 60, FOV, mp);
        drawEnemies(player, enemyData, mp->enemyCount);

        updateEnemies(mp->enemies, mp->enemyCount, &player, 60, FOV, mp);

        char buffer[64];
        sprintf(buffer, "HP: %d", player.hp);
        DrawText(buffer, SCREEN_WIDTH - 200, SCREEN_HEIGHT - 60, 20, BLACK);

        sprintf(buffer, "+");
        DrawText(buffer, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 20, (Color){245, 40, 145, 204});

        sprintf(buffer, "AMMO: %d", player.ammo);
        DrawText(buffer, SCREEN_WIDTH - 200, SCREEN_HEIGHT - 30, 20, BLACK);

        EndDrawing();

        freeCollisionData(hits, NUM_RAYS);
        freeCollisionData(enemyData, mp->enemyCount);
    }

    CloseWindow();
    return 0;
}
