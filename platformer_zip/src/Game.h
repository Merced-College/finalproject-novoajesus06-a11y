#pragma once
#include "Player.h"
#include "Level.h"
#include "Leaderboard.h"
#include <string>
#include <memory>

/*
 * Author: [Jesus Novoa vasquez]
 * Class: Game
 *
 * Description:
 * Top-level game controller. Manages the game state machine,
 * the current level, the player, and the leaderboard.
 * Handles the main game loop, input processing (terminal-based),
 * and ASCII rendering.
 */

class Game {
public:
    enum class State { MENU, PLAYING, PAUSED, DEAD, WIN, GAME_OVER };

    Game();
    void run();

private:
    void handleInput(char key);
    void update(float dt);
    void render();
    void renderMenu();
    void renderHUD();
    void renderGameOver();
    void loadLevel(int n);
    void clearScreen();

    Player                  player;
    std::unique_ptr<Level>  level;
    Leaderboard             leaderboard;

    State       state;
    int         currentLevel;
    std::string playerName;

    // HUD popup message system
    std::string hudMessage;
    float       hudTimer;

    // Simple timing
    float       timeAccum;
    bool        running;
};
