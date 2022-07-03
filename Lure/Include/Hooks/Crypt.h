#pragma once

#include <wincrypt.h>

#include "Configuration.h"

typedef void(__thiscall* Crypt__verifySignatureBase64_t)(HCRYPTPROV* _this, int a2, BYTE* pbData, int a4, int a5, int a6, DWORD dwDataLen, int a8, int a9, int a10, int a11, int a12, int a13, int a14, int a15);
void __fastcall Crypt__verifySignatureBase64_hook(HCRYPTPROV* _this, void*, int a2, BYTE* pbData, int a4, int a5, int a6, DWORD dwDataLen, int a8, int a9, int a10, int a11, int a12, int a13, int a14, int a15);
extern Crypt__verifySignatureBase64_t Crypt__verifySignatureBase64;

class Crypt
{
    HCRYPTPROV context;
    HCRYPTKEY key;

public:
    Crypt();
    ~Crypt();
    bool verifySignatureBase64(std::string message, std::string signatureBase64, ALG_ID algorithm);
};