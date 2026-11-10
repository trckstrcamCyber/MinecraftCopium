#include "stdafx.h"

#if defined(_WINDOWS64)

#include <windows.h>
#include <crtdbg.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <exception>

// ---------------------------------------------------------------------------------------------------------------------------
// neoLegacy - crash diagnostics.
//
// Not every failure is a hardware exception. assert(), abort(), std::terminate(), the CRT's invalid-parameter check and a pure
// virtual call all end in a modal "Microsoft Visual C++ Runtime Library" message box (under Wine/Proton this shows up as a #32770
// dialog with a Button, preceded by a GetCurrentPackageId stub line) and then the process just ends. That is what the
// "SurvivalCrashSplitScreenFatal" log looks like: no register dump, no backtrace, nothing that says which assertion fired.
//
// This installs handlers (once, at start-up) that
//   1. print what failed + a raw backtrace through OutputDebugString, so it lands in the same log as everything else, and
//   2. raise a real exception afterwards, so Wine / WER prints the usual symbolised "Unhandled exception ... Backtrace:" block
//      like the other crash logs do.
// Assertion text itself goes to stderr (_set_error_mode), which Proton also captures in the log, instead of into the dialog.
//
// To get the old behaviour back (dialogs for asserts), delete the two lines marked "(dialog)" in Installer below.
// ---------------------------------------------------------------------------------------------------------------------------

namespace
{
	const DWORD NEOLEGACY_FATAL_EXCEPTION = 0xE0000001;	// arbitrary software exception code

	void logBacktrace(const char *reason)
	{
		char line[1024];
		_snprintf_s(line, sizeof(line), _TRUNCATE, "[neoLegacy CrashDiagnostics] FATAL: %s\n", reason);
		OutputDebugStringA(line);

		void *frames[48];
		const USHORT count = CaptureStackBackTrace(1, 48, frames, nullptr);
		const unsigned char *base = reinterpret_cast<const unsigned char *>(GetModuleHandleA(nullptr));
		for (USHORT i = 0; i < count; ++i)
		{
			const unsigned long long offset = static_cast<unsigned long long>(reinterpret_cast<const unsigned char *>(frames[i]) - base);
			_snprintf_s(line, sizeof(line), _TRUNCATE, "[neoLegacy CrashDiagnostics]   #%u %p (exe+0x%llx)\n", static_cast<unsigned>(i), frames[i], offset);
			OutputDebugStringA(line);
		}
	}

	void failFast(const char *reason)
	{
		logBacktrace(reason);
		RaiseException(NEOLEGACY_FATAL_EXCEPTION, EXCEPTION_NONCONTINUABLE, 0, nullptr);
		TerminateProcess(GetCurrentProcess(), 3);	// not reached
	}

	void __cdecl onSigAbort(int)
	{
		failFast("abort() was called - an assert() failed, std::terminate ran, or something aborted explicitly (assert text is on stderr)");
	}

	void __cdecl onInvalidParameter(const wchar_t *expression, const wchar_t *function, const wchar_t *file, unsigned int line, uintptr_t)
	{
		char message[900];
		_snprintf_s(message, sizeof(message), _TRUNCATE, "CRT invalid parameter: expression=%ls function=%ls file=%ls line=%u",
			expression ? expression : L"?", function ? function : L"?", file ? file : L"?", line);
		failFast(message);
	}

	void __cdecl onPureCall()
	{
		failFast("pure virtual function call");
	}

	void onTerminate()
	{
		failFast("std::terminate - an exception was thrown and never caught (or escaped a noexcept function / thread)");
	}

	struct Installer
	{
		Installer()
		{
			_set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);	// (dialog) no abort message box / WER prompt
			_set_error_mode(_OUT_TO_STDERR);								// (dialog) assert() text goes to stderr instead of a message box
			signal(SIGABRT, onSigAbort);
			_set_invalid_parameter_handler(onInvalidParameter);
			_set_purecall_handler(onPureCall);
			std::set_terminate(onTerminate);	// per-thread in the MSVC runtime; abort() above covers the other threads
		}
	};

	Installer g_neoLegacyCrashDiagnosticsInstaller;
}

#endif // _WINDOWS64
