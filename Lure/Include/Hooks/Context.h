#pragma once

#include "Configuration.h"

#ifdef _DEBUG

const auto Context__isInRole = (bool(__cdecl*)(int identity, int permission))ADDRESS_CONTEXT__ISINROLE;

typedef void(__thiscall* Context__requirePermission_t)(void* _this, int permission, const char* operation);
void __fastcall Context__requirePermission_hook(void* _this, void*, int permission, const char* operation);
extern Context__requirePermission_t Context__requirePermission;

#endif