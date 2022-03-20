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

	randomiser_properties = ERRandomiserProperties(game_data_manager_class, &auto_equip_buffer, find_equipparamweapon_function,
		find_equipparamprotector_function, find_equipparamgoods_function, find_equipmtrlsetparam_function);

	if (!Shuffle()) {
		throw std::runtime_error("Failed to apply mod effects");
		return false;
	};

	DWORD old_protect = 0;
	if (MH_CreateHook((void*)item_give_hook_address_map, (void*)&ERRandomiserBase::RandomiseItemHook, nullptr) == MH_OK) {
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
	if (MH_CreateHook((void*)item_give_hook_address_lua, (void*)&ERRandomiserBase::RandomiseItemHook, nullptr) == MH_OK) {
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

	using namespace std::chrono_literals;

	// Get the two runes we're going to keep static
	static_rune_01 = static_runes.at(main_mod->GetSeededRandomUint(0, 1, randomkey_seed));
	static_rune_02 = static_runes.at(main_mod->GetSeededRandomUint(2, 3, randomkey_seed));

	uint64_t solo_param_reposiory = *(uint64_t*)(solo_param_repository_class);
	while (!solo_param_reposiory) {
		solo_param_reposiory = *(uint64_t*)(solo_param_repository_class);
		std::this_thread::sleep_for(1s);
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

			// Don't include respawning shard material in the pool
			uint32_t mapitem_id = *(uint32_t*)(itemlotparam_map_idrepository + (w * 8));
			if (mapitem_id == 997960) {
				continue;
			};

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
		RandomiseType param_randomise_type = main_mod->GetParamRandomisationPreference(main_mod->param_container_names.at(q));
		switch (param_randomise_type) {

			case(randomisetype_shuffle): {
				if (!ShuffleParamEntryTable(solo_param_reposiory, main_mod->param_container_names.at(q), enemyitem_seed, true, false)) {
					throw std::runtime_error("Shuffle generic param table failure");
				};
				break;
			};
			case(randomisetype_scramble): {
				if (!ShuffleParamEntryTable(solo_param_reposiory, main_mod->param_container_names.at(q), enemyitem_seed, false, true)) {
					throw std::runtime_error("Scramble generic param table failure");
				};
				break;
			};
			case(randomisetype_butcher): {
				if (!ShuffleParamEntryTable(solo_param_reposiory, main_mod->param_container_names.at(q), enemyitem_seed, true, true)) {
					throw std::runtime_error("Butcher generic param table failure");
				};
				break;
			};
			default: break;
		};
	};

	return true;
};

bool ERRandomiserBase::ShuffleParamEntryTable(uint64_t solo_param_repository, std::wstring param_name, uint32_t seed, bool shuffle, bool scramble) {

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

	// We need at least 2 entries in order to shuffle them around. Some like "NetworkParam" only have 1
	uint32_t param_entries = *(uint32_t*)(found_param_container - 0x0C);
	if (param_entries < 2) {
		return true;
	};

	std::function<bool(ERRandomiserProperties*, uint64_t, uint32_t, uint32_t, uint32_t)> randomiser_property_proc = randomiser_properties.GetGenerateSpecialProperties_Static(param_name);
	uint32_t start_offset = (*(uint32_t*)(found_param_container - 0x10) + 15) & -16;
	uint64_t idrepository = found_param_container + start_offset;

	std::vector<uint32_t> offset_list;
	std::vector<uint32_t> entry_list;
	for (uint32_t i = 1; i < param_entries; i++) {

		uint32_t entry = *(uint32_t*)(idrepository + (i * 8) + 4);

		if ((int)entry < 0) {
			continue;
		};

		// Call any specified special property functions. These are functions specific to a param which can be used to make special changes
		// for example not randomising a critical entry which might make the game unplayable
		if (randomiser_property_proc) {
			if (!std::invoke(randomiser_property_proc, &randomiser_properties, found_param_container , *(uint32_t*)(idrepository + (i * 8)), entry, shuffle + (scramble*2))) {
				continue;
			};
		};

		offset_list.push_back(i);
		entry_list.push_back(entry);
	};

	std::shuffle(entry_list.begin(), entry_list.end(), std::default_random_engine(seed));

	uint8_t param_container_entry_type = *(uint8_t*)(found_param_container + 0x2D);
	size_t param_entry_size = 0;
	if (scramble) {
		switch (param_container_entry_type & 127) {
		case(2): {
			param_entry_size = *(uint64_t*)(found_param_container + 0x38) - *(uint64_t*)(found_param_container + 0x34);
			break;
		};
		case(3): {
			param_entry_size = *(uint64_t*)(found_param_container + 0x48) - *(uint64_t*)(found_param_container + 0x44);
			break;
		};
		case(5): {
			if (param_container_entry_type < 128) {
				param_entry_size = *(uint64_t*)(found_param_container + 0x48) - *(uint64_t*)(found_param_container + 0x44);
				break;
			};
			// Otherwise, run case 4
		};
		case(4): {
			if (*(uint8_t*)(found_param_container + 0x2E) & 2) {
				param_entry_size = *(uint64_t*)(found_param_container + 0x60) - *(uint64_t*)(found_param_container + 0x48);
			}
			else {
				param_entry_size = *(uint64_t*)(found_param_container + 0x48) - *(uint64_t*)(found_param_container + 0x44);
			};
			break;
		};
		default: break;
		};
	};

	size_t param_container_entry_size = entry_list.size();
	std::vector<uint32_t*> param_container_vector;
	for (size_t q = 0; q < param_container_entry_size; q++) {

		uint32_t entry = entry_list.at(q);
		if (shuffle) {
			*(uint32_t*)(idrepository + (offset_list.at(q) * 8) + 4) = entry;
		};

		// Stop here if the param isn't to be scrambled
		if (param_entry_size < 4) {
			continue;
		};

		uint32_t* generic_param_container = (uint32_t*)randomiser_properties.GetParamEntry(found_param_container, entry);
		param_container_vector.push_back(generic_param_container);
	};

	// The generic randomiser doesn't know the structure of each param, so just scramble it in 4 byte increments
	param_entry_size /= 4;
	size_t vector_size = param_container_vector.size();
	for (uint32_t i = 0; i < param_entry_size; i++) {
		std::vector<uint32_t> offset_xx;
		for (size_t q = 0; q < vector_size; q++) {
			offset_xx.push_back(param_container_vector.at(q)[i]);
		};

		std::shuffle(offset_xx.begin(), offset_xx.end(), std::default_random_engine(seed+i));

		for (size_t q = 0; q < vector_size; q++) {
			param_container_vector.at(q)[i] = offset_xx.at(q);
		};

		offset_xx.clear();
	};

	return true;
};

bool ERRandomiserBase::ShouldRandomiseMapItem(ItemLotParam_map* param_container) {

	bool should_randomise = true;

	// Don't include empty item containers in the randomisation
	if (!param_container->item_id_array[0]) {
		return false;
	};

	for (int i = 0; i < sizeof(ItemLotParam_map::item_id_array) / sizeof(uint32_t); i++) {
		uint32_t item_id = param_container->item_id_array[i];
		if (item_id) {

			switch (param_container->item_type_array[i]) {

				// Don't randomise certain keys and crafting materials
				case(mapitemtype_goods): {

					ParamContainer goods_param_container = {};
					find_equipparamgoods_function(&goods_param_container, item_id);
					if (!goods_param_container.param_entry) {
						// Not a valid entry
						return false;
					};

					if (item_id < 100) {
						// Cut / broken / ?GoodsName?
						return false;
					};

					if (item_id == 106) {
						// You can't open the first door without this
						return false;
					};

					if ((item_id >= 15000) && (item_id <= 53658)) {
						// Invalid
						return false;
					};

					// Don't randomise keys if selected not to do so
					if (!random_keys) {
						for (size_t q = 0; q < excluded_items.size(); q++) {
							if (excluded_items.at(q) == item_id) {
								return false;
							};
						};				

						if ((item_id >= 8600) && (item_id <= 8618)) {
							// Maps
							return false;
						};
						
						if ((item_id == static_rune_01) || (item_id == static_rune_02)) {
							// The two runes we need to keep static to preserve progression
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
		"\x4C\x8D\x45\x34\x48\x8D\x55\x90", 
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
		"\x8B\x02\x83\xF8\x0A",
		"xxxxx",
		5,
		0,
	};
	item_give_address = (uint64_t)signature_class.FindSignature(item_give_signature);
	if (item_give_address) {
		item_give_address -= 82;
	};

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
	ERRandomiserProperties* randomiser_properties_class = &randomiser_base->randomiser_properties;
	for (uint32_t i = 0; i < item_count; i++) {
		ItemInfo* item_struct = &item_info->item_info[i];
		ItemInfo new_item_struct = ItemInfo();

		// TODO: Generate new item
		new_item_struct = *item_struct;

		// Only replace the item with our newly generated one if the item type and properties generated are acceptable
		if (randomiser_properties_class->GenerateSpecialProperties_Dynamic(&new_item_struct, i)) {
			*item_struct = new_item_struct;
		};

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