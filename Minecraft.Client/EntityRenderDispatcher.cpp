#include "stdafx.h"
#include "EntityRenderDispatcher.h"
#include "../Minecraft.World/net.minecraft.world.entity.projectile.h"
#include "../Minecraft.World/net.minecraft.world.entity.animal.h"
#include "../Minecraft.World/net.minecraft.world.entity.monster.h"
#include "../Minecraft.World/net.minecraft.world.entity.item.h"
#include "../Minecraft.World/net.minecraft.world.entity.global.h"
#include "../Minecraft.World/net.minecraft.world.entity.player.h"
#include "../Minecraft.World/net.minecraft.world.entity.boss.enderdragon.h"
#include "../Minecraft.World/net.minecraft.world.entity.npc.h"
#include "../Minecraft.World/net.minecraft.world.entity.h"
#include "../Minecraft.World/net.minecraft.world.level.h"
#include "../Minecraft.World/net.minecraft.world.level.tile.h"
#include "../Minecraft.World/net.minecraft.world.item.h"
#include "../Minecraft.World/net.minecraft.world.item.alchemy.h"
#include "SpiderRenderer.h"
#include "PigRenderer.h"
#include "SheepRenderer.h"
#include "CowRenderer.h"
#include "WolfRenderer.h"
#include "ChickenRenderer.h"
#include "CreeperRenderer.h"
#include "SlimeRenderer.h"
#include "PlayerRenderer.h"
#include "GhastRenderer.h"
#include "SquidRenderer.h"
#include "GiantMobRenderer.h"
#include "EntityRenderer.h"
#include "PaintingRenderer.h"
#include "ArrowRenderer.h"
#include "FireballRenderer.h"
#include "ItemRenderer.h"
#include "ItemSpriteRenderer.h"
#include "TntRenderer.h"
#include "FallingTileRenderer.h"
#include "MinecartRenderer.h"
#include "BoatRenderer.h"
#include "FishingHookRenderer.h"
#include "LightningBoltRenderer.h"
#include "HumanoidMobRenderer.h"
#include "DefaultRenderer.h"
#include "EndermanRenderer.h"
#include "ExperienceOrbRenderer.h"
#include "SilverfishRenderer.h"
#include "MushroomCowRenderer.h"
#include "SnowManRenderer.h"
#include "LavaSlimeRenderer.h"
#include "VillagerRenderer.h"
#include "EnderDragonRenderer.h"
#include "EnderCrystalRenderer.h"
#include "BlazeRenderer.h"
#include "SkeletonRenderer.h"
#include "WitchRenderer.h"
#include "WitherBossRenderer.h"
#include "LeashKnotRenderer.h"
#include "WitherSkullRenderer.h"
#include "TntMinecartRenderer.h"
#include "MinecartSpawnerRenderer.h"
#include "HorseRenderer.h"
#include "SpiderModel.h"
#include "PigModel.h"
#include "SheepModel.h"
#include "CowModel.h"
#include "WolfModel.h"
#include "ChickenModel.h"
#include "CreeperModel.h"
#include "SlimeModel.h"
#include "GhastModel.h"
#include "SquidModel.h"
#include "MinecartModel.h"
#include "BoatModel.h"
#include "HumanoidModel.h"
#include "SheepFurModel.h"
#include "SkeletonModel.h"
#include "ModelHorse.h"
#include "Options.h"
#include "ItemFrameRenderer.h"
#include "OcelotRenderer.h"
#include "VillagerGolemRenderer.h"
#include "CopperGolemRenderer.h"	// neoLegacy - Copper Age (Phase 5)
#include "OcelotModel.h"
#include "ZombieRenderer.h"
#include "BatRenderer.h"
#include "CaveSpiderRenderer.h"
#include "RabbitRenderer.h"
#include "PolarBearRenderer.h"
#include "PolarBearModel.h"
#include "GoatRenderer.h"
#include "JellySlimeRenderer.h"
#include "GoatModel.h"
#include "CamelRenderer.h"
#include "CamelModel.h"
#include "HuskRenderer.h"
#include "FrostbiteRenderer.h"
#include "IllagerRenderer.h"
#include "RavagerRenderer.h"
#include "VexRenderer.h"
#include "SurrenderedRenderer.h"
#include "EvokerFangsRenderer.h"
#include "IceologerRenderer.h"
#include "IceCubeRenderer.h"
#include "IllusionerRenderer.h"
#include "DrownedRenderer.h"
#include "WanderingTraderRenderer.h"

#include "ArmorStandRenderer.h"
#include "EndermiteRenderer.h"
#include "ShulkerRenderer.h"
#include "ShulkerBulletRenderer.h"
#include "GuardianRenderer.h"
#include "GuardianModel.h"
#include "MobRenderer.h"

double EntityRenderDispatcher::xOff = 0.0;
double EntityRenderDispatcher::yOff = 0.0;
double EntityRenderDispatcher::zOff = 0.0;

EntityRenderDispatcher *EntityRenderDispatcher::instance = nullptr;

void EntityRenderDispatcher::staticCtor()
{
	instance = new EntityRenderDispatcher();
}

EntityRenderDispatcher::EntityRenderDispatcher()
{
	glEnable(GL_LIGHTING);
	renderers[eTYPE_SPIDER] = new SpiderRenderer();
	renderers[eTYPE_CAVESPIDER] = new CaveSpiderRenderer();
	renderers[eTYPE_PIG] = new PigRenderer(new PigModel(), new PigModel(0.5f), 0.7f);
	renderers[eTYPE_SHEEP] = new SheepRenderer(new SheepModel(), new SheepFurModel(), 0.7f);
	renderers[eTYPE_COW] = new CowRenderer(new CowModel(), 0.7f);
	renderers[eTYPE_MUSHROOMCOW] = new MushroomCowRenderer(new CowModel(), 0.7f);
	renderers[eTYPE_WOLF] = new WolfRenderer(new WolfModel(), new WolfModel(), 0.5f);
	renderers[eTYPE_CHICKEN] = new ChickenRenderer(new ChickenModel(), 0.3f);
	renderers[eTYPE_OCELOT] = new OcelotRenderer(new OcelotModel(), 0.4f);
	renderers[eTYPE_SILVERFISH] = new SilverfishRenderer();
	renderers[eTYPE_CREEPER] = new CreeperRenderer();
	renderers[eTYPE_ENDERMAN] = new EndermanRenderer();
	renderers[eTYPE_SNOWMAN] = new SnowManRenderer();
	renderers[eTYPE_SKELETON] = new SkeletonRenderer();
	renderers[eTYPE_WITCH] = new WitchRenderer();
	renderers[eTYPE_BLAZE] = new BlazeRenderer();
	renderers[eTYPE_ZOMBIE] = new ZombieRenderer();
	renderers[eTYPE_PIGZOMBIE] = new ZombieRenderer();
	renderers[eTYPE_SLIME] = new SlimeRenderer(new SlimeModel(16), new SlimeModel(0), 0.25f);
	renderers[eTYPE_LAVASLIME] = new LavaSlimeRenderer();
	renderers[eTYPE_JELLYSLIME] = new JellySlimeRenderer(new SlimeModel(16), new SlimeModel(0), 0.25f);	// neoLegacy
	renderers[eTYPE_PLAYER] = new PlayerRenderer();
	renderers[eTYPE_GIANT] = new GiantMobRenderer(new ZombieModel(), 0.5f, 6);
	renderers[eTYPE_GHAST] = new GhastRenderer();
	renderers[eTYPE_SQUID] = new SquidRenderer(new SquidModel(), 0.7f);
	renderers[eTYPE_VILLAGER] = new VillagerRenderer();
	renderers[eTYPE_VILLAGERGOLEM] = new VillagerGolemRenderer();
	renderers[eTYPE_COPPER_GOLEM] = new CopperGolemRenderer();	// neoLegacy - Copper Age (Phase 5)
	renderers[eTYPE_BAT] = new BatRenderer();
	renderers[eTYPE_GUARDIAN] = new GuardianRenderer(new GuardianModel(), 0.5f);
	renderers[eTYPE_ELDER_GUARDIAN] = new GuardianRenderer(new GuardianModel(), 0.5f);
	

	renderers[eTYPE_MOB] = new MobRenderer(new HumanoidModel(), 0.5f);

	renderers[eTYPE_ENDERDRAGON] = new EnderDragonRenderer();
	renderers[eTYPE_ENDER_CRYSTAL] = new EnderCrystalRenderer();

	renderers[eTYPE_WITHERBOSS] = new WitherBossRenderer();

	renderers[eTYPE_RABBIT] = new RabbitRenderer();
	renderers[eTYPE_POLARBEAR] = new PolarBearRenderer(new PolarBearModel(), 0.7f);
	renderers[eTYPE_GOAT] = new GoatRenderer(new GoatModel(), 0.5f);
	renderers[eTYPE_CAMEL] = new CamelRenderer(new CamelModel(), 0.9f);
	renderers[eTYPE_HUSK] = new HuskRenderer();
	renderers[eTYPE_FROSTBITE] = new FrostbiteRenderer();
	renderers[eTYPE_VINDICATOR] = new IllagerRenderer();	// neoLegacy
	renderers[eTYPE_EVOKER] = new IllagerRenderer();
	renderers[eTYPE_VEX] = new VexRenderer();
	renderers[eTYPE_EVOKER_FANGS] = new EvokerFangsRenderer();
	renderers[eTYPE_ICEOLOGER] = new IceologerRenderer();
	renderers[eTYPE_ICE_CUBE] = new IceCubeRenderer();
	renderers[eTYPE_ILLUSIONER] = new IllusionerRenderer();
	renderers[eTYPE_PILLAGER] = new IllagerRenderer();	// neoLegacy
	renderers[eTYPE_RAVAGER] = new RavagerRenderer();	// neoLegacy
	renderers[eTYPE_PROVOKER] = new IllagerRenderer();	// neoLegacy
	renderers[eTYPE_SORCERER] = new IllagerRenderer(64, 128, 1);	// neoLegacy - wizard hat, 64x128 skin
	renderers[eTYPE_ALCHEMIST] = new IllagerRenderer(128, 64, 2);	// neoLegacy - big brim, 128x64 skin
	renderers[eTYPE_ARCHIVIST] = new IllagerRenderer();	// neoLegacy
	renderers[eTYPE_NECROMANCER] = new IllagerRenderer(64, 64, 0, 1);	// neoLegacy - robe 20 high
	renderers[eTYPE_BASHER] = new IllagerRenderer();	// neoLegacy
	renderers[eTYPE_MARAUDER] = new IllagerRenderer();	// neoLegacy
	renderers[eTYPE_INQUISITOR] = new IllagerRenderer(64, 128, 0, 4);	// neoLegacy - 64x128 skin, shoulder pads
	renderers[eTYPE_FIRECALLER] = new IllagerRenderer(128, 64, 0, 8);	// neoLegacy - 128x64 skin, staff
	renderers[eTYPE_SURRENDERED] = new SurrenderedRenderer();	// neoLegacy - a skeleton-model Vex
	renderers[eTYPE_INVOKER] = new IllagerRenderer(128, 128, 0, 2);	// neoLegacy - 128x128 skin, long robe
	renderers[eTYPE_DROWNED] = new DrownedRenderer();
	renderers[eTYPE_WANDERING_TRADER] = new WanderingTraderRenderer();
	

	renderers[eTYPE_ENTITY] = new DefaultRenderer();
	renderers[eTYPE_AREA_EFFECT_CLOUD] = new DefaultRenderer();
	renderers[eTYPE_PAINTING] = new PaintingRenderer();        
	renderers[eTYPE_ITEM_FRAME] = new ItemFrameRenderer();
	renderers[eTYPE_LEASHFENCEKNOT] = new LeashKnotRenderer();
	renderers[eTYPE_ARROW] = new ArrowRenderer();
	renderers[eTYPE_SNOWBALL] = new ItemSpriteRenderer(Item::snowball);
	renderers[eTYPE_THROWNENDERPEARL] = new ItemSpriteRenderer(Item::ender_pearl);
	renderers[eTYPE_EYEOFENDERSIGNAL] = new ItemSpriteRenderer(Item::eye_of_ender);
	renderers[eTYPE_THROWNEGG] = new ItemSpriteRenderer(Item::egg);
	renderers[eTYPE_THROWNPOTION] = new ItemSpriteRenderer(Item::potion, PotionBrewing::THROWABLE_MASK);
	renderers[eTYPE_THROWNEXPBOTTLE] = new ItemSpriteRenderer(Item::experience_bottle);
	renderers[eTYPE_FIREWORKS_ROCKET] = new ItemSpriteRenderer(Item::fireworks);
	renderers[eTYPE_LARGE_FIREBALL] = new FireballRenderer(2.0f);
	renderers[eTYPE_SMALL_FIREBALL] = new FireballRenderer(0.5f);
	renderers[eTYPE_DRAGON_FIREBALL] = new FireballRenderer(2.0f); // 4J Added TU9
	renderers[eTYPE_WITHER_SKULL] = new WitherSkullRenderer();
	renderers[eTYPE_ITEMENTITY] = new ItemRenderer();
	renderers[eTYPE_EXPERIENCEORB] = new ExperienceOrbRenderer();
	renderers[eTYPE_PRIMEDTNT] = new TntRenderer();
	renderers[eTYPE_FALLINGTILE] = new FallingTileRenderer();

	renderers[eTYPE_MINECART_TNT] = new TntMinecartRenderer();
	renderers[eTYPE_MINECART_SPAWNER] = new MinecartSpawnerRenderer();
	renderers[eTYPE_MINECART_RIDEABLE] = new MinecartRenderer();
	renderers[eTYPE_MINECART_DIAMOND] = new MinecartRenderer();
	
	renderers[eTYPE_MINECART_FURNACE] = new MinecartRenderer();
	renderers[eTYPE_MINECART_CHEST] = new MinecartRenderer();
	renderers[eTYPE_MINECART_HOPPER] = new MinecartRenderer();

	renderers[eTYPE_BOAT] = new BoatRenderer();
	renderers[eTYPE_FISHINGHOOK] = new FishingHookRenderer();

	renderers[eTYPE_HORSE] = new HorseRenderer(new ModelHorse(), .75f);

	renderers[eTYPE_LIGHTNINGBOLT] = new LightningBoltRenderer();

	renderers[eTYPE_ARMORSTAND] = new ArmorStandRenderer();
	renderers[eTYPE_ENDERMITE] = new EndermiteRenderer();
	renderers[eTYPE_SHULKER] = new ShulkerRenderer();
	renderers[eTYPE_SHULKER_BULLET] = new ShulkerBulletRenderer();
	glDisable(GL_LIGHTING);

	for( auto& it : renderers )
	{
		it.second->init(this);
	}

	isGuiRender = false;		// 4J added
	isInventoryRender = false;

}

EntityRenderer *EntityRenderDispatcher::getRenderer(eINSTANCEOF e)
{
	if( (e & eTYPE_PLAYER) == eTYPE_PLAYER) e = eTYPE_PLAYER;
	//EntityRenderer * r = renderers[e];
	auto it = renderers.find(e); // 4J Stu - The .at and [] accessors insert elements if they don't exist

	if( it == renderers.end() )
	{
		app.DebugPrintf("Couldn't find renderer for entity of type %d\n", e);
		// New renderer mapping required in above table
		DEBUG_BREAK();
	}
	/* 4J - not doing this hierarchical search anymore. We need to explicitly add renderers for any eINSTANCEOF type that we want to be able to render
	if (it == renderers.end() && e != Entity::_class)
	{
	EntityRenderer *r = getRenderer(dynamic_cast<Entity::Class *>( e->getSuperclass() ));
	renderers.insert( classToRendererMap::value_type( e, r ) );
	return r;
	//assert(false);
	}*/
	return it->second;
}

EntityRenderer *EntityRenderDispatcher::getRenderer(shared_ptr<Entity> e)
{
	return getRenderer(e->GetType());
}

void EntityRenderDispatcher::prepare(Level *level, Textures *textures, Font *font, shared_ptr<LivingEntity> player, shared_ptr<LivingEntity> crosshairPickMob, Options *options, float a)
{
	this->level = level;
	this->textures = textures;
	this->options = options;
	this->cameraEntity = player;
	this->font = font;
	this->crosshairPickMob = crosshairPickMob;

	if (player->isSleeping())
	{
		int t = level->getTile(Mth::floor(player->x), Mth::floor(player->y), Mth::floor(player->z));
		if (t == Tile::bed_Id)
		{
			int data = level->getData(Mth::floor(player->x), Mth::floor(player->y), Mth::floor(player->z));

			int direction = data & 3;
			playerRotY = static_cast<float>(direction * 90 + 180);
			playerRotX = 0;
		}
	} else {
		playerRotY = player->yRotO + (player->yRot - player->yRotO) * a;
		playerRotX = player->xRotO + (player->xRot - player->xRotO) * a;
	}

	shared_ptr<Player> pl = dynamic_pointer_cast<Player>(player);
	if (pl->ThirdPersonView() == 2)
	{
		playerRotY += 180;
		playerRotX = -playerRotX;
	}

	xPlayer = player->xOld + (player->x - player->xOld) * a;
	yPlayer = player->yOld + (player->y - player->yOld) * a;
	zPlayer = player->zOld + (player->z - player->zOld) * a;

}

void EntityRenderDispatcher::render(shared_ptr<Entity> entity, float a)
{
	if (entity == nullptr)
	{
		return;
	}

	double x = entity->xOld + (entity->x - entity->xOld) * a;
	double y = entity->yOld + (entity->y - entity->yOld) * a;
	double z = entity->zOld + (entity->z - entity->zOld) * a;

	// Fix for #61057 - TU7: Gameplay: Boat is glitching when player float forward and turning.
	// Fix to handle the case that yRot and yRotO wrap over the 0/360 line
	float rotDiff = entity->yRot - entity->yRotO;
	if( rotDiff > 180 || rotDiff < -180)
	{
		if(entity->yRot > entity->yRotO)
		{
			rotDiff = (entity->yRot - 360) - entity->yRotO;
		}
		else
		{
			rotDiff = entity->yRot - (entity->yRotO - 360);
		}
	}
	float r = entity->yRotO + (rotDiff) * a;

	int col = entity->getLightColor(a);
	if (entity->isOnFire())
	{
		col = SharedConstants::FULLBRIGHT_LIGHTVALUE;
	}
	int u = col % 65536;
	int v = col / 65536;
	glMultiTexCoord2f(GL_TEXTURE1, u / 1.0f, v / 1.0f);
	glColor4f(1, 1, 1, 1);

	render(entity, x - xOff, y - yOff, z - zOff, r, a);
}

void EntityRenderDispatcher::render(shared_ptr<Entity> entity, double x, double y, double z, float rot, float a, bool bItemFrame, bool bRenderPlayerShadow)
{
	EntityRenderer *renderer = getRenderer(entity);
	if (renderer != nullptr)
	{	
		renderer->SetItemFrame(bItemFrame);

		renderer->render(entity, x, y, z, rot, a);
		renderer->postRender(entity, x, y, z, rot, a, bRenderPlayerShadow);
	}

	// neoLegacy - Glowing: a coloured aura through walls (not around the player's own view)
	if (entity->isGlowing() && entity.get() != cameraEntity.get()) renderGlow(entity, x, y, z, a);
}

// Glowing (the Horn of Sight's effect). The Colored Glow Lib mod gives every target its own colour, with a rainbow and animated colours; this follows that idea:
// the colour comes from what the thing is, so every player sees the same one, and the aura breathes.
//   rainbow: bosses (Wither, Ender Dragon, Invoker) and illager captains    purple: illagers, Ravagers, Witches    green: undead
//   red: other monsters    yellow: animals    cyan: players    white: anything else
// It is a translucent box slightly bigger than the hitbox, drawn without the depth test, so it shows through walls.
void EntityRenderDispatcher::renderGlow(shared_ptr<Entity> entity, double x, double y, double z, float a)
{
	float cr = 1.0f, cg = 1.0f, cb = 1.0f;
	shared_ptr<Illager> illager = dynamic_pointer_cast<Illager>(entity);
	bool boss = entity->GetType() == eTYPE_WITHERBOSS || entity->GetType() == eTYPE_ENDERDRAGON || entity->GetType() == eTYPE_INVOKER;
	if (boss || (illager != nullptr && illager->isCaptain()))
	{
		// rainbow: the hue turns with time, and each entity starts at its own point of the wheel
		float hue = fmodf((entity->tickCount + a) * 4.0f + entity->entityId * 37.0f, 360.0f) / 60.0f;
		int i = static_cast<int>(hue);
		float f = hue - i;
		switch (i % 6)
		{
		case 0: cr = 1.0f; cg = f; cb = 0.0f; break;
		case 1: cr = 1.0f - f; cg = 1.0f; cb = 0.0f; break;
		case 2: cr = 0.0f; cg = 1.0f; cb = f; break;
		case 3: cr = 0.0f; cg = 1.0f - f; cb = 1.0f; break;
		case 4: cr = f; cg = 0.0f; cb = 1.0f; break;
		default: cr = 1.0f; cg = 0.0f; cb = 1.0f - f; break;
		}
	}
	else if (illager != nullptr || entity->GetType() == eTYPE_RAVAGER || entity->GetType() == eTYPE_WITCH) { cr = 0.67f; cg = 0.24f; cb = 1.0f; }
	else
	{
		shared_ptr<LivingEntity> living = dynamic_pointer_cast<LivingEntity>(entity);
		if (living != nullptr && living->getMobType() == UNDEAD) { cr = 0.27f; cg = 1.0f; cb = 0.35f; }
		else if (entity->instanceof(eTYPE_MONSTER)) { cr = 1.0f; cg = 0.24f; cb = 0.24f; }
		else if (entity->instanceof(eTYPE_ANIMAL)) { cr = 1.0f; cg = 0.86f; cb = 0.24f; }
		else if (entity->instanceof(eTYPE_PLAYER)) { cr = 0.31f; cg = 0.86f; cb = 1.0f; }
	}

	int alpha = 70 + static_cast<int>(30.0f * Mth::sin((entity->tickCount + a) * 0.2f));	// the aura breathes
	int r = static_cast<int>(cr * 255), g = static_cast<int>(cg * 255), b = static_cast<int>(cb * 255);

	glDepthMask(false);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	double hw = entity->bbWidth / 2 + 0.06;
	double y0 = y - 0.06, y1 = y + entity->bbHeight + 0.06;
	Tesselator *t = Tesselator::getInstance();
	t->begin();
	t->color(r, g, b, alpha);
	// four sides, top and bottom
	t->vertex(x - hw, y1, z - hw); t->vertex(x - hw, y0, z - hw); t->vertex(x + hw, y0, z - hw); t->vertex(x + hw, y1, z - hw);
	t->vertex(x + hw, y1, z + hw); t->vertex(x + hw, y0, z + hw); t->vertex(x - hw, y0, z + hw); t->vertex(x - hw, y1, z + hw);
	t->vertex(x + hw, y1, z - hw); t->vertex(x + hw, y0, z - hw); t->vertex(x + hw, y0, z + hw); t->vertex(x + hw, y1, z + hw);
	t->vertex(x - hw, y1, z + hw); t->vertex(x - hw, y0, z + hw); t->vertex(x - hw, y0, z - hw); t->vertex(x - hw, y1, z - hw);
	t->vertex(x - hw, y1, z - hw); t->vertex(x + hw, y1, z - hw); t->vertex(x + hw, y1, z + hw); t->vertex(x - hw, y1, z + hw);
	t->vertex(x - hw, y0, z + hw); t->vertex(x + hw, y0, z + hw); t->vertex(x + hw, y0, z - hw); t->vertex(x - hw, y0, z - hw);
	t->end();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthMask(true);
}

double EntityRenderDispatcher::distanceToSqr(double x, double y, double z)
{
	double xd = x - xPlayer;
	double yd = y - yPlayer;
	double zd = z - zPlayer;
	return xd * xd + yd * yd + zd * zd;
}

Font *EntityRenderDispatcher::getFont()
{
	return font;
}

void EntityRenderDispatcher::registerTerrainTextures(IconRegister *iconRegister)
{
	for( auto& it : renderers )
	{
		EntityRenderer *renderer = it.second;
		renderer->registerTerrainTextures(iconRegister);
	}
}

void EntityRenderDispatcher::renderHitbox(shared_ptr<Entity> entity, double x, double y, double z, float rot, float a)
{
	glDepthMask(false);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	glPushMatrix();
	Tesselator *t = Tesselator::getInstance();

	t->begin();
	t->color(255, 255, 255, 32);

	double wnx = -entity->bbWidth / 2;
	double wnz = -entity->bbWidth / 2;
	double enx = entity->bbWidth / 2;
	double enz = -entity->bbWidth / 2;

	double wsx = -entity->bbWidth / 2;
	double wsz = entity->bbWidth / 2;
	double esx = entity->bbWidth / 2;
	double esz = entity->bbWidth / 2;

	double top = entity->bbHeight;

	t->vertex(x + wnx, y + top, z + wnz);
	t->vertex(x + wnx, y, z + wnz);
	t->vertex(x + enx, y, z + enz);
	t->vertex(x + enx, y + top, z + enz);

	t->vertex(x + esx, y + top, z + esz);
	t->vertex(x + esx, y, z + esz);
	t->vertex(x + wsx, y, z + wsz);
	t->vertex(x + wsx, y + top, z + wsz);

	t->vertex(x + enx, y + top, z + enz);
	t->vertex(x + enx, y, z + enz);
	t->vertex(x + esx, y, z + esz);
	t->vertex(x + esx, y + top, z + esz);

	t->vertex(x + wsx, y + top, z + wsz);
	t->vertex(x + wsx, y, z + wsz);
	t->vertex(x + wnx, y, z + wnz);
	t->vertex(x + wnx, y + top, z + wnz);

	t->end();
	glPopMatrix();

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glDepthMask(true);
}