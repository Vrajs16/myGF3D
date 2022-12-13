#include <czmq.h>
#include <multiplayer.h>
#include <simple_logger.h>

zsock_t *CLIENT = NULL;

typedef struct
{
    int x;
    int y;
    int z;
} Position;

Position CurrentPos;

void setup_connection()
{
    CLIENT = zsock_new(ZMQ_REQ);
    int rc = zsock_connect(CLIENT, "tcp://localhost:9012");
    assert(rc == 0);

    CurrentPos.x = 0;
    CurrentPos.y = 0;
    CurrentPos.z = 0;
}

void sending(int x, int y, int z)
{

    CurrentPos.x = x;
    CurrentPos.y = y;
    CurrentPos.z = z;

    zframe_t *frame = zframe_new(&CurrentPos, sizeof(CurrentPos));
    int rc = zframe_send(&frame, CLIENT, 0);
    if (rc == -1)
    {
        slog("Error sending frame\n");
        return;
    }
}
void receiving()
{
    char *str = zstr_recv(CLIENT);
    slog("Server response: %s", str);
    zstr_free(&str);
}
