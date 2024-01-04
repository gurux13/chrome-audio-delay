#include "cmdline.h"
#include <winternl.h>
#include <psapi.h>


typedef NTSTATUS(NTAPI* pfnNtQueryInformationProcess)(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL);

std::wstring GetProcessPath(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (hProcess == INVALID_HANDLE_VALUE) {
        return L"";
    }
    TCHAR filename[MAX_PATH];
    if (GetModuleFileNameEx(hProcess, NULL, filename, MAX_PATH) == 0) {
        CloseHandle(hProcess);
        return filename;
    }
    CloseHandle(hProcess);
    return L"";
}

std::wstring GetProcessCommandLine(DWORD pid) {
    pfnNtQueryInformationProcess gNtQueryInformationProcess = (pfnNtQueryInformationProcess)GetProcAddress(
        GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");

    PPROCESS_BASIC_INFORMATION pbi = NULL;
    PEB peb = { NULL };
    RTL_USER_PROCESS_PARAMETERS process_parameters = { NULL };

    // Get process handle
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (hProcess == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    // Get process basic information
    HANDLE heap = GetProcessHeap();
    DWORD pbi_size = sizeof(PROCESS_BASIC_INFORMATION);
    pbi = (PROCESS_BASIC_INFORMATION*)HeapAlloc(heap, HEAP_ZERO_MEMORY, pbi_size);
    if (!pbi) {
        CloseHandle(hProcess);
        return FALSE;
    }

    // Get Process Environment Block (PEB)
    DWORD size_needed;
    std::wstring rv;
    NTSTATUS status = gNtQueryInformationProcess(hProcess, ProcessBasicInformation, pbi, pbi_size, &size_needed);
    if (status >= 0 && pbi->PebBaseAddress) {

        // Read PEB
        SIZE_T bytes_read;
        if (ReadProcessMemory(hProcess, pbi->PebBaseAddress, &peb, sizeof(peb), &bytes_read)) {

            // Read the processs parameters
            bytes_read = 0;
            if (ReadProcessMemory(hProcess, peb.ProcessParameters, &process_parameters, sizeof(RTL_USER_PROCESS_PARAMETERS), &bytes_read)) {
                if (process_parameters.CommandLine.Length > 0) {

                    // Allocate space to read the command line parameter
                    WCHAR* buffer = NULL;
                    buffer = (WCHAR*)HeapAlloc(heap, HEAP_ZERO_MEMORY, process_parameters.CommandLine.Length + 2);
                    
                    if (buffer) {
                        memset(buffer, 0, process_parameters.CommandLine.Length + 2);
                        if (ReadProcessMemory(hProcess, process_parameters.CommandLine.Buffer, buffer, process_parameters.CommandLine.Length, &bytes_read)) {
                            rv = buffer;
                            // Copy only as much as will fit in the commandLine property
                            CloseHandle(hProcess);
                            HeapFree(heap, 0, pbi);
                            return rv;
                        }
                    }
                }
            }
        }
    }

    CloseHandle(hProcess);
    HeapFree(heap, 0, pbi);
    return L"";
}