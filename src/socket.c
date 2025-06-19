
#include "socket.h"
#include "common.h"

void
handle_client (SOCKET sock, HANDLE ioPort)
{
    WSAOVERLAPPED wsaOverlapped    = { 0 };
    WSABUF        wsaBuf           = { 0 };
    wsaBuf.buf                     = HeapAlloc(GetProcessHeap(), 0, 1024);
    wsaBuf.len                     = 1024;
    wsaOverlapped.hEvent           = WSACreateEvent();
    int             iResult        = 0;
    DWORD           dwFlags        = 0;
    DWORD           dwBytes        = 0;
    ULONG_PTR       key            = 0;
    LPWSAOVERLAPPED pWsaOverlapped = NULL;

    iResult = WSARecv(sock, &wsaBuf, 1, NULL, &dwFlags, &wsaOverlapped, NULL);
    if (0 != iResult && WSA_IO_PENDING != WSAGetLastError())
    {
        printf("%d: %d\n", WSAGetLastError(), iResult);
        // goto END;
    }

    printf("Got connection, waiting on data\n");
    if (!GetQueuedCompletionStatus(
            ioPort, &dwBytes, &key, &pWsaOverlapped, INFINITE))
    {
        DERR("GetQueuedCompletionStatus");
        goto END;
    }
    printf("Recv'd: %lu\n", dwBytes);
    printf("%s\n", wsaBuf.buf);

    // while (1)
    // {
    //     if (WAIT_OBJECT_0 == WaitForSingleObject(wsaOverlapped.hEvent, 0))
    //     {
    //         printf("%s\n", wsaBuf.buf);
    //         break;
    //     }
    //     printf("Waiting\n");
    // }

END:
    HeapFree(GetProcessHeap(), 0, wsaBuf.buf);
    closesocket(sock);
}

SOCKET
init_sock(USHORT port)
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
    cleanup_sock(sock);
EXIT:
    return sock;
}

void
get_client ()
{
    SOCKET sock   = INVALID_SOCKET;
    SOCKET client = INVALID_SOCKET;

    HANDLE       hCompPort    = INVALID_HANDLE_VALUE;
    DWORD        dwBytes      = 0;
    ULONG_PTR    key          = 0;
    LPOVERLAPPED lpOverlapped = NULL;
    // tAcceptEx     pAcceptEx     = NULL;
    LPFN_ACCEPTEX pAcceptEx     = NULL;
    GUID          GuidAcceptEx  = WSAID_ACCEPTEX;
    int           iResult       = 0;
    CHAR          buff[1024]    = { 0 };
    CONST DWORD   dwAddrLen     = ((sizeof(SOCKADDR_IN) + 16));
    WSAOVERLAPPED wsaOverlapped = { 0 };
    BOOL          bResult       = FALSE;

    // Create completion port
    hCompPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (!hCompPort)
    {
        DERR("CreateIoCompletionPort");
        goto END;
    }

    // Create server socket
    sock = init_sock(23669);
    if (INVALID_SOCKET == sock)
    {
        goto END;
    }

    hCompPort = CreateIoCompletionPort((HANDLE)sock, hCompPort, 0, 0);
    if (!hCompPort)
    {
        DERR("CreateIoCompletionPort");
        goto END;
    }

    // Get pointer to AcceptEx
    iResult = WSAIoctl(sock,
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

    client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == client)
    {
        DERR("Socket");
        goto END;
    }

    bResult = pAcceptEx(sock,
                        client,
                        buff,
                        0,
                        dwAddrLen,
                        dwAddrLen,
                        &dwBytes,
                        &wsaOverlapped);
    if (!bResult && ERROR_IO_PENDING != WSAGetLastError())
    {
        DERR("AcceptEx");
        goto END;
    }

    // Add server socket to completion port
    hCompPort
        = CreateIoCompletionPort((HANDLE)client, hCompPort, (DWORD)client, 0);
    if (!hCompPort)
    {
        DERR("CreateIoCompletionPort");
        goto END;
    }

    if (!GetQueuedCompletionStatus(
            hCompPort, &dwBytes, &key, &lpOverlapped, INFINITE))
    {
        DERR("GetQueuedCompletionStatus");
        goto END;
    }

    handle_client(client, hCompPort);

END:
    closesocket(client);
    cleanup_sock(sock);
    return;
}

VOID
cleanup_sock (SOCKET sock)
{
    closesocket(sock);
    WSACleanup();
}
