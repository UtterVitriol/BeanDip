#pragma once

#include <Windows.h>
BOOL threadpool_init(PTP_POOL *pPool, PTP_CLEANUP_GROUP *pCleanupGroup, PTP_CALLBACK_ENVIRON pCallbackEnviron);
