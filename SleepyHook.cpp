#include <Windows.h>
#include "metahook.h"
#include "SleepyHook.h"

#include <Psapi.h>
#include <iostream>
#include <iomanip>
#define INRANGE(x,a,b)   (x >= a && x <= b)
#define GET_BYTE( x )    (GET_BITS(x[0]) << 4 | GET_BITS(x[1]))
#define GET_BITS( x )    (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))

IChatManager* g_pChatManager;
cl_enginefunc_t* g_pEngine;
int (__thiscall *Ori_OnKeyCodeTyped)(DWORD, DWORD);
int(__thiscall* Ori_LoginDlg_CallBacks)(DWORD, char*);
int(__thiscall* Ori_ConnectToServer1)(void*, DWORD, DWORD, DWORD);
int(__cdecl* Ori_ConnectToServer2)(DWORD, DWORD);

DWORD g_pfnBot_Add;
bool MP3VolChanged = false;

int __fastcall hk_OnKeyCodeTyped(int a1, int a2, int a3)
{
	int v4;
	v4 = a1;
	if (a3 == 95)
		(*(void(__thiscall**)(int, int))(*(DWORD*)a1 + 0x258))(a1, a2);
	return Ori_OnKeyCodeTyped(v4, a3);
}

std::wstring Convert2Wc(const char* c)
{
	const size_t cSize = strlen(c) + 1;
	std::wstring wc(cSize, L'#');
	mbstowcs(&wc[0], c, cSize);
	return wc;
}

int __fastcall hk_LoginDlg_CallBacks(int a1, int a2, char* a3)
{
	int result;
	char DstBuf[256];
	char Account[256];
	char Password[256];
	//Account,Password InputBox max len is 17.
	MP3VolChanged = true;

	if (!strcmp(a3, "Login"))
	{
		(*(void(__thiscall**)(DWORD, char*, signed int))(**(DWORD**)(a1 + 352) + 596))(*(DWORD*)(a1 + 352),Account,256);
		(*(void(__thiscall**)(DWORD, char*, signed int))(**(DWORD**)(a1 + 356) + 596))(*(DWORD*)(a1 + 356),Password,256);
		memcpy(&DstBuf, "", 256);
		sprintf(DstBuf, "/login %s %s", Account, Password);
		result = g_pChatManager->MessageToChat(Convert2Wc(DstBuf).c_str());
	}
	else if (!strcmp(a3, "Register"))
	{
		(*(void(__thiscall**)(DWORD, char*, signed int))(**(DWORD**)(a1 + 352) + 596))(*(DWORD*)(a1 + 352), Account, 256);
		(*(void(__thiscall**)(DWORD, char*, signed int))(**(DWORD**)(a1 + 356) + 596))(*(DWORD*)(a1 + 356), Password, 256);
		memcpy(&DstBuf, "", 256);
		sprintf(DstBuf, "/register %s %s", Account, Password);
		result = g_pChatManager->MessageToChat(Convert2Wc(DstBuf).c_str());
	}
	else
	{
		result = Ori_LoginDlg_CallBacks(a1, a3);
	}
	return result;
}


bool __fastcall hk_ConnectToServer1(void* a1, int a2, int a3, unsigned __int8 a4)
{
	int Port;
	unsigned int Address;
	Port = htons(30002);
	Address = inet_addr("26.130.196.176");
	return Ori_ConnectToServer1(a1, Address, Port, a4) != 0;
}

int hk_ConnectToServer2()
{
	int Port;
	unsigned int Address;
	Port = htons(30002);
	Address = inet_addr("26.130.196.176");
	return Ori_ConnectToServer2(Address, Port);
}

void DebugConsole()
{
	AllocConsole();
	FILE* sream;
	freopen_s(&sream, "CON", "w", stdout);
	SetConsoleTitleA("MetaHook Dev Console");
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
}

DWORD WINAPI SH_NetHookInit(LPVOID lpThreadParameter)
{
	DebugConsole();
	DWORD g_pConnectToServer = SH_FindSignature("hw.dll", "55 8B EC 6A FF 68 ? ? ? ? 64 A1 ? ? ? ? 50 64 89 25 ? ? ? ? 81 EC ? ? ? ? 89 8D ? ? ? ? 8B 8D ? ? ? ? E8 ? ? ? ? 85 C0 74 18");
	if (!g_pConnectToServer)
		MessageBoxA(0, "g_pConnectToServer == NULL!!!\nMake sure you have the latest version of the game", "Error", 0);
	MH_InlineHook((void*)g_pConnectToServer, hk_ConnectToServer1, (void*&)Ori_ConnectToServer1);

	DWORD g_pConnectToServer2 = SH_FindSignature("hw.dll", "55 8B EC 66 8B 45 0C");
	if (!g_pConnectToServer2)
		MessageBoxA(0, "g_pConnectToServer2 == NULL!!!\nMake sure you have the latest version of the game", "Error", 0);
	MH_InlineHook((void*)g_pConnectToServer2, hk_ConnectToServer2, (void*&)Ori_ConnectToServer2);
	return 0;
}

void SH_GameBotAdd()
{
	static auto fnAddBot = reinterpret_cast<int(__cdecl*)(DWORD, DWORD)>(g_pfnBot_Add);
	int ArgVal1 = 0;
	int ArgVal2 = 0;
	int Argc = g_pEngine->Cmd_Argc();
	if (Argc > 0)
	{
		ArgVal1 = atoi(g_pEngine->Cmd_Argv(1));
		if (Argc >= 2)
		{
			ArgVal2 = atoi(g_pEngine->Cmd_Argv(2));
		}
	}
	fnAddBot(ArgVal1, ArgVal2);
}

DWORD WINAPI SH_Init(LPVOID lpThreadParameter)
{
	while (!(GetModuleHandleA("client.dll")) || !(GetModuleHandleA("gameui.dll")))
		Sleep(10);
	DWORD g_pFrame_OnKeyCodeTyped = SH_FindSignature("gameui.dll", "55 8B EC 83 EC 0C 89 4D F8 8B 45 F8 8B 10 8B 4D F8 FF 92 ? ? ? ? 25 ? ? ? ? 85 C0 74 46");
	if(!g_pFrame_OnKeyCodeTyped)
		MessageBoxA(0, "Cannot Find OnKeyCodeTyped \nMake sure you have the latest version of the game.", "Error", 0);
	MH_InlineHook((void*)g_pFrame_OnKeyCodeTyped, hk_OnKeyCodeTyped, (void*&)Ori_OnKeyCodeTyped);

	DWORD g_pLoginDlg_CallBacks = SH_FindSignature("gameui.dll", "55 8B EC 81 EC ? ? ? ? 89 8D ? ? ? ? 68 ? ? ? ? 8B 45 08 50 E8 ? ? ? ? 83 C4 08 85 C0 75 7B");
	if (!g_pLoginDlg_CallBacks)
		MessageBoxA(0, "Cannot Find LoginDlgCallbacks \nMake sure you have the latest version of the game.", "Error", 0);
	MH_InlineHook((void*)g_pLoginDlg_CallBacks, hk_LoginDlg_CallBacks, (void*&)Ori_LoginDlg_CallBacks);


	g_pEngine = (cl_enginefunc_t*)(*(DWORD*)(SH_FindSignature("hw.dll", "68 ? ? ? ? FF 15 ? ? ? ? 83 C4 08 68 ? ? ? ?") + 1));
	if (!g_pEngine)
		MessageBoxA(0, "Cannot Find EngineFunc \nMake sure you have the latest version of the game.", "Error", 0);

	g_pChatManager = (IChatManager*)(*(int (**)(void))((DWORD)g_pEngine + 0x238))();
	if (!g_pChatManager)
		MessageBoxA(0, "Cannot Find ChatManager \nMake sure you have the latest version of the game.", "Error", 0);


	while (!(GetModuleHandleA("mp.dll")))
		Sleep(10);
	g_pfnBot_Add = SH_FindSignature("mp.dll", "55 8B EC 83 EC 34 33 C0 A0 ? ? ? ? 85 C0");
	if (!g_pfnBot_Add)
		MessageBoxA(0, "Cannot Find BotAddFunc \nMake sure you have the latest version of the game.", "Error", 0);
	g_pEngine->pfnAddCommand("cso_bot_add", SH_GameBotAdd);


	DWORD MP3Volume = *(DWORD*)(SH_FindSignature("hw.dll", "D9 05 ? ? ? ? DD 5D F0") + 2);
	if (!MP3Volume)
		MessageBoxA(0, "Cannot Find MP3Volume \nMake sure you have the latest version of the game.", "Error", 0);
	while (!MP3VolChanged)
	{
		*reinterpret_cast <float*> (MP3Volume) = 0.1f;
		Sleep(100);
	}
	return 0;
}

uintptr_t SH_FindSignature(const char* szModule, const char* szSignature)
{
	const char* pat = szSignature;
	DWORD firstMatch = 0;
	DWORD rangeStart = (DWORD)GetModuleHandleA(szModule);
	MODULEINFO miModInfo;
	GetModuleInformation(GetCurrentProcess(), (HMODULE)rangeStart, &miModInfo, sizeof(MODULEINFO));
	DWORD rangeEnd = rangeStart + miModInfo.SizeOfImage;
	for (DWORD pCur = rangeStart; pCur < rangeEnd; pCur++)
	{
		if (!*pat)
			return firstMatch;

		if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == GET_BYTE(pat))
		{
			if (!firstMatch)
				firstMatch = pCur;

			if (!pat[2])
				return firstMatch;

			if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
				pat += 3;

			else
				pat += 2;
		}
		else
		{
			pat = szSignature;
			firstMatch = 0;
		}
	}
	return NULL;
}