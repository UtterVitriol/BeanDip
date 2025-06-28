#include "socket.h"
#include <stdio.h>

/*
 * Server -> client Pkey
 * c -> s key
 * s -> c key
 */

int
main ()
{
    SOCKET            server       = INVALID_SOCKET;
    PTP_POOL          pool         = NULL;
    PTP_CLEANUP_GROUP cleanupGroup = NULL;

    server = init_server(23669);
    if(INVALID_SOCKET == server)
    {
        goto EXIT;
    }
    server_run(server);

EXIT:
    return 0;
}
