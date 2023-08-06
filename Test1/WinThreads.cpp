#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>

#ifdef UNICODE
#define tcout std::wcout
#elif
#define tcout std::cout
#endif 


class SmartHandle
{
public:
	SmartHandle(HANDLE handle)
	{
		_handle = handle;
	}
	~SmartHandle()
	{
		if (_handle)
		{
			CloseHandle(_handle);
		}
	}

	operator bool()
	{
		return _handle != NULL;
	}

	operator HANDLE()
	{
		return _handle;
	}

	HANDLE handle()
	{
		return _handle;
	}

private:
	HANDLE _handle = NULL;
};

struct ProcessINFO
{
	PROCESSENTRY32 pe;
	std::vector<THREADENTRY32> threads;
};

int main()
{
	setlocale(LC_ALL, "ru");
	SmartHandle processSnap(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
	SmartHandle threadSnap(CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0));
	if (!processSnap || !threadSnap)
	{
		return 1;
	}
	std::vector<ProcessINFO> processInfos;
	std::vector<THREADENTRY32> threads;
	
	THREADENTRY32 te{ sizeof(THREADENTRY32) };
	if (Thread32First(threadSnap, &te) == FALSE)
	{
		return 2;
	}
	do
	{
		threads.push_back(te);
	} while (Thread32Next(threadSnap, &te));
	
	PROCESSENTRY32 pe{ sizeof(PROCESSENTRY32) };
	if (Process32First(processSnap, &pe) == FALSE)
	{
		return 2;
	}
	do
	{
		std::vector<THREADENTRY32> subThreads;
		for (const auto& thread : threads)
		{
			if (thread.th32OwnerProcessID == pe.th32ProcessID)
			{
				subThreads.push_back(thread);
			}
		}
		processInfos.push_back(ProcessINFO{ pe, subThreads });
	} while (Process32Next(processSnap, &pe));
	
	for (const auto& ProcessINFO : processInfos)
	{
		tcout << ProcessINFO.pe.szExeFile << std::endl;
		for (const auto& thread : ProcessINFO.threads)
		{
			tcout << "    " << thread.th32ThreadID << std::endl;
		}
	}

	return 0;
}