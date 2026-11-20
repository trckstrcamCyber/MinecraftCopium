#include "stdafx.h"

#include "Consoles_SoundEngine.h"



const WCHAR *ConsoleSoundEngine::wchSoundNames[eSoundType_MAX]=
{
	L"mob.chicken.say",						//	eSoundType_MOB_CHICKEN_AMBIENT	
	L"mob.chicken.hurt",					//	eSoundType_MOB_CHICKEN_HURT
	L"mob.chicken.plop",					//	eSoundType_MOB_CHICKENPLOP
	L"mob.cow.say",							//	eSoundType_MOB_COW_AMBIENT
	L"mob.cow.hurt",						//	eSoundType_MOB_COW_HURT
	L"mob.pig.say",							//	eSoundType_MOB_PIG_AMBIENT
	L"mob.pig.death",					//	eSoundType_MOB_PIG_DEATH
	L"mob.sheep.say",						//	eSoundType_MOB_SHEEP_AMBIENT
	L"mob.wolf.growl",					//	eSoundType_MOB_WOLF_GROWL
	L"mob.wolf.whine",					//	eSoundType_MOB_WOLF_WHINE
	L"mob.wolf.panting",				//	eSoundType_MOB_WOLF_PANTING
	L"mob.wolf.bark",					//	eSoundType_MOB_WOLF_BARK
	L"mob.wolf.hurt",					//	eSoundType_MOB_WOLF_HURT
	L"mob.wolf.death",					//	eSoundType_MOB_WOLF_DEATH
	L"mob.wolf.shake",					//	eSoundType_MOB_WOLF_SHAKE
	L"mob.blaze.breathe",				//	eSoundType_MOB_BLAZE_BREATHE
	L"mob.blaze.hit",					//	eSoundType_MOB_BLAZE_HURT
	L"mob.blaze.death",					//	eSoundType_MOB_BLAZE_DEATH
	L"mob.ghast.moan",					//	eSoundType_MOB_GHAST_MOAN
	L"mob.ghast.scream",				//	eSoundType_MOB_GHAST_SCREAM
	L"mob.ghast.death",					//	eSoundType_MOB_GHAST_DEATH
	L"mob.ghast.fireball",				//	eSoundType_MOB_GHAST_FIREBALL
	L"mob.ghast.charge",				//	eSoundType_MOB_GHAST_CHARGE
	L"mob.endermen.idle",				//	eSoundType_MOB_ENDERMEN_IDLE
	L"mob.endermen.hit",				//	eSoundType_MOB_ENDERMEN_HIT
	L"mob.endermen.death",				//	eSoundType_MOB_ENDERMEN_DEATH
	L"mob.endermen.portal",				//	eSoundType_MOB_ENDERMEN_PORTAL
	L"mob.zombiepig.zpig",				//	eSoundType_MOB_ZOMBIEPIG_AMBIENT
	L"mob.zombiepig.zpighurt",			//	eSoundType_MOB_ZOMBIEPIG_HURT
	L"mob.zombiepig.zpigdeath",			//	eSoundType_MOB_ZOMBIEPIG_DEATH
	L"mob.zombiepig.zpigangry",			//	eSoundType_MOB_ZOMBIEPIG_ZPIGANGRY
	L"mob.silverfish.say",				//	eSoundType_MOB_SILVERFISH_AMBIENT,
	L"mob.silverfish.hit",				//	eSoundType_MOB_SILVERFISH_HURT
	L"mob.silverfish.kill",				//	eSoundType_MOB_SILVERFISH_DEATH,
	L"mob.silverfish.step",				//	eSoundType_MOB_SILVERFISH_STEP,
	L"mob.skeleton",					//	eSoundType_MOB_SKELETON_AMBIENT,
	L"mob.skeletonhurt",				//	eSoundType_MOB_SKELETON_HURT,
	L"mob.spider.say",						//	eSoundType_MOB_SPIDER_AMBIENT,
	L"mob.spider.death",					//	eSoundType_MOB_SPIDER_DEATH,
	L"mob.slime",						//	eSoundType_MOB_SLIME,
	L"mob.slime.attack",					//	eSoundType_MOB_SLIME_ATTACK,
	L"mob.creeper.say",						//	eSoundType_MOB_CREEPER_HURT,
	L"mob.creeper.death",				//	eSoundType_MOB_CREEPER_DEATH,
	L"mob.zombie.say",						//	eSoundType_MOB_ZOMBIE_AMBIENT,
	L"mob.zombie.hurt",					//	eSoundType_MOB_ZOMBIE_HURT,
	L"mob.zombie.death",					//	eSoundType_MOB_ZOMBIE_DEATH,
	L"mob.zombie.wood",					//	eSoundType_MOB_ZOMBIE_WOOD,
	L"mob.zombie.woodbreak",			//	eSoundType_MOB_ZOMBIE_WOOD_BREAK,
	L"mob.zombie.metal",				//	eSoundType_MOB_ZOMBIE_METAL,
	L"mob.magmacube.big",				//	eSoundType_MOB_MAGMACUBE_BIG,
	L"mob.magmacube.small",				//	eSoundType_MOB_MAGMACUBE_SMALL,
	L"mob.cat.purr",					//  eSoundType_MOB_CAT_PURR
	L"mob.cat.purreow",					//  eSoundType_MOB_CAT_PURREOW
	L"mob.cat.meow",					//  eSoundType_MOB_CAT_MEOW
	// 4J-PB - correct the name of the event for hitting ocelots
	L"mob.cat.hit",					//  eSoundType_MOB_CAT_HITT
//	L"mob.irongolem.throw",				//  eSoundType_MOB_IRONGOLEM_THROW
//	L"mob.irongolem.hit",				//  eSoundType_MOB_IRONGOLEM_HIT
//	L"mob.irongolem.death",				//  eSoundType_MOB_IRONGOLEM_DEATH
//	L"mob.irongolem.walk",				//  eSoundType_MOB_IRONGOLEM_WALK
	L"random.bow",						//	eSoundType_RANDOM_BOW,
	L"random.bowhit",					//	eSoundType_RANDOM_BOW_HIT,
	L"random.explode",					//	eSoundType_RANDOM_EXPLODE,
	L"random.fizz",						//	eSoundType_RANDOM_FIZZ,	
	L"random.pop",						//	eSoundType_RANDOM_POP,
	L"random.fuse",						//	eSoundType_RANDOM_FUSE,
	L"random.drink",					//	eSoundType_RANDOM_DRINK,
	L"random.eat",						//	eSoundType_RANDOM_EAT,
	L"random.burp",						//	eSoundType_RANDOM_BURP,
	L"random.splash",					//	eSoundType_RANDOM_SPLASH,
	L"random.click",					//	eSoundType_RANDOM_CLICK,
	L"random.glass",					//	eSoundType_RANDOM_GLASS,
	L"random.orb",						//	eSoundType_RANDOM_ORB,
	L"random.break",					//	eSoundType_RANDOM_BREAK,
	L"random.chestopen",				//	eSoundType_RANDOM_CHEST_OPEN,
	L"random.chestclosed",				//	eSoundType_RANDOM_CHEST_CLOSE,
	L"random.door_open",				//	eSoundType_RANDOM_DOOR_OPEN,
	L"random.door_close",				//	eSoundType_RANDOM_DOOR_CLOSE,
	L"ambient.weather.rain",			//	eSoundType_AMBIENT_WEATHER_RAIN,
	L"ambient.weather.thunder",			//	eSoundType_AMBIENT_WEATHER_THUNDER,
	L"ambient.cave",				//	eSoundType_CAVE_CAVE, DON'T USE FOR XBOX 360!!!
#ifdef _XBOX
	L"ambient.cave.cave2",				//	eSoundType_CAVE_CAVE2 - removed the two sounds that were at 192k in the first ambient cave event
#endif
	L"portal.portal",					//	eSoundType_PORTAL_PORTAL,
	// 4J-PB - added a couple that were still using wstring
	L"portal.trigger",					//	eSoundType_PORTAL_TRIGGER
	L"portal.travel",					//	eSoundType_PORTAL_TRAVEL
	
	L"fire.ignite",						//	eSoundType_FIRE_IGNITE,
	L"fire.fire",						//	eSoundType_FIRE_FIRE,
	L"damage.hurtflesh",				//	eSoundType_DAMAGE_HURT,
	L"damage.fallsmall",				//	eSoundType_DAMAGE_FALL_SMALL,
	L"damage.fallbig",					//	eSoundType_DAMAGE_FALL_BIG,
	L"note.harp",						//	eSoundType_NOTE_HARP,
	L"note.bd",							//	eSoundType_NOTE_BD,
	L"note.snare",						//	eSoundType_NOTE_SNARE,
	L"note.hat",						//	eSoundType_NOTE_HAT,
	L"note.bassattack",					//	eSoundType_NOTE_BASSATTACK,
	L"tile.piston.in",					//	eSoundType_TILE_PISTON_IN,
	L"tile.piston.out",					//	eSoundType_TILE_PISTON_OUT,
	L"liquid.water",					//	eSoundType_LIQUID_WATER,
	L"liquid.lavapop",					//	eSoundType_LIQUID_LAVA_POP,
	L"liquid.lava",						//	eSoundType_LIQUID_LAVA,
	L"step.stone",						//	eSoundType_STEP_STONE,
	L"step.wood",						//	eSoundType_STEP_WOOD,
	L"step.gravel",						//	eSoundType_STEP_GRAVEL,
	L"step.grass",						//	eSoundType_STEP_GRASS,
	L"step.metal",						//	eSoundType_STEP_METAL,
	L"step.cloth",						//	eSoundType_STEP_CLOTH,
	L"step.sand",						//	eSoundType_STEP_SAND,
	
	// below this are the additional sounds from the second soundbank
	L"mob.enderdragon.end",				//	eSoundType_MOB_ENDERDRAGON_END
	L"mob.enderdragon.growl",			//	eSoundType_MOB_ENDERDRAGON_GROWL
	L"mob.enderdragon.hit",				//	eSoundType_MOB_ENDERDRAGON_HIT
	L"mob.enderdragon.wings",			//	eSoundType_MOB_ENDERDRAGON_MOVE
	L"mob.irongolem.throw",				//  eSoundType_MOB_IRONGOLEM_THROW
	L"mob.irongolem.hit",				//  eSoundType_MOB_IRONGOLEM_HIT
	L"mob.irongolem.death",				//  eSoundType_MOB_IRONGOLEM_DEATH
	L"mob.irongolem.walk",				//  eSoundType_MOB_IRONGOLEM_WALK

	// TU14
	L"damage.thorns",					//  eSoundType_DAMAGE_THORNS
	L"random.anvil_break",				//  eSoundType_RANDOM_ANVIL_BREAK
	L"random.anvil_land",				//  eSoundType_RANDOM_ANVIL_LAND
	L"random.anvil_use",				//  eSoundType_RANDOM_ANVIL_USE
	L"mob.villager.haggle",				//  eSoundType_MOB_VILLAGER_HAGGLE
	L"mob.villager.idle",				//  eSoundType_MOB_VILLAGER_IDLE
	L"mob.villager.hit",				//  eSoundType_MOB_VILLAGER_HIT
	L"mob.villager.death",				//  eSoundType_MOB_VILLAGER_DEATH
	L"mob.villager.yes",				//  eSoundType_MOB_VILLAGER_YES
	L"mob.villager.no",					//  eSoundType_MOB_VILLAGER_NO
	L"mob.zombie.infect",				//  eSoundType_MOB_ZOMBIE_INFECT
	L"mob.zombie.unfect",				//  eSoundType_MOB_ZOMBIE_UNFECT
	L"mob.zombie.remedy",				//  eSoundType_MOB_ZOMBIE_REMEDY
	L"step.snow",						//  eSoundType_STEP_SNOW
	L"step.ladder",						//  eSoundType_STEP_LADDER
	L"dig.cloth",						//  eSoundType_DIG_CLOTH	
	L"dig.grass",						//  eSoundType_DIG_GRASS	
	L"dig.gravel",						//  eSoundType_DIG_GRAVEL	
	L"dig.sand",						//  eSoundType_DIG_SAND	
	L"dig.snow",						//  eSoundType_DIG_SNOW	
	L"dig.stone",						//  eSoundType_DIG_STONE	
	L"dig.wood",						//  eSoundType_DIG_WOOD	

	// 1.6.4
	L"fireworks.launch",				//eSoundType_FIREWORKS_LAUNCH,
	L"fireworks.blast",					//eSoundType_FIREWORKS_BLAST,
	L"fireworks.blast_far",				//eSoundType_FIREWORKS_BLAST_FAR,
	L"fireworks.large_blast",			//eSoundType_FIREWORKS_LARGE_BLAST,
	L"fireworks.large_blast_far",		//eSoundType_FIREWORKS_LARGE_BLAST_FAR,
	L"fireworks.twinkle",				//eSoundType_FIREWORKS_TWINKLE,
	L"fireworks.twinkle_far",			//eSoundType_FIREWORKS_TWINKLE_FAR,

	L"mob.bat.idle",					//eSoundType_MOB_BAT_IDLE,
	L"mob.bat.hurt",					//eSoundType_MOB_BAT_HURT,
	L"mob.bat.death",					//eSoundType_MOB_BAT_DEATH,
	L"mob.bat.takeoff",					//eSoundType_MOB_BAT_TAKEOFF,

	L"mob.wither.spawn",				//eSoundType_MOB_WITHER_SPAWN,
	L"mob.wither.idle",					//eSoundType_MOB_WITHER_IDLE,	
	L"mob.wither.hurt",					//eSoundType_MOB_WITHER_HURT, 
	L"mob.wither.death",				//eSoundType_MOB_WITHER_DEATH,
	L"mob.wither.shoot",				//eSoundType_MOB_WITHER_SHOOT,

	L"mob.cow.step",					//eSoundType_MOB_COW_STEP,
	L"mob.chicken.step",				//eSoundType_MOB_CHICKEN_STEP,
	L"mob.pig.step",					//eSoundType_MOB_PIG_STEP,
	L"mob.enderman.stare",				//eSoundType_MOB_ENDERMAN_STARE,
	L"mob.enderman.scream",				//eSoundType_MOB_ENDERMAN_SCREAM,
	L"mob.sheep.shear",					//eSoundType_MOB_SHEEP_SHEAR,
	L"mob.sheep.step",					//eSoundType_MOB_SHEEP_STEP,
	L"mob.skeleton.death",				//eSoundType_MOB_SKELETON_DEATH,
	L"mob.skeleton.step",				//eSoundType_MOB_SKELETON_STEP,
	L"mob.spider.step",					//eSoundType_MOB_SPIDER_STEP,
	L"mob.wolf.step",					//eSoundType_MOB_WOLF_STEP,
	L"mob.zombie.step",					//eSoundType_MOB_ZOMBIE_STEP,

	L"liquid.swim",						//eSoundType_LIQUID_SWIM,

	L"Mob.horse.land",					//eSoundType_MOB_HORSE_LAND,
	L"Mob.horse.armor",					//eSoundType_MOB_HORSE_ARMOR,
	L"Mob.horse.leather",				//eSoundType_MOB_HORSE_LEATHER,
	L"Mob.horse.zombie.death",			//eSoundType_MOB_HORSE_ZOMBIE_DEATH,
	L"Mob.horse.skeleton.death",		//eSoundType_MOB_HORSE_SKELETON_DEATH,
	L"Mob.horse.donkey.death",			//eSoundType_MOB_HORSE_DONKEY_DEATH,
	L"Mob.horse.death",					//eSoundType_MOB_HORSE_DEATH,
	L"Mob.horse.zombie.hit",			//eSoundType_MOB_HORSE_ZOMBIE_HIT,
	L"Mob.horse.skeleton.hit",			//eSoundType_MOB_HORSE_SKELETON_HIT,
	L"Mob.horse.donkey.hit",			//eSoundType_MOB_HORSE_DONKEY_HIT,
	L"Mob.horse.hit",					//eSoundType_MOB_HORSE_HIT,
	L"Mob.horse.zombie.idle",			//eSoundType_MOB_HORSE_ZOMBIE_IDLE,
	L"Mob.horse.skeleton.idle",			//eSoundType_MOB_HORSE_SKELETON_IDLE,
	L"Mob.horse.donkey.idle",			//eSoundType_MOB_HORSE_DONKEY_IDLE,
	L"Mob.horse.idle",					//eSoundType_MOB_HORSE_IDLE,
	L"Mob.horse.donkey.angry",			//eSoundType_MOB_HORSE_DONKEY_ANGRY,
	L"Mob.horse.angry",					//eSoundType_MOB_HORSE_ANGRY,
	L"Mob.horse.gallop",				//eSoundType_MOB_HORSE_GALLOP,
	L"Mob.horse.breathe",				//eSoundType_MOB_HORSE_BREATHE,
	L"Mob.horse.wood",					//eSoundType_MOB_HORSE_WOOD,
	L"Mob.horse.soft",					//eSoundType_MOB_HORSE_SOFT,
	L"Mob.horse.jump",					//eSoundType_MOB_HORSE_JUMP,

	L"mob.witch.ambient",					//eSoundType_MOB_WITCH_IDLE,
	L"mob.witch.hurt",					//eSoundType_MOB_WITCH_HURT,
	L"mob.witch.death",					//eSoundType_MOB_WITCH_DEATH,

	L"mob.slime.big",					//eSoundType_MOB_SLIME_BIG,
	L"mob.slime.small",					//eSoundType_MOB_SLIME_SMALL,

	L"eating",							//eSoundType_EATING						<--- missing
	L"random.levelup",					//eSoundType_RANDOM_LEVELUP

	// 4J-PB  - Some sounds were updated, but we can't do that for the 360 or we have to do a new sound bank
	// instead, we'll add the sounds as new ones and change the code to reference them
	L"fire.new_ignite",

	L"mob.rabbit.idle",
	L"mob.rabbit.hurt",
	L"mob.rabbit.bunnymurder",
	L"mob.rabbit.hop",

	L"item.armor.equip_leather1",
	L"item.armor.equip_leather2",
	L"item.armor.equip_leather3",
	L"item.armor.equip_leather4",
	L"item.armor.equip_leather5",
	L"item.armor.equip_leather6",

	L"item.armor.equip_chain1",
	L"item.armor.equip_chain2",
	L"item.armor.equip_chain3",
	L"item.armor.equip_chain4",
	L"item.armor.equip_chain5",
	L"item.armor.equip_chain6",

	L"item.armor.equip_iron1",
	L"item.armor.equip_iron2",
	L"item.armor.equip_iron3",
	L"item.armor.equip_iron4",
	L"item.armor.equip_iron5",
	L"item.armor.equip_iron6",

	L"item.armor.equip_gold1",
	L"item.armor.equip_gold2",
	L"item.armor.equip_gold3",
	L"item.armor.equip_gold4",
	L"item.armor.equip_gold5",
	L"item.armor.equip_gold6",

	L"item.armor.equip_diamond1",
	L"item.armor.equip_diamond2",
	L"item.armor.equip_diamond3",
	L"item.armor.equip_diamond4",
	L"item.armor.equip_diamond5",
	L"item.armor.equip_diamond6",

	L"item.armor.equip_generic1",
	L"item.armor.equip_generic2",
	L"item.armor.equip_generic3",
	L"item.armor.equip_generic4",
	L"item.armor.equip_generic5",
	L"item.armor.equip_generic6",

	L"item.armor.equip_copper1",
	L"item.armor.equip_copper2",
	L"item.armor.equip_copper3",
	L"item.armor.equip_copper4",
	L"item.armor.equip_copper5",
	L"item.armor.equip_copper5",			// neoLegacy - Copper Age (Phase 2): equip_copper6.ogg does not exist in jar1201 (orphan); the 6th slot reuses the copper5 sound

	L"damage.critical",				//eSoundType_DAMAGE_CRITICAL,
	L"item.elytra.flying",			// eSoundType_ITEM_ELYTRA_FLYING

	L"mob.guardian.attack_loop",        
	L"mob.guardian.guardian_death",             
	L"mob.guardian.guardian_hit",               

	L"mob.guardian.flop",             

	L"mob.guardian.land_death",        
	L"mob.guardian.land_hit",          
	L"mob.guardian.land_idle",         

	L"mob.guardian.curse",        
	L"mob.guardian.elder_death",        
	L"mob.guardian.elder_hit",         
	L"mob.guardian.elder_idle",

	L"mob.polarbear.babyidle",
	L"mob.polarbear.idle",
	L"mob.polarbear.hurt",
	L"mob.polarbear.death",
	L"mob.polarbear.step",
	L"mob.polarbear.warning",

	L"mob.goat.say",
	L"mob.goat.hurt",
	L"mob.goat.death",
	L"mob.goat.jump",
	L"mob.goat.ram_impact",

	L"mob.camel.dash",
	L"mob.camel.dash_ready",
	L"mob.camel.ambient",
	L"mob.camel.hurt",
	L"mob.camel.death",
	L"mob.camel.step",
	L"mob.camel.step_sand",
	L"mob.camel.sit",
	L"mob.camel.stand",
	L"mob.camel.eat",

	L"mob.drowned.idle",
	L"mob.drowned.hurt",
	L"mob.drowned.death",
	L"mob.drowned.step",
	L"mob.drowned.convert",
	L"mob.drowned.water.idle",
	L"mob.drowned.water.hurt",
	L"mob.drowned.water.death",
	L"mob.husk.idle",
	L"mob.husk.hurt",
	L"mob.husk.death",
	L"mob.husk.step",
	L"mob.husk.convert",
	L"mob.frostbite.ambient",
	L"mob.frostbite.hurt",
	L"mob.frostbite.death",
	L"mob.frostbite.step",
	L"mob.frostbite.convert",
	L"random.freezing",
	L"mob.vindicator.idle",
	L"mob.vindicator.hurt",
	L"mob.vindicator.death",
	L"mob.vindicator.celebrate",
	L"mob.evoker.idle",
	L"mob.evoker.hurt",
	L"mob.evoker.death",
	L"mob.evoker.cast",
	L"mob.evoker.prepare_attack",
	L"mob.evoker.prepare_summon",
	L"mob.evoker.prepare_wololo",
	L"mob.evoker.fangs",
	L"mob.evoker.celebrate",
	L"mob.iceologer.idle",
	L"mob.iceologer.hurt",
	L"mob.iceologer.death",
	L"mob.iceologer.cast",
	L"mob.iceologer.prepare",
	L"mob.iceologer.cube_ambient",
	L"mob.iceologer.cube_hit",
	L"mob.illusioner.idle",
	L"mob.illusioner.hurt",
	L"mob.illusioner.death",
	L"mob.illusioner.mirror_move",
	L"mob.illusioner.prepare_mirror",
	L"mob.illusioner.prepare_blind",
	L"mob.vex.idle",
	L"mob.vex.hurt",
	L"mob.vex.death",
	L"mob.vex.charge",
	L"item.crossbow.loading_start",
	L"item.crossbow.loading_middle",
	L"item.crossbow.loading_end",
	L"item.crossbow.shoot",
	L"mob.pillager.idle",
	L"mob.pillager.hurt",
	L"mob.pillager.death",
	L"mob.pillager.celebrate",
	L"mob.ravager.idle",
	L"mob.ravager.hurt",
	L"mob.ravager.death",
	L"mob.ravager.step",
	L"mob.ravager.bite",
	L"mob.ravager.roar",
	L"mob.ravager.stun",
	L"mob.ravager.celebrate",
	L"mob.provoker.idle",
	L"mob.provoker.hurt",
	L"mob.provoker.death",
	L"mob.provoker.celebrate",
	L"mob.sorcerer.idle",
	L"mob.sorcerer.hurt",
	L"mob.sorcerer.death",
	L"mob.sorcerer.celebrate",
	L"mob.sorcerer.cast",
	L"mob.sorcerer.completecast",
	L"mob.archivist.idle",
	L"mob.archivist.hurt",
	L"mob.archivist.death",
	L"mob.necromancer.summon",

	L"item.mace.smash_air",
	L"item.mace.smash_ground",
	L"item.mace.smash_ground_heavy",

	// tu43
	// witch sounds
	L"entity.witch.ambient",				//eSoundType_ENTITY_WITCH_AMBIENT,
	L"entity.witch.hurt",					//eSoundType_ENTITY_WITCH_HURT,
	L"entity.witch.death",					//eSoundType_ENTITY_WITCH_DEATH,
	L"entity.witch.throw",					//eSoundType_ENTITY_WITCH_THROW,

	// squid sounds
	L"entity.squid.ambient",					//eSoundType_ENTITY_SQUID_AMBIENT,
	L"entity.squid.hurt",					//eSoundType_ENTITY_SQUID_HURT,
	L"entity.squid.death",					//eSoundType_ENTITY_SQUID_DEATH,

	// snowman sounds
	L"entity.snowman.hurt",					//eSoundType_ENTITY_SNOWMAN_HURT,
	L"entity.snowman.death",					//eSoundType_ENTITY_SNOWMAN_DEATH,

	// armor stand sounds
	L"entity.armorstand.break",					//eSoundType_ENTITY_ARMORSTAND_BREAK,
	L"entity.armorstand.hit",					//eSoundType_ENTITY_ARMORSTAND_HIT,

	// cow sounds
	L"entity.cow.milk",					//eSoundType_ENTITY_COW_MILK,

	// horse sounds
	L"entity.horse.eat",					// eSoundType_ENTITY_EAT,

	// item frame sounds
	L"entity.itemframe.add_item",					//eSoundType_ENTITY_ITEMFRAME_ADD_ITEM,
	L"entity.itemframe.break",					//eSoundType_ENTITY_ITEMFRAME_BREAK,
	L"entity.itemframe.place",					//eSoundType_ENTITY_ITEMFRAME_PLACE,
	L"entity.itemframe.remove_item",					//eSoundType_ENTITY_ITEMFRAME_REMOVE_ITEM,
	L"entity.itemframe.rotate_item",					//eSoundType_ENTITY_ITEMFRAME_ROTATE_ITEM,

	// leash sounds
	L"entity.leashknot.break",					//eSoundType_ENTITY_LEASHKNOT_BREAK,
	L"entity.leashknot.place",					//eSoundType_ENTITY_LEASHKNOT_PLACE,

	// painting sounds
	L"entity.painting.place",					//eSoundType_ENTITY_PAINTING_PLACE,
	L"entity.painting.break",					//eSoundType_ENTITY_PAINTING_BREAK,

	// neoLegacy - Copper Age (Phase 3). Copper bulb toggle + copper chest
	// open/close. Must stay in the same order as the eSoundType enum.
	L"block.copper_bulb.toggle",				//eSoundType_COPPER_BULB_TOGGLE,
	L"block.copper_chest.copper_chest_open1",	//eSoundType_COPPER_CHEST_OPEN,
	L"block.copper_chest.copper_chest_close1",	//eSoundType_COPPER_CHEST_CLOSE,
	// neoLegacy - Copper Age (Phase 3): copper grate break/step sounds.
	L"block.copper_grate.break1",	//eSoundType_COPPER_GRATE_BREAK,
	L"block.copper_grate.step1",	//eSoundType_COPPER_GRATE_STEP,
	// neoLegacy - Copper Age (Phase 5). Copper golem + statue sounds.
	// Must stay in the same order as the eSoundType enum.
	L"entity.copper_golem.regular.spin",			//eSoundType_MOB_COPPER_GOLEM_SPIN,
	L"entity.copper_golem.regular.hurt",			//eSoundType_MOB_COPPER_GOLEM_HURT,
	L"entity.copper_golem.regular.death",			//eSoundType_MOB_COPPER_GOLEM_DEATH,
	L"entity.copper_golem.regular.step",			//eSoundType_MOB_COPPER_GOLEM_STEP,
	L"entity.copper_golem.weathered.spin",		//eSoundType_MOB_COPPER_GOLEM_WEATHERED_SPIN,
	L"entity.copper_golem.weathered.hurt",		//eSoundType_MOB_COPPER_GOLEM_WEATHERED_HURT,
	L"entity.copper_golem.weathered.death",		//eSoundType_MOB_COPPER_GOLEM_WEATHERED_DEATH,
	L"entity.copper_golem.weathered.step",		//eSoundType_MOB_COPPER_GOLEM_WEATHERED_STEP,
	L"entity.copper_golem.oxidized.spin",		//eSoundType_MOB_COPPER_GOLEM_OXIDIZED_SPIN,
	L"entity.copper_golem.oxidized.hurt",		//eSoundType_MOB_COPPER_GOLEM_OXIDIZED_HURT,
	L"entity.copper_golem.oxidized.death",		//eSoundType_MOB_COPPER_GOLEM_OXIDIZED_DEATH,
	L"entity.copper_golem.oxidized.step",		//eSoundType_MOB_COPPER_GOLEM_OXIDIZED_STEP,
	L"entity.copper_golem.spawn",				//eSoundType_MOB_COPPER_GOLEM_SPAWN,
	L"entity.copper_golem.item_drop",			//eSoundType_MOB_COPPER_GOLEM_ITEM_DROP,
	L"entity.copper_golem.item_no_drop",		//eSoundType_MOB_COPPER_GOLEM_ITEM_NO_DROP,
	L"entity.copper_golem.no_item_get",			//eSoundType_MOB_COPPER_GOLEM_NO_ITEM_GET,
	L"entity.copper_golem.no_item_no_get",		//eSoundType_MOB_COPPER_GOLEM_NO_ITEM_NO_GET,
	L"block.copper_statue.become_statue",		//eSoundType_COPPER_STATUE_BECOME_STATUE,
	L"block.copper_statue.break",				//eSoundType_COPPER_STATUE_BREAK,
	L"block.copper_statue.hit",				//eSoundType_COPPER_STATUE_HIT,
	L"block.copper_statue.place",				//eSoundType_COPPER_STATUE_PLACE,
	// neoLegacy - Copper Age (Phase 6). Must stay in the same order as
	// the eSoundType enum. TAKE_ITEM reuses the place_item files: jar1201
	// maps block.shelf.take_item to place_item1-4.
	L"block.copper_chest.copper_chest_weathered_open1",	//eSoundType_COPPER_CHEST_WEATHERED_OPEN,
	L"block.copper_chest.copper_chest_weathered_close1",	//eSoundType_COPPER_CHEST_WEATHERED_CLOSE,
	L"block.copper_chest.copper_chest_oxidized_open1",	//eSoundType_COPPER_CHEST_OXIDIZED_OPEN,
	L"block.copper_chest.copper_chest_oxidized_close1",	//eSoundType_COPPER_CHEST_OXIDIZED_CLOSE,
	L"block.shelf.activate",			//eSoundType_SHELF_ACTIVATE,
	L"block.shelf.deactivate",			//eSoundType_SHELF_DEACTIVATE,
	L"block.shelf.place_item",			//eSoundType_SHELF_PLACE_ITEM,
	L"block.shelf.place_item",			//eSoundType_SHELF_TAKE_ITEM,
	L"block.shelf.single_swap",			//eSoundType_SHELF_SINGLE_SWAP,
	L"block.shelf.multi_swap",			//eSoundType_SHELF_MULTI_SWAP,
};


const WCHAR *ConsoleSoundEngine::wchUISoundNames[eSFX_MAX]=
{
	L"back",
	L"craft",
	L"craftfail",
	L"focus",
	L"press",
	L"scroll",
	L"open_flip1",
	L"open_flip2",
	L"open_flip3"
};
