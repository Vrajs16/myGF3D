#include <czmq.h>
#include <entity.h>
#include <multiplayer.h>
#include <simple_logger.h>

zsock_t *CLIENT = NULL;

Position CurrentPos;
extern Entity *OtherTrainer;

void setup_connection()
{
    CLIENT = zsock_new(ZMQ_REQ);
    int rc = zsock_connect(CLIENT, "tcp://localhost:9012");
    assert(rc == 0);

    CurrentPos.x = 0;
    CurrentPos.y = 0;
    CurrentPos.z = 0;
    CurrentPos.rotz = M_PI;
    CurrentPos.id = -1;

    // Get the id of the player
    zframe_t *frame = zframe_new(&CurrentPos, sizeof(CurrentPos));
    rc = zframe_send(&frame, CLIENT, 0);
    if (rc == -1)
    {
        slog("Error sending frame\n");
        return;
    }
    char *str = zstr_recv(CLIENT);
    // convert string to int
    CurrentPos.id = atoi(str);
    slog("Player id: %d", CurrentPos.id);
    zstr_free(&str);
}

int sending(int x, int y, int z, float rotz, int moving, int runningAnimationFrame, int idleAnimationFrame)
{
    CurrentPos.x = x;
    CurrentPos.y = y;
    CurrentPos.z = z;
    CurrentPos.rotz = rotz;
    CurrentPos.moving = moving;
    CurrentPos.runningAnimationFrame = runningAnimationFrame;
    CurrentPos.idleAnimationFrame = idleAnimationFrame;

    zframe_t *frame = zframe_new(&CurrentPos, sizeof(CurrentPos));
    int rc = zframe_send(&frame, CLIENT, 0);
    if (rc == -1)
    {
        slog("Error sending frame\n");
        return 0;
    }
    return 1;
}
void receiving()
{
    zframe_t *frame = zframe_recv(CLIENT);
    if (frame == NULL)
    {
        slog("Error receiving frame\n");
        return;
    }
    Position OtherPos = *(Position *)zframe_data(frame);

    OtherTrainer->position.x = OtherPos.x;
    OtherTrainer->position.y = OtherPos.y;
    OtherTrainer->position.z = OtherPos.z;
    OtherTrainer->rotation.z = OtherPos.rotz;

    if (OtherPos.moving == 1)
        OtherTrainer->model = OtherTrainer->runAniModels[OtherPos.runningAnimationFrame];
    else
        OtherTrainer->model = OtherTrainer->idleAniModels[OtherPos.idleAnimationFrame];

    zframe_destroy(&frame);
}
