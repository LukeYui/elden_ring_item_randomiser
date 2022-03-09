#include "item_randomiser_main.h"

ERItemRandomiser* main_mod = nullptr;

void ERItemRandomiserLoader::CreateModFunctions() {

	using namespace std::chrono_literals;

	// Wait for Elden Ring
	std::this_thread::sleep_for(5s);

	main_mod = new ERItemRandomiser();

	main_mod->RunSaveListener();

	delete main_mod;
	return;
};

void ERItemRandomiser::RunSaveListener() {


	using namespace std::chrono_literals;

	if (!GetUserPreferences()) {
		//
	};

	hook_class = ERItemRandomiserHooks(user_preferences & option_autoequip, randomiser_seed);
	if (!hook_class.CreateMemoryEdits()) {
		//
		return;
	};

	while (is_mod_active) {

		std::this_thread::sleep_for(5s);
		/*
		std::unique_lock<std::mutex> unique_lock(save_request_mutex);
		save_request_wait.wait(unique_lock, [&] { return save_manager_wake; });
		if (signal_list_save) {
			// Save... (no longer needed)
		};	
		*/
	};

	return;
};

bool ERItemRandomiser::GetUserPreferences() {

	// INIReader
	INIReader option_reader = INIReader("ItemRandomiser//randomiserpreferences.ini");
	int error = option_reader.ParseError();
	if (error) {
		//
		return false;
	};

	std::string header_segment = "MOD";
	user_preferences = option_reader.GetBoolean(header_segment, "autoequip", true) ? static_cast<UserPreferences>(user_preferences | option_autoequip) : user_preferences;

	// Seed
	OFSTRUCT file_struct = {};
	HFILE seed_file = OpenFile("ItemRandomiser//randomiser_seed.txt", &file_struct, OF_READWRITE);
	if (seed_file == HFILE_ERROR) {
		char to_write_seed[24] = {};
		LARGE_INTEGER timestamp_counter = {};
		QueryPerformanceCounter(&timestamp_counter);
		int bytes_size = sprintf_s(to_write_seed, "%016llX", timestamp_counter.QuadPart);
		randomiser_seed = timestamp_counter.QuadPart;

		HANDLE new_file_creation = CreateFileA("ItemRandomiser//randomiser_seed.txt", GENERIC_READ | GENERIC_WRITE, 0,
			nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);

		if (new_file_creation == INVALID_HANDLE_VALUE) {

			return false;
		};

		if (!WriteFile(new_file_creation, to_write_seed, bytes_size, nullptr, nullptr)) {

			return false;
		};

		CloseHandle(new_file_creation);
		return true;
	}
	else {
		char file_contents[50] = {};
		DWORD size = 0;
		if (!ReadFile((HANDLE)seed_file, file_contents, sizeof(file_contents), &size, nullptr) || size < 16) {

			return false;
		};
		uint64_t seed = std::strtoull(file_contents, nullptr, 16);
		randomiser_seed = seed;
		CloseHandle((HANDLE)seed_file);
	};

	return true;
};

void ERItemRandomiser::RequestItemListSave(bool request_save) {
	signal_list_save = request_save;
	save_manager_wake = true;
	save_request_wait.notify_one();
	return;
};

uint32_t ERItemRandomiser::GetRandomUint(uint32_t min, uint32_t max) {
	std::random_device random_number_device;
	std::mt19937 random_number_generator(random_number_device());
	std::uniform_int_distribution<std::mt19937::result_type> random_number_distributer(min, max);
	return random_number_distributer(random_number_generator);
};
