#pragma once
#include <Windows.h>
#include <cstdint>
#include <map>
#include <functional>
#include <array>
#include <vector>

#include "../MinHook/include/MinHook.h"
#include "../RandomiserProperties/randomiser_properties.h"
//#include "../item_randomiser_globals.h"

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

	ERRandomiserBase(bool is_auto_equip, bool is_random_keys, uint64_t seed) {
		auto_equip = is_auto_equip;
		random_keys = is_random_keys;
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
		static_runes = {
			8148,	// Godrick's Great Rune
			8149,	// Radahn's Great Rune
			8151,	// Rykard's Great Rune
			10080,  // Rennala's Great Rune
		};
		excluded_items = {
			130,  // Spectral Steed Whistle
			1001, // Flask of Crimson Tears (Tutorial)
			1051, // Flask of Cerulean Tears
			8105, // Dectus Medallion (Left)
			8106, // Dectus Medallion (Right)
			8107, // Rold Medallion
			8109, // Academy Glintstone Key
			8010, // Rusty key
			8158, // Spirit Calling Bell
			8171, // Chrysalids' Memento
			8175, // Haligtree Secret Medallion (Left)
			8176, // Haligtree Secret Medallion (Right)
			8590, // Whetstone Knife
		};
		randomiser_properties = ERRandomiserProperties();
	};

	~ERRandomiserBase() {
		//MH_Uninitialize();
	};


private:
	bool FindNeededSignatures();
	bool Shuffle();
	bool ShuffleParamEntryTable(uint64_t solo_param_repository, std::wstring param_name, uint32_t seed, bool shuffle, bool scramble);
	bool ShouldRandomiseMapItem(ItemLotParam_map* param_container);
	static void RandomiseItemHook(uint64_t map_item_manager, ItemGiveStruct* item_info, void* item_details);
	uint32_t auto_equip;
	uint32_t random_keys;
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
	std::array<EquipInfo, 7> auto_equip_buffer;
	std::array<uint32_t, 4> static_runes;
	std::array<uint32_t, 13> excluded_items;
	ERRandomiserProperties randomiser_properties;
};
