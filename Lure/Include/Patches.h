#pragma once

#include <detours/detours.h>

namespace Patches
{
    typedef std::pair<void**, void*> Patch;

    extern std::vector<Patch> patchList;

    LONG Apply();
    VOID ResolveOffset();
    INT GetAddressByOffset(int address);
}

#define START_PATCH_LIST() std::vector<Patches::Patch> Patches::patchList = {
#define ADD_PATCH(a, b) { (void**)&(a), (b) }, 
#define END_PATCH_LIST() };

#ifdef RESOLVE_OFFSETS
#define LOCATION(address) Patches::GetAddressByOffset(address)
#else
#define LOCATION(address) address
#endif

#ifndef _DEBUG
#define PADDING_STRUCT 1
#else
#define PADDING_STRUCT 0
#endif