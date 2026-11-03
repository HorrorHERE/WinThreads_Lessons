#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include "RPS_h.h"
using namespace std;
#pragma  comment(lib,"rpcrt4.lib")

#define BUF_LEN 128
char username[BUF_LEN];
char password[BUF_LEN];
RPC_BINDING_HANDLE binding;
bool is_logged = false;
int printError(const char* func, int code)
{
	printf("%s - Error code: %d\n", func, code);
	exit(code);
}

void usage()
{
	printf(
		"Options:\n"
		"1)Upload\n"
		"2)Download\n"
		"3)Delete\n"
		"4)Logout\n");
}

void login()
{

	printf("\nUsername:");
	if (!scanf("%100s", username))
		exit(-1);
	username[strlen(username)] = '\0';

	printf("\nPassword:");
	if (!scanf("%100s", password))
		exit(-1);
	password[strlen(password)] = '\0';

	if (Login(binding, (const unsigned char*)username, (const unsigned char*)password))
	{
		puts("Wrong login or password");
		exit(0);
	}
	printf("Hello %s!\n\n", username);
	is_logged = 1;
	usage();
}

void do_upload(handle_t binding)
{
	printf("Enter path of SRC file(UNC format): ");
	char uploadPath[101];
	char n = getchar();
	fgets(uploadPath, 100, stdin);
	printf("Enter path of DST file(UNC format): ");
	char savePath[101];
	fgets(savePath, 100, stdin);

	// Удаляем символ новой строки
	uploadPath[strcspn(uploadPath, "\n")] = '\0';
	savePath[strcspn(savePath, "\n")] = '\0';
	printf("\n");

	FILE* inFile = fopen(uploadPath, "rb");
	if (inFile == NULL) {
		perror("Error opening source file");
		return;
	}

	// Получаем размер файла
	fseek(inFile, 0, SEEK_END);
	long fileSize = ftell(inFile);
	fseek(inFile, 0, SEEK_SET);

	if (fileSize <= 0) {
		printf("Error: File is empty or cannot determine size\n");
		fclose(inFile);
		return;
	}

	size_t bytesRead = 0;
	size_t totalBytesSent = 0;
	char* buf = (char*)malloc(1024 * 1024 * 2); // 1 Мб буфер

	if (buf == NULL) {
		perror("Error allocating memory");
		fclose(inFile);
		return;
	}

	// Читаем файл и передаем его по частям
	while ((bytesRead = fread(buf, 1, 1024 * 1024, inFile)) > 0) {
		int ret = СopyFileFromServer(binding, (const unsigned char*)savePath, (unsigned char*)buf, bytesRead, 0, 1);
		if (ret != 0) {
			// Обработка ошибок
			if (ret == EACCES) {
				printf("Error: Access denied\n");
			}
			else if (ret == ENOENT) {
				printf("Error: Incorrect path\n");
			}
			else {
				printf("Error: Unknown error occurred (code: %d)\n", ret);
			}
			free(buf);
			fclose(inFile);
			return;
		}
		totalBytesSent += bytesRead;
		printf("Uploaded %zu of %ld bytes\n", totalBytesSent, fileSize);
		Sleep(800); // Задержка между передачами
	}

	free(buf);
	fclose(inFile);
	printf("File %s fully uploaded on servers\n", uploadPath);
}



void do_download(handle_t binding)
{
	printf("Path of dest file: ");
	char n = getchar();
	char savePath[101];
	fgets(savePath, 100, stdin);
	printf("Path of source file: ");
	char downloadPath[101];
	fgets(downloadPath, 100, stdin);

	// Удаляем символ новой строки
	downloadPath[strcspn(downloadPath, "\n")] = '\0';
	savePath[strcspn(savePath, "\n")] = '\0';

	unsigned char inputBuffer[65536];
	int size = 0;
	int ret, offset = 0;
	FILE* outFile = fopen(savePath, "wb");
	if (outFile == NULL) {
		perror("Error opening destination file");
		return;
	}

	do
	{
		ret = DownloadFile(binding, (const unsigned char*)downloadPath, inputBuffer, &size, offset);
		if (ret != 0) {
			// Обработка ошибок
			if (ret == EACCES) {
				printf("Error: Access denied\n");
				remove(savePath);
			}
			else if (ret == ENOENT) {
				printf("Error: Incorrect path\n");
				remove(savePath);
			}
			else {
				printf("Error: Unknown error occurred (code: %d)\n", ret);
			}
			fclose(outFile);
			return;
		}

		// Записываем данные в файл
		size_t wrote = fwrite(inputBuffer, 1, size, outFile);
		if (wrote < size) {
			printf("Error writing to file\n");
			fclose(outFile);
			return;
		}

		offset += size;
	} while (size > 0);

	fclose(outFile);
	printf("File %s fully downloaded from server\n", downloadPath);
}


void do_delete(handle_t binding)
{
	printf("Path of file: ");
	char n = getchar();
	char deletePath[101];
	fgets(deletePath, 100, stdin);
	deletePath[strlen(deletePath) - 1] = '\0';
	int ret = DeleteFileFromServer(binding, (const unsigned char*)deletePath);
	if (!ret)
		printf("%s deleted\n", deletePath);
	else if (ret == EACCES)
		printf("Access denied\n");
	else if (ret == ENOENT)
		printf("Incorrect path\n");
}

void do_logout(handle_t binding)
{
	is_logged = false;
	Logout(binding);
}


int main()
{
	RPC_STATUS status;
	RPC_CSTR szStringBinding = NULL;

	char addr[20];
	char port[5];

	printf("Enter IP address and port (format: address:port): ");
	char input[25];
	scanf("%s", input);

	sscanf(input, "%19[^:]:%4s", addr, port);

	//printf("Address: %s\n", addr);
	//printf("Port: %s\n", port);

	status = RpcStringBindingComposeA(NULL, (RPC_CSTR)("ncacn_ip_tcp"), (RPC_CSTR)(addr), (RPC_CSTR)(port),	NULL, &szStringBinding);

	if (status)
		printError("RpcStringBindingComposeA", status);

	status = RpcBindingFromStringBindingA(szStringBinding, &binding);

	if (status)
		printError("RpcBindingFromStringBindingA", status);

	RpcTryExcept
	{
		int input = -1;
		while (input != 5)
		{
			if (is_logged)
				usage();
			else
				login();
			scanf("%d", &input);
			switch (input)
			{
			case 1:
				if (is_logged)
					do_upload(binding);
				break;
			case 2:
				if (is_logged)
					do_download(binding);
				break;
			case 3:
				if (is_logged)
					do_delete(binding);
				break;
			case 4:
				if (is_logged)
					do_logout(binding);
				break;

			}
		}

	}
		RpcExcept(1)
	{
		printf("Runtime reported exception %ld\n", RpcExceptionCode());
	}
	RpcEndExcept

		status = RpcStringFreeA(&szStringBinding);
	if (status)
		printError("RpcStringFreeA", status);

	status = RpcBindingFree(&binding);
	if (status)
		printError("RpcBindingFree", status);
	printf("Session successfully ended\n");
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