#pragma once
#include <string>
#include <vector>

/*
 * Author: [Jesus Novoa vasquez]
 * Class: Player
 *
 * Description:
 * Represents the player character in the platformer game.
 * Tracks position, velocity, health, score, and inventory.
 * Uses a vector to store collected coins.
 */

struct Vec2 {
    float x, y;
    Vec2(float x = 0, float y = 0) : x(x), y(y) {}
    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s) const { return {x * s, y * s}; }
};

class Player {
public:
    Player(float startX, float startY);

    void update(float dt);
    void jump();
    void moveLeft();
    void moveRight();
    void stopMoving();
    void applyGravity(float dt);
    void takeDamage(int amount);
    void collectCoin(int value);
    void reset();

    // Getters
    Vec2 getPos() const;
    Vec2 getSize() const;
    int  getHealth() const;
    int  getScore() const;
    int  getLives() const;
    bool isOnGround() const;
    bool isDead() const;
    bool isFacingRight() const;

    // Setters
    void setOnGround(bool grounded);
    void setPosition(float x, float y);
    void setVelocityY(float vy);

private:
    Vec2  pos;
    Vec2  vel;
    Vec2  size;

    int   health;
    int   score;
    int   lives;

    bool  onGround;
    bool  facingRight;

    float moveSpeed;
    float jumpForce;
    float gravity;

    // Data Structure 1: vector to store coin values collected this run
    std::vector<int> coinLog;

    bool  movingLeft;
    bool  movingRight;
};
