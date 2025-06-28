#include "socket.h"
#include "common.h"
#include "client.h"
#include "thread.h"

LPFN_ACCEPTEX pAcceptEx = NULL;

SOCKET
init_server(USHORT port)
{
    int     iResult = 0;
    WSADATA wsaData = { 0 };
    SOCKET  sock    = INVALID_SOCKET;

    struct sockaddr_in sockAddr = { 0 };

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        DERR("WSAStartup");
        goto ERR;
    }

    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == sock)
    {
        DERR("socket");
        goto ERR;
    }

    sockAddr.sin_family      = AF_INET;
    sockAddr.sin_addr.s_addr = INADDR_ANY;
    sockAddr.sin_port        = htons(port);

    if (SOCKET_ERROR == bind(sock, (SOCKADDR *)&sockAddr, sizeof(sockAddr)))
    {
        DERR("bind");
        goto ERR;
    }

    if (SOCKET_ERROR == listen(sock, SOMAXCONN))
    {
        DERR("listen");
        goto ERR;
    }

    goto EXIT;
ERR:
    cleanup_server(sock);
    sock = INVALID_SOCKET;
EXIT:
    return sock;
}

PCLIENT_CTX
queue_accept(SOCKET sServer,
             HANDLE hCompPort)
{
    PCLIENT_CTX  client         = NULL;
    BOOL         bResult        = FALSE;
    LPOVERLAPPED pWsaOverlapped = NULL;

    client = create_client(hCompPort);
    if (!client)
    {
        goto END;
    }

    pWsaOverlapped = HeapAlloc(
        GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*pWsaOverlapped));
    if (!pWsaOverlapped)
    {
        close_client(&client);
        goto END;
    }

    pWsaOverlapped->hEvent = WSACreateEvent();

    bResult = pAcceptEx(sServer,
                        client->sock,
                        client->buff,
                        0,
                        dwAddrLen,
                        dwAddrLen,
                        &client->dwBytes,
                        pWsaOverlapped);
    if (!bResult && ERROR_IO_PENDING != WSAGetLastError())
    {
        DERR("AcceptEx");
        close_client(&client);
        goto END;
    }

    // Add pCTX->sock socket to completion port
    CreateIoCompletionPort(
        (HANDLE)client->sock, hCompPort, (DWORD)client->sock, 0);
    if (!hCompPort)
    {
        DERR("CreateIoCompletionPort");
        close_client(&client);
        goto END;
    }

END:
    return client;
}

VOID
server_run (SOCKET sServer)
{
    PTP_POOL            pool                        = NULL;
    PTP_CLEANUP_GROUP   cleanupGroup                = NULL;
    TP_CALLBACK_ENVIRON callbackEnviron             = { 0 };
    PTP_WORK            work                        = NULL;
    HANDLE              hCompPort                   = INVALID_HANDLE_VALUE;
    DWORD               dwBytes                     = 0;
    ULONG_PTR           key                         = 0;
    LPOVERLAPPED        lpOverlapped                = NULL;
    GUID                GuidAcceptEx                = WSAID_ACCEPTEX;
    int                 iResult                     = 0;
    BOOL                bResult                     = FALSE;
    PCLIENT_CTX         client                      = NULL;

    /////////////
    /// Move this shit
    if (!threadpool_init(&pool, &cleanupGroup, &callbackEnviron))
    {
        goto END;
    }

    // Create completion port
    hCompPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (!hCompPort)
    {
        DERR("CreateIoCompletionPort");
        goto END;
    }

    hCompPort = CreateIoCompletionPort((HANDLE)sServer, hCompPort, sServer, 0);
    if (!hCompPort)
    {
        DERR("CreateIoCompletionPort");
        goto END;
    }

    // Get pointer to AccectpEx
    iResult = WSAIoctl(sServer,
                       SIO_GET_EXTENSION_FUNCTION_POINTER,
                       &GuidAcceptEx,
                       sizeof(GuidAcceptEx),
                       &pAcceptEx,
                       sizeof(pAcceptEx),
                       &dwBytes,
                       NULL,
                       NULL);
    if (SOCKET_ERROR == iResult)
    {
        DERR("WSAIoctl");
        goto END;
    }
    /// End move
    //////////////////////
    client = queue_accept(sServer, hCompPort);
    if (!client)
    {
        goto END;
    }
    for (;;)
    {
        if (!GetQueuedCompletionStatus(
                hCompPort, &dwBytes, &key, &lpOverlapped, INFINITE))
        {
            DERR("GetQueuedCompletionStatus");
            goto END;
        }

        if (key == sServer)
        {
            WSACloseEvent(lpOverlapped->hEvent);
            HeapFree(GetProcessHeap(), 0, lpOverlapped);
            iResult = WSARecv(client->sock,
                              &client->wsaBuf,
                              1,
                              NULL,
                              &client->dwFlags,
                              &client->wsaOverlapped,
                              NULL);
            if (0 != iResult && WSA_IO_PENDING != WSAGetLastError())
            {
                DERR("WSARecv");
                goto END;
            }
            DMSG("Queued Recv");
            client = queue_accept(sServer, hCompPort);
            if (!client)
            {
                goto END;
            }
        }
        else if (dwBytes == 0)
        {
            client = (PCLIENT_CTX)lpOverlapped;
            DMSG("Client connection closed\n");
            close_client(&client);
            goto END;
        }
        else
        {
            work = CreateThreadpoolWork(
                handle_client, (PCLIENT_CTX)lpOverlapped, &callbackEnviron);
            if (!work)
            {
                DERR("CreateThreadpoolWork");
                goto END;
            }
            SubmitThreadpoolWork(work);
        }
    }

END:
    DMSG("Cleaning up");
    if(client)
    {
        close_client(&client);
    }
    CloseHandle(hCompPort);
    cleanup_server(sServer);
    DMSG("Cleaned up");
    return;
}

VOID
cleanup_server (SOCKET sock)
{
    closesocket(sock);
    WSACleanup();
}
