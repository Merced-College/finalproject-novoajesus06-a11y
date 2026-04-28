#pragma once
#include "Player.h"

/*
 * Author: [Jesus Novoa vasquez]
 * Class: Coin
 *
 * Description:
 * A collectible pickup that increases the player's score.
 * Supports different coin types with different values.
 */

class Coin {
public:
    enum class Type { BRONZE = 10, SILVER = 25, GOLD = 50 };

    Coin(float x, float y, Type t = Type::BRONZE);

    void update(float dt);
    bool checkCollect(Player& player);
    bool isCollected() const;

    Vec2 getPos()  const;
    Vec2 getSize() const;
    Type getType() const;

private:
    Vec2  pos;
    Vec2  size;
    Type  type;
    bool  collected;
    float bobOffset;   // for animation
    float bobTimer;
};
