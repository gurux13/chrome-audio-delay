#include <krabs.hpp>
#include <iostream>
#include <unordered_set>
#include <Windows.h>
#include <psapi.h> // For access to GetModuleFileNameEx
#include "cmdline.h"
#include "inject.h"

int main() {
    WinMain(NULL, NULL, NULL, 0);
}
int WINAPI WinMain(HINSTANCE hInstance,    // HANDLE TO AN INSTANCE.  This is the "handle" to YOUR PROGRAM ITSELF.
    HINSTANCE hPrevInstance,// USELESS on modern windows (totally ignore hPrevInstance)
    LPSTR szCmdLine,        // Command line arguments.  similar to argv in standard C programs
    int iCmdShow)          // Start window maximized, minimized, etc. 
{
    HKEY execKeyHandle;
    if (RegOpenKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\gurux13\\ChromePatcher\\executables", &execKeyHandle) != ERROR_SUCCESS) {
        return 1;
    }

    std::unordered_set<std::wstring> enabled_browsers;
#define MAX_VALUE_NAME_LENGTH 32767
    DWORD reserved;
    DWORD length = MAX_VALUE_NAME_LENGTH;
    DWORD idx = 0;
    TCHAR buffer[MAX_VALUE_NAME_LENGTH];
    while (RegEnumValue(execKeyHandle, idx, buffer, &length, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
        enabled_browsers.insert(buffer);
        length = MAX_VALUE_NAME_LENGTH;
        ++idx;
    }
    

    krabs::kernel_trace trace(L"My magic trace");
    krabs::kernel::process_provider provider;
    provider.add_on_event_callback([enabled_browsers](const EVENT_RECORD& record, const krabs::trace_context& trace_context) {
        krabs::schema schema(record, trace_context.schema_locator);
        if (schema.event_opcode() == 1) {
            krabs::parser parser(schema);

            DWORD pid = parser.parse<uint32_t>(L"ProcessId");
            // New process
            auto cmdline = GetProcessCommandLine(pid);
            auto path = GetProcessPath(pid);
            
            //auto CommandLine = parser.parse<std::string>(L"CommandLine");
            
            if (enabled_browsers.find(path) != enabled_browsers.end()) {
                //std::wcout << "Found chrome (" << pid << "), cmdline: " << cmdline << "\n";
                if (cmdline.find(L"--utility-sub-type=audio.mojom.AudioService") != std::wstring::npos) {
                    std::wcout << "Found process " << pid << ", injecting...\n";
                    Inject(pid, L"InjectedDll.dll");
                }
            }

        }
        });
    trace.enable(provider);
    trace.start();
}