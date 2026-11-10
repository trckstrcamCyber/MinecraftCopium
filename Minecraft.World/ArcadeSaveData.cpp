#include "stdafx.h"
#include "com.mojang.nbt.h"
#include "InputOutputStream.h"
#include "ArcadeSaveData.h"
#include <cmath>

static const wchar_t *const ARCADE_TAG = L"CopiumArcade";
static const int ARCADE_DATA_VERSION = 1;
static const double ARCADE_MAX_COOKIES = 1.0e300;
static const int ARCADE_MAX_UPGRADES = 1000000;
static const long long ARCADE_MAX_SCORE = 1000000000LL;

ArcadeSaveData::ArcadeSaveData()
{
	reset();
}

void ArcadeSaveData::reset()
{
	cookies = 0.0;
	for (int i = 0; i < UPGRADE_COUNT; ++i) upgrades[i] = 0;
	cookiesPerSecond = 0.0;
	clickValue = 1;
	goldenCooldownTicks = 20 * 38;
	goldenBoostTicks = 0;
	flappyHighScore = 0;
	snakeHighScore = 0;
}

void ArcadeSaveData::sanitize()
{
	if (!(cookies >= 0.0) || !std::isfinite(cookies)) cookies = 0.0;		// also catches NaN
	if (cookies > ARCADE_MAX_COOKIES) cookies = ARCADE_MAX_COOKIES;
	for (int i = 0; i < UPGRADE_COUNT; ++i)
	{
		if (upgrades[i] < 0) upgrades[i] = 0;
		if (upgrades[i] > ARCADE_MAX_UPGRADES) upgrades[i] = ARCADE_MAX_UPGRADES;
	}
	if (!(cookiesPerSecond >= 0.0) || !std::isfinite(cookiesPerSecond)) cookiesPerSecond = 0.0;
	if (clickValue < 1) clickValue = 1;
	if (goldenCooldownTicks < 0) goldenCooldownTicks = 0;
	if (goldenCooldownTicks > 20 * 600) goldenCooldownTicks = 20 * 600;
	if (goldenBoostTicks < 0) goldenBoostTicks = 0;
	if (goldenBoostTicks > 20 * 60) goldenBoostTicks = 20 * 60;
	if (flappyHighScore < 0) flappyHighScore = 0;
	if (flappyHighScore > ARCADE_MAX_SCORE) flappyHighScore = ARCADE_MAX_SCORE;
	if (snakeHighScore < 0) snakeHighScore = 0;
	if (snakeHighScore > ARCADE_MAX_SCORE) snakeHighScore = ARCADE_MAX_SCORE;
}

void ArcadeSaveData::save(CompoundTag *tag) const
{
	if (tag == nullptr) return;
	CompoundTag *arcade = new CompoundTag();
	arcade->putInt(L"Version", ARCADE_DATA_VERSION);
	arcade->putDouble(L"Cookies", cookies);
	arcade->putDouble(L"CookiesPerSecond", cookiesPerSecond);
	arcade->putInt(L"ClickValue", clickValue);
	arcade->putInt(L"GoldenCooldown", goldenCooldownTicks);
	arcade->putInt(L"GoldenBoost", goldenBoostTicks);
	for (int i = 0; i < UPGRADE_COUNT; ++i)
	{
		wchar_t name[16];
		swprintf(name, 16, L"Upgrade%d", i);
		arcade->putInt(name, upgrades[i]);
	}
	arcade->putLong(L"FlappyHigh", flappyHighScore);
	arcade->putLong(L"SnakeHigh", snakeHighScore);
	tag->put(ARCADE_TAG, arcade);
}

void ArcadeSaveData::load(CompoundTag *tag)
{
	reset();
	if (tag == nullptr || !tag->contains(ARCADE_TAG)) return;
	CompoundTag *arcade = tag->getCompound(ARCADE_TAG);
	if (arcade == nullptr) return;
	cookies = arcade->getDouble(L"Cookies");
	cookiesPerSecond = arcade->getDouble(L"CookiesPerSecond");
	clickValue = arcade->getInt(L"ClickValue");
	goldenCooldownTicks = arcade->contains(L"GoldenCooldown") ? arcade->getInt(L"GoldenCooldown") : goldenCooldownTicks;
	goldenBoostTicks = arcade->getInt(L"GoldenBoost");
	for (int i = 0; i < UPGRADE_COUNT; ++i)
	{
		wchar_t name[16];
		swprintf(name, 16, L"Upgrade%d", i);
		upgrades[i] = arcade->getInt(name);
	}
	flappyHighScore = arcade->getLong(L"FlappyHigh");
	snakeHighScore = arcade->getLong(L"SnakeHigh");
	sanitize();
}

void ArcadeSaveData::write(DataOutputStream *dos) const
{
	dos->writeInt(ARCADE_DATA_VERSION);
	dos->writeDouble(cookies);
	dos->writeDouble(cookiesPerSecond);
	dos->writeInt(clickValue);
	dos->writeInt(goldenCooldownTicks);
	dos->writeInt(goldenBoostTicks);
	for (int i = 0; i < UPGRADE_COUNT; ++i) dos->writeInt(upgrades[i]);
	dos->writeLong(flappyHighScore);
	dos->writeLong(snakeHighScore);
}

void ArcadeSaveData::read(DataInputStream *dis)
{
	reset();
	int version = dis->readInt();
	if (version != ARCADE_DATA_VERSION) return;		// unknown layout: keep defaults rather than mis-read it
	cookies = dis->readDouble();
	cookiesPerSecond = dis->readDouble();
	clickValue = dis->readInt();
	goldenCooldownTicks = dis->readInt();
	goldenBoostTicks = dis->readInt();
	for (int i = 0; i < UPGRADE_COUNT; ++i) upgrades[i] = dis->readInt();
	flappyHighScore = dis->readLong();
	snakeHighScore = dis->readLong();
	sanitize();
}
