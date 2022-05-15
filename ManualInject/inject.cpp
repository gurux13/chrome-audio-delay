#include <stdio.h>
#include "inject.h"

typedef HMODULE(WINAPI* pLoadLibraryA)(LPCSTR);
typedef FARPROC(WINAPI* pGetProcAddress)(HMODULE, LPCSTR);

typedef BOOL(WINAPI* PDLL_MAIN)(HMODULE, DWORD, PVOID);

typedef struct _MANUAL_INJECT
{
    PVOID ImageBase;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_BASE_RELOCATION BaseRelocation;
    PIMAGE_IMPORT_DESCRIPTOR ImportDirectory;
    pLoadLibraryA fnLoadLibraryA;
    pGetProcAddress fnGetProcAddress;
}MANUAL_INJECT, * PMANUAL_INJECT;

DWORD WINAPI LoadDll(PVOID p)
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
            return FALSE;
        }

        while (OrigFirstThunk->u1.AddressOfData)
        {
            if (OrigFirstThunk->u1.Ordinal & IMAGE_ORDINAL_FLAG)
            {
                // Import by ordinal

                Function = (ULONGLONG)ManualInject->fnGetProcAddress(hModule, (LPCSTR)(OrigFirstThunk->u1.Ordinal & 0xFFFF));

                if (!Function)
                {
                    return FALSE;
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
                    return FALSE;
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
        return EntryPoint((HMODULE)ManualInject->ImageBase, DLL_PROCESS_ATTACH, NULL); // Call the entry point
    }

    return TRUE;
}

DWORD WINAPI LoadDllEnd()
{
    return 0;
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
        WriteProcessMemory(hProcess, (PVOID)((LPBYTE)image + pISH[i].VirtualAddress), (PVOID)((LPBYTE)buffer + pISH[i].PointerToRawData), pISH[i].SizeOfRawData, NULL);
        if (pISH[i].Characteristics & IMAGE_SCN_CNT_CODE) {
            VirtualProtectEx(hProcess, (PVOID)((LPBYTE)image + pISH[i].VirtualAddress), ((pISH[i].SizeOfRawData - 1) | 4095 + 1), PAGE_EXECUTE_READ, NULL);
            printf("Protected section %s (from %p size %d)\n", pISH[i].Name, (PVOID)((LPBYTE)image + pISH[i].VirtualAddress), ((pISH[i].SizeOfRawData - 1) | 4095) + 1);
        }
    }

    printf("\nAllocating memory for the loader code.\n");
    mem = VirtualAllocEx(hProcess, NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE); // Allocate memory for the loader code

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

    WriteProcessMemory(hProcess, mem, &ManualInject, sizeof(MANUAL_INJECT), NULL); // Write the loader information to target process
    WriteProcessMemory(hProcess, (PVOID)((PMANUAL_INJECT)mem + 1), LoadDll, (ULONGLONG)LoadDllEnd - (ULONGLONG)LoadDll, NULL); // Write the loader code to target process
    VirtualProtectEx(hProcess, (PVOID)((PMANUAL_INJECT)mem), (ULONGLONG)LoadDllEnd - (ULONGLONG)LoadDll + sizeof(MANUAL_INJECT), PAGE_EXECUTE_READ, NULL);
    printf("\nExecuting loader code.\n");
    hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)((PMANUAL_INJECT)mem + 1), mem, 0, NULL); // Create a remote thread to execute the loader code

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

    if (!ExitCode)
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