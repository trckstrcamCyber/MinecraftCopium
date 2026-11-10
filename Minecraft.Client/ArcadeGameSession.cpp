#include "stdafx.h"
#include "ArcadeGameSession.h"
#include "../Minecraft.World/Item.h"
#include "../Minecraft.World/Tile.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace
{
    const int kTicksPerSecond = 20;
    const int kCookieBaseCosts[4] = { 15, 100, 500, 2500 };
    const float kCookieRates[4] = { 0.2f, 1.0f, 5.0f, 50.0f };
    const wchar_t *kCookieNames[4] = { L"Wooden Pickaxe", L"Iron Pickaxe", L"Villager Farmer", L"Nether Quarry" };
    // Item-atlas/block item IDs reused from TU43; these are visuals only.
    const int kCookieIcons[4] = { Item::wooden_pickaxe_Id, Item::iron_pickaxe_Id, Item::wheat_Id, Tile::netherrack_Id };
    int randomInt(int minValue, int maxValue) { return minValue + (std::rand() % ((maxValue - minValue) + 1)); }
}

ArcadeGameSession::ArcadeGameSession()
{
    active = false;
    gameOver = false;
    saveRequested = false;
    autosaveTicks = 0;
    resetProgress();
    gameType = ARCADE_GAME_COOKIE_MINER;
    cursorX = 0.16f;
    cursorY = 0.50f;
    viewportWidth = 854.0f;
    viewportHeight = 480.0f;
}

void ArcadeGameSession::start(int type)
{
    gameType = (type >= ARCADE_GAME_COOKIE_MINER && type <= ARCADE_GAME_MINE_SNAKE) ? type : ARCADE_GAME_COOKIE_MINER;
    active = true;
    gameOver = false;
    score = 0;
    tickCounter = 0;
    axisRepeatTicks = 0;
    // Start with the pointer over Cookie Miner's cookie. A real controller can
    // still use the normal A/B + stick navigation without moving this cursor.
    cursorX = 0.16f;
    cursorY = 0.50f;
    // Cookies, upgrades and the golden apple timers are persistent now (see applySaveData), so a new
    // session no longer wipes them. Only the transient golden apple sprite is reset.
    selectedUpgrade = 0;
    goldenVisibleTicks = 0;
    goldenAppleVisible = false;
    goldenAppleX = -0.1f;
    birdY = 0.5f;
    birdVelocity = 0.0f;
    flappyTicks = 0;
    flappyGapHeight = 0.42f;
    obstacles.clear();
    flappyShieldTicks = 0;
    snake.clear();
    snakeDX = 1;
    snakeDY = 0;
    snakeStepTicks = 0;
    snakeStepDelay = 8;
    if (gameType == ARCADE_GAME_MINE_SNAKE)
    {
        snake.push_back({ 8, 7 });
        snake.push_back({ 7, 7 });
        snake.push_back({ 6, 7 });
        spawnSnakeFood();
    }
}

void ArcadeGameSession::stop()
{
    // Leaving a cabinet always saves what has been earned so far.
    if (active) saveRequested = true;
    active = false;
    gameOver = false;
}

void ArcadeGameSession::resetProgress()
{
    cookies = 0.0;
    for (int i = 0; i < 4; ++i) upgrades[i] = 0;
    clickMultiplierTicks = 0;
    goldenCooldownTicks = 20 * 38;
    flappyHighScore = 0;
    snakeHighScore = 0;
    selectedUpgrade = 0;
    goldenVisibleTicks = 0;
    goldenAppleVisible = false;
    goldenAppleX = -0.1f;
}

void ArcadeGameSession::noteScore()
{
    if (gameType == ARCADE_GAME_FLAPPY_CRAFT && score > flappyHighScore) flappyHighScore = score;
    else if (gameType == ARCADE_GAME_MINE_SNAKE && score > snakeHighScore) snakeHighScore = score;
}

void ArcadeGameSession::applySaveData(const ArcadeSaveData &data)
{
    cookies = data.cookies;
    for (int i = 0; i < 4 && i < ArcadeSaveData::UPGRADE_COUNT; ++i) upgrades[i] = data.upgrades[i];
    goldenCooldownTicks = data.goldenCooldownTicks;
    clickMultiplierTicks = data.goldenBoostTicks;
    flappyHighScore = data.flappyHighScore;
    snakeHighScore = data.snakeHighScore;
}

ArcadeSaveData ArcadeGameSession::getSaveData() const
{
    ArcadeSaveData data;
    data.cookies = cookies;
    for (int i = 0; i < 4 && i < ArcadeSaveData::UPGRADE_COUNT; ++i) data.upgrades[i] = upgrades[i];
    data.cookiesPerSecond = getCookiesPerSecond();
    data.clickValue = static_cast<int>(getClickYield());
    data.goldenCooldownTicks = goldenCooldownTicks;
    data.goldenBoostTicks = clickMultiplierTicks;
    data.flappyHighScore = flappyHighScore;
    data.snakeHighScore = snakeHighScore;
    return data;
}

void ArcadeGameSession::restartCurrentGame() { start(gameType); }

void ArcadeGameSession::tick(float axisX, float axisY, bool jumpPressed, bool cancelPressed)
{
    if (!active) return;
    if (cancelPressed) { stop(); return; }
    ++tickCounter;
    if (gameOver) return;
    const bool wasGameOver = gameOver;
    if (gameType == ARCADE_GAME_COOKIE_MINER)
    {
        // Autosave the cookie economy every five seconds while it is being played.
        if (++autosaveTicks >= 5 * kTicksPerSecond) { autosaveTicks = 0; saveRequested = true; }
        // Navigate the small upgrade shop with W/S or the left stick.
        if (axisRepeatTicks > 0) --axisRepeatTicks;
        if (axisRepeatTicks == 0)
        {
            if (axisY > 0.65f) { selectedUpgrade = (selectedUpgrade + 3) % 4; axisRepeatTicks = 7; }
            else if (axisY < -0.65f) { selectedUpgrade = (selectedUpgrade + 1) % 4; axisRepeatTicks = 7; }
        }
        tickCookieMiner();
    }
    else if (gameType == ARCADE_GAME_FLAPPY_CRAFT) tickFlappyCraft(jumpPressed);
    else tickMineSnake(axisX, axisY);
    if (gameOver && !wasGameOver) saveRequested = true;	// the run just ended: store the high score
}

double ArcadeGameSession::getCookiesPerSecond() const
{
    double rate = 0.0;
    for (int i = 0; i < 4; ++i) rate += upgrades[i] * kCookieRates[i];
    return rate;
}

int ArcadeGameSession::getUpgradeOwned(int i) const { return (i >= 0 && i < 4) ? upgrades[i] : 0; }
int ArcadeGameSession::getUpgradeCost(int i) const
{
    if (i < 0 || i >= 4) return 0;
    double cost = kCookieBaseCosts[i] * std::pow(1.15, upgrades[i]);
    if (cost > 2147480000.0) return 2147480000;
    return static_cast<int>(cost + 0.5);
}
float ArcadeGameSession::getUpgradeRate(int i) const { return (i >= 0 && i < 4) ? kCookieRates[i] : 0.0f; }
const wchar_t *ArcadeGameSession::getUpgradeName(int i) const { return (i >= 0 && i < 4) ? kCookieNames[i] : L"Unknown"; }
int ArcadeGameSession::getUpgradeIconId(int i) const { return (i >= 0 && i < 4) ? kCookieIcons[i] : Item::cookie_Id; }

void ArcadeGameSession::setViewportSize(float width, float height)
{
    if (width > 1.0f) viewportWidth = width;
    if (height > 1.0f) viewportHeight = height;
}

void ArcadeGameSession::moveCursorByPixels(float dx, float dy, float sourceWidth, float sourceHeight)
{
    if (!active || sourceWidth <= 1.0f || sourceHeight <= 1.0f) return;
    cursorX = (std::max)(0.005f, (std::min)(0.995f, cursorX + dx / sourceWidth));
    cursorY = (std::max)(0.005f, (std::min)(0.995f, cursorY + dy / sourceHeight));
}

void ArcadeGameSession::primaryActionAtCursor()
{
    if (!active) return;
    if (gameOver) { restartCurrentGame(); return; }
    if (gameType != ARCADE_GAME_COOKIE_MINER)
    {
        primaryAction();
        return;
    }

    const float px = cursorX * viewportWidth;
    const float py = cursorY * viewportHeight;
    const float third = (std::max)(1.0f, (viewportWidth - 24.0f) / 3.0f);
    const float xCookie = 8.0f;
    const float xShop = 8.0f + third * 2.0f + 8.0f;
    const float top = 30.0f;
    const float bottom = viewportHeight - 25.0f;

    // Golden Apple sprite floats along the top strip; it only triggers when clicked.
    if (goldenAppleVisible && py >= top && py <= top + 30.0f &&
        std::fabs(px - goldenAppleX * viewportWidth) <= 22.0f)
    {
        goldenAppleVisible = false;
        clickMultiplierTicks = 27 * kTicksPerSecond;
        goldenCooldownTicks = 20 * randomInt(45, 75);
        return;
    }

    // The left third is the cookie playfield. Ignore clicks outside the visible panel.
    if (px >= xCookie && px < xCookie + third && py >= top && py <= bottom)
    {
        cookies += getClickYield();
        return;
    }

    // Clicking an upgrade card selects and buys that exact row.
    if (px >= xShop && px <= viewportWidth - 8.0f && py >= top + 23.0f)
    {
        for (int u = 0; u < 4; ++u)
        {
            const float rowY = top + 25.0f + u * 42.0f;
            if (py >= rowY - 2.0f && py <= rowY + 37.0f)
            {
                selectedUpgrade = u;
                const int cost = getUpgradeCost(u);
                if (cookies >= cost)
                {
                    cookies -= cost;
                    ++upgrades[u];
                }
                return;
            }
        }
    }
}

void ArcadeGameSession::secondaryActionAtCursor()
{
    if (!active) return;
    if (gameOver) { restartCurrentGame(); return; }
    if (gameType != ARCADE_GAME_COOKIE_MINER)
    {
        secondaryAction();
        return;
    }

    const float px = cursorX * viewportWidth;
    const float py = cursorY * viewportHeight;
    const float third = (std::max)(1.0f, (viewportWidth - 24.0f) / 3.0f);
    const float xShop = 8.0f + third * 2.0f + 8.0f;
    const float top = 30.0f;
    if (px >= xShop && px <= viewportWidth - 8.0f && py >= top + 23.0f)
    {
        for (int u = 0; u < 4; ++u)
        {
            const float rowY = top + 25.0f + u * 42.0f;
            if (py >= rowY - 2.0f && py <= rowY + 37.0f)
            {
                selectedUpgrade = u;
                const int cost = getUpgradeCost(u);
                if (cookies >= cost)
                {
                    cookies -= cost;
                    ++upgrades[u];
                }
                return;
            }
        }
    }
    // Controller B / right-click away from a card buys the keyboard-selected upgrade.
    secondaryAction();
}

void ArcadeGameSession::tickCookieMiner()
{
    cookies += getCookiesPerSecond() / static_cast<double>(kTicksPerSecond);
    if (clickMultiplierTicks > 0) --clickMultiplierTicks;
    if (goldenAppleVisible)
    {
        goldenAppleX += 0.012f;
        if (goldenVisibleTicks > 0) --goldenVisibleTicks;
        if (goldenAppleX > 1.08f || goldenVisibleTicks <= 0)
        {
            goldenAppleVisible = false;
            goldenCooldownTicks = 20 * randomInt(35, 65);
        }
    }
    else if (goldenCooldownTicks > 0) --goldenCooldownTicks;
    else
    {
        goldenAppleVisible = true;
        goldenVisibleTicks = 20 * 13;
        goldenAppleX = -0.08f;
    }
}

void ArcadeGameSession::primaryAction()
{
    if (!active) return;
    if (gameOver) { restartCurrentGame(); return; }
    if (gameType == ARCADE_GAME_COOKIE_MINER)
    {
        if (goldenAppleVisible)
        {
            // The moving golden apple is an event target. The cabinet treats the primary
            // click as a hit while the event is on-screen, matching mouse/controller use.
            goldenAppleVisible = false;
            clickMultiplierTicks = 27 * kTicksPerSecond;
            goldenCooldownTicks = 20 * randomInt(45, 75);
        }
        else cookies += getClickYield();
    }
    else if (gameType == ARCADE_GAME_FLAPPY_CRAFT)
    {
        birdVelocity = -0.036f;
    }
}

void ArcadeGameSession::secondaryAction()
{
    if (!active) return;
    if (gameOver) { restartCurrentGame(); return; }
    if (gameType == ARCADE_GAME_COOKIE_MINER)
    {
        int cost = getUpgradeCost(selectedUpgrade);
        if (cookies >= cost)
        {
            cookies -= cost;
            ++upgrades[selectedUpgrade];
        }
    }
    else if (gameType == ARCADE_GAME_FLAPPY_CRAFT) birdVelocity = -0.036f;
}

void ArcadeGameSession::tickFlappyCraft(bool jumpPressed)
{
    if (flappyShieldTicks > 0) --flappyShieldTicks;
    if (jumpPressed) birdVelocity = -0.036f;
    birdVelocity += 0.0022f;
    birdY += birdVelocity;
    ++flappyTicks;
    flappyGapHeight = (std::max)(0.23f, 0.42f - static_cast<float>(score) * 0.006f);

    // Match the actual GUI playfield and ChickenModel's transformed bounds. The chicken
    // model spans local Y=9..24 and, when facing right, local Z=-8..3 which becomes X=-3..8.
    // Keeping these bounds in the same normalized space as obstacle positions avoids the
    // oversized invisible hitboxes that made the bird collide before touching a cactus.
    const float playW = (std::max)(1.0f, viewportWidth - 20.0f);
    const float playH = (std::max)(1.0f, viewportHeight - 68.0f);
    const float birdSize = (std::max)(18.0f, (std::min)(28.0f, viewportWidth * 0.045f));
    const float birdLeftExtent = (3.0f / 16.0f) * birdSize / playW;
    const float birdRightExtent = (8.0f / 16.0f) * birdSize / playW;
    const float birdTopExtent = (7.0f / 16.0f) * birdSize / playH;
    const float birdBottomExtent = (8.0f / 16.0f) * birdSize / playH;
    const int pipeWidthPixels = (std::max)(32, static_cast<int>(playW * 0.12f));
    const float pipeWidth = static_cast<float>(pipeWidthPixels) / playW;
    const float pipeOutline = 2.0f / playW;
    const float birdX = 0.24f;
    const float appleHalfWidth = 8.0f / playW;
    const float appleHalfHeight = 8.0f / playH;

    if (flappyTicks % 48 == 1)
    {
        Obstacle o;
        o.x = 1.08f;
        o.gapY = randomInt(28, 72) / 100.0f;
        o.gapHeight = flappyGapHeight;
        o.appleY = o.gapY + (randomInt(0, 1) ? 0.12f : -0.12f);
        o.appleY = (std::max)(0.08f, (std::min)(0.92f, o.appleY));
        o.hasApple = (score % 3 == 1);
        o.appleCollected = false;
        o.passed = false;
        o.shieldedCollision = false;
        obstacles.push_back(o);
    }

    for (size_t i = 0; i < obstacles.size(); ++i)
    {
        Obstacle &o = obstacles[i];
        o.x -= 0.018f;

        // The bonus icon is centered at the pipe's midpoint and at appleY. Test the
        // overlap of its 16x16 display rectangle against the chicken's visible bounds.
        const float appleX = o.x + pipeWidth * 0.5f;
        const bool overlapsAppleX = birdX + birdRightExtent > appleX - appleHalfWidth &&
                                    birdX - birdLeftExtent < appleX + appleHalfWidth;
        const bool overlapsAppleY = birdY + birdBottomExtent > o.appleY - appleHalfHeight &&
                                    birdY - birdTopExtent < o.appleY + appleHalfHeight;
        if (o.hasApple && !o.appleCollected && overlapsAppleX && overlapsAppleY)
        {
            o.appleCollected = true;
            flappyShieldTicks = 3 * kTicksPerSecond;
        }

        // Award the point only after the cactus tower and its dark outline have fully
        // cleared the chicken's trailing edge.
        if (!o.passed && o.x + pipeWidth + pipeOutline < birdX - birdLeftExtent)
        {
            o.passed = true;
            ++score;
            noteScore();
        }

        // Collide only when the visible chicken bounds overlap the actual rendered cactus
        // rectangle. The two-pixel dark outline is included in the horizontal bounds.
        const bool overlapsPipeX = o.x - pipeOutline < birdX + birdRightExtent &&
                                   o.x + pipeWidth + pipeOutline > birdX - birdLeftExtent;
        if (overlapsPipeX && !o.shieldedCollision)
        {
            const float gapTop = o.gapY - o.gapHeight * 0.5f;
            const float gapBottom = o.gapY + o.gapHeight * 0.5f;
            const bool hitsCactusVertically = birdY - birdTopExtent < gapTop ||
                                               birdY + birdBottomExtent > gapBottom;
            if (hitsCactusVertically)
            {
                if (flappyShieldTicks > 0)
                {
                    flappyShieldTicks = 0;
                    o.shieldedCollision = true; // consume one shield on this cactus tower
                }
                else gameOver = true;
            }
        }
    }

    while (!obstacles.empty() && obstacles.front().x < -0.2f) obstacles.erase(obstacles.begin());

    // World-edge collisions use the same chicken bounds as the cactus checks. If the
    // shield absorbs an edge hit, keep the complete chicken inside the playfield.
    if (birdY - birdTopExtent < 0.0f || birdY + birdBottomExtent > 1.0f)
    {
        if (flappyShieldTicks > 0)
        {
            birdY = (std::max)(birdTopExtent, (std::min)(1.0f - birdBottomExtent, birdY));
            birdVelocity = 0.0f;
            flappyShieldTicks = 0;
        }
        else gameOver = true;
    }
}

void ArcadeGameSession::setSnakeDirection(int dx, int dy)
{
    if (dx == -snakeDX && dy == -snakeDY) return; // do not reverse into the neck
    snakeDX = dx;
    snakeDY = dy;
}

void ArcadeGameSession::spawnSnakeFood()
{
    for (int tries = 0; tries < 100; ++tries)
    {
        Cell candidate = { randomInt(1, SNAKE_GRID_WIDTH - 2), randomInt(1, SNAKE_GRID_HEIGHT - 2) };
        bool occupied = false;
        for (size_t i = 0; i < snake.size(); ++i)
        {
            if (snake[i].x == candidate.x && snake[i].y == candidate.y) { occupied = true; break; }
        }
        if (!occupied) { food = candidate; return; }
    }
    food = { SNAKE_GRID_WIDTH / 2, SNAKE_GRID_HEIGHT / 2 };
}

void ArcadeGameSession::tickMineSnake(float axisX, float axisY)
{
    // Input::tick follows Minecraft's movement convention: +X is left and
    // negative Y is forward/up. Convert that convention to screen/grid directions.
    if (std::fabs(axisX) > 0.6f && std::fabs(axisX) >= std::fabs(axisY)) setSnakeDirection(axisX > 0 ? -1 : 1, 0);
    else if (std::fabs(axisY) > 0.6f) setSnakeDirection(0, axisY > 0 ? -1 : 1);

    if (--snakeStepTicks > 0) return;
    snakeStepTicks = snakeStepDelay;
    if (snake.empty()) return;
    Cell head = { snake[0].x + snakeDX, snake[0].y + snakeDY };
    if (head.x <= 0 || head.x >= SNAKE_GRID_WIDTH - 1 || head.y <= 0 || head.y >= SNAKE_GRID_HEIGHT - 1)
    {
        gameOver = true;
        return;
    }
    bool ate = (head.x == food.x && head.y == food.y);
    size_t collisionCount = ate ? snake.size() : snake.size() - 1;
    for (size_t i = 0; i < collisionCount; ++i)
    {
        if (snake[i].x == head.x && snake[i].y == head.y) { gameOver = true; return; }
    }
    snake.insert(snake.begin(), head);
    if (ate)
    {
        ++score;
        noteScore();
        snakeStepDelay = (std::max)(3, snakeStepDelay - 1);
        spawnSnakeFood();
    }
    else snake.pop_back();
}
