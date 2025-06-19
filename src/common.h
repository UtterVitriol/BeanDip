#pragma once

#include <stdio.h>

#ifndef NDEBUG
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define DERR(s) fprintf(stderr, "[-]: %s:%d:%s(): %s - %ld\n", __FILENAME__, __LINE__, __func__, s, GetLastError())
#define DMSG(s) printf("[+]: %s:%d:%s(): %s\n", __FILENAME__, __LINE__, __func__, s)
#else
#define DERR(s,d)
#define DMSG(s)
#endif
