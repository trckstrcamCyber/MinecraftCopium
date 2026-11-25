#include "stdafx.h"
#include "Random.h"
#include "JavaMath.h"
#include "Mth.h"
#include "net.minecraft.world.entity.player.h"
#include "net.minecraft.world.entity.projectile.h"
#include "net.minecraft.world.item.h"
#include "net.minecraft.world.item.enchantment.h"
#include "net.minecraft.world.level.h"
#include "net.minecraft.world.level.tile.h"
#include "net.minecraft.world.entity.h"
#include "SoundTypes.h"
#include "IconRegister.h"
#include "HtmlString.h"
#include "com.mojang.nbt.h"
#include "FireworksRocketEntity.h"
#include "CrossbowItem.h"

CrossbowItem::CrossbowItem(int id, bool loaded, bool firework) : Item(id)
{
	this->loaded = loaded;
	this->firework = firework;
	maxStackSize = 1;
	pullingIcons[0] = pullingIcons[1] = pullingIcons[2] = nullptr;
}

void CrossbowItem::registerIcons(IconRegister *iconRegister)
{
	Item::registerIcons(iconRegister);
	pullingIcons[0] = iconRegister->registerIcon(L"crossbow_pulling_0");
	pullingIcons[1] = iconRegister->registerIcon(L"crossbow_pulling_1");
	pullingIcons[2] = iconRegister->registerIcon(L"crossbow_pulling_2");
}

Icon *CrossbowItem::getDrawnIcon(int amount)
{
	return pullingIcons[amount];
}

int CrossbowItem::getUseDuration(shared_ptr<ItemInstance> itemInstance)
{
	return 20 * 60 * 60;
}

UseAnim CrossbowItem::getUseAnimation(shared_ptr<ItemInstance> itemInstance)
{
	return UseAnim_none;	// not UseAnim_bow: the bow's "pull back" arm/hand transforms make a crossbow look wrong
}

int CrossbowItem::getEnchantmentValue()
{
	return 1;
}

shared_ptr<ItemInstance> CrossbowItem::useTimeDepleted(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	return instance;
}

// ---- helpers -------------------------------------------------------------------------------------------------------------

bool CrossbowItem::isCrossbow(shared_ptr<ItemInstance> item)
{
	return item != nullptr && item->getItem() != nullptr && dynamic_cast<CrossbowItem *>(item->getItem()) != nullptr;
}

// Besides its own four, a crossbow takes Looting (it is a ranged weapon), Unbreaking, Mending and the Curse of Vanishing.
// Power, Punch, Flame, Infinity, Sharpness, Efficiency ... are all refused.
bool CrossbowItem::isEnchantmentAllowed(Enchantment *e)
{
	if (e == nullptr) return false;
	return e == Enchantment::quickCharge || e == Enchantment::multishot || e == Enchantment::piercing || e == Enchantment::torched
		|| e == Enchantment::lootBonus || e == Enchantment::digDurability || e == Enchantment::mending || e == Enchantment::vanishingCurse;
}

int CrossbowItem::getChargeTicks(shared_ptr<ItemInstance> item)
{
	int level = (item != nullptr) ? EnchantmentHelper::getEnchantmentLevel(Enchantment::quickCharge->id, item) : 0;
	int ticks = CHARGE_TICKS - level * QUICK_CHARGE_TICKS_PER_LEVEL;
	return ticks < 1 ? 1 : ticks;
}

int CrossbowItem::getAmmoId(shared_ptr<ItemInstance> item)
{
	if (item != nullptr && EnchantmentHelper::getEnchantmentLevel(Enchantment::torched->id, item) > 0) return Tile::torch_Id;
	return Item::arrow_Id;
}

shared_ptr<ItemInstance> CrossbowItem::withItem(shared_ptr<ItemInstance> from, int newId)
{
	shared_ptr<ItemInstance> stack = from->copy();	// copy() clones the tag, so the enchantments and the custom name survive
	stack->id = newId;
	return stack;
}

// One arrow shot from the player's eyes, turned `yawOffset` degrees to the side. Power 2.1 is speed 3.15 in this engine's units.
static shared_ptr<Arrow> createCrossbowArrow(Level *level, shared_ptr<Player> player, float yawOffset)
{
	shared_ptr<Arrow> arrow = std::make_shared<Arrow>(level, player, 2.1f);
	if (yawOffset != 0.0f)
	{
		float yRot = player->yRot + yawOffset;
		float xRot = player->xRot;
		double xd = -Mth::sin(yRot / 180 * PI) * Mth::cos(xRot / 180 * PI);
		double zd = Mth::cos(yRot / 180 * PI) * Mth::cos(xRot / 180 * PI);
		double yd = -Mth::sin(xRot / 180 * PI);
		arrow->shoot(xd, yd, zd, 2.1f * 1.5f, 1.0f);
	}
	arrow->setCritArrow(true);
	return arrow;
}

// A firework rocket leaving the player's eyes, turned `yawOffset` degrees to the side, at speed 1.6.
static shared_ptr<FireworksRocketEntity> createCrossbowRocket(Level *level, shared_ptr<Player> player, shared_ptr<ItemInstance> rocketItem, float yawOffset)
{
	float yRot = player->yRot + yawOffset;
	float xRot = player->xRot;
	double xd = -Mth::sin(yRot / 180 * PI) * Mth::cos(xRot / 180 * PI);
	double zd = Mth::cos(yRot / 180 * PI) * Mth::cos(xRot / 180 * PI);
	double yd = -Mth::sin(xRot / 180 * PI);

	// the same spot the arrows start from: at the eyes, a little to the side and down
	double px = player->x - Mth::cos(player->yRot / 180 * PI) * 0.16;
	double py = player->y + player->getHeadHeight() - 0.1;
	double pz = player->z - Mth::sin(player->yRot / 180 * PI) * 0.16;

	const double speed = 1.6;
	return std::make_shared<FireworksRocketEntity>(level, rocketItem, player, px, py, pz, xd * speed, yd * speed, zd * speed);
}

// Everything the crossbow holds leaves it (server side only).
void CrossbowItem::shootLoaded(shared_ptr<ItemInstance> weapon, Level *level, shared_ptr<Player> player)
{
	const int multishot = EnchantmentHelper::getEnchantmentLevel(Enchantment::multishot->id, weapon);
	const int piercing = EnchantmentHelper::getEnchantmentLevel(Enchantment::piercing->id, weapon);
	const int looting = EnchantmentHelper::getEnchantmentLevel(Enchantment::lootBonus->id, weapon);
	const bool torch = EnchantmentHelper::getEnchantmentLevel(Enchantment::torched->id, weapon) > 0;
	const bool creative = player->abilities.instabuild;
	const float angles[3] = { 0.0f, -static_cast<float>(MULTISHOT_SPREAD_DEGREES), static_cast<float>(MULTISHOT_SPREAD_DEGREES) };

	if (weapon->id == Item::crossbow_loaded_firework_Id)
	{
		// the rocket it was loaded with, kept in the crossbow's tag
		shared_ptr<ItemInstance> rocketItem;
		if (weapon->hasTag())
		{
			CompoundTag *rocketTag = weapon->getTag()->getCompound(L"ChargedFirework");
			if (rocketTag != nullptr) rocketItem = ItemInstance::fromTag(rocketTag);
		}
		if (rocketItem == nullptr) rocketItem = std::make_shared<ItemInstance>(Item::fireworks);

		const int rockets = (multishot > 0) ? 3 : 1;
		for (int i = 0; i < rockets; i++)
		{
			level->addEntity(createCrossbowRocket(level, player, rocketItem->copy(), angles[i]));
		}
		return;
	}

	const int count = (multishot > 0 && !torch) ? 3 : 1;

	// The arrow this crossbow was loaded with, kept in its tag (same
	// pattern as the ChargedFirework rocket). A tipped arrow keeps its
	// potion value, a spectral arrow its type; a plain arrow — or no
	// stored arrow (Creative load with no ammo) — shoots a plain arrow,
	// as before. Multishot fires three of the loaded type for one arrow.
	shared_ptr<ItemInstance> chargedArrow;
	if (weapon->hasTag())
	{
		CompoundTag *arrowTag = weapon->getTag()->getCompound(L"ChargedArrow");
		if (arrowTag != nullptr) chargedArrow = ItemInstance::fromTag(arrowTag);
	}

	for (int i = 0; i < count; i++)
	{
		shared_ptr<Arrow> arrow = createCrossbowArrow(level, player, angles[i]);
		if (chargedArrow != nullptr) arrow->setFromItem(chargedArrow);
		if (piercing > 0) arrow->setPierceLevel(piercing);

		// the arrow remembers the Looting of the crossbow it came from, so swapping weapons before it lands changes nothing
		arrow->setLootingLevel(looting);

		if (torch)
		{
			arrow->setTorchArrow(true);
			arrow->setOnFire(100);
			arrow->pickup = Arrow::PICKUP_DISALLOWED;
		}
		else if (creative || i > 0)
		{
			arrow->pickup = Arrow::PICKUP_CREATIVE_ONLY;	// the two extra Multishot arrows cannot be picked up in Survival
		}
		level->addEntity(arrow);
	}
}

// The first thing in the inventory (hotbar first) this crossbow can load: any arrow — plain, spectral or tipped (Arrow::isArrowItem, the
// same predicate the bow uses) — or a firework rocket, or for a Torched crossbow a torch.
int CrossbowItem::findAmmoSlot(shared_ptr<Player> player, shared_ptr<ItemInstance> crossbow)
{
	const bool torched = getAmmoId(crossbow) == Tile::torch_Id;
	for (unsigned int i = 0; i < player->inventory->items.length; i++)	// the 36 item slots, not the armour slots after them
	{
		shared_ptr<ItemInstance> stack = player->inventory->getItem(i);
		if (stack == nullptr || stack->count <= 0) continue;
		if (torched)
		{
			if (stack->id == Tile::torch_Id) return static_cast<int>(i);
		}
		else if (Arrow::isArrowItem(stack->id) || stack->id == Item::fireworks_Id)
		{
			return static_cast<int>(i);
		}
	}
	return -1;
}

// ---- use ------------------------------------------------------------------------------------------------------------------

shared_ptr<ItemInstance> CrossbowItem::use(shared_ptr<ItemInstance> instance, Level *level, shared_ptr<Player> player)
{
	if (loaded)
	{
		// fire: always a critical arrow at 3.15 speed (or a rocket at 1.6), then the crossbow is empty again
		if (!level->isClientSide)
		{
			shootLoaded(instance, level, player);
		}
		level->playEntitySound(player, eSoundType_ITEM_CROSSBOW_SHOOT, 1.0f, 1.0f / (player->getRandom()->nextFloat() * 0.5f + 1.8f) + 0.45f);
		if (EnchantmentHelper::getEnchantmentLevel(Enchantment::multishot->id, instance) > 0 && EnchantmentHelper::getEnchantmentLevel(Enchantment::torched->id, instance) == 0)
		{
			// the two side shots each get their own shot sound, one higher and one lower (Java)
			level->playEntitySound(player, eSoundType_ITEM_CROSSBOW_SHOOT, 1.0f, 1.0f / (player->getRandom()->nextFloat() * 0.5f + 1.8f) + 0.63f);
			level->playEntitySound(player, eSoundType_ITEM_CROSSBOW_SHOOT, 1.0f, 1.0f / (player->getRandom()->nextFloat() * 0.5f + 1.8f) + 0.43f);
		}

		shared_ptr<ItemInstance> empty = withItem(instance, Item::crossbow_Id);
		if (empty->hasTag())
		{
			empty->getTag()->remove(L"ChargedFirework");
			empty->getTag()->remove(L"ChargedArrow");
		}
		empty->hurtAndBreak(firework ? 3 : 1, player);	// a rocket costs 3 durability
		return empty;
	}

	if (player->abilities.instabuild || findAmmoSlot(player, instance) >= 0)
	{
		player->startUsingItem(instance, getUseDuration(instance));
		level->playEntitySound(player, eSoundType_ITEM_CROSSBOW_LOADING_START, 0.5f, 1.0f);
	}
	return instance;
}

// released after a full draw: take the ammunition and swap to the loaded crossbow (a rocket is kept in the tag)
void CrossbowItem::releaseUsing(shared_ptr<ItemInstance> itemInstance, Level *level, shared_ptr<Player> player, int durationLeft)
{
	if (loaded) return;
	int held = getUseDuration(itemInstance) - durationLeft;
	if (held < getChargeTicks(itemInstance)) return;

	const bool creative = player->abilities.instabuild;
	int slot = findAmmoSlot(player, itemInstance);
	if (slot < 0 && !creative) return;

	bool rocket = false;
	shared_ptr<ItemInstance> rocketItem;
	shared_ptr<ItemInstance> arrowItem;
	if (slot >= 0)
	{
		shared_ptr<ItemInstance> stack = player->inventory->getItem(slot);
		if (stack != nullptr && stack->id == Item::fireworks_Id)
		{
			rocket = true;
			rocketItem = stack->copy();
			rocketItem->count = 1;
		}
		else if (stack != nullptr && Arrow::isArrowItem(stack->id))
		{
			// The loaded arrow is kept in the crossbow's tag (same
			// pattern as the ChargedFirework rocket), so shootLoaded
			// can fire the same type — tipped arrows keep their potion
			// value, spectral arrows their type. Only the count is
			// forced to 1: one arrow is consumed, whatever the stack.
			arrowItem = stack->copy();
			arrowItem->count = 1;
		}
		if (!creative) player->inventory->removeItem(slot, 1);
	}

	level->playEntitySound(player, eSoundType_ITEM_CROSSBOW_LOADING_END, 1.0f, 1.0f);

	shared_ptr<ItemInstance> loadedStack = withItem(itemInstance, rocket ? Item::crossbow_loaded_firework_Id : Item::crossbow_loaded_Id);
	if (rocket)
	{
		if (!loadedStack->hasTag()) loadedStack->setTag(new CompoundTag());
		loadedStack->getTag()->putCompound(L"ChargedFirework", rocketItem->save(new CompoundTag()));
	}
	if (arrowItem != nullptr)
	{
		if (!loadedStack->hasTag()) loadedStack->setTag(new CompoundTag());
		loadedStack->getTag()->putCompound(L"ChargedArrow", arrowItem->save(new CompoundTag()));
	}
	else if (loadedStack->hasTag())
	{
		// A plain-arrow load — or a Creative load with no ammo — must
		// not inherit a stale ChargedArrow from the previous load:
		// withItem copies the tag, so clear it here. (use() clears it
		// again when the crossbow is fired.)
		loadedStack->getTag()->remove(L"ChargedArrow");
	}
	player->inventory->setItem(player->inventory->selected, loadedStack);
}

// ---------------------------------------------------------------------------------------------------------------------------
// Crossbow enchantments. Vanilla values: Quick Charge uncommon, Multishot rare, Piercing common.
// ---------------------------------------------------------------------------------------------------------------------------

// ---- Quick Charge ----------------------------------------------------------------------------------------------------------
QuickChargeEnchantment::QuickChargeEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::crossbow)
{
	// Name comes from the getFullname() override below instead of a descriptionId - see AdrenalineEnchantment.cpp for why.
}

int QuickChargeEnchantment::getMinCost(int level)
{
	return 12 + (level - 1) * 20;
}

int QuickChargeEnchantment::getMaxCost(int level)
{
	return 50;
}

int QuickChargeEnchantment::getMaxLevel()
{
	return 3;
}

HtmlString QuickChargeEnchantment::getFullname(int level)
{
	wchar_t formatted[64];
	swprintf(formatted, 64, L"Quick Charge%ls", getLevelSuffix(level).c_str());
	return HtmlString(formatted, eHTMLColor_f);
}

// ---- Multishot -------------------------------------------------------------------------------------------------------------
MultishotEnchantment::MultishotEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::crossbow)
{
}

int MultishotEnchantment::getMinCost(int level)
{
	return 20;
}

int MultishotEnchantment::getMaxCost(int level)
{
	return 50;
}

int MultishotEnchantment::getMaxLevel()
{
	return 1;
}

bool MultishotEnchantment::isCompatibleWith(Enchantment *other) const
{
	if (other == this) return false;
	return other != Enchantment::piercing && other != Enchantment::torched;
}

HtmlString MultishotEnchantment::getFullname(int level)
{
	return HtmlString(L"Multishot", eHTMLColor_f);
}

// ---- Piercing --------------------------------------------------------------------------------------------------------------
PiercingEnchantment::PiercingEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::crossbow)
{
}

int PiercingEnchantment::getMinCost(int level)
{
	return 1 + (level - 1) * 10;
}

int PiercingEnchantment::getMaxCost(int level)
{
	return 50;
}

int PiercingEnchantment::getMaxLevel()
{
	return 4;
}

bool PiercingEnchantment::isCompatibleWith(Enchantment *other) const
{
	if (other == this) return false;
	return other != Enchantment::multishot;
}

HtmlString PiercingEnchantment::getFullname(int level)
{
	wchar_t formatted[64];
	swprintf(formatted, 64, L"Piercing%ls", getLevelSuffix(level).c_str());
	return HtmlString(formatted, eHTMLColor_f);
}

// ---- Torched ---------------------------------------------------------------------------------------------------------------
TorchedEnchantment::TorchedEnchantment(int id, int frequency) : Enchantment(id, frequency, EnchantmentCategory::crossbow)
{
}

int TorchedEnchantment::getMinCost(int level)
{
	return 20;
}

int TorchedEnchantment::getMaxCost(int level)
{
	return 50;
}

int TorchedEnchantment::getMaxLevel()
{
	return 1;
}

bool TorchedEnchantment::isCompatibleWith(Enchantment *other) const
{
	if (other == this) return false;
	return other != Enchantment::multishot;
}

HtmlString TorchedEnchantment::getFullname(int level)
{
	return HtmlString(L"Torched", eHTMLColor_f);
}
