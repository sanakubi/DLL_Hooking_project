#include <process.h>
#include <Windows.h>
#include <iostream>

int main() {
    while (1) { printf("Process id: %d\n", _getpid()); Sleep(2000); }
    return 0;
}