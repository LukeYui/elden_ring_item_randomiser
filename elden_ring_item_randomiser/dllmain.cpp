#include "item_randomiser_main.h"

std::thread main_mod_thread;

void BeginMod() {
	main_mod_thread = std::thread(ERItemRandomiserLoader::CreateModFunctions);
	return;
};

std::string ERItemRandomiser::module_file_path = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH: {
		//char module_dir[MAX_PATH + 1];
		//GetModuleFileNameA(hModule, module_dir, sizeof(module_dir));
		//char* module_file_path_point = strrchr(module_dir, '\\');
		//if (!module_file_path_point) {
		//	MessageBoxA(0, "Failed to parse filepath string", "Item Randomiser Mod - Error", MB_ICONERROR);
		//	throw std::runtime_error("Failed to parse filepath string");
		//};
		//*module_file_path_point = '\0';
		//ERItemRandomiser::module_file_path = module_dir;
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

