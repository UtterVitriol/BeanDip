#pragma once

#include <system_error>
#include <Windows.h>
#include <winternl.h>

#ifndef NDEBUG
#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#define DERR(s, d)\
	fprintf(\
		stderr,\
		"[-]: %s:%d:%s(): %s: %s - %d\n",\
		__FILENAME__,\
		__LINE__,\
		__func__,\
		s,\
		std::system_category().message(d).c_str(),\
		d\
	)
#define DMSG(s) printf("[+]: %s:%d:%s(): %s\n", __FILENAME__, __LINE__, __func__, s)
#else
#define DERR(s,d)
#define DMSG(s)
#endif

#define FULLY_QALIFIED_PATH_PREFIX "\\??\\"

int to_unicode(_In_ char* original, _In_ SIZE_T len, _Out_ PUNICODE_STRING puStr);