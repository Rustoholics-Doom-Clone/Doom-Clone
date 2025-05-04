#include "raylib.h"
#include "raycast.h"
#include "movement.h"
#include "map.h"
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define MAP_WIDTH 10
#define MAP_HEIGHT 10
#define MAX_WALLS 1024
#define NUM_RAYS 200
#define FOV 60.0f

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

void drawScene(Player p1, CollisionData **enemyColl, int enemycount, CollisionData **wallhits, int raycount, CollisionData **projectileData, Image *floorImage, Texture2D *floorTextureBuffer, Image floorTexture, Image roofTexture)
{

    CollisionData **allData = malloc(sizeof(CollisionData *) * (enemycount + raycount + MAXPROJECTILES));
    if (!allData)
        return;
    memcpy(allData, enemyColl, enemycount * sizeof(CollisionData *));
    memcpy(allData + enemycount, wallhits, raycount * sizeof(CollisionData *));
    memcpy(allData + enemycount + raycount, projectileData, MAXPROJECTILES * sizeof(CollisionData *));

    qsort(allData, (enemycount + raycount + MAXPROJECTILES), sizeof(CollisionData *), compareEnemyDistance);

    Color *pixels = floorImage->data; // Pointer to the Image pixel buffer

    Vec2 plane = {
        -p1.dir.y * tanf(DEG_TO_RAD(FOV / 2)),
        p1.dir.x * tanf(DEG_TO_RAD(FOV / 2))};

    float posScale = 0.0150f; // Achieved through trial and error. If the floor is moving to much in the same direction as the player. I.e moves infront of the player. This is to low and vice versa

    for (int y = SCREEN_HEIGHT / 2 + 1; y < SCREEN_HEIGHT; y++)
    {
        float rayDirX0 = p1.dir.x - plane.x;
        float rayDirY0 = p1.dir.y - plane.y;
        float rayDirX1 = p1.dir.x + plane.x;
        float rayDirY1 = p1.dir.y + plane.y;

        float rowDistance = (float)SCREEN_HEIGHT / (2.0f * y - SCREEN_HEIGHT);
        float stepX = rowDistance * (rayDirX1 - rayDirX0) / SCREEN_WIDTH;
        float stepY = rowDistance * (rayDirY1 - rayDirY0) / SCREEN_WIDTH;

        float floorX = p1.pos.x * posScale + rowDistance * rayDirX0;
        float floorY = p1.pos.y * posScale + rowDistance * rayDirY0;

        for (int x = 0; x < SCREEN_WIDTH; ++x)
        {

            // float scaleFactor = 0.8f;

            float repeatScale = 1.0f; // how much world space each texture tile covers

            int tx = (int)((floorX / repeatScale) * floorTexture.width) % floorTexture.width;
            int ty = (int)((floorY / repeatScale) * floorTexture.height) % floorTexture.height;

            // Ensure wrapping is safe for negative values
            tx = (tx + floorTexture.width) % floorTexture.width;
            ty = (ty + floorTexture.height) % floorTexture.height;

            int ceilingY = SCREEN_HEIGHT - y;

            // Get the floor and ceiling colors
            Color floorColor = GetImageColor(floorTexture, tx, ty);
            Color ceilingColor = GetImageColor(roofTexture, tx, ty);

            // Set the pixels in the Image data directly (faster than DrawPixel)
            pixels[y * SCREEN_WIDTH + x] = floorColor;
            pixels[ceilingY * SCREEN_WIDTH + x] = ceilingColor;

            floorX += stepX;
            floorY += stepY;
        }
    }

    // After updating the floorImage, we update the floorTextureBuffer
    UpdateTexture(*floorTextureBuffer, floorImage->data);

    // Draw the modified floorImage (both floor and ceiling) to the screen
    DrawTexture(*floorTextureBuffer, 0, 0, WHITE); // You can adjust the position here

    int wallSliceIndex = 0;
    for (int c = 0; c < (enemycount + raycount + MAXPROJECTILES); c++)
    {
        if (!allData[c])
            continue;

        switch (isnan(allData[c]->textureOffset))
        {
        case 1: // Not a wall
        {
            Vec2 enemyPos = allData[c]->position;

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

            Texture2D sprite = allData[c]->texture;

            // Preserve sprite aspect ratio
            float aspectRatio = (float)sprite.width / (float)sprite.height;

            float dist = allData[c]->d;
            float corrected = dist * allData[c]->angle;                 // Correct fisheye effect
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
            break;
        }
        default: // A wall
        {

            float dist = allData[c]->d;
            float corrected = dist * cosf(DEG_TO_RAD(allData[c]->angle)); // Correct fisheye effect
            float wallHeight = ((TILE_SIZE * SCREEN_HEIGHT) / corrected); // Wall height based on screen size

            Texture2D texture = allData[c]->texture;

            float sliceWidth = (float)SCREEN_WIDTH / NUM_RAYS;
            float screenX = allData[c]->id * sliceWidth;
            float wallTop = (SCREEN_HEIGHT / 2.0f) - (wallHeight / 2.0f);
            float wallBottom = wallTop + wallHeight;

            // --- Draw walls ---
            float texX = allData[c]->textureOffset * texture.width;
            // Source rectangle: a vertical slice of the wall texture
            Rectangle source = {
                texX,
                0,
                1,
                (float)texture.height};

            // Destination rectangle: the scaled vertical slice on screen
            Rectangle destination = {
                screenX, // X on screen
                (SCREEN_HEIGHT / 2.0f) - (wallHeight / 2.0f),
                sliceWidth, // stretches pixels in source retangel to slicewith
                wallHeight};

            DrawTexturePro(texture, source, destination, (Vector2){0, 0}, 0.0f, WHITE);

            wallSliceIndex++;
        }

        break;
        }
    }
    free(allData);
}

void drawWeapon(Weapon *wpns, int wpnid)
{
    switch (wpns[wpnid].currentCooldown) // draws Different sprite depending on cooldown
    {
    case 0:
    {
        Rectangle src = {
            0, 0,
            (float)wpns[wpnid].normalSprite.width,
            (float)wpns[wpnid].normalSprite.height};

        Rectangle dest = {
            wpns[wpnid].screenPos.x,
            SCREEN_HEIGHT - (wpns[wpnid].normalSprite.height * wpns[wpnid].normalScale.y) + wpns[wpnid].screenPos.y,
            wpns[wpnid].normalSprite.width * wpns[wpnid].normalScale.x,
            wpns[wpnid].normalSprite.height * wpns[wpnid].normalScale.y};

        DrawTexturePro(wpns[wpnid].normalSprite, src, dest, (Vector2){0, 0}, 0.0f, WHITE);
        break;
    }
    default:
    {
        Rectangle src = {
            0, 0,
            (float)wpns[wpnid].shootingSprite.width,
            (float)wpns[wpnid].shootingSprite.height};

        Rectangle dest = {
            wpns[wpnid].screenPos.x,
            SCREEN_HEIGHT - (wpns[wpnid].shootingSprite.height * wpns[wpnid].shootingScale.y) + wpns[wpnid].screenPos.y,
            wpns[wpnid].shootingSprite.width * wpns[wpnid].shootingScale.x,
            wpns[wpnid].shootingSprite.height * wpns[wpnid].shootingScale.y};

        DrawTexturePro(wpns[wpnid].shootingSprite, src, dest, (Vector2){0, 0}, 0.0f, WHITE);
        break;
    }
    }
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Raycasting in raylib");
    SetTargetFPS(60);
    srand(time(NULL));

    Player player = PLAYERINIT;

    Map *mp = loadMap("testmap1.csv");

    Image floorImage = GenImageColor(SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
    Texture2D floorTextureBuffer = LoadTextureFromImage(floorImage);

    Image floorTexture = LoadImage("Sprites/Ground.png");
    Image roofTexture = LoadImage("Sprites/Sky.png");

    Weapon *weapons = getWeapons(SCREEN_WIDTH, SCREEN_HEIGHT);

    Enemy **projectiles = weapons[2].projectiles; // Contains all the projectiles from the projectile weapon.

    int currentwpn = 0;

    while (!WindowShouldClose())
    {
        if (weapons[currentwpn].currentCooldown > 0)
            weapons[currentwpn].currentCooldown--;
        if (IsKeyDown(KEY_RIGHT))
            rotateRight(&player);
        if (IsKeyDown(KEY_LEFT))
            rotateLeft(&player);
        if (IsKeyDown('W'))
            wishMoveForward(&player);
        if (IsKeyDown('A'))
            wishMoveLeft(&player);
        if (IsKeyDown('S'))
            wishMoveBack(&player);
        if (IsKeyDown('D'))
            wishMoveRight(&player);
        if (IsKeyDown(KEY_SPACE) && weapons[currentwpn].currentCooldown == 0 && weapons[currentwpn].ammo > 0)
            attackEnemy(&weapons[currentwpn], &player, mp);
        if (IsKeyDown('1'))
            currentwpn = 0;
        if (IsKeyDown('2'))
            currentwpn = 1;
        if (IsKeyDown('3'))
            currentwpn = 2;
        if (IsKeyDown('1'))
            currentwpn = 0;
        if (IsKeyDown('2'))
            currentwpn = 1;
        if (IsKeyDown('3'))
            currentwpn = 2;
        if (IsKeyDown('Q'))
            weapons[currentwpn].currentCooldown = 1;
        if (IsKeyDown('E'))
            weapons[currentwpn].currentCooldown = 0;

        executeMovement(&player, mp->walls, mp->numOfWalls);

        CollisionData **hits = multiRayShot(player.pos, player.dir, FOV, mp->numOfWalls, mp->walls, NUM_RAYS); // Gets wall CollisionData

        CollisionData **enemyData = rayShotEnemies(player, FOV, mp, mp->enemies, mp->enemyCount); // Gets enemy CollisionData

        CollisionData **projectileData = rayShotProjectile(player, FOV, mp, projectiles); // Gets projectile CollisionData

        BeginDrawing();
        ClearBackground(BLACK);

        drawScene(player, enemyData, mp->enemyCount, hits, NUM_RAYS, projectileData, &floorImage, &floorTextureBuffer, floorTexture, roofTexture);

        updateEnemies(mp->enemies, mp->enemyCount, &player, 60, FOV, mp, mp->walls, mp->numOfWalls);

        updateEnemies(mp->enemies, mp->enemyCount, &player, 60, FOV, mp, mp->walls, mp->numOfWalls); // Yes we know it's a repeat. It looks better like this for now

        drawWeapon(weapons, currentwpn);
        updateProjectiles(projectiles, player, mp->enemies, mp->enemyCount, &weapons[2]);

        drawWeapon(weapons, currentwpn);

        char buffer[64];
        sprintf(buffer, "HP: %d", player.hp);
        DrawText(buffer, SCREEN_WIDTH - 200, SCREEN_HEIGHT - 60, 20, BLACK);

        sprintf(buffer, "+");
        DrawText(buffer, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 20, (Color){245, 40, 145, 204});

        sprintf(buffer, "AMMO: %d", weapons[currentwpn].ammo);
        DrawText(buffer, SCREEN_WIDTH - 200, SCREEN_HEIGHT - 30, 20, BLACK);

        EndDrawing();

        freeCollisionData(hits, NUM_RAYS);
        freeCollisionData(enemyData, mp->enemyCount);
        freeCollisionData(projectileData, MAXPROJECTILES);
    }

    // --- Shutdown / Cleanup ---

    UnloadImage(floorImage);
    UnloadImage(roofTexture);
    UnloadTexture(floorTextureBuffer);
    UnloadImage(floorTexture);

    CloseWindow();

    return 0;
}
