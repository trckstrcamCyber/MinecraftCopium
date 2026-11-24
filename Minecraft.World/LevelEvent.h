#pragma once

// 4J - brought forward new sound events from 1.2.3
class LevelEvent
{
public:
	static const int SOUND_CLICK = 1000;
	static const int SOUND_CLICK_FAIL = 1001;
	static const int SOUND_LAUNCH = 1002;
	static const int SOUND_OPEN_DOOR = 1003;
	static const int SOUND_FIZZ = 1004;
	static const int SOUND_PLAY_RECORDING = 1005;

	static const int SOUND_GHAST_WARNING = 1007;
	static const int SOUND_GHAST_FIREBALL = 1008;
	static const int SOUND_BLAZE_FIREBALL = 1009;

	static const int SOUND_ZOMBIE_WOODEN_DOOR = 1010;
	static const int SOUND_ZOMBIE_IRON_DOOR = 1011;
	static const int SOUND_ZOMBIE_DOOR_CRASH = 1012;
	static const int SOUND_WITHER_BOSS_SPAWN = 1013;
	static const int SOUND_WITHER_BOSS_SHOOT = 1014;
	static const int SOUND_BAT_LIFTOFF = 1015;
	static const int SOUND_ZOMBIE_INFECTED = 1016;
	static const int SOUND_ZOMBIE_CONVERTED = 1017;
	static const int SOUND_DRAGON_DEATH = 1018;

	static const int SOUND_ANVIL_BROKEN = 1020;
	static const int SOUND_ANVIL_USED = 1021;
	static const int SOUND_ANVIL_LAND = 1022;
	static const int SOUND_PORTAL_TRAVEL = 1032;
	static const int SOUND_CHORUS_GROW = 1033;
	static const int SOUND_CHORUS_DEATH = 1034;
	static const int SOUND_OPEN_WOODEN_DOOR = 1040;
	static const int SOUND_CLOSE_WOODEN_DOOR = 1041;
	static const int SOUND_OPEN_IRON_DOOR = 1042;
	static const int SOUND_CLOSE_IRON_DOOR = 1043;
	static const int SOUND_OPEN_WOODEN_TRAP_DOOR = 1044;
	static const int SOUND_CLOSE_WOODEN_TRAP_DOOR = 1045;
	static const int SOUND_OPEN_IRON_TRAP_DOOR = 1046;
	static const int SOUND_CLOSE_IRON_TRAP_DOOR = 1047;
	static const int SOUND_OPEN_FENCE_GATE = 1048;
	static const int SOUND_CLOSE_FENCE_GATE = 1049;

	static const int PARTICLES_SHOOT = 2000;
	static const int PARTICLES_DESTROY_BLOCK = 2001;
	static const int PARTICLES_POTION_SPLASH = 2002;
	static const int PARTICLES_EYE_OF_ENDER_DEATH = 2003;
	static const int PARTICLES_MOBTILE_SPAWN = 2004;
	static const int PARTICLES_PLANT_GROWTH = 2005;

	// neoLegacy - Zoro sword combo enchantment cues (position = the player's feet block)
	static const int PARTICLES_ZORO_CRIT_STACK = 2100;	// data: bits 0-7 = current crit chance in %, bit 8 = a critical hit just landed
	static const int PARTICLES_ZORO_RAGE_STACK = 2101;	// data: current damage bonus in % (0-200)
	static const int PARTICLES_MACE_SMASH = 2102;		// x,y,z = block the target stood on; data: see MaceEffectParticle::FLAG_*

	static const int ANIMATION_END_GATEWAY_SPAWN = 3000;
	static const int ANIMATION_DRAGON_SUMMON_ROAR = 3001;

	//static const int ENDERDRAGON_KILLED = 9000; // 4J Added to signal the the enderdragon was killed
	static const int ENDERDRAGON_FIREBALL_SPLASH = 9001;
	static const int END_EGG_TELEPORT = 9002;
};