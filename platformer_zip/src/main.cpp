/*
 * Author: [Jesus Novoa vasquez]
 * File:   main.cpp
 *
 * Entry point for Platform Quest.
 * Creates the Game object and calls run(), which handles
 * the full game loop until the player quits.
 */

#include "Game.h"

int main() {
    Game game;
    game.run();
    return 0;
}
