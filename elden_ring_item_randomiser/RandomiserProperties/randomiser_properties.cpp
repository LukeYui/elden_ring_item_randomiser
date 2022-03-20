#include "../item_randomiser_main.h"

extern ERItemRandomiser* main_mod;

// Used to generate properties for any param specified *at game launch*
std::function<bool(ERRandomiserProperties*, uint64_t, uint32_t, uint32_t, uint32_t)> ERRandomiserProperties::GetGenerateSpecialProperties_Static(std::wstring param_name) {
	auto property_functions = randomise_static_property_functions.find(param_name);
	if (property_functions != randomise_static_property_functions.end()) {
		return property_functions->second;
	}
	return nullptr;
};

bool ERRandomiserProperties::RandomiseProperty_EquipParamWeapon(uint64_t param_container_base, uint32_t param_id, uint32_t param_offsetid, uint32_t randomiser_action) {

	// Don't scramble or shuffle "unarmed"
	if (param_id == 110000) {
		return false;
	};

	return true;
};

bool ERRandomiserProperties::RandomiseProperty_EquipParamGoods(uint64_t param_container_base, uint32_t param_id, uint32_t param_offsetid, uint32_t randomiser_action) {

	// Don't scramble / shuffle the whistle
	if (param_id >= 115 && param_id <= 130) {
		return false;
	};

	// Don't scramble / shuffle the estus flasks
	if (param_id >= 1000 && param_id <= 1091) {
		return false;
	};

	return true;
};

// Used to generate properties for weapons, armour, rings, and goods *as you pick them up*
bool ERRandomiserProperties::GenerateSpecialProperties_Dynamic(ItemInfo* item_info, uint32_t entry) {
	auto property_functions = randomise_dynamic_property_functions.find(item_info->item_id >> 28);
	if (property_functions != randomise_dynamic_property_functions.end()) {
		if (!std::invoke(property_functions->second, this, item_info, entry)) {
			std::runtime_error("std::invoke: Dyanamic properties - failed to generate special property");
		};
	}
	return true;
};
bool ERRandomiserProperties::RandomiseProperty_Weapon(ItemInfo* item_info, uint32_t entry) {

	EquipParamWeaponParamContainer param_container = EquipParamWeaponParamContainer();
	std::invoke(find_equipparamweapon_function, &param_container, item_info->item_id & 0x0FFFFFFF);

	EquipParamWeapon* weapon_info = param_container.equipparamweapon_param_entry;
	if (weapon_info == nullptr) {
		// Item does not exist
		return false;
	};

	// First remove any pre-existing properties
	item_info->item_id -= (item_info->item_id % 10000);

	bool is_ammo = false;
	bool special_property_exempt = false;
	uint32_t weapon_slot_equip = __UINT32_MAX__;
	switch (weapon_info->weapon_type) {

		// Special case for these, to make sure they don't get special properties later on
	case(weapon_type_lightbow):
	case(weapon_type_bow):
	case(weapon_type_greatbow):
	case(weapon_type_crossbow):
	case(weapon_type_ballista):
	case(weapon_type_glintstone_staff):
	case(weapon_type_sacred_seal): {
		weapon_slot_equip = 1;
		special_property_exempt = true;
		break;
	};

								 // Equip these in the player's offhand
	case(weapon_type_small_shield):
	case(weapon_type_med_shield):
	case(weapon_type_great_shield): {
		weapon_slot_equip = 0;
		break;
	};

								  // Treat these like goods in terms of quantity. Equip in primary arrow slot
	case(weapon_type_arrow):
	case(weapon_type_greatarrow): {
		weapon_slot_equip = 6;
		is_ammo = true;
		break;
	};

								// Treat these like goods in terms of quantity. Equip in primary bolt slot
	case(weapon_type_bolt):
	case(weapon_type_greatbolt): {
		weapon_slot_equip = 7;
		is_ammo = true;
		break;
	};

							   // Otherwise, equip in the player's mainhand
	default: {
		weapon_slot_equip = 1;
		break;
	};

	};

	// If the weapon isn't ammunition, attempt to modify the weapon drop further
	// otherwise generate a quantity value for the amount of arrows/bolts to give
	uint32_t ashes_of_war = __UINT32_MAX__;
	if (!is_ammo) {

		// Reinforce level
		uint8_t upgrade_lv = 0;
		uint64_t player_game_data = *(uint64_t*)(*(uint64_t*)(game_data_manager_class)+0x08);
		if (player_game_data) {

			uint8_t weapon_level_max = *(uint8_t*)(player_game_data + 0xE2);
			ParamContainer param_container = ParamContainer();
			std::invoke(find_equipmtrlsetparam_function, &param_container, weapon_info->material + 1);

			EquipMtrlSetParam* mtrlsetparam = (EquipMtrlSetParam*)param_container.param_entry;
			if (mtrlsetparam) {

				// Smithing stones are used to upgrade, so maximum will be +25
				if (mtrlsetparam->upgrade_material_id == 10100) {
					if (weapon_level_max && weapon_level_max <= 25) {
						upgrade_lv = main_mod->GetRandomUint(0, weapon_level_max);
					};
				}
				// Somber stones are used to upgrade, so maximum will be +10
				else if (mtrlsetparam->upgrade_material_id == 10160) {
					if (weapon_level_max && weapon_level_max <= 25) {
						weapon_level_max = specialweapon_max_converter.at(weapon_level_max);
						upgrade_lv = main_mod->GetRandomUint(0, weapon_level_max);
					};
					special_property_exempt = true;
				}
				// Error
				else {
					special_property_exempt = true;
				};

			};

		};

		// Add weapon special property
		uint32_t weapon_property = 0;
		if (!special_property_exempt) {
			weapon_property = main_mod->GetRandomUint(0, 12) * 100;

			// Should we generate a custom ash of war?
			if (main_mod->GetRandomUint(0, 100) > 75) {
				ashes_of_war = 2147483648 | ashes_selection.at(main_mod->GetRandomUint(0, ashes_selection.size() - 1));
			};

		};


		item_info->item_id += (weapon_property + upgrade_lv);
		item_info->item_quantity = 1;
	}
	else {
		item_info->item_quantity = main_mod->GetRandomUint(1, 50);
	};

	if (entry >= auto_equip_buffer->size()) {
		throw std::runtime_error("Attempt to access an element exceeding the maximum size");
	};

	// Signal a call to the auto equip function after the item has been delivered
	EquipInfo* equip_info = &auto_equip_buffer->at(entry);
	equip_info->equipment_slot = weapon_slot_equip;
	equip_info->item_id = item_info->item_id;

	item_info->item_relayvalue = 0;
	item_info->item_ashes_of_war = ashes_of_war;

	return true;
};
bool ERRandomiserProperties::RandomiseProperty_Armour(ItemInfo* item_info, uint32_t entry) {

	EquipParamProtectorParamContainer param_container = EquipParamProtectorParamContainer();
	std::invoke(find_equipparamprotector_function, &param_container, item_info->item_id & 0x0FFFFFFF);

	// Get the param entry for the armour in question
	EquipParamProtector* protector_info = param_container.equipparamprotector_param_entry;
	if (protector_info == nullptr) {
		// Item does not exist
		return false;
	};

	// Sort what "type" of armour it is (helmet, gloves, etc)
	uint32_t equipment_slot_id = 0;
	switch (protector_info->protector_type) {
	case(protector_type_head): {
		equipment_slot_id = 12;
		break;
	};
	case(protector_type_chest): {
		equipment_slot_id = 13;
		break;
	};
	case(protector_type_legs): {
		equipment_slot_id = 14;
		break;
	};
	case(protector_type_hands): {
		equipment_slot_id = 15;
		break;
	};
	default: {
		// Cannot fit item
		return false;
	};
	};

	if (entry >= auto_equip_buffer->size()) {
		throw std::runtime_error("Attempt to access an element exceeding the maximum size");
	};

	// Signal a call to the auto equip function after the item has been delivered
	EquipInfo* equip_info = &auto_equip_buffer->at(entry);
	equip_info->equipment_slot = equipment_slot_id;
	equip_info->item_id = item_info->item_id;

	// Constants
	item_info->item_quantity = 1;
	item_info->item_relayvalue = 0;
	item_info->item_ashes_of_war = __UINT32_MAX__;
	return true;
};
bool ERRandomiserProperties::RandomiseProperty_Accessory(ItemInfo* item_info, uint32_t entry) {

	// Grab the amount of talisman pouches the player owns
	uint64_t player_data = *(uint64_t*)(*(uint64_t*)(game_data_manager_class)+0x08);
	uint32_t equip_slot = __UINT32_MAX__;
	if (player_data) {
		uint8_t max_talisman_count = *(uint8_t*)(player_data + 0xC6);
		max_talisman_count = max_talisman_count <= 4 ? max_talisman_count : 4;
		equip_slot = main_mod->GetRandomUint(0, max_talisman_count) + 17;
	};

	if (equip_slot != __UINT32_MAX__) {
		// Signal a call to the auto equip function after the item has been delivered
		EquipInfo* equip_info = &auto_equip_buffer->at(entry);
		equip_info->equipment_slot = equip_slot;
		equip_info->item_id = item_info->item_id;
	};

	// Constants
	item_info->item_quantity = 1;
	item_info->item_ashes_of_war = __UINT32_MAX__;
	item_info->item_relayvalue = 0;
	return true;
};
bool ERRandomiserProperties::RandomiseProperty_Goods(ItemInfo* item_info, uint32_t entry) {

	// Lookup EquipParamGoods
	// key item? Where is it sorted?
	uint32_t item_id_holder = item_info->item_id;
	ParamContainer param_container = ParamContainer();
	std::invoke(find_equipparamgoods_function, &param_container, item_id_holder & 0x0FFFFFFF);

	EquipParamGoods* goods_info = (EquipParamGoods*)param_container.param_entry;
	if (goods_info == nullptr) {
		// Item does not exist
		return false;
	};

	// Generate a quantity for the goods. If the max is 1 then just give 1
	uint16_t anticipated_quantity = 1;
	uint16_t sensible_max_quantity = goods_info->max_inventory;
	switch (goods_info->item_type) {
	case(itemtype_tools): {}
	case(itemtype_sorceries): {}
	case(itemtype_ashes): {}
	case(itemtype_info): {}
	case(itemtype_incantations): {
		sensible_max_quantity = 1;
		break;
	};
	default: break;
	};

	// Don't touch the quantity of runes
	if ((item_id_holder >= 0x40000B54) && (item_id_holder <= 0x40000BAE)) {
		sensible_max_quantity = item_info->item_quantity;
		anticipated_quantity = item_info->item_quantity;
	};

	// Don't touch the quantity of critical items such as talisman pouches
	if ((item_id_holder >= 0x4000251C) && (item_id_holder <= 0x40002760)) {
		sensible_max_quantity = item_info->item_quantity;
		anticipated_quantity = item_info->item_quantity;
	};

	if (anticipated_quantity < sensible_max_quantity) {
		uint32_t max_gen = sensible_max_quantity > 5 ? 5 : sensible_max_quantity;
		anticipated_quantity = main_mod->GetRandomUint(anticipated_quantity, max_gen);
	};

	// Constants
	item_info->item_quantity = anticipated_quantity;
	item_info->item_relayvalue = 0;
	item_info->item_ashes_of_war = __UINT32_MAX__;
	return true;
};

// Helper function used if we need to grab a specific container
void* ERRandomiserProperties::GetParamEntry(uint64_t param_container, uint32_t param_entry) {
	uint8_t param_container_entry_type = *(uint8_t*)(param_container + 0x2D);
	switch (param_container_entry_type & 127) {
	case(2): {
		return reinterpret_cast<void*>(param_container + *(uint64_t*)(param_container + (param_entry * 4) + 0x34));
	};
	case(3): {
		return reinterpret_cast<void*>(param_container + *(uint64_t*)(param_container + (param_entry * 4) + 0x44));
	};
	case(5): {
		if (param_container_entry_type < 128) {
			return reinterpret_cast<void*>(param_container + *(uint64_t*)(param_container + (param_entry * 4) + 0x44));
		};
	};
	case(4): {
		if (*(uint8_t*)(param_container + 0x2E) & 2) {
			uint32_t container_offset = (param_entry + 3) * 3;
			return reinterpret_cast<void*>(param_container + *(uint64_t*)(param_container + (container_offset * 8)));
		}
		else {
			return reinterpret_cast<void*>(param_container + *(uint64_t*)(param_container + (param_entry * 4) + 0x44));
		};
		break;
	};
	default: break;
	};

	return nullptr;
};
