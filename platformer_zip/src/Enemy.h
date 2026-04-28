#pragma once
#include "Player.h"

/*
 * Author: [Jesus Novoa vasquez]
 * Class: Enemy
 *
 * Description:
 * Represents an enemy that patrols back and forth on platforms.
 * Can be stomped by the player jumping on top of it.
 */

class Enemy {
public:
    enum class Type { WALKER, JUMPER };

    Enemy(float x, float y, float patrolRange, Type t = Type::WALKER);

    void update(float dt);
    bool checkPlayerCollision(Player& player);  // returns true if enemy is defeated
    bool isDefeated() const;

    Vec2 getPos()  const;
    Vec2 getSize() const;

private:
    Vec2  pos;
    Vec2  vel;
    Vec2  size;
    float startX;
    float patrolRange;
    float speed;

    Type type;
    bool defeated;
    bool movingRight;

    float jumpTimer;   // for JUMPER type
};
