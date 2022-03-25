# Elden Ring dynamic item randomiser (+ auto equip)

Item randomiser for Elden Ring, with additional option to automatically equip wearable items.

Features a high degree of randomisation - All 4039 map items and 2886 enemy items (loot tables) are shuffled. Weapons have their reinforce level, infusion, and assigned Ashes of War set at runtime. You may also share the random number seeds if you like.

Works by shuffling ItemLotParam_map and ItemLotParam_enemy on game startup. With specific cases:
* Some key items are needed for progression and should not be randomised
* Materials such as berries and flowers regenerate, so are excluded to discourage easy re-rerolling of items and preserve the crafting system
* Invalid items (such as placeholders) have been removed

## Generic param randomisation

One feature is generic param randomisation. This lets users either shuffle or scramble any game param of their choice. There are over 180 game params in Elden Ring
 
## Libraries used:

* MinHook: https://github.com/TsudaKageyu/minhook
* INIReader: https://github.com/benhoyt/inih
