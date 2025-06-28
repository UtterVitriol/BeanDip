#include "common.h"
#include <Windows.h>
#include <stdio.h>


VOID CALLBACK
MyWorkCallback (PTP_CALLBACK_INSTANCE Instance, PVOID Parameter, PTP_WORK WORK)
{
    static int a = 0;
    Sleep(1000);
    printf("Hello from work: %d | %s\n", a, (PCHAR)Parameter);

    a++;
}

BOOL
threadpool_init (PTP_POOL *pPool, PTP_CLEANUP_GROUP *pCleanupGroup, PTP_CALLBACK_ENVIRON pCallbackEnviron)
{
    BOOL bRet = FALSE;
    PTP_WORK            work            = NULL;

    if(!pPool || !pCleanupGroup || !pCallbackEnviron)
    {
        DERR("NULL PTR");
        goto EXIT;
    }

    InitializeThreadpoolEnvironment(pCallbackEnviron);

    *pPool = CreateThreadpool(NULL);
    if (NULL == *pPool)
    {
        DERR("CreateThreadpool");
        goto EXIT;
    }

    SetThreadpoolThreadMaximum(*pPool, 0);
    bRet = SetThreadpoolThreadMinimum(*pPool, 1);
    if (!bRet)
    {
        DERR("SetThreadpoolThreadMinimum");
        goto EXIT;
    }

    *pCleanupGroup = CreateThreadpoolCleanupGroup();
    if (!*pCleanupGroup)
    {
        DERR("CreateThreadpoolCleanupGroup");
        goto EXIT;
    }

    SetThreadpoolCallbackPool(pCallbackEnviron, *pPool);
    SetThreadpoolCallbackCleanupGroup(pCallbackEnviron, *pCleanupGroup, NULL);
    
    bRet = TRUE;

EXIT:
    return bRet;
}

// int
// threadpool_test ()
// {
//     BOOL                bRet            = FALSE;
//     PTP_POOL            pool            = NULL;
//     PTP_CLEANUP_GROUP   cleanupGroup    = NULL;
//     TP_CALLBACK_ENVIRON callbackEnviron = { 0 };
//     PTP_WORK            work            = NULL;
//
//     InitializeThreadpoolEnvironment(&callbackEnviron);
//
//     pool = CreateThreadpool(NULL);
//     if (NULL == pool)
//     {
//         printf("CrateThreadPool: %lu\n", GetLastError());
//         goto EXIT;
//     }
//
//     SetThreadpoolThreadMaximum(pool, 0);
//     bRet = SetThreadpoolThreadMinimum(pool, 1);
//     if (!bRet)
//     {
//         printf("SetTreadPoolMinimum: %lu\n", GetLastError());
//         goto EXIT;
//     }
//
//     cleanupGroup = CreateThreadpoolCleanupGroup();
//     if (!cleanupGroup)
//     {
//         printf("CreateThreadpoolCleanupGroup %lu\n", GetLastError());
//         goto EXIT;
//     }
//
//     SetThreadpoolCallbackPool(&callbackEnviron, pool);
//     SetThreadpoolCallbackCleanupGroup(&callbackEnviron, cleanupGroup, NULL);
//
//     for (int x = 0; x < 10; x++)
//     {
//         work = CreateThreadpoolWork(MyWorkCallback, "Beans", &callbackEnviron);
//         if (!work)
//         {
//             printf("CreateThreadpoolWork: %lu\n", GetLastError());
//             goto EXIT;
//         }
//
//         SubmitThreadpoolWork(work);
//     }
//
//     printf("here");
//
//     CloseThreadpoolCleanupGroupMembers(cleanupGroup, FALSE, NULL);
//
// EXIT:
//     if (pool)
//     {
//         CloseThreadpoolCleanupGroupMembers(cleanupGroup, FALSE, NULL);
//         CloseThreadpoolCleanupGroup(cleanupGroup);
//         CloseThreadpool(pool);
//     }
//
//     return 0;
// }
