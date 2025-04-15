#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "map.h"

int saveMap(int numOfWalls, Wall *walls, char *filename)
{
    FILE *mfile = fopen(filename, "w");
    if (!mfile)
        return 0;
    fprintf(mfile, "%d,0\n", numOfWalls);
    for (int i = 0; i < numOfWalls; i++)
        fprintf(mfile, "%f,%f\n", walls[i].start, walls[i].stop);
    fclose(mfile);
    return 1;
}

Map *loadMap(char *filename)
{
    FILE *mfile = fopen(filename, "r");
    if (!mfile)
        return NULL;
    Map *result = malloc(sizeof(Map));
    if (!result)
        return NULL;

    char buffer[50];
    if (!fgets(buffer, sizeof(buffer), mfile))
    {
        fclose(mfile);
        free(result);
        return NULL;
    }
    int nwalls, nenemy;
    sscanf(buffer, "%d,%d", &nwalls, &nenemy);

    result->walls = malloc(sizeof(Wall) * nwalls);
    if (!result->walls)
    {
        fclose(mfile);
        free(result);
        return NULL;
    }
    // Enemies here. TODO!

    for (int i = 0; i < nwalls && fgets(buffer, sizeof(buffer), mfile); i++)
        sscanf(buffer, "%f,%f", &result->walls[i].start, &result->walls[i].stop);

    fclose(mfile);
    return result;
}

void freeMap(Map *m)
{
    if (m->walls)
        free(m->walls);
    if (m->enemyPos)
        free(m->enemyPos);
    free(m);
}