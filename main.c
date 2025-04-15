#include "raylib.h"
#include "raycast.h"
#include <math.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define TILE_SIZE 64
#define MAP_WIDTH 10
#define MAP_HEIGHT 10
#define MAX_WALLS 1024
#define NUM_RAYS SCREEN_WIDTH  // One ray per column
#define FOV 60.0f

int map[MAP_HEIGHT][MAP_WIDTH] = {
    {1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,1},
    {1,0,1,0,1,0,1,0,0,1},
    {1,0,1,0,1,0,1,0,0,1},
    {1,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,1,1,1,1,0,1},
    {1,0,1,0,0,0,0,1,0,1},
    {1,0,1,0,1,1,0,1,0,1},
    {1,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1}
};

int buildWallsFromMap(Wall *walls, int maxWalls) {
    int count = 0;
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (map[y][x] == 1) {
                Vec2 tl = { x * TILE_SIZE, y * TILE_SIZE };
                Vec2 tr = { (x + 1) * TILE_SIZE, y * TILE_SIZE };
                Vec2 bl = { x * TILE_SIZE, (y + 1) * TILE_SIZE };
                Vec2 br = { (x + 1) * TILE_SIZE, (y + 1) * TILE_SIZE };

                if (count + 4 < maxWalls) {
                    walls[count++] = (Wall){ tl, tr };
                    walls[count++] = (Wall){ tr, br };
                    walls[count++] = (Wall){ br, bl };
                    walls[count++] = (Wall){ bl, tl };
                }
            }
        }
    }
    return count;
}

void updateCamera(Vec2 *pos, Vec2 *dir) {
    float speed = 2.0f;
    Vec2 forward = *dir;
    Vec2 strafe = { -dir->y, dir->x }; // Perpendicular to dir

    if (IsKeyDown(KEY_W)) vectorAdd(*pos, (Vec2){ forward.x * speed, forward.y * speed }, pos);
    if (IsKeyDown(KEY_S)) vectorAdd(*pos, (Vec2){ -forward.x * speed, -forward.y * speed }, pos);
    if (IsKeyDown(KEY_A)) vectorAdd(*pos, (Vec2){ -strafe.x * speed, -strafe.y * speed }, pos);
    if (IsKeyDown(KEY_D)) vectorAdd(*pos, (Vec2){ strafe.x * speed, strafe.y * speed }, pos);

    if (IsKeyDown(KEY_LEFT)) rotate(dir, -0.05f);
    if (IsKeyDown(KEY_RIGHT)) rotate(dir, 0.05f);
}

void draw3DView(CollisionData **hits, int rayCount) {
    for (int i = 0; i < rayCount; i++) {
        if (!hits[i]) continue;

        float dist = hits[i]->d;
        float corrected = dist * cosf(DEG_TO_RAD(hits[i]->angle)); // fix fisheye
        float wallHeight = (TILE_SIZE * 500) / corrected;

        float brightness = 255 - (dist * 0.5f);
        if (brightness < 0) brightness = 0;
        if (brightness > 255) brightness = 255;

        Color wallColor = (Color){ brightness, brightness, brightness, 255 };
        DrawRectangle(i, (SCREEN_HEIGHT / 2) - wallHeight / 2, 1, wallHeight, wallColor);
    }
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Raycasting in raylib");
    SetTargetFPS(60);

    Vec2 camPos = { 160.0f, 160.0f };
    Vec2 camDir = { 1.0f, 0.0f };

    Wall walls[MAX_WALLS];
    int wallCount = buildWallsFromMap(walls, MAX_WALLS);

    while (!WindowShouldClose()) {
        updateCamera(&camPos, &camDir);

        CollisionData **hits = multiRayShot(camPos, camDir, FOV, wallCount, walls, NUM_RAYS);

        BeginDrawing();
        ClearBackground(DARKBLUE);

        draw3DView(hits, NUM_RAYS);
        DrawText("Use WSAD + arrow keys", 20, 20, 20, RAYWHITE);

        EndDrawing();

        freeCollisionData(hits, NUM_RAYS);
    }

    CloseWindow();
    return 0;
}
