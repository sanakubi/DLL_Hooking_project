#include "pch.h"

#define SIZE 6
typedef int (WINAPI* hkFunction)(HWND, LPCWSTR, LPCWSTR, UINT);

VOID WINAPI mysleep(DWORD);
void redirect(DWORD);
VOID loging(char msg[]);
hkFunction orig_func_addr = NULL;
BYTE oldBytes[SIZE] = { 0 };
BYTE JMP[SIZE] = { 0 };
DWORD oldProtect, myProtect = PAGE_EXECUTE_READWRITE;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved){
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        orig_func_addr = (hkFunction)GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "Sleep");
        if (orig_func_addr != NULL){
            redirect((DWORD)&mysleep);
        }
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return true;
}

VOID redirect(DWORD arg){
    BYTE tempJMP[SIZE] = { 0xE9, 0x90, 0x90, 0x90, 0x90, 0xC3 };
    memcpy(JMP, tempJMP, SIZE);

    DWORD JMPSize = (arg - (DWORD)orig_func_addr - 5);

    VirtualProtect((LPVOID)orig_func_addr, SIZE, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy(oldBytes, orig_func_addr, SIZE);
    memcpy(&JMP[1], &JMPSize, 4);
    memcpy(orig_func_addr, JMP, SIZE);
    VirtualProtect((LPVOID)orig_func_addr, SIZE, oldProtect, NULL);
}

VOID WINAPI mysleep(DWORD dwMillisecondse){
    VirtualProtect((LPVOID)orig_func_addr, SIZE, myProtect, NULL);
    memcpy(orig_func_addr, oldBytes, SIZE);

    Sleep(dwMillisecondse);
    //MessageBoxW(NULL, L"Worked", L"Hooked", MB_OK);
    char name[] = "Sleep #";
    loging(name);

    memcpy(orig_func_addr, JMP, SIZE);
    VirtualProtect((LPVOID)orig_func_addr, SIZE, oldProtect, NULL);
}

VOID loging(char buff[]) {
        HANDLE hPipe;
        DWORD dwWritten;

        hPipe = CreateFile(TEXT("\\\\.\\pipe\\Pipe"), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hPipe != INVALID_HANDLE_VALUE){
            char buffer[60];
            time_t seconds = time(NULL);
            tm* timeinfo = localtime(&seconds);
            const char* format = "%d %Y %I:%M:%S";
            strftime(buffer, 80, format, timeinfo);
            char msg[100];
            sprintf(msg, "%s %s\n", buff, buffer);
            WriteFile(hPipe, msg, sizeof(msg), &dwWritten, NULL);
            CloseHandle(hPipe);
        }
}