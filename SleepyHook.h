#pragma once
#include <Windows.h>
#include "HLSDK/Manager/IChatManager.h"
#include "xorstr.h"

DWORD WINAPI SH_Init(LPVOID lpThreadParameter);
DWORD WINAPI SH_NetHookInit(LPVOID lpThreadParameter);
uintptr_t SH_FindSignature(const char* szModule, const char* szSignature);
extern IChatManager* g_pChatManager;
extern cl_enginefunc_t* g_pEngine;