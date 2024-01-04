#include <string>
#include <Windows.h>

std::wstring GetProcessCommandLine(DWORD pid);
std::wstring GetProcessPath(DWORD pid);