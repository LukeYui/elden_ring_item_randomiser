#include "../item_randomiser_main.h"

extern ERItemRandomiser* main_mod;

bool ERRandomiser::Shuffle() {


	using namespace std::chrono_literals;

	// Get the two runes we're going to keep static
	static_rune_01 = static_runes.at(main_mod->GetSeededRandomUint(0, 1, randomkey_seed));
	static_rune_02 = static_runes.at(main_mod->GetSeededRandomUint(2, 3, randomkey_seed));

	uint64_t solo_param_repository_ld = *(uint64_t*)(solo_param_repository);
	while (!solo_param_repository_ld) {
		solo_param_repository_ld = *(uint64_t*)(solo_param_repository);
		std::this_thread::sleep_for(1s);
	};

	// As a standard, make sure this param is loaded already
	uint64_t itemlotparam_map = *(uint64_t*)(solo_param_repository_ld + 0x670);
	while (!itemlotparam_map) {
		itemlotparam_map = *(uint64_t*)(solo_param_repository_ld + 0x670);
		std::this_thread::sleep_for(1s);
	};

	// Just randomise all of the selected params
	for (size_t q = 0; q < main_mod->param_container_names.size(); q++) {
		RandomiseType param_randomise_type = main_mod->GetParamRandomisationPreference(main_mod->param_container_names.at(q));
		if (param_randomise_type != randomisetype_none) {
			auto random_preference = param_randomisation_instruction_map.find(main_mod->param_container_names.at(q));
			if (random_preference != param_randomisation_instruction_map.end()) {
				random_preference->second(this, solo_param_repository_ld, main_mod->param_container_names.at(q), enemyitem_seed, param_randomise_type & randomisetype_shuffle, param_randomise_type & randomisetype_scramble);
			}
		};
	};

	return true;
};

bool ERRandomiser::ShuffleItemLotParam_map(uint64_t solo_param_repository, std::wstring param_name, uint32_t seed, bool shuffle, bool scramble) {

	// Shuffle ItemLotParam_map entries carefully

	uint64_t itemlotparam_map = *(uint64_t*)(solo_param_repository + 0x670);
	itemlotparam_map = *(uint64_t*)(itemlotparam_map + 0x80);
	itemlotparam_map = *(uint64_t*)(itemlotparam_map + 0x80);

	uint32_t param_entries = *(uint32_t*)(itemlotparam_map - 0x0C);
	std::vector< ItemLotParam_map*> mapitem_list;
	std::vector< ItemLotParam_map*> mapitem_list_copy;

	uint32_t start_offset = (*(uint32_t*)(itemlotparam_map - 0x10) + 15) & -16;
	uint64_t itemlotparam_map_idrepository = itemlotparam_map + start_offset;
	for (uint32_t i = 1; i < param_entries; i++) {

		// Don't include respawning shard material in the pool
		uint32_t mapitem_id = *(uint32_t*)(itemlotparam_map_idrepository + (i * 8));
		if (mapitem_id == 997960) {
			continue;
		};

		uint32_t entry = *(uint32_t*)(itemlotparam_map_idrepository + (i * 8) + 4);
		if ((int)entry < 0) {
			continue;
		};

		uint32_t container_offset = (entry + 3) * 3;
		ItemLotParam_map* param_container = reinterpret_cast<ItemLotParam_map*>(itemlotparam_map + *(uint64_t*)(itemlotparam_map + (container_offset * 8)));
		if (!ShouldRandomiseMapItem(param_container, mapitem_id)) {
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

	return true;
};

bool ERRandomiser::ShuffleShopLineupParam(uint64_t solo_param_repository, std::wstring param_name, uint32_t seed, bool shuffle, bool scramble) {

	uint64_t shoplineupparam = *(uint64_t*)(solo_param_repository + 0x820);
	shoplineupparam = *(uint64_t*)(shoplineupparam + 0x80);
	shoplineupparam = *(uint64_t*)(shoplineupparam + 0x80);

	uint32_t param_entries = *(uint32_t*)(shoplineupparam - 0x0C);
	std::vector<ShopLineupParam*> shopentry_list;
	std::vector<ShopLineupParam*> shopentry_list_copy;

	uint32_t start_offset = (*(uint32_t*)(shoplineupparam - 0x10) + 15) & -16;
	uint64_t shoplineupparam_idrepository = shoplineupparam + start_offset;
	for (uint32_t i = 1; i < param_entries; i++) {

		uint32_t shopitem_id = *(uint32_t*)(shoplineupparam_idrepository + (i * 8));
		uint32_t entry = *(uint32_t*)(shoplineupparam_idrepository + (i * 8) + 4);
		if ((int)entry < 0) {
			continue;
		};

		// Crafting kit (Kale)
		if (shopitem_id == 100501) {
			continue;
		};

		uint32_t container_offset = (entry + 3) * 3;
		ShopLineupParam* param_container = reinterpret_cast<ShopLineupParam*>(shoplineupparam + *(uint64_t*)(shoplineupparam + (container_offset * 8)));
		shopentry_list.push_back(param_container);
	};

	// Shuffle the list copy
	shopentry_list_copy = shopentry_list;
	std::shuffle(shopentry_list_copy.begin(), shopentry_list_copy.end(), std::default_random_engine(enemyitem_seed));

	// Instead of shuffling entries, shuffle the items within the param container itself
	for (size_t q = 0; q < shopentry_list_copy.size(); q++) {
		ShopLineupParam* param_container_original_order = shopentry_list.at(q);
		ShopLineupParam* param_container_shuffle = shopentry_list_copy.at(q);
		param_container_original_order->item_id = param_container_shuffle->item_id;
		param_container_original_order->item_cost = param_container_shuffle->item_cost;
		param_container_original_order->item_quantity = param_container_shuffle->item_quantity;
		param_container_original_order->item_type = param_container_shuffle->item_type;
		param_container_original_order->currency_type = param_container_shuffle->currency_type;
	};

	return true;

};

bool ERRandomiser::ShouldRandomiseMapItem(ItemLotParam_map* param_container, uint32_t mapitem_id) {

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
				equipparamgoods_function(&goods_param_container, item_id);
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

				if ((item_id >= 9100) && (item_id <= 9195)) {
					// Tutorials
					return false;
				};

				if ((item_id >= 15000) && (item_id <= 53658)) {
					// Invalid
					return false;
				};

				// Don't estus flask upgrade materials if selected not to do so
				if (!randomise_estusupgrade) {
					if ((item_id == 10010) || (item_id == 10020)) {
						return false;
					};
				};

				if (!randomise_materials) {
					if (item_id >= 10100 && item_id <= 10919) {
						// ;
						return false;
					};
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

bool ERRandomiser::RandomiseGenericParamContainer(uint64_t solo_param_repository, std::wstring param_name, uint32_t seed, bool shuffle, bool scramble) {

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

#if ITEM_DEBUG
	wprintf_s(L"Randomising %ws\n", param_name.c_str());
#endif

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
			if (!std::invoke(randomiser_property_proc, &randomiser_properties, found_param_container, *(uint32_t*)(idrepository + (i * 8)), entry, shuffle + (scramble * 2))) {
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

		std::shuffle(offset_xx.begin(), offset_xx.end(), std::default_random_engine(seed + i));

		for (size_t q = 0; q < vector_size; q++) {
			param_container_vector.at(q)[i] = offset_xx.at(q);
		};

		offset_xx.clear();
	};

	return true;
};

ERRandomiserProperties* ERRandomiser::GetSpecialPropertyClass() {
	return &randomiser_properties;
};