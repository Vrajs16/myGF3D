#include <czmq.h>

typedef struct
{
    int x;
    int y;
    int z;
} Position;

Position pos;

int main(void)
{
    //  Socket to talk to clients
    pos.x = 0;
    pos.y = 0;
    pos.z = 0;
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

        if (p.x == pos.x && p.y == pos.y && p.z == pos.z)
        {
            zstr_send(responder, "Already at that position");
        }
        else
        {
            pos.x = p.x;
            pos.y = p.y;
            pos.z = p.z;
            zstr_send(responder, "Moving");
        }
        zframe_destroy(&frame);
    }
    printf("\nClosing server!\n");
    zsock_destroy(&responder);
    return 0;
}