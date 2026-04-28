#pragma once
#include "Platform.h"
#include "Enemy.h"
#include "Coin.h"
#include <vector>
#include <queue>
#include <string>

/*
 * Author: [Jesus Novoa vasquez]
 * Class: Level
 *
 * Description:
 * Manages all game objects in a single level: platforms, enemies,
 * and coins. Handles batch updates and collision processing.
 *
 * Data Structures used here:
 *   - std::vector<Platform> : random-access list of platforms
 *   - std::vector<Enemy>    : list of enemies
 *   - std::vector<Coin>     : list of collectible coins
 *   - std::queue<std::string>: event message queue for HUD popups
 */

class Level {
public:
    Level(int levelNumber);

    void update(Player& player, float dt);

    // Accessors for rendering
    const std::vector<Platform>& getPlatforms() const;
    const std::vector<Enemy>&    getEnemies()   const;
    const std::vector<Coin>&     getCoins()     const;

    bool isComplete() const;   // true when all coins are collected
    bool playerFell(const Player& player) const;

    // Pop next HUD event message (empty string if none)
    std::string popEvent();

    int getLevelNumber() const;

private:
    void buildLevel();    // construct platforms/enemies/coins for this level

    int  levelNum;
    bool complete;

    std::vector<Platform>    platforms;
    std::vector<Enemy>       enemies;
    std::vector<Coin>        coins;

    // Data Structure: queue for event messages shown to the player
    std::queue<std::string>  eventQueue;
};
