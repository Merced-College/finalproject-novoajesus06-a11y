/*
 * Author: [Your Name]
 * Class: Coin
 *
 * Description:
 * Collectible coin with a sine-wave bobbing animation
 * and AABB pickup detection.
 */

#include "Coin.h"
#include <cmath>

Coin::Coin(float x, float y, Type t)
    : pos(x, y), size(16, 16), type(t),
      collected(false), bobOffset(0), bobTimer(0)
{}

void Coin::update(float dt) {
    if (collected) return;
    bobTimer  += dt * 3.0f;
    bobOffset  = std::sin(bobTimer) * 4.0f;
}

bool Coin::checkCollect(Player& player) {
    if (collected) return false;

    Vec2 pPos  = player.getPos();
    Vec2 pSize = player.getSize();
    Vec2 cPos  = {pos.x, pos.y + bobOffset};

    bool overlap = pPos.x < cPos.x + size.x &&
                   pPos.x + pSize.x > cPos.x &&
                   pPos.y < cPos.y + size.y  &&
                   pPos.y + pSize.y > cPos.y;

    if (overlap) {
        collected = true;
        player.collectCoin(static_cast<int>(type));
        return true;
    }
    return false;
}

bool      Coin::isCollected() const { return collected; }
Vec2      Coin::getPos()      const { return {pos.x, pos.y + bobOffset}; }
Vec2      Coin::getSize()     const { return size; }
Coin::Type Coin::getType()    const { return type; }
