#pragma once
#include <Windows.h>
#include <cstdint>
#include <array>
#include <map>
#include <functional>
#include <string>

#include "../item_randomiser_globals.h"

class ERRandomiserProperties {
public:

	std::function<bool(ERRandomiserProperties*, uint64_t, uint32_t, uint32_t, uint32_t)> GetGenerateSpecialProperties_Static(std::wstring param_name);
	bool GenerateSpecialProperties_Dynamic(ItemInfo* item_info, uint32_t entry);
	void* GetParamEntry(uint64_t param_container, uint32_t param_entry);

	ERRandomiserProperties() {

	};

	ERRandomiserProperties(uint64_t game_data_manager_class, std::array<EquipInfo, 7>* auto_equip_buffer, get_equipparamweapon_entry* find_equipparamweapon_function,
		get_equipparamprotector_entry* find_equipparamprotector_function, get_equipparamgoods_entry* find_equipparamgoods_function, get_equipmtrlsetparam_entry* find_equipmtrlsetparam_function) {
		this->game_data_manager_class = game_data_manager_class;
		this->auto_equip_buffer = auto_equip_buffer;
		this->find_equipparamweapon_function = find_equipparamweapon_function;
		this->find_equipparamprotector_function = find_equipparamprotector_function;
		this->find_equipparamgoods_function = find_equipparamgoods_function;
		this->find_equipmtrlsetparam_function = find_equipmtrlsetparam_function;
		this->specialweapon_max_converter = { 0, 0, 1, 1, 1, 2, 2, 3, 3, 3, 3, 4, 4, 5, 5, 5, 6, 6, 7, 7, 7, 8, 8, 9, 9, 9, 10 };
		this->ashes_selection = {
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

		// Properites: Statically generated: std::wstring (ParamName)		
		randomise_static_property_functions.emplace(L"EquipParamWeapon", &ERRandomiserProperties::RandomiseProperty_EquipParamWeapon);
		randomise_static_property_functions.emplace(L"EquipParamGoods", &ERRandomiserProperties::RandomiseProperty_EquipParamGoods);

		// Properties: Runtime generated: 0 = Weapon type | 1 = Armour type | 2 = Accessory type | 4 = Goods type
		randomise_dynamic_property_functions.emplace(0, &ERRandomiserProperties::RandomiseProperty_Weapon);
		randomise_dynamic_property_functions.emplace(1, &ERRandomiserProperties::RandomiseProperty_Armour);
		randomise_dynamic_property_functions.emplace(2, &ERRandomiserProperties::RandomiseProperty_Accessory);
		randomise_dynamic_property_functions.emplace(4, &ERRandomiserProperties::RandomiseProperty_Goods);
	};

private:
	uint64_t game_data_manager_class;
	get_equipparamweapon_entry* find_equipparamweapon_function;
	get_equipparamprotector_entry* find_equipparamprotector_function;
	get_equipparamgoods_entry* find_equipparamgoods_function;
	get_equipmtrlsetparam_entry* find_equipmtrlsetparam_function;
	std::array<EquipInfo, 7>* auto_equip_buffer;
	std::array<uint8_t, 27> specialweapon_max_converter;
	std::array<uint32_t, 91> ashes_selection;
	
	// Properties: Statically generated
	std::map<std::wstring, std::function<bool(ERRandomiserProperties*, uint64_t, uint32_t, uint32_t, uint32_t)>> randomise_static_property_functions;
	bool RandomiseProperty_EquipParamWeapon(uint64_t param_container_base, uint32_t param_id, uint32_t param_offsetid, uint32_t randomiser_action);
	bool RandomiseProperty_EquipParamGoods(uint64_t param_container_base, uint32_t param_id, uint32_t param_offsetid, uint32_t randomiser_action);

	// Properties: Runtime generated
	std::map<uint32_t, bool(ERRandomiserProperties::*)(ItemInfo*, uint32_t)> randomise_dynamic_property_functions;
	bool RandomiseProperty_Weapon(ItemInfo* item_info, uint32_t entry);
	bool RandomiseProperty_Armour(ItemInfo* item_info, uint32_t entry);
	bool RandomiseProperty_Accessory(ItemInfo* item_info, uint32_t entry);
	bool RandomiseProperty_Goods(ItemInfo* item_info, uint32_t entry);

};