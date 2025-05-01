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

void draw3DView(CollisionData **hits, int rayCount)
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
    }
    
}

void drawFloorAndRoof(Color *floorPixels, Color *roofPixels, Player player, Color *renderPixels, Image floorImage, Image roofImage)
{
    float fovRad = DEG_TO_RAD(FOV);
    float halfScreenHeight = SCREEN_HEIGHT / 2.0f;

    // Texture scale factor (adjust as needed)
    float textureScale = 8.0f;

    // Loop over each row of the screen (lower half for floor, upper half for ceiling)
    for (int y = halfScreenHeight; y < SCREEN_HEIGHT; y++)
    {
        float rowDistance = (TILE_SIZE * halfScreenHeight) / (y - halfScreenHeight);

        // Calculate the direction of the ray at the left and right edges of the FOV
        Vec2 rayDirLeft = {
            player.dir.x - player.dir.y * tanf(fovRad / 2),
            player.dir.y + player.dir.x * tanf(fovRad / 2)};
        Vec2 rayDirRight = {
            player.dir.x + player.dir.y * tanf(fovRad / 2),
            player.dir.y - player.dir.x * tanf(fovRad / 2)};

        for (int x = 0; x < SCREEN_WIDTH; x++)
        {
            // Calculate the direction of the ray for this column
            float cameraX = (float)x / SCREEN_WIDTH;
            Vec2 rayDir = {
                rayDirLeft.x + cameraX * (rayDirRight.x - rayDirLeft.x),
                rayDirLeft.y + cameraX * (rayDirRight.y - rayDirLeft.y)};

            // Calculate the floor and ceiling texture coordinates based on the ray direction
            // Normalize the ray direction to get texture coordinates
            float tileX = rayDir.x * rowDistance;
            float tileY = rayDir.y * rowDistance;

            // Use modulo for texture wrapping and scaling
            int texX = (int)(tileX * textureScale) % floorImage.width;
            int texY = (int)(tileY * textureScale) % floorImage.height;

            // Ensure texX and texY are within bounds
            if (texX < 0) texX += floorImage.width;
            if (texY < 0) texY += floorImage.height;

            // Sample the floor texture
            Color floorColor = GetImageColor(floorImage, texX, texY);
            renderPixels[y * SCREEN_WIDTH + x] = floorColor;

            // Sample the ceiling texture (same as floor but mirrored)
            if (roofPixels)
            {
                Color ceilColor = GetImageColor(roofImage, texX, texY);
                renderPixels[(SCREEN_HEIGHT - y - 1) * SCREEN_WIDTH + x] = ceilColor;
            }
        }
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

    Color *renderBuffer = malloc(SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(Color));
    Image screenImage = GenImageColor(SCREEN_WIDTH, SCREEN_HEIGHT, BLACK); // dummy image
    Texture2D screenTexture = LoadTextureFromImage(screenImage);
    UnloadImage(screenImage); // Texture now owns the pixel memory


    Player player = PLAYERINIT;

    Map *mp = loadMap("testmap1.csv");
    
    Image floorImage = LoadImage("Sprites/Ground.png");
    Color *floorPixels = LoadImageColors(floorImage);
    
    Image roofImage = LoadImage("Sprites/Sky.png");
    Color *roofPixels = LoadImageColors(roofImage);

    Image floorRender = GenImageColor(SCREEN_WIDTH, SCREEN_HEIGHT, BLANK);
    Color *renderPixels = LoadImageColors(floorRender);  // ← allocates memory once
    Texture2D floorTexture = LoadTextureFromImage(floorRender);  // send to GPU once
    

    Weapon *weapons = getWeapons();

    Enemy **projectiles = weapons[2].projectiles; // Contains all the projectiles from the projectile weapon.

    int currentwpn = 0;

    while (!WindowShouldClose())
    {
        if (weapons[currentwpn].currentCooldown > 0)
        {
            weapons[currentwpn].currentCooldown--;
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

        if (IsKeyDown(KEY_SPACE) && weapons[currentwpn].currentCooldown == 0 && weapons[currentwpn].ammo > 0)
        {
            attackEnemy(&weapons[currentwpn], &player, mp);
        }
        if (IsKeyDown('1'))
        {
            currentwpn = 0;
        }
        if (IsKeyDown('2'))
        {
            currentwpn = 1;
        }
        if (IsKeyDown('3'))
        {
            currentwpn = 2;
        }
        if (IsKeyDown('1'))
        {
            currentwpn = 0;
        }
        if (IsKeyDown('2'))
        {
            currentwpn = 1;
        }
        if (IsKeyDown('3'))
        {
            currentwpn = 2;
        }
        if (IsKeyDown('Q'))
        {
            weapons[currentwpn].currentCooldown = 1;
        }
        if (IsKeyDown('E'))
        {
            weapons[currentwpn].currentCooldown = 0;
        }

        executeMovement(&player, mp->walls, mp->numOfWalls);

        CollisionData **hits = multiRayShot(player.pos, player.dir, FOV, mp->numOfWalls, mp->walls, NUM_RAYS); // Gets wall CollisionData

        CollisionData **enemyData = rayShotEnemies(player, FOV, mp, mp->enemies, mp->enemyCount); // Gets enemy CollisionData

        CollisionData **projectileData = rayShotProjectile(player, FOV, mp, projectiles); // Gets projectile CollisionData

        BeginDrawing();
        ClearBackground(BLACK);

        drawFloorAndRoof(floorPixels, roofPixels, player, renderBuffer, floorImage, roofImage);
        UpdateTexture(screenTexture, renderBuffer);
        DrawTexture(screenTexture, 0, 0, WHITE);


        draw3DView(hits, NUM_RAYS);

        drawEnemies(player, enemyData, mp->enemyCount);

        drawEnemies(player, enemyData, mp->enemyCount);
        updateEnemies(mp->enemies, mp->enemyCount, &player, 60, FOV, mp);

        drawEnemies(player, enemyData, mp->enemyCount);
        updateEnemies(mp->enemies, mp->enemyCount, &player, 60, FOV, mp); // Yes we know it's a repeat. It looks better like this for now

        drawWeapon(weapons, currentwpn);
        updateProjectiles(projectiles, player, mp->enemies, mp->enemyCount, &weapons[2]);
        drawEnemies(player, projectileData, MAXPROJECTILES);

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
    }

    // --- Shutdown / Cleanup ---
    UnloadImageColors(renderPixels);
    UnloadImageColors(floorPixels);
    UnloadImageColors(roofPixels);
    UnloadImage(floorRender);
    UnloadImage(floorImage);
    UnloadImage(roofImage);
    UnloadTexture(floorTexture);
    free(renderBuffer);
    UnloadTexture(screenTexture);


    CloseWindow();

    return 0;
}
