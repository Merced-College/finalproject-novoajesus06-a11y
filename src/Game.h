#pragma once
/*
 * Author: [Your Name]
 * Class: Game
 */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <algorithm>

#include "Player.h"
#include "Level.h"
#include "Leaderboard.h"
#include <string>
#include <memory>

static const int WORLD_W = 1400;
static const int WORLD_H = 600;

class Game {
public:
    enum class State { MENU, PLAYING, PAUSED, DEAD, WIN, GAME_OVER };

    Game();
    ~Game();

    void update(float dt);
    void render(HDC hdc, int clientW, int clientH);

    void keyDown(int vk);
    void keyUp(int vk);

    void setPlayerName(const std::string& name);

    State getState()  const { return state; }
    bool  isRunning() const { return running; }

private:
    void loadLevel(int n);
    void drawRect(HDC hdc, float wx, float wy, float ww, float wh,
                  COLORREF color, float scaleX, float scaleY,
                  int offX, int offY);
    void drawString(HDC hdc, const std::string& text, int x, int y,
                    COLORREF color, int fontSize = 16);

    Player                  player;
    std::unique_ptr<Level>  level;
    Leaderboard             leaderboard;

    State       state;
    int         currentLevel;
    std::string playerName;

    std::string hudMessage;
    float       hudTimer;

    bool running;

    // Key state (kLeft/kRight/kJump to avoid name clash with keyUp() method)
    bool kLeft, kRight, kJump;

    HBITMAP hBuffer;
    HDC     hdcBuffer;
    int     bufW, bufH;
};
