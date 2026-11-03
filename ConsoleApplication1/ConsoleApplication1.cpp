#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <windows.h>
#include "RPS_h.h"

#pragma comment(lib, "rpcrt4.lib")

handle_t handler = 0;

void printError(const char* func, int code) {
    printf("%s - Error code: %d\n", func, code);
    exit(code);
}

int DeleteFileFromServer(handle_t handler, const unsigned char* path)
{
    printf("deleting %s\n", path);
    if (remove((const char*)path) == -1)
    {
        switch (errno)
        {
        case EACCES:
            printf("Only for reading!\n");
            break;
        case ENOENT:
            printf("Incorrect path!\n");
            break;
        }
        return errno;
    }
    printf("File %s deleted!\n", path);
    return 0;
}

int CopyFile(handle_t handler, const unsigned char* path, unsigned char* data, int size, int offset, int mode)
{
    FILE* inputFile = NULL;
    if (mode == 0)
        inputFile = fopen((const char*)path, "wb+");
    else
    {
        inputFile = fopen((const char*)path, "ab+");
        while (inputFile == NULL)
        {
            inputFile = fopen((const char*)path, "ab+");
            Sleep(100);
        }

    }
    if (inputFile == NULL)
    {
        if (errno == EACCES)
            printf("Access denied\n");
        else if (errno == ENOENT)
            printf("Incorrect path\n");
        else
            printf("Unknown error\n");
        return errno;
    }
    int tmp = fwrite(data, 1, size, inputFile);
    Sleep(5);
    fclose(inputFile);

    printf("%d bytes of %s fully copied!\n", size + offset, path);
    return 0;
}


int DownloadFile(handle_t handler, const unsigned char* path, unsigned char data[65536], int* size, int offset)
{
    FILE* outputFile = NULL;
    if ((outputFile = fopen((const char*)path, "rb")) == NULL)
    {
        printf("Access denied\n");
        return errno;
    }
    int read = 0, tmp = 0;
    fseek(outputFile, offset, SEEK_SET);
    tmp = fread(data, 1, 65536, outputFile);
    Sleep(20);
    read += tmp;
    fclose(outputFile);
    *size = read;
    printf("%d bytes of %s fully downloaded!\n", read, path);
    return 0;
}

int ÑopyFileFromServer(handle_t handler, const unsigned char* path, unsigned char* data, int size, int offset, int mode)
{
    FILE* inputFile = NULL;
    if (mode == 0)
        inputFile = fopen((const char*)path, "wb+");
    else
    {
        inputFile = fopen((const char*)path, "ab+");
        while (inputFile == NULL)
        {
            inputFile = fopen((const char*)path, "ab+");
            Sleep(100);
        }

    }
    if (inputFile == NULL)
    {
        return errno;
    }
    int tmp = fwrite(data, 1, size, inputFile);
    Sleep(800);
    fclose(inputFile);

    printf("File %s copied!\n Total: %d bytes\n", path, size + offset);
    return 0;
}

int Login(handle_t handler, const unsigned char* login, const unsigned char* password)
{
    if (LogonUserA((LPCSTR)login, NULL, (LPCSTR)password, LOGON32_LOGON_INTERACTIVE, LOGON32_PROVIDER_DEFAULT, &handler) == 0)
    {
        printf("Error LogonUserA: %d(user %s)\n", GetLastError(), login);
        return 1;
    }
    else
    {
        if (ImpersonateLoggedOnUser(handler) == 0)
            return 2;
        else
        {
            printf("%s connected\n", login);
            return 0;
        }
    }
}

void Logout(handle_t handler)
{
    if (RevertToSelf() == 0) {
        wprintf(L"%d\n", GetLastError());
    }
}


int main() {
    RPC_STATUS status;

    status = RpcServerUseProtseqEpA((RPC_CSTR)("ncacn_ip_tcp"), RPC_C_PROTSEQ_MAX_REQS_DEFAULT, (RPC_CSTR)("9000"), NULL);
    if (status)
        printError("RpcServerUseProtseqEpA", status);

    status = RpcServerRegisterIfEx(InterfaceRPC_v0_0_s_ifspec, NULL, NULL, RPC_IF_ALLOW_CALLBACKS_WITH_NO_AUTH, RPC_C_LISTEN_MAX_CALLS_DEFAULT, NULL);
    if (status)
        printError("RpcServerRegisterIfEx", status);

    status = RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, 0);
    if (status)
        printError("RpcServerListen", status);

    return 0;
}

void __RPC_FAR* __RPC_API midl_user_allocate(size_t len)
{
	return(malloc(len));
}

void __RPC_API midl_user_free(void __RPC_FAR* ptr)
{
	free(ptr);
}