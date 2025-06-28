#include "client.h"
#include "common.h"
#include <stdlib.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>

PCLIENT_CTX
create_client(HANDLE hCompPort)
{
    PCLIENT_CTX client = NULL;

    client = (PCLIENT_CTX)HeapAlloc(
        GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*client));
    if (!client)
    {
        DERR("HeapAlloc");
        goto END;
    }
    client->wsaBuf.buf           = HeapAlloc(GetProcessHeap(), 0, 1);
    client->wsaBuf.len           = 1;
    client->wsaOverlapped.hEvent = WSACreateEvent();

    client->ioPort = hCompPort;
    client->sock   = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == client->sock)
    {
        DERR("Socket");
        HeapFree(GetProcessHeap(), 0, client->wsaBuf.buf);
        HeapFree(GetProcessHeap(), 0, client);
        client = NULL;
        goto END;
    }

END:
    return client;
}

void
close_client (PPCLIENT_CTX ppCtx)
{
    PCLIENT_CTX pCtx = NULL;

    if (!ppCtx || !*ppCtx)
    {
        DERR("NULL ptr");
        exit(1);
    }

    pCtx = *ppCtx;

    closesocket(pCtx->sock);

    if (pCtx->wsaBuf.buf)
    {

        HeapFree(GetProcessHeap(), 0, pCtx->wsaBuf.buf);
    }

    WSACloseEvent(pCtx->wsaOverlapped.hEvent);

    HeapFree(GetProcessHeap(), 0, pCtx);

    *ppCtx = NULL;
}

VOID CALLBACK
handle_client (PTP_CALLBACK_INSTANCE Instance, PVOID Parameter, PTP_WORK WORK)
{
    PCLIENT_CTX ctx = (PCLIENT_CTX)Parameter;

    WSAOVERLAPPED   wsaOverlapped  = { 0 };
    DWORD           dwBytes        = 0;
    ULONG_PTR       key            = 0;
    LPWSAOVERLAPPED pWsaOverlapped = NULL;
    int             iResult        = 0;

    printf("%c\n", *(PBYTE)ctx->wsaBuf.buf);
    DMSG("Queued Recv Internal");
    iResult = WSARecv(ctx->sock,
                      &ctx->wsaBuf,
                      1,
                      NULL,
                      &ctx->dwFlags,
                      &ctx->wsaOverlapped,
                      NULL);
    if (0 != iResult && WSA_IO_PENDING != WSAGetLastError())
    {
        DERR("WSARecv");
    }
}

