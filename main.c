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
#define NUM_MAPS 4

typedef enum
{
    MAINMENU,
    GAMEPLAY,
    PAUSEMENU,
    ENDSCREEN,
    DEATHSCREEN,
    THEEND
} GameState;

const char *Maps[] = {
    "Maps/map1.csv",
    "Maps/map2.csv",
    "Maps/Map3.csv",
    "Maps/Map4.csv"};

Color CERISE = {230, 65, 133, 255};

int compareEnemyDistance(const void *a, const void *b)
{
    // Load in thwo collisiondata pointers
    CollisionData *f1 = *(CollisionData **)a;
    CollisionData *f2 = *(CollisionData **)b;
    // Compare their distance
    if (!f1 || !f2)
        return -1;
    float cmp = f1->d - f2->d;
    if (cmp == 0.0)
        return 0;
    if (cmp < 0.0)
        return 1;
    return -1;
}

void drawScene(Player p1, CollisionData **enemyColl, int enemycount, CollisionData **wallhits, int raycount, CollisionData **projectileData, Image *floorImage, Texture2D *floorTextureBuffer, Image floorTexture, Image roofTexture)
{

    // Group all the collisiondata into one huge array
    CollisionData **allData = malloc(sizeof(CollisionData *) * (enemycount + raycount + MAXPROJECTILES));
    if (!allData)
        return;
    memcpy(allData, enemyColl, enemycount * sizeof(CollisionData *));
    memcpy(allData + enemycount, wallhits, raycount * sizeof(CollisionData *));
    memcpy(allData + enemycount + raycount, projectileData, MAXPROJECTILES * sizeof(CollisionData *));

    // quicksort the array based on the distance of the collisions
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

    // After updating the floorImage, update the floorTextureBuffer
    UpdateTexture(*floorTextureBuffer, floorImage->data);

    // Draw the modified floorImage (both floor and ceiling) to the screen
    DrawTexture(*floorTextureBuffer, 0, 0, WHITE); // You can adjust the position here

    for (int c = 0; c < (enemycount + raycount + MAXPROJECTILES); c++)
    {
        if (!allData[c]) // skip null data
            continue;

        switch (isnan(allData[c]->textureOffset)) // Collisions with non wall objects have textureOffset as Nan
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
            float spriteHeight = spritesScale * (SCREEN_HEIGHT / transformY) * 1.8f * ((float)sprite.height / 64.0); // 1.8 = tune to taste
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
        }

        break;
        }
    }
    free(allData); // Since we memcpy the only thing stored is pointers to the other pointers and thus the data itself will be freed later
}

void drawWeapon(Weapon *wpns, int wpnid)
{
    switch (wpns[wpnid].currentCooldown > (wpns[wpnid].baseCooldown / 2)) // draws Different sprite depending on cooldown
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

// These are hud elements. They are global so that they don't have to be passed by reference or reloaded into graphics memory every time the hud is drawn
Texture2D wpnslct1;
Texture2D wpnslct2;
Texture2D wpnslct3;
Texture2D kngligDoomGuy;
Font jupiter;

void drawHud(Player player, Weapon wpn, int wpnn)
{

    float hudHeightScale = 0.8f * (float)SCREEN_HEIGHT / 1080.0;

    DrawRectangle(0, (SCREEN_HEIGHT - 90 * hudHeightScale) - 4, SCREEN_WIDTH, 90 * hudHeightScale, WHITE);
    DrawRectangle(0, SCREEN_HEIGHT - 90 * hudHeightScale, SCREEN_WIDTH, 90 * hudHeightScale, CERISE);

    Rectangle src = {
        0, 0, kngligDoomGuy.width, kngligDoomGuy.height};
    Rectangle dest = {
        (SCREEN_WIDTH - kngligDoomGuy.width * hudHeightScale) / 2,
        SCREEN_HEIGHT - hudHeightScale * kngligDoomGuy.height,
        kngligDoomGuy.width * hudHeightScale,
        kngligDoomGuy.height * hudHeightScale};
    DrawTexturePro(kngligDoomGuy, src, dest, (Vector2){0.0, 0.0}, 0.0f, WHITE);

    src = (Rectangle){0, 0, wpnslct1.width, wpnslct1.height};
    dest = (Rectangle){(SCREEN_WIDTH + kngligDoomGuy.width * hudHeightScale) / 2, SCREEN_HEIGHT - hudHeightScale * kngligDoomGuy.height, wpnslct1.width, wpnslct1.height};

    switch (wpnn)
    {
    case 0:
        DrawTexturePro(wpnslct1, src, dest, (Vector2){0.0, 0.0}, 0.0f, WHITE);
        break;
    case 1:
        DrawTexturePro(wpnslct2, src, dest, (Vector2){0.0, 0.0}, 0.0f, WHITE);
        break;
    case 2:
        DrawTexturePro(wpnslct3, src, dest, (Vector2){0.0, 0.0}, 0.0f, WHITE);
        break;
    default:
        break;
    }

    DrawRectangle(((SCREEN_WIDTH + kngligDoomGuy.width * hudHeightScale) / 2) + wpnslct1.width + 4, SCREEN_HEIGHT - 90 * hudHeightScale + 4, 300, 90 * hudHeightScale - 8, BLACK);
    DrawRectangle(((SCREEN_WIDTH - kngligDoomGuy.width * hudHeightScale) / 2) - 204, SCREEN_HEIGHT - 90 * hudHeightScale + 4, 200, 90 * hudHeightScale - 8, BLACK);

    char buffer[64];
    sprintf(buffer, "HP: %d", player.hp);
    // DrawText(buffer, SCREEN_WIDTH - 200, SCREEN_HEIGHT - 60, 20, BLACK);
    DrawTextEx(jupiter, buffer, (Vector2){((SCREEN_WIDTH - kngligDoomGuy.width * hudHeightScale) / 2) - 200, SCREEN_HEIGHT - 90 * hudHeightScale + 4}, 75, 2, RED);

    sprintf(buffer, "+");
    DrawText(buffer, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 20, (Color){245, 40, 145, 204});

    if (wpnn == 0)
        sprintf(buffer, "AMMO: inf");
    else
        sprintf(buffer, "AMMO: %d", wpn.ammo);

    DrawTextEx(jupiter, buffer, (Vector2){((SCREEN_WIDTH + kngligDoomGuy.width * hudHeightScale) / 2) + wpnslct1.width + 8, SCREEN_HEIGHT - 90 * hudHeightScale + 4}, 75, 2, RED);
}

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Raycasting in raylib");
    SetTargetFPS(60);
    srand(time(NULL));
    SetExitKey(KEY_BACKSPACE);
    ToggleFullscreen();
    HideCursor();
    Player player = PLAYERINIT;
    GameState gameState = MAINMENU;

    Map *mp = loadMap("Maps/map1.csv");

    Font font = LoadFont("Sprites/Fonts/setback.png");

    wpnslct1 = LoadTexture("Sprites/HUD/Weaponselect1.png");
    wpnslct2 = LoadTexture("Sprites/HUD/Weaponselect2.png");
    wpnslct3 = LoadTexture("Sprites/HUD/Weaponselect3.png");
    kngligDoomGuy = LoadTexture("Sprites/HUD/85ed57ab85bbe08a0edfd3cfa5edfc38.jpg");
    jupiter = LoadFont("Sprites/HUD/fonts/jupiter_crash.png");

    Image floorImage = GenImageColor(SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
    Texture2D floorTextureBuffer = LoadTextureFromImage(floorImage);

    Image floorTexture = LoadImage("Sprites/Ground.png");
    Image roofTexture = LoadImage("Sprites/Sky.png");

    Weapon *weapons;

    int currentMap = 0;
    int currentwpn = 0;
    const char *exit = "Exit game [ Backspace ]";
    const char *ret = "Main Menu [ Esc ]";

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(BLACK);

        CollisionData **hits = multiRayShot(player.pos, player.dir, FOV, mp->numOfWalls, mp->walls, NUM_RAYS); // Gets wall CollisionData

        CollisionData **enemyData = rayShotEnemies(player, FOV, mp, mp->enemies, mp->enemyCount); // Gets enemy CollisionData

        CollisionData **projectileData = rayShotProjectile(player, FOV, mp, mp->projectiles); // Gets projectile CollisionData

        switch (gameState)
        {
        case MAINMENU:

            if (IsKeyPressed(KEY_ENTER))
            {
                gameState = GAMEPLAY;
                player = PLAYERINIT;
                freeMap(mp);
                mp = loadMap(Maps[currentMap]); // This is very inefficient, but I don't know how to reset a map in a better way
                weapons = getWeapons(SCREEN_WIDTH, SCREEN_HEIGHT, mp->projectiles);
                currentwpn = 0;
            }

            rotate(&player.dir, ROTSPEED / 10);
            drawScene(player, enemyData, mp->enemyCount, hits, NUM_RAYS, projectileData, &floorImage, &floorTextureBuffer, floorTexture, roofTexture);

            const char *title = "Schlem on Campus";
            const char *start = "Start Game [ Enter ]";
            DrawTextEx(font, title, (Vector2){SCREEN_WIDTH / 2 - MeasureTextEx(font, title, font.baseSize * 10, 5).x / 2, SCREEN_HEIGHT / 6}, font.baseSize * 10, 10, BLACK);
            DrawTextEx(font, start, (Vector2){SCREEN_WIDTH / 2 - MeasureTextEx(font, start, font.baseSize * 5, 5).x / 2, SCREEN_HEIGHT / 2}, font.baseSize * 5, 5, BLACK);
            DrawTextEx(font, exit, (Vector2){SCREEN_WIDTH / 2 - MeasureTextEx(font, exit, font.baseSize * 5, 5).x / 2, SCREEN_HEIGHT / 2 + font.baseSize * 5}, font.baseSize * 5, 5, BLACK);
            break;

        case GAMEPLAY:
            if (weapons[currentwpn].currentCooldown > 0)
                weapons[currentwpn].currentCooldown--;

            if (IsKeyPressed(KEY_ESCAPE))
            {
                gameState = PAUSEMENU;
            }

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
            if (IsKeyDown('Q'))
                weapons[currentwpn].currentCooldown = 1;
            if (IsKeyDown('E'))
                weapons[currentwpn].currentCooldown = 0;

            executeMovement(&player, mp->walls, mp->numOfWalls);

            int deadEnemies = 0;
            for (int i = 0; i < mp->enemyCount; i++)
            {
                if (mp->enemies[i].status == DEAD)
                {
                    deadEnemies++;
                }
            }
            if (deadEnemies == mp->enemyCount)
            {
                gameState = ENDSCREEN;
            }
            if (player.hp <= 0)
            {
                gameState = DEATHSCREEN;
            }

            drawScene(player, enemyData, mp->enemyCount, hits, NUM_RAYS, projectileData, &floorImage, &floorTextureBuffer, floorTexture, roofTexture);

            updateEnemies(mp->enemies, mp->enemyCount, &player, 60, FOV, mp, mp->walls, mp->numOfWalls);

            updateEnemies(mp->enemies, mp->enemyCount, &player, 60, FOV, mp, mp->walls, mp->numOfWalls); // Yes we know it's a repeat. It looks better like this for now

            drawWeapon(weapons, currentwpn);
            updateProjectiles(mp->projectiles, &player, mp->enemies, mp->enemyCount, &weapons[2], &mp->ppointer);

            drawWeapon(weapons, currentwpn);

            drawHud(player, weapons[currentwpn], currentwpn);

            break;

        case PAUSEMENU:

            if (IsKeyPressed(KEY_ESCAPE))
            {
                gameState = GAMEPLAY;
            }
            if (IsKeyPressed(KEY_ENTER))
            {
                player = PLAYERINIT;
                gameState = MAINMENU;
            }

            drawScene(player, enemyData, mp->enemyCount, hits, NUM_RAYS, projectileData, &floorImage, &floorTextureBuffer, floorTexture, roofTexture);
            drawWeapon(weapons, currentwpn);
            drawHud(player, weapons[currentwpn], currentwpn);

            const char *resume = "Resume [ Esc ]";
            const char *main = "Main Menu [ Enter ]";
            DrawTextEx(font, resume, (Vector2){SCREEN_WIDTH / 2 - MeasureTextEx(font, resume, font.baseSize * 5, 5).x / 2, SCREEN_HEIGHT / 6}, font.baseSize * 5, 5, BLACK);
            DrawTextEx(font, main, (Vector2){SCREEN_WIDTH / 2 - MeasureTextEx(font, main, font.baseSize * 5, 5).x / 2, SCREEN_HEIGHT / 6 + font.baseSize * 5}, font.baseSize * 5, 5, BLACK);
            DrawTextEx(font, exit, (Vector2){SCREEN_WIDTH / 2 - MeasureTextEx(font, exit, font.baseSize * 5, 5).x / 2, SCREEN_HEIGHT / 6 + font.baseSize * 10}, font.baseSize * 5, 5, BLACK);
            break;

        case ENDSCREEN:

            if (IsKeyPressed(KEY_ESCAPE))
            {
                gameState = MAINMENU;
            }
            if (IsKeyPressed(KEY_ENTER))
            {
                currentMap++; // Advance to next map
                gameState = GAMEPLAY;
                player = PLAYERINIT; // Reset player

                // Free data before mp changes in order to avoid memory leaks and segmentation faults
                freeCollisionData(hits, NUM_RAYS);
                freeCollisionData(enemyData, mp->enemyCount);
                freeCollisionData(projectileData, MAXPROJECTILES);
                free(weapons);
                EndDrawing();

                freeMap(mp);                    // Unload old map
                mp = loadMap(Maps[currentMap]); // load next Map
                weapons = getWeapons(SCREEN_WIDTH, SCREEN_HEIGHT, mp->projectiles);
                currentwpn = 0;

                continue; // Only one should be needed
                break;    // Extra just in case
            }

            drawScene(player, enemyData, mp->enemyCount, hits, NUM_RAYS, projectileData, &floorImage, &floorTextureBuffer, floorTexture, roofTexture);
            drawWeapon(weapons, currentwpn);
            drawHud(player, weapons[currentwpn], currentwpn);

            const char *next = "Next level [ Enter ]";
            DrawTextEx(font, next, (Vector2){SCREEN_WIDTH / 2 - MeasureTextEx(font, next, font.baseSize * 5, 5).x / 2, SCREEN_HEIGHT / 6}, font.baseSize * 5, 5, BLACK);
            DrawTextEx(font, ret, (Vector2){SCREEN_WIDTH / 2 - MeasureTextEx(font, ret, font.baseSize * 5, 5).x / 2, SCREEN_HEIGHT / 6 + font.baseSize * 5}, font.baseSize * 5, 5, BLACK);
            DrawTextEx(font, exit, (Vector2){SCREEN_WIDTH / 2 - MeasureTextEx(font, exit, font.baseSize * 5, 5).x / 2, SCREEN_HEIGHT / 6 + font.baseSize * 10}, font.baseSize * 5, 5, BLACK);
            break;

        case DEATHSCREEN:
            if (IsKeyPressed(KEY_ESCAPE))
            {
                gameState = MAINMENU;
            }
            if (IsKeyPressed(KEY_ENTER))
            {
                currentMap = 0; // Advance to next map
                gameState = GAMEPLAY;
                player = PLAYERINIT; // Reset player

                // Free data before mp changes in order to avoid memory leaks and segmentation faults
                freeCollisionData(hits, NUM_RAYS);
                freeCollisionData(enemyData, mp->enemyCount);
                freeCollisionData(projectileData, MAXPROJECTILES);
                free(weapons);
                EndDrawing();

                freeMap(mp);                    // Unload old map
                mp = loadMap(Maps[currentMap]); // load next Map
                weapons = getWeapons(SCREEN_WIDTH, SCREEN_HEIGHT, mp->projectiles);
                currentwpn = 0;

                continue; // Only one should be needed
                break;    // Extra just in case
            }

            drawScene(player, enemyData, mp->enemyCount, hits, NUM_RAYS, projectileData, &floorImage, &floorTextureBuffer, floorTexture, roofTexture);
            drawWeapon(weapons, currentwpn);
            drawHud(player, weapons[currentwpn], currentwpn);

            const char *dead = "YOU DIED";
            const char *retry = "Retry Level [ Enter ]";
            DrawTextEx(font, dead, (Vector2){SCREEN_WIDTH / 2 - MeasureTextEx(font, dead, font.baseSize * 8, 5).x / 2, SCREEN_HEIGHT / 10}, font.baseSize * 8, 8, BLACK);
            DrawTextEx(font, retry, (Vector2){SCREEN_WIDTH / 2 - MeasureTextEx(font, retry, font.baseSize * 5, 5).x / 2, SCREEN_HEIGHT / 6 + font.baseSize * 5}, font.baseSize * 5, 5, BLACK);
            DrawTextEx(font, ret, (Vector2){SCREEN_WIDTH / 2 - MeasureTextEx(font, ret, font.baseSize * 5, 5).x / 2, SCREEN_HEIGHT / 6 + font.baseSize * 10}, font.baseSize * 5, 5, BLACK);
            DrawTextEx(font, exit, (Vector2){SCREEN_WIDTH / 2 - MeasureTextEx(font, exit, font.baseSize * 5, 5).x / 2, SCREEN_HEIGHT / 6 + font.baseSize * 15}, font.baseSize * 5, 5, BLACK);
            break;

        case THEEND:
            break;
        default:
            break;
        }
        freeCollisionData(hits, NUM_RAYS);
        freeCollisionData(enemyData, mp->enemyCount);
        freeCollisionData(projectileData, MAXPROJECTILES);
        EndDrawing();
    }

    // --- Shutdown / Cleanup ---

    UnloadImage(floorImage);
    UnloadImage(roofTexture);
    UnloadTexture(floorTextureBuffer);
    UnloadImage(floorTexture);

    freeMap(mp);

    CloseWindow();

    return 0;
}
