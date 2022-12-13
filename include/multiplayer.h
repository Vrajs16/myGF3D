#ifndef __MULTIPLAYER__
#define __MULTIPLAYER__
/* Setup */
void setup_connection(void);

/* sending */
void sending(int x, int y, int z);

/* receiving */
void receiving(void);

#endif