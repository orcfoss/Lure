#include "pch.h"

#include "Patches.h"

int addressOffset;

LONG Patches::Apply()
{
    DetourTransactionBegin();

    for (Patch patch : patchList)
    {
        DetourAttach(&(PVOID&)*patch.first, patch.second);
    }

    return DetourTransactionCommit();
}

VOID Patches::ResolveOffset()
{
    MEMORY_BASIC_INFORMATION mbi;
    DWORD processId = GetCurrentProcessId();
    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, processId);

    unsigned int opcode = 0xC7F18B56;
    int base = 0x00001000;

    int offset = 0x00010000;
    int searches = 0xFFF;

    for (int i = 1; i <= searches; i++)
    {
        int searchAddress = offset * i;
        int address = searchAddress + base;
        if (VirtualQuery((LPVOID)address, &mbi, sizeof(mbi)) == 0)
        {
            continue;
        }

        std::vector<int> buffer(mbi.RegionSize);

        if (!ReadProcessMemory(handle, (LPCVOID)address, &buffer[0], buffer.size(), NULL))
        {
            continue;
        }

        if (buffer[0] == opcode)
        {
            addressOffset = searchAddress;
            break;
        }

        if (searchAddress == offset * searches)
        {
            ExitProcess(EXIT_FAILURE);
        }
    }
}

INT Patches::GetAddressByOffset(int address)
{
    if (!addressOffset) ResolveOffset();
    return address + addressOffset;
}