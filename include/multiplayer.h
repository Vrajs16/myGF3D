#ifndef __MULTIPLAYER__
#define __MULTIPLAYER__

typedef struct
{
    int x;
    int y;
    int z;
    int id;
} Position;

/* Setup */
void setup_connection(void);

/* sending */
int sending(int x, int y, int z);

/* receiving */
void receiving(void);

#endif
