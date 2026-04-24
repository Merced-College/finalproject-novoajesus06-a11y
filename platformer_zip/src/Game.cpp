/*
 * Author: [Your Name]
 * Class: Game
 *
 * Description:
 * Implements the game loop using fixed-timestep simulation,
 * a terminal-based input model, and a 2D ASCII grid renderer
 * that maps world coordinates to screen columns/rows.
 */

#include "Game.h"
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>
#include <cstring>

#ifdef _WIN32
  #include <conio.h>
  #define CLEAR "cls"
#else
  #include <termios.h>
  #include <unistd.h>
  #include <fcntl.h>
  #define CLEAR "clear"
#endif

// ── Terminal helpers ──────────────────────────────────────────────────────────

#ifndef _WIN32
static struct termios orig_termios;

static void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN]  = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

static void disableRawMode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

static char kbhit_read() {
    char c = 0;
    (void)read(STDIN_FILENO, &c, 1);
    return c;
}
#endif

// ── Screen dimensions ─────────────────────────────────────────────────────────
static const int SCREEN_W = 80;
static const int SCREEN_H = 24;

// World → screen scale
static const float SCALE_X = SCREEN_W  / 1400.f;
static const float SCALE_Y = SCREEN_H  /  600.f;

static int toScreenX(float wx) { return (int)(wx * SCALE_X); }
static int toScreenY(float wy) { return (int)(wy * SCALE_Y); }

// ── Game ──────────────────────────────────────────────────────────────────────

Game::Game()
    : player(100, 490),
      state(State::MENU),
      currentLevel(1),
      hudTimer(0),
      timeAccum(0),
      running(true)
{}

void Game::run() {
#ifndef _WIN32
    enableRawMode();
#endif

    // Ask for name at startup
    clearScreen();
    std::cout << "╔══════════════════════════════╗\n";
    std::cout << "║     PLATFORM QUEST  v1.0     ║\n";
    std::cout << "╚══════════════════════════════╝\n";
    std::cout << "\nEnter your name: ";
#ifndef _WIN32
    disableRawMode();
#endif
    std::getline(std::cin, playerName);
    if (playerName.empty()) playerName = "Player";
#ifndef _WIN32
    enableRawMode();
#endif

    loadLevel(1);
    state = State::MENU;

    auto prev = std::chrono::steady_clock::now();

    while (running) {
        auto  now  = std::chrono::steady_clock::now();
        float dt   = std::chrono::duration<float>(now - prev).count();
        prev = now;
        if (dt > 0.05f) dt = 0.05f;  // clamp

#ifdef _WIN32
        if (_kbhit()) handleInput((char)_getch());
#else
        char c = kbhit_read();
        if (c) handleInput(c);
#endif
        update(dt);
        render();

        // ~30 FPS
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }

#ifndef _WIN32
    disableRawMode();
#endif
    clearScreen();
    leaderboard.display();
}

void Game::handleInput(char key) {
    switch (state) {
        case State::MENU:
            if (key == '\n' || key == '\r' || key == ' ') {
                state = State::PLAYING;
            }
            if (key == 'q' || key == 'Q') running = false;
            break;

        case State::PLAYING:
            if (key == 'a' || key == 'A') player.moveLeft();
            if (key == 'd' || key == 'D') player.moveRight();
            if (key == 'w' || key == 'W' || key == ' ') player.jump();
            if (key == 'p' || key == 'P') state = State::PAUSED;
            if (key == 'q' || key == 'Q') running = false;
            // Stop on key release isn't possible in raw terminal — movement
            // is toggled on keypress for demo; full version uses ncurses.
            // For this version pressing the same key again stops movement:
            {
                static char lastKey = 0;
                if ((key == 'a' || key == 'A') && lastKey == key) player.stopMoving();
                if ((key == 'd' || key == 'D') && lastKey == key) player.stopMoving();
                lastKey = key;
            }
            break;

        case State::PAUSED:
            if (key == 'p' || key == 'P') state = State::PLAYING;
            if (key == 'q' || key == 'Q') running = false;
            break;

        case State::DEAD:
        case State::WIN:
        case State::GAME_OVER:
            if (key == '\n' || key == '\r' || key == ' ') {
                if (state == State::WIN) {
                    currentLevel++;
                    if (currentLevel > 3) {
                        leaderboard.addScore(playerName, player.getScore(), 3);
                        running = false;
                    } else {
                        loadLevel(currentLevel);
                        state = State::PLAYING;
                    }
                } else if (state == State::DEAD) {
                    player.reset();
                    player.setPosition(100, 490);
                    loadLevel(currentLevel);
                    state = State::PLAYING;
                } else {
                    leaderboard.addScore(playerName, player.getScore(), currentLevel);
                    running = false;
                }
            }
            if (key == 'q' || key == 'Q') {
                leaderboard.addScore(playerName, player.getScore(), currentLevel);
                running = false;
            }
            break;
    }
}

void Game::update(float dt) {
    if (state != State::PLAYING) return;

    player.update(dt);
    level->update(player, dt);

    // HUD message
    std::string evt = level->popEvent();
    if (!evt.empty()) { hudMessage = evt; hudTimer = 1.5f; }
    if (hudTimer > 0) hudTimer -= dt;

    if (level->playerFell(player)) {
        player.takeDamage(3);   // instant kill
        if (player.isDead()) {
            leaderboard.addScore(playerName, player.getScore(), currentLevel);
            state = State::GAME_OVER;
        } else {
            player.setPosition(100, 490);
            state = State::DEAD;
        }
    }

    if (level->isComplete()) {
        state = State::WIN;
    }
}

void Game::clearScreen() {
    std::cout << "\033[2J\033[H";
}

// ── ASCII Renderer ─────────────────────────────────────────────────────────────
/*
 * Algorithm: 2D Grid Rasterizer
 *
 * Steps:
 * 1. Allocate a 2D char buffer of size SCREEN_W × SCREEN_H, filled with spaces.
 * 2. For each game object, compute screen coordinates by scaling world coords.
 * 3. Write the object's character(s) into the buffer at those coordinates.
 * 4. Print the buffer row-by-row to stdout.
 *
 * Time Complexity: O(W*H + N) where N is total number of objects.
 */
void Game::render() {
    if (state == State::MENU)     { renderMenu();    return; }
    if (state == State::GAME_OVER){ renderGameOver(); return; }

    // Build char buffer
    char buf[SCREEN_H][SCREEN_W + 1];
    for (int r = 0; r < SCREEN_H; ++r) {
        memset(buf[r], ' ', SCREEN_W);
        buf[r][SCREEN_W] = '\0';
    }

    auto draw = [&](float wx, float wy, char ch) {
        int sx = toScreenX(wx);
        int sy = toScreenY(wy);
        if (sx >= 0 && sx < SCREEN_W && sy >= 0 && sy < SCREEN_H)
            buf[sy][sx] = ch;
    };

    // Platforms
    for (const auto& p : level->getPlatforms()) {
        if (p.isBroken()) continue;
        Vec2 pp = p.getPos(), ps = p.getSize();
        int x0 = toScreenX(pp.x), x1 = toScreenX(pp.x + ps.x);
        int y0 = toScreenY(pp.y);
        char tile = (p.getType() == Platform::Type::MOVING) ? '~' :
                    (p.getType() == Platform::Type::BREAKABLE) ? '#' : '=';
        for (int x = x0; x <= x1 && x < SCREEN_W; ++x)
            if (y0 >= 0 && y0 < SCREEN_H) buf[y0][x] = tile;
    }

    // Coins
    for (const auto& c : level->getCoins()) {
        if (c.isCollected()) continue;
        char ch = (c.getType() == Coin::Type::GOLD)   ? '$' :
                  (c.getType() == Coin::Type::SILVER) ? 'o' : '.';
        draw(c.getPos().x, c.getPos().y, ch);
    }

    // Enemies
    for (const auto& e : level->getEnemies()) {
        draw(e.getPos().x, e.getPos().y, 'M');
    }

    // Player
    draw(player.getPos().x, player.getPos().y,
         player.isFacingRight() ? '>' : '<');

    // Draw buffer
    clearScreen();
    renderHUD();
    for (int r = 0; r < SCREEN_H; ++r)
        std::cout << buf[r] << "\n";

    if (state == State::PAUSED) {
        std::cout << "\n  ** PAUSED — press P to resume, Q to quit **\n";
    }
    if (state == State::WIN) {
        std::cout << "\n  ★  LEVEL COMPLETE! Press SPACE for next level  ★\n";
    }
    if (state == State::DEAD) {
        std::cout << "\n  ✗  You fell!  Lives left: " << player.getLives()
                  << "  Press SPACE to retry\n";
    }
}

void Game::renderHUD() {
    std::cout << "  Level: " << currentLevel
              << "   Score: " << player.getScore()
              << "   HP: " << player.getHealth()
              << "   Lives: " << player.getLives();
    if (hudTimer > 0 && !hudMessage.empty())
        std::cout << "   [" << hudMessage << "]";
    std::cout << "\n";
    std::cout << "  Controls: A/D move  W/Space jump  P pause  Q quit\n";
}

void Game::renderMenu() {
    clearScreen();
    std::cout << "\n\n";
    std::cout << "  ╔══════════════════════════════════════╗\n";
    std::cout << "  ║         PLATFORM QUEST  v1.0         ║\n";
    std::cout << "  ╚══════════════════════════════════════╝\n\n";
    std::cout << "  Player: " << playerName << "\n\n";
    std::cout << "  Controls:\n";
    std::cout << "    A / D      — move left / right\n";
    std::cout << "    W / Space  — jump\n";
    std::cout << "    P          — pause\n";
    std::cout << "    Q          — quit\n\n";
    std::cout << "  Symbols:\n";
    std::cout << "    > <        — player (facing direction)\n";
    std::cout << "    M          — enemy (stomp to defeat!)\n";
    std::cout << "    . o $      — coin (bronze / silver / gold)\n";
    std::cout << "    =          — solid platform\n";
    std::cout << "    ~          — moving platform\n";
    std::cout << "    #          — breakable platform\n\n";
    std::cout << "  Collect ALL coins to complete each level!\n\n";
    std::cout << "  Press SPACE or ENTER to start...\n";
    leaderboard.display();
}

void Game::renderGameOver() {
    clearScreen();
    std::cout << "\n\n";
    std::cout << "  ╔══════════════════════╗\n";
    std::cout << "  ║      GAME OVER       ║\n";
    std::cout << "  ╚══════════════════════╝\n\n";
    std::cout << "  Final Score: " << player.getScore() << "\n";
    std::cout << "  Highest Level: " << currentLevel << "\n\n";
    std::cout << "  Press SPACE to save score and exit.\n";
}

void Game::loadLevel(int n) {
    level = std::make_unique<Level>(n);
    player.setPosition(100, 490);
    player.reset();
}
