//#pragma once
#include <cstdint>
#include <windows.h>
#include <functional>
#include <string>

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
	ItemInfo item_info[10];
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

struct GenericParamEntryTableStruct {
	uint32_t param_entry_iteration;
	uint32_t param_offset;
};


typedef void give_item_function(uint64_t, ItemGiveStruct*, void*);
typedef void equip_item_function(EquipItemStruct*);
typedef void get_equipparamweapon_entry(EquipParamWeaponParamContainer*, uint32_t);
typedef void get_equipparamprotector_entry(EquipParamProtectorParamContainer*, uint32_t);
typedef void get_equipparamgoods_entry(ParamContainer*, uint32_t);
typedef void get_equipmtrlsetparam_entry(ParamContainer*, uint32_t);
typedef int get_inventoryid(uint64_t, uint32_t*);

