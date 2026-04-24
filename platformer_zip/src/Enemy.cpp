/*
 * Author: [Your Name]
 * Class: Enemy
 *
 * Description:
 * Implements enemy patrol AI and stomp detection.
 * WALKER patrols a fixed range. JUMPER also periodically jumps.
 */

#include "Enemy.h"
#include <cmath>

Enemy::Enemy(float x, float y, float range, Type t)
    : pos(x, y), vel(0, 0), size(32, 32),
      startX(x), patrolRange(range),
      speed(80.f), type(t), defeated(false),
      movingRight(true), jumpTimer(1.5f)
{}

void Enemy::update(float dt) {
    if (defeated) return;

    // Patrol AI
    if (movingRight) {
        vel.x = speed;
        if (pos.x >= startX + patrolRange) movingRight = false;
    } else {
        vel.x = -speed;
        if (pos.x <= startX) movingRight = true;
    }

    // Jumper type: jump periodically
    if (type == Type::JUMPER) {
        jumpTimer -= dt;
        if (jumpTimer <= 0.f) {
            vel.y = -320.f;
            jumpTimer = 2.0f;
        }
        vel.y += 900.f * dt;
        if (vel.y > 800.f) vel.y = 800.f;
        pos.y += vel.y * dt;
        // Simple ground snap (handled by level)
    }

    pos.x += vel.x * dt;
}

/*
 * Algorithm: Stomp Detection
 *
 * Steps:
 * 1. Check AABB overlap between player and enemy.
 * 2. If overlapping AND the player's bottom edge is near the enemy's top,
 *    the player stomped the enemy.
 * 3. Otherwise the player takes damage.
 *
 * Time Complexity: O(1)
 */
bool Enemy::checkPlayerCollision(Player& player) {
    if (defeated) return false;

    Vec2 pPos  = player.getPos();
    Vec2 pSize = player.getSize();

    // AABB check
    bool overlap = pPos.x < pos.x + size.x &&
                   pPos.x + pSize.x > pos.x &&
                   pPos.y < pos.y + size.y  &&
                   pPos.y + pSize.y > pos.y;

    if (!overlap) return false;

    float playerBottom = pPos.y + pSize.y;
    float enemyTop     = pos.y;
    float stompThreshold = 12.f;

    if (playerBottom - enemyTop < stompThreshold) {
        // Stomp!
        defeated = true;
        player.setVelocityY(-260.f);   // bounce player up
        player.collectCoin(100);
        return true;
    } else {
        player.takeDamage(1);
        return false;
    }
}

bool Enemy::isDefeated() const { return defeated; }
Vec2 Enemy::getPos()     const { return pos; }
Vec2 Enemy::getSize()    const { return size; }
