#include "stdafx.h"
#include "CrashHandler.h"

#include <dbghelp.h>
#include <time.h>
#include <intrin.h>
#include <dxgi.h>

static FILE* s_logFile = nullptr;
static FILE* s_crashlogFile = nullptr;
static CRITICAL_SECTION s_cs;
static bool s_csInit = false;
static char s_logPath[MAX_PATH] = {};
static char s_exeDir[MAX_PATH] = {};

static LONG WINAPI UnhandledExceptionHandler(EXCEPTION_POINTERS* ep);

static HWND s_crashWindow = nullptr;
static DWORD WINAPI CrashWindowThread(void*);

void CrashHandler::Initialize() {
    InitializeCriticalSection(&s_cs);
    s_csInit = true;

    char exePath[MAX_PATH] = {};
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);

    char* slash = strrchr(exePath, '\\');
    if (slash) *(slash + 1) = '\0';

    strcpy_s(s_exeDir, exePath);
    sprintf_s(s_logPath, "%sneoLegacy_log.txt", s_exeDir);

    fopen_s(&s_logFile, s_logPath, "w");
    if (!s_logFile) return;

    SetUnhandledExceptionFilter(UnhandledExceptionHandler);
}

void CrashHandler::Shutdown() {
    if (s_logFile) {
        fclose(s_logFile);
        s_logFile = nullptr;
    }
    if (s_csInit) {
        DeleteCriticalSection(&s_cs);
        s_csInit = false;
    }
}

#undef OutputDebugStringA
#undef OutputDebugStringW
#undef OutputDebugString
static void writeLineA(const char* msg);

void CrashHandler::interceptIggy(const char* msg) {
#ifndef _FINAL_BUILD
    OutputDebugStringA(msg);
#endif
}

void CrashHandler::interceptA(const char* msg) {
#ifndef _FINAL_BUILD
    OutputDebugStringA(msg);
#endif
    writeLineA(msg);
}

void CrashHandler::interceptW(const wchar_t* msg) {
#ifndef _FINAL_BUILD
    OutputDebugStringW(msg);
#endif
    if (!s_logFile) return;
    int needed = WideCharToMultiByte(CP_UTF8, 0, msg, -1, nullptr, 0, nullptr, nullptr);
    if (needed <= 0) return;
    char* buf = new char[needed];
    WideCharToMultiByte(CP_UTF8, 0, msg, -1, buf, needed, nullptr, nullptr);
    writeLineA(buf);
    delete[] buf;
}


static void getCPUName(char* buf, size_t size);
static void getGPUName(char* buf, size_t size);
static void getOSName(char* buf, size_t size);
static bool isWineRuntime();
static std::string getBuildCompilerName();
static void getTimestamp(char* buf, size_t size);
static const char* ExceptionCodeToString(DWORD code);

static const char* getCleanedSourcePath(const char* path);

static LONG WINAPI UnhandledExceptionHandler(EXCEPTION_POINTERS* ep) {
    if (!s_logFile) return EXCEPTION_CONTINUE_SEARCH;

    CreateThread(nullptr, 0, CrashWindowThread, nullptr, 0, nullptr);

    char timestamp[64], folder[MAX_PATH], crashPath[MAX_PATH], cachedLog[MAX_PATH];
    getTimestamp(timestamp, sizeof(timestamp));

    for (char* p = timestamp; *p; p++) if (*p == ':' || *p == ' ') *p = '_';

    sprintf_s(folder, "%scrashlogs", s_exeDir);
    if (!CreateDirectoryA(folder, nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) return EXCEPTION_CONTINUE_SEARCH;

    sprintf_s(folder, "%s\\%s", folder, timestamp);
    CreateDirectoryA(folder, nullptr);
    if (!CreateDirectoryA(folder, nullptr) && GetLastError() != ERROR_ALREADY_EXISTS) return EXCEPTION_CONTINUE_SEARCH;

    sprintf_s(crashPath, "%s\\gamecrash.log", folder);
    sprintf_s(cachedLog, "%s\\debug.log", folder);

    EnterCriticalSection(&s_cs);

    fflush(s_logFile);
    fclose(s_logFile);
    s_logFile = nullptr;

    MoveFileExA(s_logPath, cachedLog, MOVEFILE_REPLACE_EXISTING);

    if (fopen_s(&s_crashlogFile, crashPath, "w") != 0 || !s_crashlogFile) {
        LeaveCriticalSection(&s_cs);
        return EXCEPTION_CONTINUE_SEARCH;
    }

#ifdef MINECRAFT_SERVER_BUILD
    const char* LogID = "Server";
#else
    const char* LogID = "Client";
#endif
    fprintf(s_crashlogFile, "NeoLegacy %s Crash:\n", LogID);
    fprintf(s_crashlogFile, "  Branch: %ls\n", VER_BRANCHVERSION_STR_W);
    fprintf(s_crashlogFile, "  Commit: %ls\n", VER_PRODUCTVERSION_STR_W);
    fprintf(s_crashlogFile, "  Compiler: %s\n", getBuildCompilerName().c_str());

#ifdef _CONTENT_PACKAGE
    fprintf(s_crashlogFile, "  Content Package: true\n");
#else
    fprintf(s_crashlogFile, "  Content Package: false\n");
#endif
#ifdef _DEBUG
    fprintf(s_crashlogFile, "  _DEBUG Flag: true\n");
#else
    fprintf(s_crashlogFile, "  _DEBUG Flag: false\n");
#endif

    fprintf(s_crashlogFile, "\nSystem Configuration:\n");
    {
        char buf[128];
        getOSName(buf, sizeof(buf));
        fprintf(s_crashlogFile, "  OS: %s\n", buf);
    }
    {
        char buf[128];
        getOSName(buf, sizeof(buf));
        fprintf(s_crashlogFile, "  Native Windows: %s\n", isWineRuntime() ? "false" : "true");
    }
    {
        char buf[128];
        getCPUName(buf, sizeof(buf));
        fprintf(s_crashlogFile, "  CPU: %s\n", buf);
    }
    {
        char buf[128];
        getGPUName(buf, sizeof(buf));
        fprintf(s_crashlogFile, "  GPU: %s\n", buf);
    }
    {
        MEMORYSTATUSEX mem = {};
        mem.dwLength = sizeof(mem);
        GlobalMemoryStatusEx(&mem);
        DWORDLONG freeMB = mem.ullAvailPhys / (1024 * 1024);
        DWORDLONG totalMB = mem.ullTotalPhys / (1024 * 1024);
        fprintf(s_crashlogFile, "  Memory: %llu bytes (%llu MB) free / %llu bytes (%llu MB) total\n",  mem.ullAvailPhys, freeMB, mem.ullTotalPhys, totalMB);
    }

    DWORD code = ep->ExceptionRecord->ExceptionCode;

    fprintf(s_crashlogFile, "\nException:\n");
    fprintf(s_crashlogFile, "  Code: %s (0x%08X)\n", ExceptionCodeToString(code), code);

    if (code == EXCEPTION_ACCESS_VIOLATION && ep->ExceptionRecord->NumberParameters >= 2) {
        const char* op = ep->ExceptionRecord->ExceptionInformation[0] == 1 ? "write" : "read";
        fprintf(s_crashlogFile, "  Message: Attempted to %s from address 0x%p\n", op, (void*)ep->ExceptionRecord->ExceptionInformation[1]);
    }

    fprintf(s_crashlogFile, "\nStack Trace:\n");

    HANDLE proc = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();

    SymInitialize(proc, nullptr, TRUE);
    SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);

    CONTEXT ctx = *ep->ContextRecord;

    STACKFRAME64 frame = {};
#ifdef _WIN64
    DWORD machineType = IMAGE_FILE_MACHINE_AMD64;
    frame.AddrPC.Offset = ctx.Rip;
    frame.AddrStack.Offset = ctx.Rsp;
#else
    DWORD machineType = IMAGE_FILE_MACHINE_I386;
    frame.AddrPC.Offset = ctx.Eip;
    frame.AddrStack.Offset = ctx.Esp;
#endif
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrStack.Mode = AddrModeFlat;

    char symBuf[sizeof(SYMBOL_INFO) + MAX_SYM_NAME];
    SYMBOL_INFO* sym = (SYMBOL_INFO*)symBuf;

    IMAGEHLP_LINE64 line = {};
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

    for (int i = 0; i < 64; i++) {
        if (!StackWalk64(machineType, proc, thread, &frame, &ctx, nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr)) break;
        if (frame.AddrPC.Offset == 0) break;

        sym->SizeOfStruct = sizeof(SYMBOL_INFO);
        sym->MaxNameLen = MAX_SYM_NAME;

        DWORD64 disp64 = 0;
        DWORD   disp32 = 0;

        if (SymFromAddr(proc, frame.AddrPC.Offset, &disp64, sym)) {
            if (SymGetLineFromAddr64(proc, frame.AddrPC.Offset, &disp32, &line)) {
                fprintf(s_crashlogFile, "  at %s+0x%llx (%s:%lu)\n", sym->Name, disp64, getCleanedSourcePath(line.FileName), line.LineNumber);
            } else {
                fprintf(s_crashlogFile, "  at %s+0x%llx\n", getCleanedSourcePath(sym->Name), disp64);
            }
        } else {
            fprintf(s_crashlogFile, "  at 0x%016llX\n", frame.AddrPC.Offset);
        }
    }

    SymCleanup(proc);

    fflush(s_crashlogFile);
    fclose(s_crashlogFile);
    s_crashlogFile = nullptr;

    LeaveCriticalSection(&s_cs);

    return EXCEPTION_CONTINUE_SEARCH;
}

static void getTimestamp(char* buf, size_t size) {
    time_t t = time(nullptr);
    struct tm tm_info;
    localtime_s(&tm_info, &t);
    strftime(buf, size, "%Y-%m-%d %H:%M:%S", &tm_info);
}

// stop clang from overwriting cpuid
#if defined(__cpuid)
#undef __cpuid
#endif

static void getCPUName(char* buf, size_t size) {
    int regs[4] = {};
    char name[49] = {};
    __cpuidex(regs, 0x80000002, 0); memcpy(name, regs, 16);
    __cpuidex(regs, 0x80000003, 0); memcpy(name + 16, regs, 16);
    __cpuidex(regs, 0x80000004, 0); memcpy(name + 32, regs, 16);
    name[48] = '\0';
    const char* p = name;
    while (*p == ' ') p++;
    strncpy_s(buf, size, p, _TRUNCATE);
}

static void getOSName(char* buf, size_t size) {
    DWORD major = 0, minor = 0, build = 0;

    HKEY hKey = nullptr;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD val = 0, sz = sizeof(val);
        if (RegQueryValueExW(hKey, L"CurrentMajorVersionNumber", nullptr, nullptr, (LPBYTE)&val, &sz) == ERROR_SUCCESS) major = val;
        sz = sizeof(val);
        if (RegQueryValueExW(hKey, L"CurrentMinorVersionNumber", nullptr, nullptr, (LPBYTE)&val, &sz) == ERROR_SUCCESS) minor = val;
        sz = sizeof(val);
        if (RegQueryValueExW(hKey, L"CurrentBuildNumber", nullptr, nullptr, (LPBYTE)&val, &sz) == ERROR_SUCCESS) build = val;
        if (build == 0) {
            wchar_t strBuild[16] = {};
            DWORD strSz = sizeof(strBuild);
            if (RegQueryValueExW(hKey, L"CurrentBuildNumber", nullptr, nullptr, (LPBYTE)strBuild, &strSz) == ERROR_SUCCESS)
                build = (DWORD)_wtoi(strBuild);
        }
        RegCloseKey(hKey);
    }

    SYSTEM_INFO si = {};
    GetNativeSystemInfo(&si);
    const char* arch = (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) ? "amd64" : "x86";

    snprintf(buf, size, "Windows %lu.%lu (Build %lu) (%s)", major, minor, build, arch);
}

static void getGPUName(char* buf, size_t size) {
    strncpy_s(buf, size, "UNKNOWN", _TRUNCATE);

    IDXGIFactory* factory = nullptr;
    if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory))) return;

    IDXGIAdapter* adapter = nullptr;
    if (SUCCEEDED(factory->EnumAdapters(0, &adapter))) {
        DXGI_ADAPTER_DESC desc = {};
        if (SUCCEEDED(adapter->GetDesc(&desc))) {
            WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, buf, (int)size, nullptr, nullptr);
        }

        adapter->Release();
    }
    factory->Release();
}

//please dont try and spoof this, its for debugging 
static bool isWineRuntime() {
    HKEY key;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Wine", 0, KEY_READ, &key) == ERROR_SUCCESS) {
        RegCloseKey(key);
        return true;
    }

    const HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (ntdll) {
        if (GetProcAddress(ntdll, "wine_get_version")) return true;
    }

    return false;
}

static std::string getBuildCompilerName() {
#if defined(__clang__)
    return "Clang " __clang_version__;
#elif defined(_MSC_VER)
    return "MSVC " + std::to_string(_MSC_VER);
#elif defined(__GNUC__)
    return "GCC " __VERSION__;
#else
    return "UNKNOWN";
#endif
}

static const char* ExceptionCodeToString(DWORD code) {
    switch (code) {
        case EXCEPTION_ACCESS_VIOLATION:         return "ACCESS_VIOLATION";
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:    return "ARRAY_BOUNDS_EXCEEDED";
        case EXCEPTION_BREAKPOINT:               return "BREAKPOINT";
        case EXCEPTION_DATATYPE_MISALIGNMENT:    return "DATATYPE_MISALIGNMENT";
        case EXCEPTION_FLT_DENORMAL_OPERAND:     return "FLT_DENORMAL_OPERAND";
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:       return "FLT_DIVIDE_BY_ZERO";
        case EXCEPTION_FLT_INEXACT_RESULT:       return "FLT_INEXACT_RESULT";
        case EXCEPTION_FLT_INVALID_OPERATION:    return "FLT_INVALID_OPERATION";
        case EXCEPTION_FLT_OVERFLOW:             return "FLT_OVERFLOW";
        case EXCEPTION_FLT_STACK_CHECK:          return "FLT_STACK_CHECK";
        case EXCEPTION_FLT_UNDERFLOW:            return "FLT_UNDERFLOW";
        case EXCEPTION_ILLEGAL_INSTRUCTION:      return "ILLEGAL_INSTRUCTION";
        case EXCEPTION_IN_PAGE_ERROR:            return "IN_PAGE_ERROR";
        case EXCEPTION_INT_DIVIDE_BY_ZERO:       return "INT_DIVIDE_BY_ZERO";
        case EXCEPTION_INT_OVERFLOW:             return "INT_OVERFLOW";
        case EXCEPTION_INVALID_DISPOSITION:      return "INVALID_DISPOSITION";
        case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "NONCONTINUABLE_EXCEPTION";
        case EXCEPTION_PRIV_INSTRUCTION:         return "PRIV_INSTRUCTION";
        case EXCEPTION_SINGLE_STEP:              return "SINGLE_STEP";
        case EXCEPTION_STACK_OVERFLOW:           return "STACK_OVERFLOW";
        default:                                 return "UNKNOWN";
    }
}

static const char* getCleanedSourcePath(const char* path) {
    const char* project = strstr(path, "Minecraft.Client\\");
    if (!project) project = strstr(path, "Minecraft.World\\");
    if (project) return project;

    const char* slash = strrchr(path, '\\');
    return slash ? slash + 1 : path;
}

static void writeLineA(const char* msg) {
    if (!s_logFile || !msg) return;

    EnterCriticalSection(&s_cs);

    size_t len = strlen(msg);
    while (len > 0 && (msg[len - 1] == '\n' || msg[len - 1] == '\r')) {
        len--;
    }

    fwrite(msg, 1, len, s_logFile);
    fputc('\n', s_logFile);
    fflush(s_logFile);

    LeaveCriticalSection(&s_cs);
}


static DWORD WINAPI CrashWindowThread(void*) {
    WNDCLASSA wc = {};
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.lpszClassName = "MinecraftCrashHandlerClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassA(&wc);

    HWND window = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        "MinecraftCrashHandlerClass", nullptr,
        WS_POPUP,
        (GetSystemMetrics(SM_CXSCREEN) - 300) / 2,
        (GetSystemMetrics(SM_CYSCREEN) - 50) / 2,
        300, 50,
        nullptr, nullptr, wc.hInstance, nullptr
    );

    if (!window) return 0;

    HWND text = CreateWindowExA(
        0, "STATIC", "Generating CrashLog",
        WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
        10, 15, 280, 25,
        window, nullptr, wc.hInstance, nullptr
    );

    HFONT font = CreateFontA(
        20, 0, 0, 0, FW_SEMIBOLD,
        FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE, "Segoe UI"
    );

    SendMessageA(text, WM_SETFONT, (WPARAM)font, TRUE);

    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);

    MSG msg;
    while (GetMessageA(&msg, nullptr, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 0;
}