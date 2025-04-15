#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include "map.h"

void freeMap(Map *m)
{
    if (m->walls)
        free(m->walls);
    if (m->enemyPos)
        free(m->enemyPos);
    free(m);
}