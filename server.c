#include <czmq.h>
#include <time.h>

typedef struct
{
    int x;
    int y;
    int z;
    int id;
} Position;

int main(void)
{
    //  Socket to talk to clients
    Position positions[2];
    int currentPlayers = -1;

    positions[0].x = 0;
    positions[0].y = 0;
    positions[0].z = 0;
    positions[0].id = -1;

    positions[1].x = 0;
    positions[1].y = 0;
    positions[1].z = 0;
    positions[1].id = -1;

    zsock_t *responder = zsock_new(ZMQ_REP);
    int rc = zsock_bind(responder, "tcp://*:9012");
    assert(rc == 9012);

    while (!zsys_interrupted)
    {
        // Recieve a frame of type Position
        zframe_t *frame = zframe_recv(responder);
        if (frame == NULL)
        {
            printf("Error receiving frame\n");
            return -1;
        }
        // Get frame data without it being a pointer
        Position p = *(Position *)zframe_data(frame);
        if (p.id == -1)
        {
            currentPlayers++;
            positions[currentPlayers].x = p.x;
            positions[currentPlayers].y = p.y;
            positions[currentPlayers].z = p.z;
            positions[currentPlayers].id = currentPlayers;
            zstr_sendf(responder, "%d", currentPlayers);
            zframe_destroy(&frame);
        }
        else
        {
            positions[p.id].x = p.x;
            positions[p.id].y = p.y;
            positions[p.id].z = p.z;

            if (p.id)
                zstr_sendf(responder, "Player %d: %d %d %d", positions[0].id, positions[0].x, positions[0].y, positions[0].z);
            else
                zstr_sendf(responder, "Player %d: %d %d %d", positions[1].id, positions[1].x, positions[1].y, positions[1].z);

            zframe_destroy(&frame);
        }
    }
    printf("\nClosing server!\n");
    zsock_destroy(&responder);
    return 0;
}