#pragma once
#include <Windows.h>
#include <thread>
#include <condition_variable>
#include <random>
#include <string>

#include "INIReader/INIReader.h"
#include "Hooks/item_randomiser_hooks.h"

namespace ERItemRandomiserLoader {
	void CreateModFunctions();
};

enum UserPreferences {
	option_none = 0,
	option_autoequip = 1 << 0,
	option_randomisemaps = 1 << 1,
};

class ERItemRandomiser {
public:
	void RunSaveListener();
	bool GetUserPreferences();
	void RequestItemListSave(bool request_save = true);
	uint32_t GetRandomUint(uint32_t min, uint32_t max);

	ERItemRandomiser() {
		hook_class = {};
		is_mod_active = 1;
		user_preferences = option_none;
		signal_list_save = false;
		save_manager_wake = false;
		randomiser_seed = 0;
	};

	ERItemRandomiserHooks hook_class;

private:
	uint32_t is_mod_active;
	UserPreferences user_preferences;
	uint64_t randomiser_seed;
	bool signal_list_save;
	bool save_manager_wake;
	std::mutex save_request_mutex;
	std::condition_variable save_request_wait;

};
