#pragma once
#include "Player.h"  // for Vec2

/*
 * Author: [Jesus Novoa vasquez]
 * Class: Platform
 *
 * Description:
 * Represents a solid rectangular platform tile.
 * Provides AABB collision detection with the player.
 */

class Platform {
public:
    enum class Type { SOLID, MOVING, BREAKABLE };

    Platform(float x, float y, float w, float h, Type type = Type::SOLID);

    void update(float dt);

    // AABB collision check
    bool collidesWith(Vec2 pos, Vec2 size) const;

    // Resolve player collision — returns true if player landed on top
    bool resolveCollision(Player& player) const;

    Vec2 getPos()  const;
    Vec2 getSize() const;
    Type getType() const;

    bool isBroken() const;
    void breakIt();

private:
    Vec2 pos;
    Vec2 size;
    Type type;

    // Moving platform data
    float moveRange;
    float moveSpeed;
    float startX;
    bool  movingRight;

    bool broken;
};
