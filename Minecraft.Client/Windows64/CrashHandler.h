#pragma once

class CrashHandler {
public:
	static void Initialize();
	static void Shutdown();

	static void interceptIggy(const char* msg);
	static void interceptA(const char* msg);
	static void interceptW(const wchar_t* msg);
};