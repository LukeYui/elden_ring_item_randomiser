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

	game_data_manager_class = game_data_manager_address + *(int*)((char*)game_data_manager_address + 3) + 7;
	solo_param_repository_class = solo_param_repository_address + *(int*)((char*)solo_param_repository_address + 3) + 7;

	// Change the save file extension to prevent accidently uploading saves coming back online
	if (save_extension_address != nullptr)
		memcpy(save_extension_address, L".rd2", 8);

	//bool is_random_keys, bool is_randomise_estusupgrade, uint64_t seed, uint64_t solo_param_repository_class, get_equipparamgoods_entry* find_equipparamgoods_function

	randomiser = ERRandomiser(random_keys, randomise_estusupgrade, randomise_mtrlupgrade, mapitem_seed, enemyitem_seed, solo_param_repository_class, find_equipparamgoods_function);

	*randomiser.GetSpecialPropertyClass() = ERRandomiserProperties(game_data_manager_class, &auto_equip_buffer, find_equipparamweapon_function,
		find_equipparamprotector_function, find_equipparamgoods_function, find_equipmtrlsetparam_function);

	if (!randomiser.Shuffle()) {
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
		&& find_inventoryid_function;
};

void ERRandomiserBase::RandomiseItemHook(uint64_t map_item_manager, ItemGiveStruct* item_info, void* item_details) {

	ERRandomiserBase* randomiser_base = &main_mod->hook_class;
	uint32_t item_count = item_info->item_struct_count;
	if (item_count > (sizeof(item_info->item_info) / sizeof(ItemInfo))) {
		throw std::runtime_error("Item count exceeds stack size!");
	};

	// Randomise the contents of the map item object the player has picked up
	ERRandomiserProperties* randomiser_properties_class = randomiser_base->randomiser.GetSpecialPropertyClass();
	for (uint32_t i = 0; i < item_count; i++) {
		ItemInfo* item_struct = &item_info->item_info[i];
		ItemInfo new_item_struct = ItemInfo();

		// Generate new item
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