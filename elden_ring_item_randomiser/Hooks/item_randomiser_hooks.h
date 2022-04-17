#pragma once
#include <Windows.h>
#include <cstdint>
#include <map>
#include <functional>
#include <array>
#include <vector>

#include "../MinHook/include/MinHook.h"
#include "../Randomiser/randomiser.h"

struct Signature {
	const char* signature;
	const char* mask;
	size_t length;
	DWORD ret;
};

class SigScan {
public:
	bool GetImageInfo();
	void* FindSignature(Signature& fnSig);

private:
	HMODULE module_handle;
	void* base_address = nullptr;
	size_t image_size = 0;
};

class ERRandomiserBase {
public:
	bool CreateMemoryEdits();

	ERRandomiserBase() {

	};

	ERRandomiserBase(bool is_auto_equip, bool is_random_keys, bool is_randomise_estusupgrade, bool is_randomise_mtrlupgrade, uint64_t seed) {
		auto_equip = is_auto_equip;
		random_keys = is_random_keys;
		random_keys = is_random_keys;
		randomise_estusupgrade = is_randomise_estusupgrade;
		randomise_mtrlupgrade = is_randomise_mtrlupgrade;
		mapitem_seed = (uint32_t)seed;
		enemyitem_seed = (uint32_t)(seed >> 32 ^ mapitem_seed);
		randomkey_seed = 0 - (mapitem_seed + enemyitem_seed);
		static_rune_01 = 0;
		static_rune_02 = 0;
		game_data_manager_address = 0;
		game_data_manager_class = 0;
		solo_param_repository_address = 0;
		solo_param_repository_class = 0;
		item_give_hook_address_map = 0;
		item_give_hook_address_lua = 0;
		item_give_address = 0;
		equip_item_address = 0;
		item_give_function = nullptr;
		item_equip_function = nullptr;
		find_equipparamweapon_function = nullptr;
		find_equipparamprotector_function = nullptr;
		find_equipparamgoods_function = nullptr;
		find_equipmtrlsetparam_function = nullptr;
		find_inventoryid_function = nullptr;
		minhook_active = MH_UNKNOWN;
		signature_class = SigScan();
		auto_equip_buffer.fill(EquipInfo());
		randomiser = ERRandomiser();

	};

	~ERRandomiserBase() {
		//MH_Uninitialize() ;
	};


private:
	bool FindNeededSignatures();
	static void RandomiseItemHook(uint64_t map_item_manager, ItemGiveStruct* item_info, void* item_details);
	uint32_t auto_equip;
	uint32_t random_keys;
	uint32_t randomise_estusupgrade;
	uint32_t randomise_mtrlupgrade;
	uint32_t mapitem_seed;
	uint32_t enemyitem_seed;
	uint32_t randomkey_seed;
	uint32_t static_rune_01;
	uint32_t static_rune_02;
	uint64_t game_data_manager_address;
	uint64_t game_data_manager_class;
	uint64_t solo_param_repository_address;
	uint64_t solo_param_repository_class;
	uint64_t item_give_hook_address_map;
	uint64_t item_give_hook_address_lua;
	uint64_t item_give_address;
	uint64_t equip_item_address;
	give_item_function* item_give_function;
	equip_item_function* item_equip_function;
	get_equipparamweapon_entry* find_equipparamweapon_function;
	get_equipparamprotector_entry* find_equipparamprotector_function;
	get_equipparamgoods_entry* find_equipparamgoods_function;
	get_equipmtrlsetparam_entry* find_equipmtrlsetparam_function;
	get_inventoryid* find_inventoryid_function;
	void* save_extension_address;
	MH_STATUS minhook_active;
	SigScan signature_class;
	std::array<EquipInfo, 11> auto_equip_buffer;
	ERRandomiser randomiser;
};
