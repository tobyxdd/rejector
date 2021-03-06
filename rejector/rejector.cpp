// rejector.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "rejector.h"

HANDLE GetProcessHandle(const wchar_t* processName) {
	HANDLE hProcessSnap;
	HANDLE hProcess;
	PROCESSENTRY32 pe32;

	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE) {
		return NULL;
	}

	pe32.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hProcessSnap, &pe32)) {
		CloseHandle(hProcessSnap);
		return NULL;
	}

	do {
		if (!wcscmp(pe32.szExeFile, processName)) {
			hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
			if (hProcess != NULL) {
				return hProcess;
			}
			else {
				return NULL;
			}
		}
	} while (Process32Next(hProcessSnap, &pe32));

	return NULL;
}

bool LoadRemoteDLL(HANDLE hProcess, const wchar_t* dllPath) {
	auto pathLen = wcslen(dllPath) * sizeof(wchar_t);

	LPVOID dllPathAddressInRemoteMemory = VirtualAllocEx(hProcess, NULL, pathLen, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (dllPathAddressInRemoteMemory == NULL) {
		return false;
	}

	//Write DLL path to remote process
	BOOL succeededWriting = WriteProcessMemory(hProcess, dllPathAddressInRemoteMemory, dllPath, pathLen, NULL);

	if (!succeededWriting) {
		return false;
	}
	else {
		//Pointer to the LoadLibrary address
		LPVOID loadLibraryAddress = (LPVOID)GetProcAddress(GetModuleHandle(DLLNAME_KERNEL32), APINAME_LOADLIBRARYW);
		if (loadLibraryAddress == NULL) {
			return false;
		}
		else {
			HANDLE remoteThread = CreateRemoteThread(hProcess, NULL, NULL, (LPTHREAD_START_ROUTINE)loadLibraryAddress, dllPathAddressInRemoteMemory, NULL, NULL);
			if (remoteThread == NULL) {
				return false;
			}
		}
	}

	CloseHandle(hProcess);
	return TRUE;
}

int wmain(int argc, wchar_t* argv[]) {
	if (argc != 3) {
		std::cout << "Invalid arguments!" << std::endl;
		return 1;
	}
	auto handle = GetProcessHandle(argv[1]);
	if (handle != NULL) {
		auto succeeded = LoadRemoteDLL(handle, argv[2]);
		if (succeeded) {
			std::cout << "Done." << std::endl;
			return 0;
		}
		else {
			std::cout << "Failed to inject!" << std::endl;
			return 3;
		}
	}
	else {
		std::cout << "Failed to get process handle!" << std::endl;
		return 2;
	}
}
