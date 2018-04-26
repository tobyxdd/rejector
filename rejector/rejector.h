#pragma once

#include "stdafx.h"

#define DLLNAME_KERNEL32		L"kernel32.dll"

#define APINAME_LOADLIBRARYW	"LoadLibraryW"

HANDLE GetProcessHandle(const wchar_t* processName);

bool LoadRemoteDLL(HANDLE hProcess, const wchar_t* dllPath);
