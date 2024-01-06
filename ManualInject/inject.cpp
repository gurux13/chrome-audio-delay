#include <stdio.h>
#include "inject.h"
#include <processthreadsapi.h>

typedef HMODULE(WINAPI* pLoadLibraryA)(LPCSTR);
typedef FARPROC(WINAPI* pGetProcAddress)(HMODULE, LPCSTR);
typedef VOID(WINAPI* pExitThread)(DWORD);

typedef BOOL(WINAPI* PDLL_MAIN)(HMODULE, DWORD, PVOID);

typedef struct _MANUAL_INJECT
{
    PVOID ImageBase;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_BASE_RELOCATION BaseRelocation;
    PIMAGE_IMPORT_DESCRIPTOR ImportDirectory;
    pLoadLibraryA fnLoadLibraryA;
    pGetProcAddress fnGetProcAddress;
    //pExitThread fnExitThread;
}MANUAL_INJECT, * PMANUAL_INJECT;

extern "C" __declspec(noinline) DWORD WINAPI LoadDll(PVOID p)
{
    PMANUAL_INJECT ManualInject;

    HMODULE hModule;
    DWORD i, count;
    ULONGLONG Function, delta;

    PULONGLONG ptr;
    PWORD list;

    PIMAGE_BASE_RELOCATION pIBR;
    PIMAGE_IMPORT_DESCRIPTOR pIID;
    PIMAGE_IMPORT_BY_NAME pIBN;
    PIMAGE_THUNK_DATA FirstThunk, OrigFirstThunk;

    PDLL_MAIN EntryPoint;
    ManualInject = (PMANUAL_INJECT)p;


    pIBR = ManualInject->BaseRelocation;
    delta = (ULONGLONG)((LPBYTE)ManualInject->ImageBase - ManualInject->NtHeaders->OptionalHeader.ImageBase); // Calculate the delta

    // Relocate the image

    while (pIBR->VirtualAddress)
    {
        if (pIBR->SizeOfBlock >= sizeof(IMAGE_BASE_RELOCATION))
        {
            count = (pIBR->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
            list = (PWORD)(pIBR + 1);

            for (i = 0; i < count; i++)
            {
                if (list[i])
                {
                    ptr = (PULONGLONG)((LPBYTE)ManualInject->ImageBase + (pIBR->VirtualAddress + (list[i] & 0xFFF)));
                    *ptr += delta;
                }
            }
        }

        pIBR = (PIMAGE_BASE_RELOCATION)((LPBYTE)pIBR + pIBR->SizeOfBlock);
    }

    pIID = ManualInject->ImportDirectory;

    // Resolve DLL imports

    while (pIID->Characteristics)
    {
        OrigFirstThunk = (PIMAGE_THUNK_DATA)((LPBYTE)ManualInject->ImageBase + pIID->OriginalFirstThunk);
        FirstThunk = (PIMAGE_THUNK_DATA)((LPBYTE)ManualInject->ImageBase + pIID->FirstThunk);

        hModule = ManualInject->fnLoadLibraryA((LPCSTR)ManualInject->ImageBase + pIID->Name);

        if (!hModule)
        {
            //ManualInject->fnExitThread(1);
            return 1;
        }

        while (OrigFirstThunk->u1.AddressOfData)
        {
            if (OrigFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
            {
                // Import by ordinal

                Function = (ULONGLONG)ManualInject->fnGetProcAddress(hModule, (LPCSTR)(OrigFirstThunk->u1.Ordinal & 0xFFFF));

                if (!Function)
                {
                    // ManualInject->fnExitThread(1);
                    return 1;
                }

                FirstThunk->u1.Function = Function;
            }

            else
            {
                // Import by name

                pIBN = (PIMAGE_IMPORT_BY_NAME)((LPBYTE)ManualInject->ImageBase + OrigFirstThunk->u1.AddressOfData);
                Function = (ULONGLONG)ManualInject->fnGetProcAddress(hModule, (LPCSTR)pIBN->Name);

                if (!Function)
                {
                    // ManualInject->fnExitThread(1);
                    return 1;
                }

                FirstThunk->u1.Function = Function;
            }

            OrigFirstThunk++;
            FirstThunk++;
        }

        pIID++;
    }

    if (ManualInject->NtHeaders->OptionalHeader.AddressOfEntryPoint)
    {
        EntryPoint = (PDLL_MAIN)((LPBYTE)ManualInject->ImageBase + ManualInject->NtHeaders->OptionalHeader.AddressOfEntryPoint);
        EntryPoint((HMODULE)ManualInject->ImageBase, DLL_PROCESS_ATTACH, NULL); // Call the entry point
        // ManualInject->fnExitThread(0);
        return 0;
    }
    // ManualInject->fnExitThread(0);
    return 0;
}

extern "C" __declspec(noinline) DWORD WINAPI LoadDllEnd()
{
    return 0;
}

bool IsCfgActive(HANDLE hProcess) {
    PROCESS_MITIGATION_CONTROL_FLOW_GUARD_POLICY policy_status;
    if (!GetProcessMitigationPolicy(hProcess, ProcessControlFlowGuardPolicy, &policy_status, sizeof(policy_status))) {
        return true;
    }
    return policy_status.EnableControlFlowGuard;
}

int Inject(DWORD pid, std::wstring dll)
{
    PIMAGE_DOS_HEADER pIDH;
    PIMAGE_NT_HEADERS pINH;
    PIMAGE_SECTION_HEADER pISH;

    HANDLE hProcess, hThread, hFile, hToken;
    PVOID buffer, image, mem;
    DWORD i, FileSize, ProcessId, ExitCode, read;

    TOKEN_PRIVILEGES tp;
    MANUAL_INJECT ManualInject;
    LoadDllEnd();
    if (OpenProcessToken((HANDLE)-1, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        tp.PrivilegeCount = 1;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        tp.Privileges[0].Luid.LowPart = 20;
        tp.Privileges[0].Luid.HighPart = 0;

        AdjustTokenPrivileges(hToken, FALSE, &tp, 0, NULL, NULL);
        CloseHandle(hToken);
    }

    printf("\nOpening the DLL.\n");
    hFile = CreateFile(dll.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL); // Open the DLL

    if (hFile == INVALID_HANDLE_VALUE)
    {
        printf("\nError: Unable to open the DLL (%d)\n", GetLastError());
        return -1;
    }

    FileSize = GetFileSize(hFile, NULL);
    buffer = VirtualAlloc(NULL, FileSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (!buffer)
    {
        printf("\nError: Unable to allocate memory for DLL data (%d)\n", GetLastError());

        CloseHandle(hFile);
        return -1;
    }

    // Read the DLL

    if (!ReadFile(hFile, buffer, FileSize, &read, NULL))
    {
        printf("\nError: Unable to read the DLL (%d)\n", GetLastError());

        VirtualFree(buffer, 0, MEM_RELEASE);
        CloseHandle(hFile);

        return -1;
    }

    CloseHandle(hFile);

    pIDH = (PIMAGE_DOS_HEADER)buffer;

    if (pIDH->e_magic != IMAGE_DOS_SIGNATURE)
    {
        printf("\nError: Invalid executable image.\n");

        VirtualFree(buffer, 0, MEM_RELEASE);
        return -1;
    }

    pINH = (PIMAGE_NT_HEADERS)((LPBYTE)buffer + pIDH->e_lfanew);

    if (pINH->Signature != IMAGE_NT_SIGNATURE)
    {
        printf("\nError: Invalid PE header.\n");

        VirtualFree(buffer, 0, MEM_RELEASE);
        return -1;
    }

    if (!(pINH->FileHeader.Characteristics & IMAGE_FILE_DLL))
    {
        printf("\nError: The image is not DLL.\n");

        VirtualFree(buffer, 0, MEM_RELEASE);
        return -1;
    }

    ProcessId = pid;

    printf("\nOpening target process.\n");
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessId);

    if (!hProcess)
    {
        printf("\nError: Unable to open target process (%d)\n", GetLastError());

        VirtualFree(buffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);

        return -1;
    }

    printf("\nAllocating memory for the DLL.\n");
    image = VirtualAllocEx(hProcess, NULL, pINH->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE); // Allocate memory for the DLL

    if (!image)
    {
        printf("\nError: Unable to allocate memory for the DLL (%d)\n", GetLastError());

        VirtualFree(buffer, 0, MEM_RELEASE);
        CloseHandle(hProcess);

        return -1;
    }

    // Copy the header to target process

    printf("\nCopying headers into target process.\n");

    if (!WriteProcessMemory(hProcess, image, buffer, pINH->OptionalHeader.SizeOfHeaders, NULL))
    {
        printf("\nError: Unable to copy headers to target process (%d)\n", GetLastError());

        VirtualFreeEx(hProcess, image, 0, MEM_RELEASE);
        CloseHandle(hProcess);

        VirtualFree(buffer, 0, MEM_RELEASE);
        return -1;
    }

    pISH = (PIMAGE_SECTION_HEADER)(pINH + 1);

    // Copy the DLL to target process

    printf("\nCopying sections to target process.\n");

    for (i = 0; i < pINH->FileHeader.NumberOfSections; i++)
    {
        if (!WriteProcessMemory(hProcess, (PVOID)((LPBYTE)image + pISH[i].VirtualAddress), (PVOID)((LPBYTE)buffer + pISH[i].PointerToRawData), pISH[i].SizeOfRawData, NULL)) {
            perror("copying section");
        }
        if (pISH[i].Characteristics & IMAGE_SCN_CNT_CODE) {
            VirtualProtectEx(hProcess, (PVOID)((LPBYTE)image + pISH[i].VirtualAddress), ((pISH[i].SizeOfRawData - 1) | 4095 + 1), PAGE_EXECUTE_READ, NULL);
            printf("Protected section %s (from %p size %d)\n", pISH[i].Name, (PVOID)((LPBYTE)image + pISH[i].VirtualAddress), ((pISH[i].SizeOfRawData - 1) | 4095) + 1);
        }
    }

    printf("\nAllocating memory for the loader code.\n");
    mem = VirtualAllocEx(hProcess, NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE); // Allocate memory for the loader code

    if (!mem)
    {
        printf("\nError: Unable to allocate memory for the loader code (%d)\n", GetLastError());

        VirtualFreeEx(hProcess, image, 0, MEM_RELEASE);
        CloseHandle(hProcess);

        VirtualFree(buffer, 0, MEM_RELEASE);
        return -1;
    }

    printf("\nLoader code allocated at %p\n", mem);
    memset(&ManualInject, 0, sizeof(MANUAL_INJECT));

    ManualInject.ImageBase = image;
    ManualInject.NtHeaders = (PIMAGE_NT_HEADERS)((LPBYTE)image + pIDH->e_lfanew);
    ManualInject.BaseRelocation = (PIMAGE_BASE_RELOCATION)((LPBYTE)image + pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
    ManualInject.ImportDirectory = (PIMAGE_IMPORT_DESCRIPTOR)((LPBYTE)image + pINH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    ManualInject.fnLoadLibraryA = LoadLibraryA;
    ManualInject.fnGetProcAddress = GetProcAddress;

    printf("\nWriting loader code to target process.\n");
    unsigned char loader_buffer[4096];
    memset(loader_buffer, 0, 4096);
    auto ptr = loader_buffer;
    memcpy(ptr, &ManualInject, sizeof(MANUAL_INJECT));
    ptr += sizeof(MANUAL_INJECT);
    memcpy(ptr, &LoadDll, (ULONGLONG)LoadDllEnd - (ULONGLONG)LoadDll);
    printf("Loader data to be sent: ");
    for (int i = 0; i < (ULONGLONG)LoadDllEnd - (ULONGLONG)LoadDll; ++i) {
        printf("%02x ", ptr[i]);
    }
    printf("\n");
    if (!WriteProcessMemory(hProcess, mem, loader_buffer, sizeof(MANUAL_INJECT) + (char*)(LoadDllEnd)-(char*)(LoadDll), NULL)) { // Write the loader information to target process
        perror("Write loader");
        VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
        VirtualFreeEx(hProcess, image, 0, MEM_RELEASE);

        CloseHandle(hProcess);

        VirtualFree(buffer, 0, MEM_RELEASE);
        return 1;
    }
    DWORD oldprotect;
    if (!VirtualProtectEx(hProcess, mem, 4096, PAGE_EXECUTE_READ, &oldprotect)) {
        printf("Error: %d\n", GetLastError());
    }
    printf("\nExecuting loader code.\n");
    LPTHREAD_START_ROUTINE sleep = (LPTHREAD_START_ROUTINE)Sleep;
    DWORD64 proper_start = (DWORD64)((PMANUAL_INJECT)mem + 1);
    DWORD tid;

    if (IsCfgActive(hProcess)) {
        printf("Configuring CFG");
        CFG_CALL_TARGET_INFO info;
        info.Offset = proper_start - (DWORD64)mem;
        info.Flags = CFG_CALL_TARGET_VALID;
        if (!SetProcessValidCallTargets(hProcess, mem, 4096, 1, &info)) {
            perror("CFG");
        }
    }
    hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)proper_start, mem, 0, &tid); // Create a remote thread to execute the loader code
    printf("Created thread %u\n", tid);

    if (!hThread)
    {
        printf("\nError: Unable to execute loader code (%d)\n", GetLastError());

        VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
        VirtualFreeEx(hProcess, image, 0, MEM_RELEASE);

        CloseHandle(hProcess);

        VirtualFree(buffer, 0, MEM_RELEASE);
        return -1;
    }
    
    WaitForSingleObject(hThread, INFINITE);
    GetExitCodeThread(hThread, &ExitCode);
    printf("Exit code: %x\n", ExitCode);

    if (ExitCode)
    {
        VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);
        VirtualFreeEx(hProcess, image, 0, MEM_RELEASE);

        CloseHandle(hThread);
        CloseHandle(hProcess);

        VirtualFree(buffer, 0, MEM_RELEASE);
        return -1;
    }

    CloseHandle(hThread);
    VirtualFreeEx(hProcess, mem, 0, MEM_RELEASE);

    CloseHandle(hProcess);

    printf("\nDLL injected at %p\n", image);

    if (pINH->OptionalHeader.AddressOfEntryPoint)
    {
        printf("\nDLL entry point: %p\n", (PVOID)((LPBYTE)image + pINH->OptionalHeader.AddressOfEntryPoint));
    }

    VirtualFree(buffer, 0, MEM_RELEASE);
    return 0;
}