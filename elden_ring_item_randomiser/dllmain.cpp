#include "item_randomiser_main.h"

std::thread main_mod_thread;

void BeginMod() {
    main_mod_thread = std::thread(ERItemRandomiserLoader::CreateModFunctions);
    return;
};

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH: {
            BeginMod();
            break;
        };
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

