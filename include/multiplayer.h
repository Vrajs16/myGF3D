#ifndef __MULTIPLAYER__
#define __MULTIPLAYER__

typedef struct
{
    int x;
    int y;
    int z;
    float rotz;
    int id;
    int moving;
    int runningAnimationFrame;
    int idleAnimationFrame;
} Position;

/* Setup */
void setup_connection(void);

/* sending */
int sending(int x, int y, int z, float rotz, int moving, int runningAnimationFrame, int idleAnimationFrame);

/* receiving */
void receiving(void);

#endif
