#include <czmq.h>
#include <time.h>

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

int main(void)
{
    //  Socket to talk to clients
    Position positions[2];
    int currentPlayers = -1;

    positions[0].x = 0;
    positions[0].y = 0;
    positions[0].z = -1000;
    positions[0].rotz = M_PI;
    positions[0].id = -1;

    positions[1].x = 0;
    positions[1].y = 0;
    positions[1].z = -1000;
    positions[1].rotz = M_PI;
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
            printf("\nError receiving frame\n");
            zframe_destroy(&frame);
            zsock_destroy(&responder);
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
            positions[currentPlayers].rotz = p.rotz;
            positions[currentPlayers].id = currentPlayers;
            zstr_sendf(responder, "%d", currentPlayers);
        }
        else
        {
            positions[p.id].x = p.x;
            positions[p.id].y = p.y;
            positions[p.id].z = p.z;
            positions[p.id].rotz = p.rotz;
            positions[p.id].moving = p.moving;
            positions[p.id].runningAnimationFrame = p.runningAnimationFrame;
            positions[p.id].idleAnimationFrame = p.idleAnimationFrame;

            if (p.id)
            {
                zframe_t *sendingFrame = zframe_new(&positions[0], sizeof(Position));
                zframe_send(&sendingFrame, responder, 0);
            }
            else
            {
                zframe_t *sendingFrame = zframe_new(&positions[1], sizeof(Position));
                zframe_send(&sendingFrame, responder, 0);
            }
        }
        zframe_destroy(&frame);
    }
    printf("\nClosing server!\n");
    zsock_destroy(&responder);
    return 0;
}