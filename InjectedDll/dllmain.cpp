#include "pch.h"

#include "framework.h"
#include "threads.hpp"


using lockguard_t = std::lock_guard<std::mutex>;

#pragma region logging

//#define _DEBUG
#ifdef _DEBUG
#define IFD(x) x
#else
#define IFD(x) 
#endif

#ifdef _DEBUG
class Logger {
public:
    class LogStream : public std::wstringstream {
    private:
        Logger& log;
    public:
        LogStream(Logger& log) : log(log) {
        }
        ~LogStream() {
            log.output(*this);
        }
    };
    LogStream* CurrentlogStream = nullptr;
    LogStream & log() {
        if (CurrentlogStream != nullptr) {
            delete CurrentlogStream;
        }
        CurrentlogStream = new LogStream(*this);
        return *CurrentlogStream;
    }
    Logger() {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        sockaddr_in connect_to;
        inet_pton(AF_INET, "127.0.0.1", &connect_to.sin_addr);
        //connect_to.sin_addr.S_un.S_addr = inet_pton();// inet_addr("127.0.0.1");
        connect_to.sin_port = htons(7777);
        connect_to.sin_family = AF_INET;
        connect(sock, (sockaddr*)&connect_to, sizeof(connect_to));
    }
    friend class LogStream;
private:
    SOCKET sock;
    void output(const LogStream& stream) {
        auto wstr = stream.str();
        char* str = (char*)malloc(wstr.length() + 1);
        if (!str) {
            return;
        }
        WideCharToMultiByte(1251, 0, wstr.c_str(), (int) wstr.length(), str, (int)wstr.length() + 1, 0, NULL);
        send(sock, str, (int)wstr.length(), 0);
        free(str);
    }
};
Logger logger;
#endif
#pragma endregion

#pragma region hacky offsets
struct Offsets {
    int clockToClientPtr = -1;
    int clientToIdPtr = -1;
};
Offsets offsets;
void FillOffsets(IMMDevice* device, IAudioClient* client, IAudioClock* clock) {
    for (int i = 0; i < 100; ++i) {
        if (*(((void**)clock) + i) == client) {
            offsets.clockToClientPtr = i;
            break;
        }
    }
    LPWSTR id;
    device->GetId(&id);
    auto len = lstrlenW(id);
    for (int i = 0; i < 100; ++i) {
        LPWSTR ptr = *((LPWSTR*)client + i);
        if (IsBadReadPtr(ptr, len * 2 + 2)) {
            continue;
        }
        if (memcmp(id, ptr, len * 2) == 0) {
            offsets.clientToIdPtr = i;
            break;
        }
    }
    CoTaskMemFree(id);
    IFD(logger.log() << "Got offsets: " << offsets.clockToClientPtr << ", " << offsets.clientToIdPtr << "\n");
}

#pragma endregion

#define REFTIMES_PER_SEC  10000000
#define REFTIMES_PER_MILLISEC  10000
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioClock = __uuidof(IAudioClock);
const IID IID_IMMDevice = __uuidof(IMMDevice);
const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);

template<typename TKey, typename TValue>
class sync_map : public std::unordered_map<TKey, TValue> {
public:
    std::mutex mtx;
};

sync_map<LPVOID, LPVOID> audioClientToDeviceMap;
sync_map<LPVOID, std::wstring> clockToDeviceMap;
sync_map<std::wstring, DWORD> deviceDelays;

namespace Hook
{
    STDMETHODIMP Activate(IUnknown* This, REFIID iid, DWORD dwClsCtx, PROPVARIANT* pActivationParams, void** ppInterface);
    STDMETHODIMP GetService(IUnknown* This, REFIID riid, void** ppv);
    STDMETHODIMP GetPosition(IUnknown* This, UINT64* position, UINT64* timestamp);
    STDMETHODIMP ReleaseDevice(IUnknown* This);
    STDMETHODIMP ReleaseClient(IUnknown* This);
    STDMETHODIMP ReleaseClock(IUnknown* This);
}

struct Context
{
    PVOID m_OriginalGetPosition;
    PVOID m_OriginalGetService;
    PVOID m_OriginalActivate;
    PVOID m_OriginalDeviceRelease;
    PVOID m_OriginalClientRelease;
    PVOID m_OriginalClockRelease;
};
std::unique_ptr<Context> g_Context;
HRESULT HookMethod(IUnknown* original, PVOID proxyMethod, PVOID* originalMethod, DWORD vtableOffset)
{
    PVOID* originalVtable = *(PVOID**)original;
    if (originalVtable[vtableOffset] == proxyMethod)
        return S_OK;
    *originalMethod = originalVtable[vtableOffset];
    originalVtable[vtableOffset] = proxyMethod;
    return S_OK;
}

HRESULT InstallComInterfaceHooks(IUnknown* originalInterface, REFIID riid)
{
    HRESULT hr = S_OK;
    if (!g_Context) {
        g_Context.reset(new Context);
    }
    if (riid == IID_IAudioClock)
    {
        ATL::CComPtr<IAudioClock> so;
        HRESULT hr = originalInterface->QueryInterface(IID_IAudioClock, (void**)&so);
        if (FAILED(hr)) return hr;

        DWORD dwOld = 0;
        if (!::VirtualProtect(*(PVOID**)(originalInterface), sizeof(LONG_PTR) * 5, PAGE_READWRITE, &dwOld)) {
            IFD(logger.log() << "VirtualProtect failed, last error: " << GetLastError() << "\n");
            return E_FAIL;
        }

        HookMethod(so, (PVOID)Hook::GetPosition, &g_Context->m_OriginalGetPosition, 4);
        HookMethod(so, (PVOID)Hook::ReleaseClock, &g_Context->m_OriginalClockRelease, 2);
        DWORD tmp;
        ::VirtualProtect(*(PVOID**)(originalInterface), sizeof(LONG_PTR) * 5, dwOld, &tmp);
    }
    if (riid == IID_IAudioClient)
    {
        ATL::CComPtr<IAudioClient> so;
        HRESULT hr = originalInterface->QueryInterface(IID_IAudioClient, (void**)&so);
        if (FAILED(hr)) return hr;

        DWORD dwOld = 0;
        if (!::VirtualProtect(*(PVOID**)(originalInterface), sizeof(LONG_PTR) * 15, PAGE_READWRITE, &dwOld)) {
            IFD(logger.log() << "VirtualProtect failed, last error: " << GetLastError() << "\n");
            return E_FAIL;
        }
        HookMethod(so, (PVOID)Hook::GetService, &g_Context->m_OriginalGetService, 14);
        HookMethod(so, (PVOID)Hook::ReleaseClient, &g_Context->m_OriginalClientRelease, 2);
        DWORD tmp;
        ::VirtualProtect(*(PVOID**)(originalInterface), sizeof(LONG_PTR) * 15, dwOld, &tmp);
    }
    if (riid == IID_IMMDevice)
    {
        ATL::CComPtr<IAudioClient> so;
        HRESULT hr = originalInterface->QueryInterface(IID_IMMDevice, (void**)&so);
        if (FAILED(hr)) return hr;

        DWORD dwOld = 0;
        if (!::VirtualProtect(*(PVOID**)(originalInterface), sizeof(LONG_PTR) * 4, PAGE_READWRITE, &dwOld)) {
            IFD(logger.log() << "VirtualProtect failed, last error: " << GetLastError() << "\n");
            return E_FAIL;
        }

        HookMethod(so, (PVOID)Hook::Activate, &g_Context->m_OriginalActivate, 3);
        DWORD tmp;
        ::VirtualProtect(*(PVOID**)(originalInterface), sizeof(LONG_PTR) * 4, dwOld, &tmp);
    }
    return hr;
}

typedef HRESULT(WINAPI* GetPosition_T)(IUnknown* This, UINT64* position, UINT64* timestamp);
sync_map<PVOID, UINT64> frequencies;
STDMETHODIMP Hook::GetPosition(IUnknown* This, UINT64* position, UINT64* timestamp)
{
    auto result = ((GetPosition_T)g_Context->m_OriginalGetPosition)(This, position, timestamp);
    if (FAILED(result)) {
        return result;
    }
    IFD(logger.log() << "GetPosition for " << (void*)This << ", normally: " << *position << ", " << *timestamp << "\n");
    UINT64 frequency = 0;
    {
        lockguard_t guard(frequencies.mtx);
        auto freqIter = frequencies.find(This);
        if (freqIter == frequencies.end()) {
            ((IAudioClock*)This)->GetFrequency(&frequency);
            frequencies[This] = frequency;
        }
        else {
            frequency = freqIter->second;
        }
    }
    DWORD delay = 0;
    {
        lockguard_t guard(clockToDeviceMap.mtx);
        auto myDevice = clockToDeviceMap.find(This);
        std::wstring* myDeviceStr = nullptr;
        if (myDevice == clockToDeviceMap.end()) {
            IFD(logger.log() << "Clock has no device - fetching offsets!\n");
            if (offsets.clientToIdPtr != -1 && offsets.clockToClientPtr != -1) {
                void* clientPtr = *((void**)This + offsets.clockToClientPtr);
                LPWSTR idPtr = *((LPWSTR*)clientPtr + offsets.clientToIdPtr);
                std::wstring idStr = idPtr;
                clockToDeviceMap[This] = idStr;
                myDeviceStr = &clockToDeviceMap[This];
                IFD(logger.log() << "Fetched this clock's id: " << idStr << "\n");
            }
        }
        else {
            myDeviceStr = &myDevice->second;
        }
        if (!myDeviceStr) {
            return result;
        }
        lockguard_t guard2(deviceDelays.mtx);
        auto deviceDelay = deviceDelays.find(*myDeviceStr);
        if (deviceDelay == deviceDelays.end()) {
            IFD(logger.log() << "Clock for device " << myDevice->second << " has no delay - ignoring\n");
            return result;
        }
        else {
            delay = deviceDelay->second;
        }

    }
    if (delay == 0) {
        return result;
    }
    UINT64 delayInFrequencies = delay * frequency / 1000;
    auto oldPos = *position;
    if (*position > delayInFrequencies) {
        *position -= delayInFrequencies;
    }
    IFD(logger.log() << "Modified position from " << oldPos << " to " << *position << " with delay " << delayInFrequencies << ", aka " << delay << "ms\n");

    return result;
}

typedef HRESULT(WINAPI* Release_T)(IUnknown* This);

STDMETHODIMP_(HRESULT __stdcall) Hook::ReleaseClient(IUnknown* This)
{
    auto result = ((Release_T)(g_Context->m_OriginalClientRelease))(This);
    if (result == 0) {
        lockguard_t guard(audioClientToDeviceMap.mtx);
        audioClientToDeviceMap.erase(This);
    }
    return result;
}

STDMETHODIMP_(HRESULT __stdcall) Hook::ReleaseClock(IUnknown* This)
{
    auto result = ((Release_T)(g_Context->m_OriginalClockRelease))(This);
    if (result == 0) {
        lockguard_t guard2(clockToDeviceMap.mtx);
        lockguard_t guard3(frequencies.mtx);
        clockToDeviceMap.erase(This);
        frequencies.erase(This);
    }
    return result;
}

typedef HRESULT(WINAPI* GetService_T)(IUnknown* This, REFIID riid, void** ppv);
STDMETHODIMP Hook::GetService(IUnknown* This, REFIID riid, void** ppv)
{
    static std::mutex mtx;
    auto result = ((GetService_T)g_Context->m_OriginalGetService)(This, riid, ppv);

    if (FAILED(result)) {
        return result;
    }
    if (riid != IID_IAudioClock) {
        return result;
    }
    IFD(logger.log() << "GetService for clock, this: " << (void*)This << "\n");
    IMMDevice* device = (IMMDevice*)audioClientToDeviceMap[This];
    if (!device) {
        IFD(logger.log() << "Couldn't find device for client!\n");
        return result;
    }
    LPWSTR id;
    if (FAILED(device->GetId(&id)) || !id) {
        IFD(logger.log() << "Couldn't get device id!\n");
        return result;
    }
    {
        lockguard_t guard(mtx);
        clockToDeviceMap[*ppv] = std::wstring(id);
    }
    CoTaskMemFree(id);
    return result;
}

typedef HRESULT(WINAPI* Activate_T)(IUnknown* This, REFIID iid, DWORD dwClsCtx, PROPVARIANT* pActivationParams, void** ppInterface);
STDMETHODIMP Hook::Activate(IUnknown* This, REFIID iid, DWORD dwClsCtx, PROPVARIANT* pActivationParams, void** ppInterface)
{
    static std::mutex mtx;
    auto result = ((Activate_T)g_Context->m_OriginalActivate)(This, iid, dwClsCtx, pActivationParams, ppInterface);
    if (FAILED(result)) {
        return result;
    }
    if (iid != IID_IAudioClient) {
        return result;
    }
    IFD(logger.log() << "Activate for audio client, this: " << (void*)This << ", device: " << (void*)*ppInterface << "\n");
    {
        lockguard_t guard(mtx);
        audioClientToDeviceMap[*ppInterface] = This;
    }
    return result;
}

#define EXIT_ON_ERROR(call)  \
	{HRESULT hr = call; \
              if (FAILED(hr)) { IFD(logger.log() << "ERROR IN " #call ": " << hr << "\n"); goto Exit; }}


void PatchAudioDelay() {
    REFERENCE_TIME hnsRequestedDuration = REFTIMES_PER_SEC;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDevice* pDevice = NULL;
    IAudioClient* pAudioClient = NULL;
    WAVEFORMATEX* pwfx = NULL;

    EXIT_ON_ERROR(CoInitialize(NULL));

    EXIT_ON_ERROR(CoCreateInstance(
        CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, IID_IMMDeviceEnumerator,
        (void**)&pEnumerator));

    EXIT_ON_ERROR(pEnumerator->GetDefaultAudioEndpoint(
        eRender, eConsole, &pDevice));



    EXIT_ON_ERROR(pDevice->Activate(
        IID_IAudioClient, CLSCTX_ALL,
        NULL, (void**)&pAudioClient));



    EXIT_ON_ERROR(pAudioClient->GetMixFormat(&pwfx));

    EXIT_ON_ERROR(pAudioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        0,
        hnsRequestedDuration,
        0,
        pwfx,
        NULL));

    IAudioClock* pClock;

    EXIT_ON_ERROR(pAudioClient->GetService(IID_PPV_ARGS(&pClock)));

    FillOffsets(pDevice, pAudioClient, pClock);

    EXIT_ON_ERROR(InstallComInterfaceHooks(pDevice, IID_IMMDevice));
    EXIT_ON_ERROR(InstallComInterfaceHooks(pAudioClient, IID_IAudioClient));
    EXIT_ON_ERROR(InstallComInterfaceHooks(pClock, IID_IAudioClock));
    pClock->Release();
    pAudioClient->Release();
    pDevice->Release();
    pEnumerator->Release();
Exit:
    return;
}

// Most of the code here is quite useless (it comes from older version that used other methods of injecting), but won't be removed (too lazy) (e. g. checking if it is Chrome)

BOOL APIENTRY RegistryThreadMain(LPVOID _) {
    while (true) {
        HKEY hkey;
        DWORD dispo;
        bool isChromeExe = false;
        if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\gurux13\\ChromePatcher\\devices", NULL, NULL, NULL, KEY_READ | KEY_QUERY_VALUE, NULL, &hkey, &dispo) == ERROR_SUCCESS) { // Only continue with the DLL in Chrome processes
            WCHAR valueName[64];
            DWORD valueNameLength = 64, valueType, dwIndex = 0;
            DWORD value;
            DWORD valueLength = sizeof(DWORD);

            LSTATUS STATUS;
            while (STATUS = RegEnumValue(hkey, dwIndex, valueName, &valueNameLength, NULL, &valueType, (LPBYTE)&value, &valueLength) == ERROR_SUCCESS) {
                if (valueType == REG_DWORD) {
                    lockguard_t guard(deviceDelays.mtx);
                    std::wstring deviceId = valueName;
                    deviceDelays[deviceId] = value;
                }

                valueLength = sizeof(DWORD);
                valueNameLength = 64;
                dwIndex++;
            }

            RegCloseKey(hkey);
        }
        Sleep(1000);
    }
}

BOOL APIENTRY ThreadMain(LPVOID lpModule) {
    IFD(logger.log() << "START THREAD MAIN\n");
    std::wstring cmdLine = GetCommandLine();

    WCHAR _exePath[1024];
    GetModuleFileNameW(NULL, _exePath, 1024);
    std::wstring exePath(_exePath);

    FILE* fout = nullptr;
    FILE* ferr = nullptr;

    std::wstring mutexStr = std::wstring(L"ChromeDllMutex") + std::to_wstring(GetCurrentProcessId());
    HANDLE mutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, mutexStr.c_str()); // Never allow the dll to be injected twice
    if (mutex) {
        return TRUE;
    }
    else {
        mutex = CreateMutex(0, FALSE, mutexStr.c_str()); // Mutex closes automatically after the process exits
    }

    HANDLE proc = GetCurrentProcess();

    IFD(logger.log() << "Suspending all threads\n");
    ChromePatch::SuspendOtherThreads();
    IFD(logger.log() << "Patching audio delay\n");
    PatchAudioDelay();
    IFD(logger.log() << "Closing handle\n");
    CloseHandle(proc);
    IFD(logger.log() << "Resuming threads\n");
    ChromePatch::ResumeOtherThreads();
    IFD(logger.log() << "ALL DONE\n");
    return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: {
        DisableThreadLibraryCalls(hModule);
        ThreadMain(hModule);
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)RegistryThreadMain, NULL, NULL, NULL);
        break;
    }

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }

    return TRUE;
}
