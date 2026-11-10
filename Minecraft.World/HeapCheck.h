#pragma once

// neoLegacy - opt-in heap corruption finder.
//
// Two of the random-crash logs (a Zombie destructor and a Bat tick, both on the server thread) are what heap corruption looks like:
// an object whose memory was freed or overwritten by something else earlier. The crash site is never the culprit. This validates the whole
// process heap at a few checkpoints so the FIRST place the heap is found corrupt is logged, which narrows the guilty code to the work done
// between two checkpoints (or to one specific entity tick).
//
// Off by default and free when off. To turn it on set an environment variable before launching, e.g. in Steam launch options:
//     NEOLEGACY_HEAPCHECK=1 %command%
// The value N means "validate on every N-th checkpoint" (1 = every one; use 50 or 200 if that is too slow in a big world).
// When the heap is found corrupt it writes "[HeapCheck] heap corrupted ..." (+ the entity pointer/type when known) to the log and raises an
// exception so Wine prints a backtrace.
#if defined(_WINDOWS64)
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

namespace HeapCheck
{
	inline int interval()
	{
		static int value = []() -> int
		{
#pragma warning(push)
#pragma warning(disable : 4996)
			const char *env = getenv("NEOLEGACY_HEAPCHECK");
#pragma warning(pop)
			if (env == nullptr) return 0;
			int n = atoi(env);
			return (n > 0) ? n : 1;
		}();
		return value;
	}

	inline void check(const char *where, const void *object = nullptr, unsigned objectType = 0)
	{
		const int n = interval();
		if (n <= 0) return;
		static unsigned long long counter = 0;
		if ((++counter % static_cast<unsigned long long>(n)) != 0) return;

		if (!HeapValidate(GetProcessHeap(), 0, nullptr))
		{
			char line[512];
			_snprintf_s(line, sizeof(line), _TRUNCATE, "[HeapCheck] heap corrupted at/before: %s (object=%p type=0x%x)\n", where, object, objectType);
			OutputDebugStringA(line);
			RaiseException(0xE0000002, EXCEPTION_NONCONTINUABLE, 0, nullptr);
		}
	}
}
#else
namespace HeapCheck
{
	inline void check(const char *, const void * = nullptr, unsigned = 0) {}
}
#endif
