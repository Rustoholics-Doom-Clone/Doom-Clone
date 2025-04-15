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

void freeMap(Map *m)
{
    if (m->walls)
        free(m->walls);
    if (m->enemyPos)
        free(m->enemyPos);
    free(m);
}