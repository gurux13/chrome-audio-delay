#include <krabs.hpp>
#include <iostream>
#include "cmdline.h"
#include "inject.h"
int WINAPI WinMain(HINSTANCE hInstance,    // HANDLE TO AN INSTANCE.  This is the "handle" to YOUR PROGRAM ITSELF.
    HINSTANCE hPrevInstance,// USELESS on modern windows (totally ignore hPrevInstance)
    LPSTR szCmdLine,        // Command line arguments.  similar to argv in standard C programs
    int iCmdShow)          // Start window maximized, minimized, etc. 
{
    /*if (argc == 2) {
        Inject(atoi(argv[1]), L"ChromePatcherDll.dll");
        return;
    }*/
    krabs::kernel_trace trace(L"My magic trace");
    krabs::kernel::process_provider provider;
    provider.add_on_event_callback([](const EVENT_RECORD& record, const krabs::trace_context& trace_context) {
        krabs::schema schema(record, trace_context.schema_locator);
        if (schema.event_opcode() == 1) {
            krabs::parser parser(schema);

            DWORD pid = parser.parse<uint32_t>(L"ProcessId");
            // New process
            auto cmdline = GetProcessCommandLine(pid);
            //auto CommandLine = parser.parse<std::string>(L"CommandLine");
            if (cmdline.find(L"chrome") != std::wstring::npos) {
                //std::wcout << "Found chrome (" << pid << "), cmdline: " << cmdline << "\n";
                if (cmdline.find(L"--utility-sub-type=audio.mojom.AudioService") != std::wstring::npos) {
                    std::wcout << "Found process " << pid << ", injecting...\n";
                    Inject(pid, L"ChromePatcherDll.dll");
                }
            }

        }
        });
    trace.enable(provider);
    trace.start();
}