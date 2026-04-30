/*
 * Author: [Your Name]
 * Class: Level
 *
 * Description:
 * Builds level geometry for levels 1–3, processes all collisions,
 * removes defeated enemies and collected coins, and pushes HUD
 * event messages into the event queue.
 */

#include "Level.h"
#include <algorithm>

Level::Level(int n) : levelNum(n), complete(false) {
    buildLevel();
}

void Level::buildLevel() {
    platforms.clear();
    enemies.clear();
    coins.clear();

    // ── Level layout constants ──────────────────────────
    // Ground floor
    platforms.emplace_back(0,   550, 800, 50);
    platforms.emplace_back(900, 550, 800, 50);

    if (levelNum == 1) {
        platforms.emplace_back(150, 450,  120, 20);
        platforms.emplace_back(320, 370,  120, 20);
        platforms.emplace_back(510, 290,  120, 20);
        platforms.emplace_back(700, 370,  120, 20);
        platforms.emplace_back(860, 450,  120, 20);
        platforms.emplace_back(1020,350, 120, 20);
        platforms.emplace_back(1200,260, 120, 20);
        // Moving platform
        platforms.emplace_back(450, 200, 100, 20, Platform::Type::MOVING);

        enemies.emplace_back(160, 510, 100);
        enemies.emplace_back(700, 510, 120);
        enemies.emplace_back(1020,310, 80, Enemy::Type::JUMPER);

        // Scatter coins
        for (int i = 0; i < 5; ++i)
            coins.emplace_back(180 + i * 30.f, 430, Coin::Type::BRONZE);
        coins.emplace_back(370, 350, Coin::Type::SILVER);
        coins.emplace_back(560, 270, Coin::Type::GOLD);
        coins.emplace_back(1250,240, Coin::Type::GOLD);

    } else if (levelNum == 2) {
        // More complex layout
        platforms.emplace_back(100, 470, 100, 20);
        platforms.emplace_back(270, 390, 100, 20);
        platforms.emplace_back(440, 310, 100, 20);
        platforms.emplace_back(610, 230, 100, 20);
        platforms.emplace_back(780, 310, 100, 20);
        platforms.emplace_back(950, 390, 100, 20);
        platforms.emplace_back(1120,310, 100, 20);
        platforms.emplace_back(1290,230, 100, 20);
        platforms.emplace_back(450, 460, 80,  20, Platform::Type::MOVING);
        platforms.emplace_back(800, 180, 80,  20, Platform::Type::MOVING);
        // Breakable
        platforms.emplace_back(200, 300, 80, 20, Platform::Type::BREAKABLE);

        enemies.emplace_back(110, 430, 70);
        enemies.emplace_back(620, 190, 70, Enemy::Type::JUMPER);
        enemies.emplace_back(960, 350, 90);
        enemies.emplace_back(1300,190, 70, Enemy::Type::JUMPER);

        for (int i = 0; i < 4; ++i)
            coins.emplace_back(115 + i * 20.f, 450, Coin::Type::BRONZE);
        coins.emplace_back(460, 290, Coin::Type::SILVER);
        coins.emplace_back(630, 210, Coin::Type::GOLD);
        coins.emplace_back(1130,290, Coin::Type::SILVER);
        coins.emplace_back(1300,210, Coin::Type::GOLD);

    } else {  // Level 3 — hardest
        platforms.emplace_back(80,  490, 90, 20);
        platforms.emplace_back(230, 410, 90, 20);
        platforms.emplace_back(380, 330, 90, 20);
        platforms.emplace_back(530, 250, 90, 20);
        platforms.emplace_back(680, 170, 90, 20);
        platforms.emplace_back(830, 250, 90, 20);
        platforms.emplace_back(980, 170, 90, 20);
        platforms.emplace_back(1130,250, 90, 20);
        platforms.emplace_back(1280,170, 90, 20);
        platforms.emplace_back(350, 460, 70, 20, Platform::Type::MOVING);
        platforms.emplace_back(700, 380, 70, 20, Platform::Type::MOVING);
        platforms.emplace_back(1000,300, 70, 20, Platform::Type::MOVING);

        for (int i = 0; i < 5; ++i)
            enemies.emplace_back(90 + i * 240.f, 510, 80,
                                 i % 2 == 0 ? Enemy::Type::WALKER : Enemy::Type::JUMPER);

        for (int i = 0; i < 6; ++i)
            coins.emplace_back(95 + i * 230.f, 450, Coin::Type::BRONZE);
        coins.emplace_back(545, 230, Coin::Type::GOLD);
        coins.emplace_back(990, 150, Coin::Type::GOLD);
        coins.emplace_back(1290,150, Coin::Type::GOLD);
    }
}

void Level::update(Player& player, float dt) {
    // Update moving platforms
    for (auto& p : platforms) p.update(dt);

    // Player-platform collisions
    player.setOnGround(false);
    for (auto& p : platforms) {
        if (p.getType() == Platform::Type::BREAKABLE) {
            bool landed = p.resolveCollision(player);
            if (landed) {
                // const_cast to allow breaking (design choice)
                // In a real engine this would go through a command object
                const_cast<Platform&>(p).breakIt();
                eventQueue.push("Breakable platform!");
            }
        } else {
            p.resolveCollision(player);
        }
    }

    // Update enemies and handle player collision
    for (auto& e : enemies) {
        e.update(dt);
        if (e.checkPlayerCollision(player)) {
            eventQueue.push("+100 Stomp!");
        }
    }

    // Remove defeated enemies
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
        [](const Enemy& e){ return e.isDefeated(); }), enemies.end());

    // Update coins and handle collection
    for (auto& c : coins) {
        c.update(dt);
        if (c.checkCollect(player)) {
            std::string msg = "+";
            msg += std::to_string(static_cast<int>(c.getType()));
            eventQueue.push(msg);
        }
    }

    // Check level completion: all coins collected
    bool allGone = true;
    for (const auto& c : coins)
        if (!c.isCollected()) { allGone = false; break; }
    if (allGone) complete = true;
}

std::string Level::popEvent() {
    if (eventQueue.empty()) return "";
    std::string msg = eventQueue.front();
    eventQueue.pop();
    return msg;
}

bool Level::isComplete() const { return complete; }

bool Level::playerFell(const Player& player) const {
    return player.getPos().y > 700;
}

int Level::getLevelNumber() const { return levelNum; }

const std::vector<Platform>& Level::getPlatforms() const { return platforms; }
const std::vector<Enemy>&    Level::getEnemies()   const { return enemies; }
const std::vector<Coin>&     Level::getCoins()     const { return coins; }
