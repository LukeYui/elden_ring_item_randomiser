#pragma once
#include <Windows.h>
#include <cstdint>
#include <map>
#include <functional>
#include <array>
#include <vector>

#include "../MinHook/include/MinHook.h"

struct Signature {
	char* signature;
	char* mask;
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

struct EquipInfo {
	uint32_t item_id = -1;
	uint32_t inventory_slot = -1;
	uint32_t equipment_slot = -1;
};

struct ItemInfo {
	ItemInfo() {
		item_id = __UINT32_MAX__;
		item_quantity = 0;
		item_quantity = 0;
		item_relayvalue = 0;
	};
	uint32_t item_id;
	uint32_t item_quantity;
	uint32_t item_relayvalue;
	uint32_t item_ashes_of_war;
};

struct ItemGiveStruct {
	uint32_t item_struct_count;
	ItemInfo item_info[6];
};

struct EquipItemStruct {
	uint8_t unused_01[8];
	uint32_t equipment_slot;
	uint8_t unused_02[76];
	uint32_t inventory_slot;
};

// Param containers / losely mapped structs with only the information needed for this mod

enum ProtectorType : uint8_t {
	protector_type_head = 0,
	protector_type_chest = 1,
	protector_type_legs = 2,
	protector_type_hands = 3,
};

struct EquipParamProtector {
	uint8_t filler_bytes[214];
	ProtectorType protector_type;
};

enum WeaponType : uint16_t {
	weapon_type_lightbow = 50,
	weapon_type_bow = 51,
	weapon_type_greatbow = 53,
	weapon_type_crossbow = 55,
	weapon_type_ballista = 56,
	weapon_type_glintstone_staff = 57,
	weapon_type_sacred_seal = 61,
	weapon_type_small_shield = 65,
	weapon_type_med_shield = 67,
	weapon_type_great_shield = 69,
	weapon_type_arrow = 81,
	weapon_type_greatarrow = 83,
	weapon_type_bolt = 85,
	weapon_type_greatbolt = 86,
};

struct ReinforceParamWeapon {
	uint8_t filler_bytes[87];
	uint8_t equip_reinforce_level;
};

struct EquipParamWeapon {
	uint8_t filler_bytes_01[92];
	uint32_t material;
	uint8_t filler_bytes_02[122];
	uint16_t reinforce_param_addoffset;
	uint8_t filler_bytes_03[202];
	WeaponType weapon_type;
};


struct EquipParamWeaponParamContainer {
	uint32_t equipparamweapon_entry_id = __UINT32_MAX__;
	EquipParamWeapon* equipparamweapon_param_entry = nullptr;
	uint32_t equipparamweapon_raw_param_entry_id = __UINT32_MAX__;
	uint32_t weaponreinforceparam_entry_id = __UINT32_MAX__;
	ReinforceParamWeapon* weaponreinforceparam_param_entry = nullptr;
	uint32_t unk = 0;
	uint64_t buffer = 0;
};

struct EquipParamProtectorParamContainer {
	uint32_t equipparamprotector_entry_id = __UINT32_MAX__;
	EquipParamProtector* equipparamprotector_param_entry = nullptr;
	uint32_t equipparamprotector_raw_param_entry_id = __UINT32_MAX__;
	uint32_t equipreinforceparam_entry_id = __UINT32_MAX__;
	void* equipreinforceparam_param_entry = nullptr;
	uint32_t unk = 0;
	uint64_t buffer = 0;
};

enum ItemType : uint8_t {
	itemtype_tools = 0,
	itemtype_key = 1,
	itemtype_crafting = 2,
	itemtype_tools_critical = 3,
	itemtype_sorceries = 5,
	itemtype_ashes = 7,
	itemtype_info = 12,
	itemtype_inaccessible = 13,
	itemtype_bolsteringmaterials = 14,
	itemtype_incantations = 16,
};

struct EquipParamGoods {
	uint8_t filler_bytes[58];
	uint16_t max_inventory;
	uint16_t unk;
	ItemType item_type;
};

struct EquipMtrlSetParam {
	uint32_t upgrade_material_id;
};

enum MapItemType : uint32_t {
	mapitemtype_goods = 1,
	mapitemtype_weapon = 2,
	mapitemtype_armour = 3,
	mapitemtype_accessory = 4,
	mapitemtype_gem = 5,
};

struct ItemLotParam_map {
	uint32_t item_id_array[8];
	MapItemType item_type_array[8];
	uint16_t unk_array_01[8];
	uint16_t unk_array_02[8];
	uint32_t item_completeevent_type[8];
};
	

struct ParamContainer {
	uint32_t entry_id = __UINT32_MAX__;
	void* param_entry = nullptr;
};

typedef void give_item_function(uint64_t, ItemGiveStruct*, void*);
typedef void equip_item_function(EquipItemStruct*);
typedef void get_equipparamweapon_entry(EquipParamWeaponParamContainer*, uint32_t);
typedef void get_equipparamprotector_entry(EquipParamProtectorParamContainer*, uint32_t);
typedef void get_equipparamgoods_entry(ParamContainer*, uint32_t);
typedef void get_equipmtrlsetparam_entry(ParamContainer*, uint32_t);
typedef int get_inventoryid(uint64_t, uint32_t*);

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
		randomise_property_functions.emplace(0, &ERRandomiserBase::RandomiseProperty_Weapon);
		randomise_property_functions.emplace(1, &ERRandomiserBase::RandomiseProperty_Armour);
		randomise_property_functions.emplace(2, &ERRandomiserBase::RandomiseProperty_Accessory);
		randomise_property_functions.emplace(4, &ERRandomiserBase::RandomiseProperty_Goods);
		auto_equip_buffer.fill(EquipInfo());
		specialweapon_max_converter = { 0, 0, 1, 1, 1, 2, 2, 3, 3, 3, 3, 4, 4, 5, 5, 5, 6, 6, 7, 7, 7, 8, 8, 9, 9, 9, 10 };
		excluded_items = {
			130,  // Spectral Steed Whistle
			1001, // Flask of Crimson Tears (Tutorial)
			1051, // Flask of Cerulean Tears
			8105, // Dectus Medallion (Left)
			8106, // Dectus Medallion (Right)
			8107, // Rold Medallion
			8109, // Academy Glintstone Key
			8158, // Spirit Calling Bell
		};
		static_runes = {
			8148,	// Godrick's Great Rune
			8149,	// Radahn's Great Rune
			8151,	// Rykard's Great Rune
			10080,  // Rennala's Great Rune
		};
		ashes_selection = {
			10000,	//Ash of War: Lion's Claw
			10100,	//Ash of War: Impaling Thrust
			10200,	//Ash of War: Piercing Fang
			10300,	//Ash of War: Spinning Slash
			10500,	//Ash of War: Charge Forth
			10600,	//Ash of War: Stamp (Upward Cut)
			10700,	//Ash of War: Stamp (Sweep)
			10800,	//Ash of War: Blood Tax
			10900,	//Ash of War: Repeating Thrust
			11000,	//Ash of War: Wild Strikes
			11100,	//Ash of War: Spinning Strikes
			11200,	//Ash of War: Double Slash
			11300,	//Ash of War: Prelate's Charge
			11400,	//Ash of War: Unsheathe
			11500,	//Ash of War: Square Off
			11600,	//Ash of War: Giant Hunt
			11800,	//Ash of War: Loretta's Slash
			11900,	//Ash of War: Poison Moth Flight
			12000,	//Ash of War: Spinning Weapon
			12200,	//Ash of War: Storm Assault
			12300,	//Ash of War: Stormcaller
			12400,	//Ash of War: Sword Dance
			20000,	//Ash of War: Glintblade Phalanx
			20100,	//Ash of War: Sacred Blade
			20200,	//Ash of War: Ice Spear
			20300,	//Ash of War: Glintstone Pebble
			20400,	//Ash of War: Bloody Slash
			20500,	//Ash of War: Lifesteal Fist
			20700,	//Ash of War: Eruption
			20800,	//Ash of War: Prayerful Strike
			20900,	//Ash of War: Gravitas
			21000,	//Ash of War: Storm Blade
			21200,	//Ash of War: Earthshaker
			21300,	//Ash of War: Golden Land
			21400,	//Ash of War: Flaming Strike
			21600,	//Ash of War: Thunderbolt
			21700,	//Ash of War: Lightning Slash
			21800,	//Ash of War: Carian Grandeur
			21900,	//Ash of War: Carian Greatsword
			22000,	//Ash of War: Vacuum Slice
			22100,	//Ash of War: Black Flame Tornado
			22200,	//Ash of War: Sacred Ring of Light
			22400,	//Ash of War: Blood Blade
			22500,	//Ash of War: Phantom Slash
			22600,	//Ash of War: Spectral Lance
			22700,	//Ash of War: Chilling Mist
			22800,	//Ash of War: Poisonous Mist
			30000,	//Ash of War: Shield Bash
			30100,	//Ash of War: Barricade Shield
			30200,	//Ash of War: Parry
			30500,	//Ash of War: Carian Retaliation
			30600,	//Ash of War: Storm Wall
			30700,	//Ash of War: Golden Parry
			30800,	//Ash of War: Shield Crash
			30900,	//Ash of War: No Skill
			31000,	//Ash of War: Thops's Barrier
			40000,	//Ash of War: Through and Through
			40100,	//Ash of War: Barrage
			40200,	//Ash of War: Mighty Shot
			40400,	//Ash of War: Enchanted Shot
			40500,	//Ash of War: Sky Shot
			40600,	//Ash of War: Rain of Arrows
			50100,	//Ash of War: Hoarfrost Stomp
			50200,	//Ash of War: Storm Stomp
			50300,	//Ash of War: Kick
			50400,	//Ash of War: Lightning Ram
			50500,	//Ash of War: Flame of the Redmanes
			50600,	//Ash of War: Ground Slam
			50700,	//Ash of War: Golden Slam
			50800,	//Ash of War: Waves of Darkness
			50900,	//Ash of War: Hoarah Loux's Earthshaker
			60000,	//Ash of War: Determination
			60100,	//Ash of War: Royal Knight's Resolve
			60200,	//Ash of War: Assassin's Gambit
			60300,	//Ash of War: Golden Vow
			60400,	//Ash of War: Sacred Order
			60500,	//Ash of War: Shared Order
			60600,	//Ash of War: Seppuku
			60700,	//Ash of War: Cragblade
			65000,	//Ash of War: Barbaric Roar
			65100,	//Ash of War: War Cry
			65200,	//Ash of War: Beast's Roar
			65300,	//Ash of War: Troll's Roar
			65400,	//Ash of War: Braggart's Roar
			70000,	//Ash of War: Endure
			70100,	//Ash of War: Vow of the Indomitable
			70200,	//Ash of War: Holy Ground
			80000,	//Ash of War: Quickstep
			80100,	//Ash of War: Bloodhound's Step
			80200,	//Ash of War: Raptor of the Mists
			85000,	//Ash of War: White Shadow's Lure
		};
	};

	~ERRandomiserBase() {
		//MH_Uninitialize();
	};


private:
	bool FindNeededSignatures();
	bool Shuffle();
	bool ShuffleParamEntryTable(std::wstring param_name, uint32_t seed);
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
	std::map<uint32_t, bool(ERRandomiserBase::*)(ItemInfo*, uint32_t)> randomise_property_functions;
	bool RandomiseProperty_Weapon(ItemInfo* item_info, uint32_t entry);
	bool RandomiseProperty_Armour(ItemInfo* item_info, uint32_t entry);
	bool RandomiseProperty_Accessory(ItemInfo* item_info, uint32_t entry);
	bool RandomiseProperty_Goods(ItemInfo* item_info, uint32_t entry);
	std::array<EquipInfo, 7> auto_equip_buffer;
	std::array<uint8_t, 27> specialweapon_max_converter;
	std::array<uint32_t, 8> excluded_items;
	std::array<uint32_t, 4> static_runes;
	std::array<uint32_t, 91> ashes_selection;
};
