/*
 * Author: [Your Name]
 * Class: Game
 *
 * Description:
 * Win32 GDI implementation. Uses a double-buffered offscreen bitmap to
 * draw colored rectangles for every game object, scaled to fit the window.
 * Input comes from WM_KEYDOWN / WM_KEYUP messages routed through keyDown()
 * and keyUp().
 */

#include "Game.h"
#include <string>
#include <sstream>

// ── Color palette ─────────────────────────────────────────────────────────────
#define COL_SKY        RGB( 30,  30,  60)
#define COL_GROUND     RGB( 80, 160,  80)
#define COL_PLATFORM   RGB(120,  80,  40)
#define COL_MOVING     RGB( 60, 180, 200)
#define COL_BREAKABLE  RGB(200, 140,  60)
#define COL_PLAYER     RGB( 80, 160, 255)
#define COL_ENEMY_W    RGB(220,  60,  60)
#define COL_ENEMY_J    RGB(220, 120,  20)
#define COL_COIN_B     RGB(180, 100,  30)
#define COL_COIN_S     RGB(180, 180, 200)
#define COL_COIN_G     RGB(255, 215,   0)
#define COL_WHITE      RGB(255, 255, 255)
#define COL_YELLOW     RGB(255, 230,   0)
#define COL_RED        RGB(255,  60,  60)
#define COL_GREEN      RGB( 60, 220,  60)

Game::Game()
    : player(100, 490),
      state(State::MENU),
      currentLevel(1),
      hudTimer(0),
      running(true),
      kLeft(false), kRight(false), kJump(false),
      hBuffer(nullptr), hdcBuffer(nullptr),
      bufW(0), bufH(0),
      playerName("Player")
{
    loadLevel(1);
}

Game::~Game() {
    if (hBuffer)   DeleteObject(hBuffer);
    if (hdcBuffer) DeleteDC(hdcBuffer);
}

// ── Input ─────────────────────────────────────────────────────────────────────

void Game::setPlayerName(const std::string& name) {
    playerName = name.empty() ? "Player" : name;
}

void Game::keyDown(int vk) {
    switch (state) {
    case State::MENU:
        state = State::PLAYING;
        break;

    case State::PLAYING:
        if (vk == VK_LEFT  || vk == 'A') kLeft  = true;
        if (vk == VK_RIGHT || vk == 'D') kRight = true;
        if (vk == VK_UP    || vk == 'W' || vk == VK_SPACE) {
            if (!kJump) { player.jump(); kJump = true; }
        }
        if (vk == VK_ESCAPE || vk == 'P') state = State::PAUSED;
        break;

    case State::PAUSED:
        if (vk == VK_ESCAPE || vk == 'P') state = State::PLAYING;
        break;

    case State::WIN:
        if (vk == VK_RETURN || vk == VK_SPACE) {
            currentLevel++;
            if (currentLevel > 3) {
                leaderboard.addScore(playerName, player.getScore(), 3);
                state = State::GAME_OVER;
            } else {
                loadLevel(currentLevel);
                state = State::PLAYING;
            }
        }
        if (vk == VK_ESCAPE) running = false;
        break;

    case State::DEAD:
        // DEAD = lost a life but still has lives left — retry current level
        if (vk == VK_RETURN || vk == VK_SPACE) {
            player.reset();
            player.setPosition(100, 490);
            loadLevel(currentLevel);
            state = State::PLAYING;
        }
        if (vk == VK_ESCAPE) running = false;
        break;

    case State::GAME_OVER:
        // GAME_OVER = all lives gone — show YOU DIED, space/enter restarts, escape quits
        if (vk == VK_RETURN || vk == VK_SPACE) {
            // Restart from level 1
            currentLevel = 1;
            player = Player(100, 490);
            loadLevel(1);
            state = State::PLAYING;
        }
        if (vk == VK_ESCAPE) running = false;
        break;
    }
}

void Game::keyUp(int vk) {
    if (vk == VK_LEFT  || vk == 'A') kLeft  = false;
    if (vk == VK_RIGHT || vk == 'D') kRight = false;
    if (vk == VK_UP    || vk == 'W' || vk == VK_SPACE) kJump = false;
}

// ── Update ────────────────────────────────────────────────────────────────────

void Game::update(float dt) {
    if (state != State::PLAYING) return;

    // Apply movement from held keys
    if (kLeft)  player.moveLeft();
    if (kRight) player.moveRight();
    if (!kLeft && !kRight) player.stopMoving();

    player.update(dt);
    level->update(player, dt);

    std::string evt = level->popEvent();
    if (!evt.empty()) { hudMessage = evt; hudTimer = 1.5f; }
    if (hudTimer > 0) hudTimer -= dt;

    if (level->playerFell(player)) {
        player.takeDamage(3);
        if (player.isDead()) {
            // All lives gone — save score and show YOU DIED screen
            leaderboard.addScore(playerName, player.getScore(), currentLevel);
            state = State::GAME_OVER;
        } else {
            // Still has lives — show retry screen
            player.setPosition(100, 490);
            state = State::DEAD;
        }
    }

    // Also check enemy damage killed player mid-level
    if (state == State::PLAYING && player.isDead()) {
        leaderboard.addScore(playerName, player.getScore(), currentLevel);
        state = State::GAME_OVER;
    }

    if (level->isComplete()) state = State::WIN;
}

// ── Rendering helpers ─────────────────────────────────────────────────────────

void Game::drawRect(HDC hdc, float wx, float wy, float ww, float wh,
                    COLORREF color, float scaleX, float scaleY,
                    int offX, int offY)
{
    int x = offX + (int)(wx * scaleX);
    int y = offY + (int)(wy * scaleY);
    int w = std::max(2, (int)(ww * scaleX));
    int h = std::max(2, (int)(wh * scaleY));

    HBRUSH brush = CreateSolidBrush(color);
    RECT   r     = { x, y, x + w, y + h };
    FillRect(hdc, &r, brush);
    DeleteObject(brush);
}

void Game::drawString(HDC hdc, const std::string& text, int x, int y,
                      COLORREF color, int fontSize)
{
    HFONT font = CreateFontA(fontSize, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                             CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                             DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    HFONT old = (HFONT)SelectObject(hdc, font);
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    TextOutA(hdc, x, y, text.c_str(), (int)text.size());
    SelectObject(hdc, old);
    DeleteObject(font);
}

// ── Main render ───────────────────────────────────────────────────────────────

/*
 * Algorithm: Double-Buffer GDI Render
 *
 * Steps:
 * 1. If the window size changed, recreate the offscreen bitmap.
 * 2. Fill the bitmap with the sky color.
 * 3. Draw all game objects by scaling world coords to screen coords.
 * 4. BitBlt the offscreen bitmap to the real window DC.
 *
 * Time Complexity: O(N) where N = total objects drawn.
 */
void Game::render(HDC hdc, int clientW, int clientH) {
    // Recreate buffer if window resized
    if (clientW != bufW || clientH != bufH) {
        if (hBuffer)   DeleteObject(hBuffer);
        if (hdcBuffer) DeleteDC(hdcBuffer);
        hdcBuffer = CreateCompatibleDC(hdc);
        hBuffer   = CreateCompatibleBitmap(hdc, clientW, clientH);
        SelectObject(hdcBuffer, hBuffer);
        bufW = clientW;
        bufH = clientH;
    }

    // Scale + letterbox: maintain aspect ratio
    float scaleX = (float)clientW / WORLD_W;
    float scaleY = (float)clientH / WORLD_H;
    float scale  = std::min(scaleX, scaleY);
    int   offX   = (int)((clientW - WORLD_W * scale) / 2);
    int   offY   = (int)((clientH - WORLD_H * scale) / 2);

    // Clear background
    {
        HBRUSH bg = CreateSolidBrush(COL_SKY);
        RECT   rc = {0, 0, clientW, clientH};
        FillRect(hdcBuffer, &rc, bg);
        DeleteObject(bg);
    }

    if (state == State::MENU) {
        // ── Menu screen ──────────────────────────────────────────────────────
        drawString(hdcBuffer, "PLATFORM  QUEST", clientW/2 - 140, clientH/2 - 120, COL_YELLOW, 36);
        drawString(hdcBuffer, "Player: " + playerName,            clientW/2 - 90,  clientH/2 - 60,  COL_WHITE, 18);
        drawString(hdcBuffer, "Arrow Keys / WASD  -  Move & Jump",clientW/2 - 150, clientH/2,       COL_WHITE, 16);
        drawString(hdcBuffer, "Collect ALL coins to complete each level!", clientW/2-190, clientH/2+30, RGB(180,255,180), 16);
        drawString(hdcBuffer, "Stomp enemies for bonus points!",  clientW/2 - 160, clientH/2 + 60,  RGB(255,180,180), 16);
        drawString(hdcBuffer, "Press  SPACE  or  ENTER  to start",clientW/2 - 150, clientH/2 + 110, COL_YELLOW, 18);

        // Show leaderboard entries
        const auto& entries = leaderboard.getEntries();
        if (!entries.empty()) {
            drawString(hdcBuffer, "-- TOP SCORES --", clientW/2 - 70, clientH/2 + 150, COL_YELLOW, 16);
            for (int i = 0; i < (int)entries.size() && i < 5; ++i) {
                std::string line = std::to_string(i+1) + ". " +
                                   entries[i].name + "   " +
                                   std::to_string(entries[i].score);
                drawString(hdcBuffer, line, clientW/2 - 60, clientH/2 + 175 + i * 22, COL_WHITE, 15);
            }
        }

    } else if (state == State::GAME_OVER) {
        // Big red YOU DIED text
        drawString(hdcBuffer, "YOU  DIED",            clientW/2-160, clientH/2-130, RGB(220,0,0),      72);
        drawString(hdcBuffer, "You ran out of lives.", clientW/2-140, clientH/2- 30, COL_WHITE,         20);
        drawString(hdcBuffer, "Final Score: " + std::to_string(player.getScore()),
                                                       clientW/2-110, clientH/2+  10, COL_YELLOW,       20);
        drawString(hdcBuffer, "Level Reached: " + std::to_string(currentLevel),
                                                       clientW/2-110, clientH/2+  45, COL_WHITE,        18);
        drawString(hdcBuffer, "SPACE / ENTER  -  Restart",  clientW/2-130, clientH/2+ 95, COL_GREEN,   17);
        drawString(hdcBuffer, "ESCAPE         -  Quit",     clientW/2-130, clientH/2+120, RGB(180,180,180), 17);

    } else {
        // ── Gameplay / Paused / Dead / Win ────────────────────────────────────

        auto drawW = [&](float wx, float wy, float ww, float wh, COLORREF c) {
            drawRect(hdcBuffer, wx, wy, ww, wh, c, scale, scale, offX, offY);
        };

        // Platforms
        for (const auto& p : level->getPlatforms()) {
            if (p.isBroken()) continue;
            COLORREF c = (p.getType() == Platform::Type::MOVING)    ? COL_MOVING    :
                         (p.getType() == Platform::Type::BREAKABLE) ? COL_BREAKABLE :
                                                                       COL_PLATFORM;
            drawW(p.getPos().x, p.getPos().y, p.getSize().x, p.getSize().y, c);
        }

        // Coins
        for (const auto& co : level->getCoins()) {
            if (co.isCollected()) continue;
            COLORREF c = (co.getType() == Coin::Type::GOLD)   ? COL_COIN_G :
                         (co.getType() == Coin::Type::SILVER) ? COL_COIN_S :
                                                                 COL_COIN_B;
            drawW(co.getPos().x, co.getPos().y, co.getSize().x, co.getSize().y, c);
        }

        // Enemies
        for (const auto& e : level->getEnemies()) {
            COLORREF c = (e.getType() == Enemy::Type::JUMPER) ? COL_ENEMY_J : COL_ENEMY_W;
            drawW(e.getPos().x, e.getPos().y, e.getSize().x, e.getSize().y, c);
            // Eyes
            float ex = e.getPos().x, ey = e.getPos().y;
            drawW(ex + 6,  ey + 6,  8, 8, COL_WHITE);
            drawW(ex + 18, ey + 6,  8, 8, COL_WHITE);
            drawW(ex + 9,  ey + 8,  4, 4, RGB(0,0,0));
            drawW(ex + 21, ey + 8,  4, 4, RGB(0,0,0));
        }

        // Player body
        Vec2 pp = player.getPos(), ps = player.getSize();
        drawW(pp.x, pp.y, ps.x, ps.y, COL_PLAYER);
        // Player face
        drawW(pp.x + 6,  pp.y + 8,  8, 8, COL_WHITE);
        drawW(pp.x + 18, pp.y + 8,  8, 8, COL_WHITE);
        drawW(pp.x + 9,  pp.y + 10, 4, 4, RGB(0,0,0));
        drawW(pp.x + 21, pp.y + 10, 4, 4, RGB(0,0,0));
        // Direction indicator
        if (player.isFacingRight())
            drawW(pp.x + ps.x - 4, pp.y + 16, 4, 6, COL_WHITE);
        else
            drawW(pp.x,             pp.y + 16, 4, 6, COL_WHITE);

        // ── HUD ───────────────────────────────────────────────────────────────
        std::string hud = "Level: " + std::to_string(currentLevel) +
                          "   Score: " + std::to_string(player.getScore()) +
                          "   HP: " + std::to_string(player.getHealth()) +
                          "   Lives: " + std::to_string(player.getLives());
        drawString(hdcBuffer, hud, 10, 8, COL_WHITE, 16);

        // Health bar
        int barX = 10, barY = 30, barW = 120, barH = 10;
        RECT barBg = { barX, barY, barX+barW, barY+barH };
        FillRect(hdcBuffer, &barBg, (HBRUSH)GetStockObject(BLACK_BRUSH));
        HBRUSH hpBrush = CreateSolidBrush(COL_GREEN);
        RECT   barFg   = { barX, barY, barX + (int)(barW * player.getHealth() / 3.0f), barY+barH };
        FillRect(hdcBuffer, &barFg, hpBrush);
        DeleteObject(hpBrush);

        // Popup event message
        if (hudTimer > 0 && !hudMessage.empty())
            drawString(hdcBuffer, hudMessage, clientW/2 - 40, 50, COL_YELLOW, 20);

        // Overlay messages
        if (state == State::PAUSED)
            drawString(hdcBuffer, "PAUSED  -  Press ESC to resume",
                       clientW/2 - 160, clientH/2, COL_YELLOW, 24);

        if (state == State::WIN) {
            drawString(hdcBuffer, "LEVEL COMPLETE!",
                       clientW/2 - 130, clientH/2 - 30, COL_GREEN, 28);
            drawString(hdcBuffer, "Press SPACE for next level",
                       clientW/2 - 140, clientH/2 + 20, COL_WHITE, 18);
        }

        if (state == State::DEAD) {
            drawString(hdcBuffer, "You fell!  Lives: " + std::to_string(player.getLives()),
                       clientW/2 - 120, clientH/2 - 20, COL_RED, 22);
            drawString(hdcBuffer, "Press SPACE to retry",
                       clientW/2 - 100, clientH/2 + 20, COL_WHITE, 18);
        }
    }

    // Flip buffer to screen
    BitBlt(hdc, 0, 0, clientW, clientH, hdcBuffer, 0, 0, SRCCOPY);
}

void Game::loadLevel(int n) {
    level = std::make_unique<Level>(n);
    player.setPosition(100, 490);
    player.reset();
}
