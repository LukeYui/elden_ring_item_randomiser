#pragma once
#include <Windows.h>
#include <thread>
#include <condition_variable>
#include <random>
#include <string>

#include "INIReader/INIReader.h"
#include "Hooks/item_randomiser_hooks.h"

#define ITEM_DEBUG 0

namespace ERItemRandomiserLoader {
	void CreateModFunctions();
};

enum UserPreferences {
	option_none = 0,
	option_autoequip = 1 << 0,
	option_randomisekeys = 1 << 1,
	option_randomiseestusupgrades = 1 << 2,
};

enum RandomiseType : uint8_t  {
	randomisetype_none = 0,
	randomisetype_shuffle = 1,
	randomisetype_scramble = 2,
	randomisetype_butcher = 3,
};

class ERItemRandomiser {
public:
	void RunSaveListener();
	bool GetUserPreferences();
	void RequestItemListSave(bool request_save = true); 
	void SetParamRandomisationPreference(std::wstring param_name, RandomiseType option);
	RandomiseType GetParamRandomisationPreference(std::wstring param_name);
	uint32_t GetRandomUint(uint32_t min, uint32_t max);
	uint32_t GetSeededRandomUint(uint32_t min, uint32_t max, uint32_t seed);

	ERItemRandomiser() {
		hook_class = {};
		is_mod_active = 1;
		user_preferences = option_none;
		signal_list_save = false;
		save_manager_wake = false;
		randomiser_seed = 0;
		param_container_names = {
			L"EquipParamWeapon",
			L"EquipParamProtector",
			L"EquipParamAccessory",
			L"EquipParamGoods",
			L"ReinforceParamWeapon",
			L"ReinforceParamProtector",
			L"NpcParam",
			L"AtkParam_Npc",
			L"AtkParam_Pc",
			L"NpcThinkParam",
			L"Bullet",
			L"BulletCreateLimitParam",
			L"BehaviorParam",
			L"BehaviorParam_PC",
			L"Magic",
			L"SpEffectParam",
			L"SpEffectVfxParam",
			L"SpEffectSetParam",
			L"TalkParam",
			L"MenuColorTableParam",
			L"ItemLotParam_enemy",
			L"ItemLotParam_map",
			L"MoveParam",
			L"CharaInitParam",
			L"EquipMtrlSetParam",
			L"FaceParam",
			L"FaceRangeParam",
			L"ShopLineupParam",
			L"ShopLineupParam_Recipe",
			L"GameAreaParam",
			L"CalcCorrectGraph",
			L"LockCamParam",
			L"ObjActParam",
			L"HitMtrlParam",
			L"KnockBackParam",
			L"DecalParam",
			L"ActionButtonParam",
			L"AiSoundParam",
			L"PlayRegionParam",
			L"NetworkAreaParam",
			L"NetworkParam",
			L"NetworkMsgParam",
			L"BudgetParam",
			L"BonfireWarpParam",
			L"BonfireWarpTabParam",
			L"BonfireWarpSubCategoryParam",
			L"MenuPropertySpecParam",
			L"MenuPropertyLayoutParam",
			L"MenuValueTableParam",
			L"Ceremony",
			L"PhantomParam",
			L"CharMakeMenuTopParam",
			L"CharMakeMenuListItemParam",
			L"HitEffectSfxConceptParam",
			L"HitEffectSfxParam",
			L"WepAbsorpPosParam",
			L"ToughnessParam",
			L"SeMaterialConvertParam",
			L"ThrowDirectionSfxParam",
			L"DirectionCameraParam",
			L"RoleParam",
			L"WaypointParam",
			L"ThrowParam",
			L"GrassTypeParam",
			L"GrassTypeParam_Lv1",
			L"GrassTypeParam_Lv2",
			L"GrassLodRangeParam",
			L"NpcAiActionParam",
			L"PartsDrawParam",
			L"AssetEnvironmentGeometryParam",
			L"AssetModelSfxParam",
			L"AssetMaterialSfxParam",
			L"AttackElementCorrectParam",
			L"FootSfxParam",
			L"MaterialExParam",
			L"HPEstusFlaskRecoveryParam",
			L"MPEstusFlaskRecoveryParam",
			L"MultiPlayCorrectionParam",
			L"MenuOffscrRendParam",
			L"ClearCountCorrectParam",
			L"MapMimicryEstablishmentParam",
			L"WetAspectParam",
			L"SwordArtsParam",
			L"KnowledgeLoadScreenItemParam",
			L"MultiHPEstusFlaskBonusParam",
			L"MultiMPEstusFlaskBonusParam",
			L"MultiSoulBonusRateParam",
			L"WorldMapPointParam",
			L"WorldMapPieceParam",
			L"WorldMapLegacyConvParam",
			L"WorldMapPlaceNameParam",
			L"ChrModelParam",
			L"LoadBalancerParam",
			L"LoadBalancerDrawDistScaleParam",
			L"LoadBalancerDrawDistScaleParam_ps4",
			L"LoadBalancerDrawDistScaleParam_ps5",
			L"LoadBalancerDrawDistScaleParam_xb1",
			L"LoadBalancerDrawDistScaleParam_xb1x",
			L"LoadBalancerDrawDistScaleParam_xss",
			L"LoadBalancerDrawDistScaleParam_xsx",
			L"LoadBalancerNewDrawDistScaleParam_win64",
			L"LoadBalancerNewDrawDistScaleParam_ps4",
			L"LoadBalancerNewDrawDistScaleParam_ps5",
			L"LoadBalancerNewDrawDistScaleParam_xb1",
			L"LoadBalancerNewDrawDistScaleParam_xb1x",
			L"LoadBalancerNewDrawDistScaleParam_xss",
			L"LoadBalancerNewDrawDistScaleParam_xsx",
			L"WwiseValueToStrParam_Switch_AttackType",
			L"WwiseValueToStrParam_Switch_DamageAmount",
			L"WwiseValueToStrParam_Switch_DeffensiveMaterial",
			L"WwiseValueToStrParam_Switch_HitStop",
			L"WwiseValueToStrParam_Switch_OffensiveMaterial",
			L"WwiseValueToStrParam_Switch_GrassHitType",
			L"WwiseValueToStrParam_Switch_PlayerShoes",
			L"WwiseValueToStrParam_Switch_PlayerEquipmentTops",
			L"WwiseValueToStrParam_Switch_PlayerEquipmentBottoms",
			L"WwiseValueToStrParam_Switch_PlayerVoiceType",
			L"WwiseValueToStrParam_Switch_AttackStrength",
			L"WwiseValueToStrParam_EnvPlaceType",
			L"WeatherParam",
			L"WeatherLotParam",
			L"WeatherAssetCreateParam",
			L"WeatherAssetReplaceParam",
			L"SpeedtreeParam",
			L"RideParam",
			L"SeActivationRangeParam",
			L"RollingObjLotParam",
			L"NpcAiBehaviorProbability",
			L"BuddyParam",
			L"GparamRefSettings",
			L"RandomAppearParam",
			L"MapGridCreateHeightLimitInfoParam",
			L"EnvObjLotParam",
			L"MapDefaultInfoParam",
			L"BuddyStoneParam",
			L"LegacyDistantViewPartsReplaceParam",
			L"SoundCommonIngameParam",
			L"SoundAutoEnvSoundGroupParam",
			L"SoundAutoReverbEvaluationDistParam",
			L"SoundAutoReverbSelectParam",
			L"EnemyCommonParam",
			L"GameSystemCommonParam",
			L"GraphicsCommonParam",
			L"MenuCommonParam",
			L"PlayerCommonParam",
			L"CutsceneGparamWeatherParam",
			L"CutsceneGparamTimeParam",
			L"CutsceneTimezoneConvertParam",
			L"CutsceneWeatherOverrideGparamConvertParam",
			L"SoundCutsceneParam",
			L"ChrActivateConditionParam",
			L"CutsceneMapIdParam",
			L"CutSceneTextureLoadParam",
			L"GestureParam",
			L"EquipParamGem",
			L"EquipParamCustomWeapon",
			L"GraphicsConfig",
			L"SoundChrPhysicsSeParam",
			L"FeTextEffectParam",
			L"CoolTimeParam",
			L"WhiteSignCoolTimeParam",
			L"MapPieceTexParam",
			L"MapNameTexParam",
			L"WeatherLotTexParam",
			L"KeyAssignParam_TypeA",
			L"KeyAssignParam_TypeB",
			L"KeyAssignParam_TypeC",
			L"MapGdRegionInfoParam",
			L"MapGdRegionDrawParam",
			L"KeyAssignMenuItemParam",
			L"SoundAssetSoundObjEnableDistParam",
			L"SignPuddleParam",
			L"AutoCreateEnvSoundParam",
			L"WwiseValueToStrParam_BgmBossChrIdConv",
			L"ResistCorrectParam",
			L"PostureControlParam_WepRight",
			L"PostureControlParam_WepLeft",
			L"PostureControlParam_Gender",
			L"PostureControlParam_Pro",
			L"RuntimeBoneControlParam",
			L"TutorialParam",
			L"BaseChrSelectMenuParam",
			L"MimicryEstablishmentTexParam",
			L"SfxBlockResShareParam",
			L"HitEffectSeParam",
		};
	};

	ERRandomiserBase hook_class;
	std::array<std::wstring, 185> param_container_names;

private:
	uint32_t is_mod_active;
	UserPreferences user_preferences;
	uint64_t randomiser_seed;
	bool signal_list_save;
	bool save_manager_wake;
	std::mutex save_request_mutex;
	std::condition_variable save_request_wait;
	std::map<std::wstring, RandomiseType> param_container_random_preferences;
};
