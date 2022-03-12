#include "../item_randomiser_main.h"

extern ERItemRandomiser* main_mod;

bool ERRandomiserBase::CreateMemoryEdits() {

	minhook_active = MH_Initialize();
	if (minhook_active != MH_OK) {
		throw std::runtime_error("MH_Initialize != MH_OK");
		return false;
	};

	if (!FindNeededSignatures()) {
		throw std::runtime_error("Failed to find function signatures");
		return false;
	};

	game_data_manager_class = game_data_manager_address + *(uint32_t*)((char*)game_data_manager_address + 3) + 7;
	solo_param_repository_class = solo_param_repository_address + *(uint32_t*)((char*)solo_param_repository_address + 3) + 7;

	// Change the save file extension to prevent accidently uploading saves coming back online
	memcpy(save_extension_address, L".rd2", 8);

	if (!Shuffle()) {
		throw std::runtime_error("");
		return false;
	};

	DWORD old_protect = 0;
	if (MH_CreateHook((void*)item_give_hook_address_map, &ERRandomiserBase::RandomiseItemHook, nullptr) == MH_OK) {
		MH_EnableHook((void*)item_give_hook_address_map);
		if (VirtualProtect((void*)item_give_hook_address_map, 8, PAGE_EXECUTE_READWRITE, &old_protect)) {
			uint8_t call_bytes = 0xE8;
			memcpy((void*)item_give_hook_address_map, &call_bytes, sizeof(call_bytes));
			VirtualProtect((void*)item_give_hook_address_map, 8, old_protect, &old_protect);
		};
		item_give_function = (give_item_function*)item_give_address;
		item_equip_function = (equip_item_function*)equip_item_address;
	};

	old_protect = 0;
	if (MH_CreateHook((void*)item_give_hook_address_lua, &ERRandomiserBase::RandomiseItemHook, nullptr) == MH_OK) {
		MH_EnableHook((void*)item_give_hook_address_lua);
		if (VirtualProtect((void*)item_give_hook_address_lua, 8, PAGE_EXECUTE_READWRITE, &old_protect)) {
			uint8_t call_bytes = 0xE8;
			memcpy((void*)item_give_hook_address_lua, &call_bytes, sizeof(call_bytes));
			VirtualProtect((void*)item_give_hook_address_lua, 8, old_protect, &old_protect);
		};
		return true;
	};

	return false;
};

bool ERRandomiserBase::Shuffle() {

	// Get the two runes we're going to keep static
	static_rune_01 = static_runes.at(main_mod->GetSeededRandomUint(0, 1, randomkey_seed));
	static_rune_02 = static_runes.at(main_mod->GetSeededRandomUint(2, 3, randomkey_seed));

	uint64_t solo_param_reposiory = *(uint64_t*)(solo_param_repository_class);
	while (!solo_param_reposiory) {
		solo_param_reposiory = *(uint64_t*)(solo_param_repository_class);
		Sleep(1000);
	};

	// Shuffle ItemLotParam_map entries carefully
	{
		uint64_t itemlotparam_map = *(uint64_t*)(solo_param_reposiory + 0x670);
		while (!itemlotparam_map) {
			itemlotparam_map = *(uint64_t*)(solo_param_reposiory + 0x670);
			Sleep(1000);
		};

		itemlotparam_map = *(uint64_t*)(itemlotparam_map + 0x80);
		itemlotparam_map = *(uint64_t*)(itemlotparam_map + 0x80);

		uint16_t param_entries = *(uint16_t*)(itemlotparam_map - 0x0C);
		std::vector< ItemLotParam_map*> mapitem_list;
		std::vector< ItemLotParam_map*> mapitem_list_copy;

		uint32_t start_offset = (*(uint32_t*)(itemlotparam_map - 0x10) + 15) & -16;
		uint64_t itemlotparam_map_idrepository = itemlotparam_map + start_offset;
		for (uint16_t w = 1; w < param_entries; w++) {

			uint32_t entry = *(uint32_t*)(itemlotparam_map_idrepository + (w * 8) + 4);
			if ((int)entry < 0) {
				continue;
			};

			uint32_t container_offset = (entry + 3) * 3;
			ItemLotParam_map* param_container = reinterpret_cast<ItemLotParam_map*>(itemlotparam_map + *(uint64_t*)(itemlotparam_map + (container_offset * 8)));
			if (!ShouldRandomiseMapItem(param_container)) {
				continue;
			};

			mapitem_list.push_back(param_container);
		};

		// Shuffle the list copy
		mapitem_list_copy = mapitem_list;
		std::shuffle(mapitem_list_copy.begin(), mapitem_list_copy.end(), std::default_random_engine(mapitem_seed));

		// Instead of shuffling entries, shuffle the items within the param container itself
		for (size_t q = 0; q < mapitem_list_copy.size(); q++) {
			ItemLotParam_map* param_container_original_order = mapitem_list.at(q);
			ItemLotParam_map* param_container_shuffle = mapitem_list_copy.at(q);
			for (int i = 0; i < sizeof(ItemLotParam_map::item_id_array) / sizeof(uint32_t); i++) {
				param_container_original_order->item_id_array[i] = param_container_shuffle->item_id_array[i];
				param_container_original_order->item_type_array[i] = param_container_shuffle->item_type_array[i];
			};
		};
	};

	// Just randomise all of the selected params
	for (size_t q = 0; q < main_mod->param_container_names.size(); q++) {
		if (main_mod->GetParamRandomisationPreference(main_mod->param_container_names.at(q))) {
			if (!ShuffleParamEntryTable(main_mod->param_container_names.at(q), enemyitem_seed)) {
				throw std::runtime_error("Generic param table randomisation failure");
			};
		};
	};

	return true;
};

bool ERRandomiserBase::ShuffleParamEntryTable(std::wstring param_name, uint32_t seed) {

	// Make sure solo param repository is active
	uint64_t solo_param_repository = *(uint64_t*)(solo_param_repository_class);
	while (!solo_param_repository) {
		solo_param_repository = *(uint64_t*)(solo_param_repository_class);
		Sleep(1000);
	};

	// Get offset by name
	uint64_t found_param_container = 0;
	for (int i = 0; i < 185; i++) {
		int param_offset = i * 0x48;
		if (*(int*)(solo_param_repository + param_offset + 0x80) > 0) {
			uint64_t param_container = *(uint64_t*)(solo_param_repository + param_offset + 0x88);
			wchar_t* container_name = (wchar_t*)(param_container + 0x18);
			if (*(uint32_t*)(param_container + 0x28) >= 8) {
				container_name = (wchar_t*)(*(uint64_t*)container_name);
			};
			if (wcsncmp(param_name.c_str(), container_name, param_name.size()) == 0) {
				found_param_container = param_container;
				break;
			};
		};
	};

	if (!found_param_container) {
		return false;
	};

	// Shuffle param container
	found_param_container = *(uint64_t*)(found_param_container + 0x80);
	found_param_container = *(uint64_t*)(found_param_container + 0x80);

	uint16_t param_entries = *(uint16_t*)(found_param_container - 0x0C);
	std::vector<uint32_t> offset_list;

	uint32_t start_offset = (*(uint32_t*)(found_param_container - 0x10) + 15) & -16;
	uint64_t idrepository = found_param_container + start_offset;
	for (uint16_t w = 1; w < param_entries; w++) {
		uint32_t entry = *(uint32_t*)(idrepository + (w * 8) + 4);
		if ((int)entry < 0) {
			continue;
		};

		offset_list.push_back(entry);
	};

	std::shuffle(offset_list.begin(), offset_list.end(), std::default_random_engine(seed));

	for (uint16_t q = 1; q < offset_list.size(); q++) {
		*(uint32_t*)(idrepository + (q * 8) + 4) = offset_list.at(q - 1);
	};

	return true;
};

bool ERRandomiserBase::ShouldRandomiseMapItem(ItemLotParam_map* param_container) {

	bool should_randomise = true;
	for (int i = 0; i < sizeof(ItemLotParam_map::item_id_array) / sizeof(uint32_t); i++) {
		uint32_t item_id = param_container->item_id_array[i];
		if (item_id) {

			switch (param_container->item_type_array[i]) {

				// Don't randomise certain keys and crafting materials
				case(mapitemtype_goods): {

					// Invalid
					if (item_id < 100) {
						return false;
					};

					// You can't open the first door without this
					if (item_id == 106) {
						return false;
					};

					// Invalid
					if ((item_id >= 15000) && (item_id <= 53658)) {
						return false;
					};
					// Don't randomise keys if selected not to do so
					if (!random_keys) {
						for (size_t q = 0; q < excluded_items.size(); q++) {
							if (excluded_items.at(q) == item_id) {
								return false;
							};
						};
						
						// Maps
						if ((item_id >= 8600) && (item_id <= 8618)) {
							return false;
						};
						
						// The two runes we need to keep static to preserve progression
						if ((item_id == static_rune_01) || (item_id == static_rune_02)) {
							return false;
						};

					};

					break;
				};
				case(mapitemtype_weapon): {

					// Invalid
					if (item_id == 100000) {
						return false;
					};

					break;
				};
				case(mapitemtype_armour): {

					// Invalid
					if ((item_id >= 1000) && (item_id <= 10300)) {
						return false;
					};

					break;
				};
				case(mapitemtype_accessory):
				case(mapitemtype_gem):
				default: break;
			};
		};
	};
	return should_randomise;
};

bool ERRandomiserBase::FindNeededSignatures() {

	if (!signature_class.GetImageInfo()) {
		//
		return false;
	};

	Signature game_data_manager_signature = {
		"\x48\x8B\x05\xFF\xFF\xFF\xFF\x48\x8B\x48\x08\x48\x85\xC9\x74\xFF\x0F\xB6\x81\xE4",
		"xxx????xxxxxxxx?xxxx",
		20,
		0,
	};
	game_data_manager_address = (uint64_t)signature_class.FindSignature(game_data_manager_signature);

	Signature solo_param_repository_signature = {
		"\x48\x8B\x0D\xFF\xFF\xFF\xFF\x48\x85\xC9\x0F\x84\xFF\xFF\xFF\xFF\x45\x33\xC0\xBA\x90",
		"xxx????xxxxx????xxxxx",
		21,
		0,
	};
	solo_param_repository_address = (uint64_t)signature_class.FindSignature(solo_param_repository_signature);

	// CS::MapItemManagerImp
	Signature item_give_hook_signature_map = {
		"\x4C\x8D\x45\xF4\x48\x8D\x55\x90", 
		"xxxxxxxx",
		8,
		0,
	};
	item_give_hook_address_map = (uint64_t)signature_class.FindSignature(item_give_hook_signature_map);
	item_give_hook_address_map += 11;

	// CS::LuaEventManagerImp (event tied items)
	Signature item_give_hook_signature_lua = {
		"\xE8\xFF\xFF\xFF\xFF\x83\x7D\x98\x00\x74",
		"x????xxxxx",
		10,
		0,
	};
	item_give_hook_address_lua = (uint64_t)signature_class.FindSignature(item_give_hook_signature_lua);

	Signature item_give_signature = {
		"\x40\x55\x56\x57\x41\x54\x41\x55\x41\x56\x41\x57\x48\x8D\x6C\x24\xB0\x48\x81\xEC\x50\x01\x00\x00\x48\xC7\x45\xC0\xFE\xFF\xFF\xFF",
		"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
		32,
		0,
	};
	item_give_address = (uint64_t)signature_class.FindSignature(item_give_signature);

	Signature equip_item_signature = {
		"\x40\x53\x48\x83\xEC\x20\x48\x8B\xD9\x48\x8D\x51\x10",
		"xxxxxxxxxxxxx",
		13,
		0,
	};
	equip_item_address = (uint64_t)signature_class.FindSignature(equip_item_signature);

	Signature find_equipparamweapon_signature = {
		"\x40\x57\x41\x56\x41\x57\x48\x83\xEC\x40\x48\xC7\x44\x24\x20\xFE\xFF\xFF\xFF\x48\x89\x5C\x24\x60\x48\x89\x6C\x24\x68\x48\x89\x74\x24\x70\x8B",
		"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
		35,
		0,
	};
	find_equipparamweapon_function = (get_equipparamweapon_entry*)signature_class.FindSignature(find_equipparamweapon_signature);

	Signature find_equipparamprotector_signature = {
		"\x41\x54\x41\x56\x41\x57\x48\x83\xEC\x40\x48\xC7\x44\x24\x20\xFE\xFF\xFF\xFF\x48\x89\x5C\x24\x60\x48\x89\x6C\x24\x68\x48\x89\x74\x24\x70\x48\x89\x7C\x24\x78\x8B\xEA\x4C\x8B\xF1\x33",
		"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx",
		45,
		0,
	};
	find_equipparamprotector_function = (get_equipparamprotector_entry*)signature_class.FindSignature(find_equipparamprotector_signature);

	Signature find_equipparamgoods_signature = {
		"\x45\x33\xC0\x41\x8D\x50\x03\xE8\xFF\xFF\xFF\xFF\x48\x85\xC0\x0F\x84\xFF\xFF\xFF\xFF\x48\x8B\x80\x80\x00\x00\x00\x48\x8B\x90",
		"xxxxxxxx????xxxxx????xxxxxxxxxx",
		31,
		0,
	};
	find_equipparamgoods_function = (get_equipparamgoods_entry*)((uint64_t)signature_class.FindSignature(find_equipparamgoods_signature) - 103);

	Signature find_equipmtrlsetparam_signature = {
		"\x45\x33\xC0\x41\x8D\x50\x18\xE8\xFF\xFF\xFF\xFF\x48\x85\xC0\x0F\x84",
		"xxxxxxxx????xxxxx",
		17,
		0,
	};
	find_equipmtrlsetparam_function = (get_equipmtrlsetparam_entry*)((uint64_t)signature_class.FindSignature(find_equipmtrlsetparam_signature) - 103);

	Signature find_getinventoryid_signature = {
		"\x40\x56\x48\x83\xEC\x20\x83\xCE",
		"xxxxxxxx",
		8,
		0,
	};
	find_inventoryid_function = (get_inventoryid*)signature_class.FindSignature(find_getinventoryid_signature);

	Signature get_save_file_extension_signature = {
		"\x2E\x00\x73\x00\x6C\x00\x32\x00",
		"xxxxxxxx",
		8,
		0,
	};
	save_extension_address = signature_class.FindSignature(get_save_file_extension_signature);

	return game_data_manager_address && item_give_address && item_give_hook_address_map && item_give_hook_address_lua && equip_item_address
		&& find_equipparamweapon_function && find_equipparamprotector_function && find_equipparamgoods_function && find_equipmtrlsetparam_function
		&& find_inventoryid_function && save_extension_address;
};

void ERRandomiserBase::RandomiseItemHook(uint64_t map_item_manager, ItemGiveStruct* item_info, void* item_details) {

	ERRandomiserBase* randomiser_base = &main_mod->hook_class;
	uint32_t item_count = item_info->item_struct_count;
	if (item_count > (sizeof(item_info->item_info) / sizeof(ItemInfo))) {
		throw std::runtime_error("Item count exceeds stack size!");
	};

	// Randomise the contents of the map item object the player has picked up
	for (uint32_t i = 0; i < item_count; i++) {
		ItemInfo* item_struct = &item_info->item_info[i];
		ItemInfo new_item_struct = ItemInfo();

		// TODO: Generate new item
		new_item_struct = *item_struct;

		// Add new properties depending on the item generated
		auto property_functions = randomiser_base->randomise_property_functions.find(new_item_struct.item_id >> 28);
		if (property_functions != randomiser_base->randomise_property_functions.end()) {

			// Only replace the item with our newly generated one if the item type and properties generated are acceptable
			if (std::invoke(property_functions->second, randomiser_base, &new_item_struct, i)) {
				*item_struct = new_item_struct;
			};

		}
	};

	// Put the newly generated item in the player inventory
	randomiser_base->item_give_function(map_item_manager, item_info, item_details);

	// Check and reset autoequip buffer
	if (randomiser_base->auto_equip) {
		for (size_t q = 0; q < randomiser_base->auto_equip_buffer.size(); q++) {

			// Equip any items flagged by the property generator functions
			EquipInfo* equip_info_check = &randomiser_base->auto_equip_buffer.at(q);
			if (equip_info_check->item_id != -1) {

				uint64_t player_inventory_manager = *(uint64_t*)(*(uint64_t*)(randomiser_base->game_data_manager_class) + 0x08);
				if (player_inventory_manager) {
					EquipItemStruct equip_struct = {};
					equip_struct.equipment_slot = equip_info_check->equipment_slot;
					equip_struct.inventory_slot = randomiser_base->find_inventoryid_function(player_inventory_manager + 0x408, &equip_info_check->item_id);
					randomiser_base->item_equip_function(&equip_struct);
				};
			};

		};
		randomiser_base->auto_equip_buffer.fill(EquipInfo());
	};

	return;
};

bool ERRandomiserBase::RandomiseProperty_Weapon(ItemInfo* item_info, uint32_t entry) {

	EquipParamWeaponParamContainer param_container = EquipParamWeaponParamContainer();
	find_equipparamweapon_function(&param_container, item_info->item_id & 0x0FFFFFFF);

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
		uint64_t player_game_data = *(uint64_t*)(*(uint64_t*)(game_data_manager_class) + 0x08);
		if (player_game_data) {

			uint8_t weapon_level_max = *(uint8_t*)(player_game_data + 0xE2);
			ParamContainer param_container = ParamContainer();
			find_equipmtrlsetparam_function(&param_container, weapon_info->material + 1);

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

	if (entry >= auto_equip_buffer.size()) {
		throw std::runtime_error("Attempt to access an element exceeding the maximum size");
	};

	// Signal a call to the auto equip function after the item has been delivered
	EquipInfo* equip_info = &auto_equip_buffer.at(entry);
	equip_info->equipment_slot = weapon_slot_equip;
	equip_info->item_id = item_info->item_id;

	item_info->item_relayvalue = 0;
	item_info->item_ashes_of_war = ashes_of_war;

	return true;
};

bool ERRandomiserBase::RandomiseProperty_Armour(ItemInfo* item_info, uint32_t entry) {

	EquipParamProtectorParamContainer param_container = EquipParamProtectorParamContainer();
	find_equipparamprotector_function(&param_container, item_info->item_id & 0x0FFFFFFF);

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
	
	if (entry >= auto_equip_buffer.size()) {
		throw std::runtime_error("Attempt to access an element exceeding the maximum size");
	};

	// Signal a call to the auto equip function after the item has been delivered
	EquipInfo* equip_info = &auto_equip_buffer.at(entry);
	equip_info->equipment_slot = equipment_slot_id;
	equip_info->item_id = item_info->item_id;

	// Constants
	item_info->item_quantity = 1;
	item_info->item_relayvalue = 0;
	item_info->item_ashes_of_war = __UINT32_MAX__;
	return true;
};

bool ERRandomiserBase::RandomiseProperty_Accessory(ItemInfo* item_info, uint32_t entry) {

	// Grab the amount of talisman pouches the player owns
	uint64_t player_data = *(uint64_t*)(*(uint64_t*)(game_data_manager_class) + 0x08);
	uint32_t equip_slot = __UINT32_MAX__;
	if (player_data) {
		uint8_t max_talisman_count = *(uint8_t*)(player_data + 0xC6);
		max_talisman_count = max_talisman_count <= 4 ? max_talisman_count : 4;
		equip_slot = main_mod->GetRandomUint(0, max_talisman_count) + 17;
	};

	if (equip_slot != __UINT32_MAX__) {
		// Signal a call to the auto equip function after the item has been delivered
		EquipInfo* equip_info = &auto_equip_buffer.at(entry);
		equip_info->equipment_slot = equip_slot;
		equip_info->item_id = item_info->item_id;
	};

	// Constants
	item_info->item_quantity = 1;
	item_info->item_ashes_of_war = __UINT32_MAX__;
	item_info->item_relayvalue = 0;
	return true;
};

bool ERRandomiserBase::RandomiseProperty_Goods(ItemInfo* item_info, uint32_t entry) {

	// Lookup EquipParamGoods
	// key item? Where is it sorted?
	uint32_t item_id_holder = item_info->item_id;
	ParamContainer param_container = ParamContainer();
	find_equipparamgoods_function(&param_container, item_id_holder & 0x0FFFFFFF);

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

// Sigscan

bool SigScan::GetImageInfo() {

	bool bSuccess = false;

	module_handle = GetModuleHandleA("eldenring.exe");
	if (module_handle) {
		MEMORY_BASIC_INFORMATION memInfo;
		if (VirtualQuery((void*)module_handle, &memInfo, sizeof(memInfo)) != 0) {
			IMAGE_DOS_HEADER* hDos = (IMAGE_DOS_HEADER*)module_handle;
			IMAGE_NT_HEADERS* hPe = (IMAGE_NT_HEADERS*)((ULONG64)memInfo.AllocationBase + (ULONG64)hDos->e_lfanew);

			if ((hDos->e_magic == IMAGE_DOS_SIGNATURE) && (hPe->Signature == IMAGE_NT_SIGNATURE)) {
				bSuccess = true;
				base_address = (void*)memInfo.AllocationBase;
				image_size = (SIZE_T)hPe->OptionalHeader.SizeOfImage;
			};
		};
	};

	return bSuccess;
};

void* SigScan::FindSignature(Signature& fnSig) {

	char* pScan = (char*)base_address;
	char* max_address = pScan + image_size - fnSig.length;
	INT iMaxLength = 0;

	while (pScan < max_address) {
		SIZE_T szLength = 0;

		for (INT i = 0; i < fnSig.length; i++) {
			if (!((pScan[i] == fnSig.signature[i]) || (fnSig.mask[i] == '?'))) break;
			szLength++;
		};

		if (szLength > iMaxLength) iMaxLength = (INT)szLength;
		if (szLength == fnSig.length) return pScan;

		pScan++;
	};

	return nullptr;
};