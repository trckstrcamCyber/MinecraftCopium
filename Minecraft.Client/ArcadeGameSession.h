#pragma once

#include "../Minecraft.World/ArcadeGameTypes.h"
#include "../Minecraft.World/ArcadeSaveData.h"
#include <vector>
#include <string>

// Simulation-only game state. One instance belongs to each LocalPlayer, not to Minecraft
// globally, so a second split-screen player can use another cabinet independently.
class ArcadeGameSession
{
public:
    struct Obstacle
    {
        float x;           // normalized horizontal position (0..1)
        float gapY;        // normalized gap centre (0..1)
        float gapHeight;   // normalized clear gap height
        float appleY;      // normalized bonus position
        bool hasApple;
        bool appleCollected;
        bool passed;
        bool shieldedCollision;
    };
    struct Cell { int x; int y; };

    ArcadeGameSession();
    void start(int type);
    void stop();
    bool isActive() const { return active; }
    bool isGameOver() const { return gameOver; }
    int getGameType() const { return gameType; }
    long long getScore() const { return score; }

    // Persistence (see ArcadeSaveData). Cookie Miner progress and both high scores outlive a single play session.
    void applySaveData(const ArcadeSaveData &data);
    ArcadeSaveData getSaveData() const;
    bool takeSaveRequest() { bool r = saveRequested; saveRequested = false; return r; }
    long long getFlappyHighScore() const { return flappyHighScore; }
    long long getSnakeHighScore() const { return snakeHighScore; }

    // Input is supplied by the owning LocalPlayer after the normal key/controller poll.
    void tick(float axisX, float axisY, bool jumpPressed, bool cancelPressed);
    void primaryAction();
    void secondaryAction();
    // Mouse support uses a per-player virtual cursor while the world mouse remains captured.
    void setViewportSize(float width, float height);
    void moveCursorByPixels(float dx, float dy, float sourceWidth, float sourceHeight);
    float getCursorX() const { return cursorX; } // normalized 0..1
    float getCursorY() const { return cursorY; } // normalized 0..1
    void primaryActionAtCursor();
    void secondaryActionAtCursor();

    // Cookie Miner display/state.
    double getCookies() const { return cookies; }
    double getCookiesPerSecond() const;
    double getClickYield() const { return clickMultiplierTicks > 0 ? 7.0 : 1.0; }
    bool isGoldenAppleVisible() const { return goldenAppleVisible; }
    float getGoldenAppleX() const { return goldenAppleX; }
    int getGoldenBoostTicks() const { return clickMultiplierTicks; }
    int getSelectedUpgrade() const { return selectedUpgrade; }
    int getUpgradeOwned(int i) const;
    int getUpgradeCost(int i) const;
    float getUpgradeRate(int i) const;
    const wchar_t *getUpgradeName(int i) const;
    int getUpgradeIconId(int i) const;

    // Flappy Craft display/state.
    float getBirdY() const { return birdY; }
    const std::vector<Obstacle> &getObstacles() const { return obstacles; }
    bool hasFlappyShield() const { return flappyShieldTicks > 0; }
    int getFlappyShieldTicks() const { return flappyShieldTicks; }

    // MineSnake display/state (a 20 x 14 grid including the obsidian wall ring).
    static const int SNAKE_GRID_WIDTH = 20;
    static const int SNAKE_GRID_HEIGHT = 14;
    const std::vector<Cell> &getSnake() const { return snake; }
    Cell getFood() const { return food; }

private:
    void tickCookieMiner();
    void tickFlappyCraft(bool jumpPressed);
    void tickMineSnake(float axisX, float axisY);
    void setSnakeDirection(int dx, int dy);
    void spawnSnakeFood();
    void restartCurrentGame();
    void resetProgress();
    void noteScore();
    int gameType;
    bool active;
    bool gameOver;
    long long score;
    int tickCounter;

    // Persisted state (everything below up to "birdY" except the transient golden apple sprite)
    bool saveRequested;
    int autosaveTicks;
    long long flappyHighScore;
    long long snakeHighScore;

    double cookies;
    int upgrades[4];
    int selectedUpgrade;
    int clickMultiplierTicks;
    int goldenCooldownTicks;
    int goldenVisibleTicks;
    bool goldenAppleVisible;
    float goldenAppleX;
    int axisRepeatTicks;

    float birdY;
    float birdVelocity;
    int flappyTicks;
    float flappyGapHeight;
    std::vector<Obstacle> obstacles;
    int flappyShieldTicks;

    std::vector<Cell> snake;
    Cell food;
    int snakeDX, snakeDY;
    int snakeStepTicks;
    int snakeStepDelay;

    // Input and layout state is stored on the session so split-screen players remain isolated.
    float cursorX;
    float cursorY;
    float viewportWidth;
    float viewportHeight;
};
