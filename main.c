#include "raylib.h"
#include "raycast.h"
#include "movement.h"
#include <math.h>
#include <stdio.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define TILE_SIZE 64
#define MAP_WIDTH 10
#define MAP_HEIGHT 10
#define MAX_WALLS 1024
#define NUM_RAYS 200
#define FOV 60.0f

int map[MAP_HEIGHT][MAP_WIDTH] = {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1}};

int buildWallsFromMap(Wall *walls, int maxWalls)
{
    int count = 0;
    for (int y = 0; y < MAP_HEIGHT; y++)
    {
        for (int x = 0; x < MAP_WIDTH; x++)
        {
            if (map[y][x] == 1)
            {
                Vec2 tl = {x * TILE_SIZE, y * TILE_SIZE};
                Vec2 tr = {(x + 1) * TILE_SIZE, y * TILE_SIZE};
                Vec2 bl = {x * TILE_SIZE, (y + 1) * TILE_SIZE};
                Vec2 br = {(x + 1) * TILE_SIZE, (y + 1) * TILE_SIZE};

                if (count + 4 < maxWalls)
                {
                    walls[count++] = (Wall){tl, tr};
                    walls[count++] = (Wall){tr, br};
                    walls[count++] = (Wall){br, bl};
                    walls[count++] = (Wall){bl, tl};
                }
            }
        }
    }
    return count;
}

void draw3DView(CollisionData **hits, int rayCount)
{
    for (int i = 0; i < rayCount; i++)
    {
        if (!hits[i])
            continue;

        float dist = hits[i]->d;
        float corrected = dist * cosf(DEG_TO_RAD(hits[i]->angle));  // Correct fisheye effect
        float wallHeight = (TILE_SIZE * SCREEN_HEIGHT) / corrected; // Wall height based on screen size

        float brightness = 255 - (dist * 0.5f);
        if (brightness < 0)
            brightness = 0;
        if (brightness > 255)
            brightness = 255;

        Color wallColor = (Color){brightness, brightness, brightness, 255};
        DrawRectangle(i, (SCREEN_HEIGHT / 2) - (wallHeight / 2), 1, wallHeight, wallColor);
    }
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Raycasting in raylib");
    SetTargetFPS(60);

    Player player = PLAYERINIT;

    Wall walls[MAX_WALLS];
    int wallCount = buildWallsFromMap(walls, MAX_WALLS);

    while (!WindowShouldClose())
    {
        rotateRight(&player);
        CollisionData **hits = multiRayShot(player.pos, player.dir, FOV, wallCount, walls, NUM_RAYS);

        BeginDrawing();
        ClearBackground(DARKBLUE);

        draw3DView(hits, NUM_RAYS);

        char buffer[64];

        sprintf(buffer, "HP: %d", player.hp);
        DrawText(buffer, SCREEN_WIDTH - 200, SCREEN_HEIGHT - 60, 20, BLACK);

        sprintf(buffer, "AMMO: %d", player.ammo);
        DrawText(buffer, SCREEN_WIDTH - 200, SCREEN_HEIGHT - 30, 20, BLACK);

        EndDrawing();

        freeCollisionData(hits, NUM_RAYS);
    }

    CloseWindow();
    return 0;
}
