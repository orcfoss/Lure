#include "pch.h"
#include "Configuration.h"
#include "Patches.h"

#include "Hooks/Http.h"
#include "Hooks/Crypt.h"

#ifdef _DEBUG
#include "Hooks/Context.h"
#endif

START_PATCH_LIST()

ADD_PATCH(Http__httpGetPostWinInet, Http__httpGetPostWinInet_hook)
ADD_PATCH(Http__trustCheck, Http__trustCheck_hook)

ADD_PATCH(Crypt__verifySignatureBase64, Crypt__verifySignatureBase64_hook)

#ifdef _DEBUG
ADD_PATCH(Context__requirePermission, Context__requirePermission_hook)
#endif

END_PATCH_LIST()

void __declspec(dllexport) import() {}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        LONG patchesError = Patches::Apply();
        if (patchesError != NO_ERROR)
        {
#ifdef _DEBUG
            std::string message = "Patches::Apply returned " + std::to_string(patchesError);
            MessageBoxA(nullptr, message.c_str(), nullptr, MB_ICONERROR);
#endif

            ExitProcess(EXIT_FAILURE);
        }

        CURLcode curlError = curl_global_init(CURL_GLOBAL_DEFAULT);
        if (curlError != CURLE_OK)
        {
#ifdef _DEBUG
            std::string message = "curl_global_init returned " + std::to_string(curlError);
            MessageBoxA(nullptr, message.c_str(), nullptr, MB_ICONERROR);
#endif

            ExitProcess(EXIT_FAILURE);
        }
    }

    if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        curl_global_cleanup();
    }

    return TRUE;
}