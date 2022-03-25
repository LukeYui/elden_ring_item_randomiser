#pragma once
#include <Windows.h>
#include <cstdint>
#include <array>
#include <map>
#include <functional>
#include <string>

#include "../RandomiserProperties/randomiser_properties.h"

class ERRandomiser {
public:
	bool Shuffle();


	bool ShouldRandomiseMapItem(ItemLotParam_map* param_container);
	ERRandomiserProperties* GetSpecialPropertyClass();

	ERRandomiser() {

	};

	ERRandomiser(bool is_random_keys, bool is_randomise_estusupgrade, uint32_t mapitem_seed, uint32_t enemyitem_seed, uint64_t solo_param_repository_class, get_equipparamgoods_entry* find_equipparamgoods_function) {
		this->random_keys = is_random_keys;
		this->randomise_estusupgrade = is_randomise_estusupgrade;
		this->mapitem_seed = mapitem_seed;
		this->enemyitem_seed = enemyitem_seed;
		this->randomkey_seed = 0 - (mapitem_seed + enemyitem_seed);
		this->static_rune_01 = 0;
		this->static_rune_02 = 0;
		this->static_runes = {
			8148,	// Godrick's Great Rune
			8149,	// Radahn's Great Rune
			8151,	// Rykard's Great Rune
			10080,  // Rennala's Great Rune
		};
		this->excluded_items = {
			130,  // Spectral Steed Whistle
			1001, // Flask of Crimson Tears (Tutorial)
			1051, // Flask of Cerulean Tears
			8010, // Rusty key
			8105, // Dectus Medallion (Left)
			8106, // Dectus Medallion (Right)
			8107, // Rold Medallion
			8109, // Academy Glintstone Key
			8111, // Carian Inverted Statue
			8121, // Dark Moon Ring
			8134, // Drawing-Room Key
			8146, // Miniature Ranni
			8158, // Spirit Calling Bell
			8159, // Fingerslayer Blade
			8171, // Chrysalids' Memento
			8175, // Haligtree Secret Medallion (Left)
			8176, // Haligtree Secret Medallion (Right)
			8186, // Imbued key
			8199, // Discarded Palace Key
			8590, // Whetstone Knife
		};
		this->solo_param_repository = solo_param_repository_class;
		this->equipparamgoods_function = find_equipparamgoods_function;
		this->randomiser_properties = ERRandomiserProperties();
		param_randomisation_instruction_map.emplace(L"EquipParamWeapon", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"EquipParamProtector", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"EquipParamAccessory", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"EquipParamGoods", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"ReinforceParamWeapon", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"ReinforceParamProtector", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"NpcParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"AtkParam_Npc", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"AtkParam_Pc", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"NpcThinkParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"Bullet", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"BulletCreateLimitParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"BehaviorParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"BehaviorParam_PC", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"Magic", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"SpEffectParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"SpEffectVfxParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"SpEffectSetParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"TalkParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"MenuColorTableParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"ItemLotParam_enemy", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"ItemLotParam_map", &ERRandomiser::ShuffleItemLotParam_map);
		param_randomisation_instruction_map.emplace(L"MoveParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"CharaInitParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"EquipMtrlSetParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"FaceParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"FaceRangeParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"ShopLineupParam", &ERRandomiser::ShuffleShopLineupParam);
		param_randomisation_instruction_map.emplace(L"ShopLineupParam_Recipe", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"GameAreaParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"CalcCorrectGraph", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"LockCamParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"ObjActParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"HitMtrlParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"KnockBackParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"DecalParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"ActionButtonParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"AiSoundParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"PlayRegionParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"NetworkAreaParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"NetworkParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"NetworkMsgParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"BudgetParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"BonfireWarpParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"BonfireWarpTabParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"BonfireWarpSubCategoryParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"MenuPropertySpecParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"MenuPropertyLayoutParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"MenuValueTableParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"Ceremony", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"PhantomParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"CharMakeMenuTopParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"CharMakeMenuListItemParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"HitEffectSfxConceptParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"HitEffectSfxParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WepAbsorpPosParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"ToughnessParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"SeMaterialConvertParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"ThrowDirectionSfxParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"DirectionCameraParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"RoleParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WaypointParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"ThrowParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"GrassTypeParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"GrassTypeParam_Lv1", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"GrassTypeParam_Lv2", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"GrassLodRangeParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"NpcAiActionParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"PartsDrawParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"AssetEnvironmentGeometryParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"AssetModelSfxParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"AssetMaterialSfxParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"AttackElementCorrectParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"FootSfxParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"MaterialExParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"HPEstusFlaskRecoveryParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"MPEstusFlaskRecoveryParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"MultiPlayCorrectionParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"MenuOffscrRendParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"ClearCountCorrectParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"MapMimicryEstablishmentParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WetAspectParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"SwordArtsParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"KnowledgeLoadScreenItemParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"MultiHPEstusFlaskBonusParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"MultiMPEstusFlaskBonusParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"MultiSoulBonusRateParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WorldMapPointParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WorldMapPieceParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WorldMapLegacyConvParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WorldMapPlaceNameParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"ChrModelParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"LoadBalancerParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"LoadBalancerDrawDistScaleParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"LoadBalancerDrawDistScaleParam_ps4", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"LoadBalancerDrawDistScaleParam_ps5", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"LoadBalancerDrawDistScaleParam_xb1", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"LoadBalancerDrawDistScaleParam_xb1x", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"LoadBalancerDrawDistScaleParam_xss", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"LoadBalancerDrawDistScaleParam_xsx", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"LoadBalancerNewDrawDistScaleParam_win64", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"LoadBalancerNewDrawDistScaleParam_ps4", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"LoadBalancerNewDrawDistScaleParam_ps5", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"LoadBalancerNewDrawDistScaleParam_xb1", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"LoadBalancerNewDrawDistScaleParam_xb1x", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"LoadBalancerNewDrawDistScaleParam_xss", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"LoadBalancerNewDrawDistScaleParam_xsx", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WwiseValueToStrParam_Switch_AttackType", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WwiseValueToStrParam_Switch_DamageAmount", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WwiseValueToStrParam_Switch_DeffensiveMaterial", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WwiseValueToStrParam_Switch_HitStop", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WwiseValueToStrParam_Switch_OffensiveMaterial", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WwiseValueToStrParam_Switch_GrassHitType", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WwiseValueToStrParam_Switch_PlayerShoes", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WwiseValueToStrParam_Switch_PlayerEquipmentTops", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WwiseValueToStrParam_Switch_PlayerEquipmentBottoms", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WwiseValueToStrParam_Switch_PlayerVoiceType", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WwiseValueToStrParam_Switch_AttackStrength", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WwiseValueToStrParam_EnvPlaceType", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WeatherParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WeatherLotParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WeatherAssetCreateParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WeatherAssetReplaceParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"SpeedtreeParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"RideParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"SeActivationRangeParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"RollingObjLotParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"NpcAiBehaviorProbability", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"BuddyParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"GparamRefSettings", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"RandomAppearParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"MapGridCreateHeightLimitInfoParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"EnvObjLotParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"MapDefaultInfoParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"BuddyStoneParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"LegacyDistantViewPartsReplaceParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"SoundCommonIngameParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"SoundAutoEnvSoundGroupParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"SoundAutoReverbEvaluationDistParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"SoundAutoReverbSelectParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"EnemyCommonParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"GameSystemCommonParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"GraphicsCommonParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"MenuCommonParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"PlayerCommonParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"CutsceneGparamWeatherParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"CutsceneGparamTimeParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"CutsceneTimezoneConvertParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"CutsceneWeatherOverrideGparamConvertParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"SoundCutsceneParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"ChrActivateConditionParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"CutsceneMapIdParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"CutSceneTextureLoadParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"GestureParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"EquipParamGem", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"EquipParamCustomWeapon", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"GraphicsConfig", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"SoundChrPhysicsSeParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"FeTextEffectParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"CoolTimeParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WhiteSignCoolTimeParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"MapPieceTexParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"MapNameTexParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WeatherLotTexParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"KeyAssignParam_TypeA", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"KeyAssignParam_TypeB", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"KeyAssignParam_TypeC", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"MapGdRegionInfoParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"MapGdRegionDrawParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"KeyAssignMenuItemParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"SoundAssetSoundObjEnableDistParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"SignPuddleParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"AutoCreateEnvSoundParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"WwiseValueToStrParam_BgmBossChrIdConv", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"ResistCorrectParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"PostureControlParam_WepRight", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"PostureControlParam_WepLeft", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"PostureControlParam_Gender", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"PostureControlParam_Pro", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"RuntimeBoneControlParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"TutorialParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"BaseChrSelectMenuParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"MimicryEstablishmentTexParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"SfxBlockResShareParam", &ERRandomiser::RandomiseGenericParamContainer);
		param_randomisation_instruction_map.emplace(L"HitEffectSeParam", &ERRandomiser::RandomiseGenericParamContainer);
	};

private:
	uint32_t random_keys;
	uint32_t randomise_estusupgrade;
	uint32_t mapitem_seed;
	uint32_t enemyitem_seed;
	uint32_t randomkey_seed;
	uint32_t static_rune_01;
	uint32_t static_rune_02;
	std::array<uint32_t, 4> static_runes;
	std::array<uint32_t, 20> excluded_items;
	uint64_t solo_param_repository;
	get_equipparamgoods_entry* equipparamgoods_function;
	ERRandomiserProperties randomiser_properties;


	std::map<std::wstring, std::function<bool(ERRandomiser*, uint64_t, std::wstring, uint32_t, bool, bool)>> param_randomisation_instruction_map;
	bool ShuffleItemLotParam_map(uint64_t solo_param_repository, std::wstring param_name, uint32_t seed, bool shuffle, bool scramble);
	bool ShuffleShopLineupParam(uint64_t solo_param_repository, std::wstring param_name, uint32_t seed, bool shuffle, bool scramble);
	bool RandomiseGenericParamContainer(uint64_t solo_param_repository, std::wstring param_name, uint32_t seed, bool shuffle, bool scramble);
};