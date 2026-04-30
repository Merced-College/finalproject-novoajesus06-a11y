/*
 * Author: [Your Name]
 * Class: Player
 *
 * Description:
 * Implements all player behavior: movement, jumping, gravity,
 * collision response, health/score tracking, and coin logging.
 */

#include "Player.h"
#include <algorithm>

Player::Player(float startX, float startY)
    : pos(startX, startY), vel(0, 0), size(32, 48),
      health(3), score(0), lives(3),
      onGround(false), facingRight(true),
      moveSpeed(180.f), jumpForce(-420.f), gravity(900.f),
      movingLeft(false), movingRight(false)
{}

void Player::update(float dt) {
    // Apply horizontal movement
    vel.x = 0;
    if (movingLeft)  { vel.x = -moveSpeed; facingRight = false; }
    if (movingRight) { vel.x =  moveSpeed; facingRight = true;  }

    // Integrate position
    applyGravity(dt);
    pos.x += vel.x * dt;
    pos.y += vel.y * dt;
}

void Player::applyGravity(float dt) {
    if (!onGround) {
        vel.y += gravity * dt;
        // Terminal velocity cap
        if (vel.y > 800.f) vel.y = 800.f;
    }
}

void Player::jump() {
    if (onGround) {
        vel.y = jumpForce;
        onGround = false;
    }
}

void Player::moveLeft()    { movingLeft  = true;  }
void Player::moveRight()   { movingRight = true;  }
void Player::stopMoving()  { movingLeft  = false; movingRight = false; }

void Player::takeDamage(int amount) {
    health -= amount;
    if (health <= 0) {
        lives--;
        health = 3;
    }
}

void Player::collectCoin(int value) {
    score += value;
    coinLog.push_back(value);   // Store in vector for logging
}

void Player::reset() {
    vel = {0, 0};
    health = 3;
    onGround = false;
    movingLeft = movingRight = false;
}

// -- Getters --
Vec2 Player::getPos()        const { return pos; }
Vec2 Player::getSize()       const { return size; }
int  Player::getHealth()     const { return health; }
int  Player::getScore()      const { return score; }
int  Player::getLives()      const { return lives; }
bool Player::isOnGround()    const { return onGround; }
bool Player::isDead()        const { return lives <= 0; }
bool Player::isFacingRight() const { return facingRight; }

// -- Setters --
void Player::setOnGround(bool grounded)    { onGround = grounded; }
void Player::setPosition(float x, float y) { pos = {x, y}; }
void Player::setVelocityY(float vy)        { vel.y = vy; }
