/*
 * Author: [Your Name]
 * Class: Platform
 *
 * Description:
 * Implements platform behavior including AABB collision detection
 * and resolution, and horizontal movement for moving platforms.
 */

#include "Platform.h"
#include <cmath>

Platform::Platform(float x, float y, float w, float h, Type t)
    : pos(x, y), size(w, h), type(t),
      moveRange(100.f), moveSpeed(60.f), startX(x), movingRight(true),
      broken(false)
{}

void Platform::update(float dt) {
    if (type == Type::MOVING && !broken) {
        if (movingRight) {
            pos.x += moveSpeed * dt;
            if (pos.x >= startX + moveRange) movingRight = false;
        } else {
            pos.x -= moveSpeed * dt;
            if (pos.x <= startX) movingRight = true;
        }
    }
}

/*
 * Algorithm: AABB (Axis-Aligned Bounding Box) Collision Detection
 *
 * Steps:
 * 1. Compute the right and bottom edges of each rectangle.
 * 2. Check for overlap on both the X and Y axes.
 * 3. If both axes overlap, a collision exists.
 *
 * Time Complexity: O(1) — constant number of comparisons.
 */
bool Platform::collidesWith(Vec2 pPos, Vec2 pSize) const {
    if (broken) return false;
    float pRight  = pPos.x + pSize.x;
    float pBottom = pPos.y + pSize.y;
    float tRight  = pos.x  + size.x;
    float tBottom = pos.y  + size.y;

    return pPos.x < tRight  &&
           pRight  > pos.x  &&
           pPos.y < tBottom &&
           pBottom > pos.y;
}

/*
 * Algorithm: AABB Collision Resolution
 *
 * Steps:
 * 1. Detect overlap amounts on each axis.
 * 2. Determine the minimum penetration axis.
 * 3. Push the player out along that axis.
 * 4. If resolved on Y-axis from above, set player grounded.
 *
 * Time Complexity: O(1)
 */
bool Platform::resolveCollision(Player& player) const {
    if (broken) return false;
    Vec2 pPos  = player.getPos();
    Vec2 pSize = player.getSize();

    if (!collidesWith(pPos, pSize)) return false;

    float overlapLeft  = (pPos.x + pSize.x) - pos.x;
    float overlapRight = (pos.x  + size.x)  - pPos.x;
    float overlapTop   = (pPos.y + pSize.y) - pos.y;
    float overlapBot   = (pos.y  + size.y)  - pPos.y;

    float minOverlapX = std::min(overlapLeft,  overlapRight);
    float minOverlapY = std::min(overlapTop,   overlapBot);

    bool landedOnTop = false;

    if (minOverlapX < minOverlapY) {
        // Resolve horizontally
        if (overlapLeft < overlapRight)
            player.setPosition(pos.x - pSize.x, pPos.y);
        else
            player.setPosition(pos.x + size.x,  pPos.y);
    } else {
        // Resolve vertically
        if (overlapTop < overlapBot) {
            // Player came from above → landed on platform
            player.setPosition(pPos.x, pos.y - pSize.y);
            player.setVelocityY(0);
            player.setOnGround(true);
            landedOnTop = true;
        } else {
            // Player hit bottom of platform
            player.setPosition(pPos.x, pos.y + size.y);
            player.setVelocityY(0);
        }
    }
    return landedOnTop;
}

Vec2          Platform::getPos()   const { return pos; }
Vec2          Platform::getSize()  const { return size; }
Platform::Type Platform::getType() const { return type; }
bool          Platform::isBroken() const { return broken; }
void          Platform::breakIt()        { broken = true; }
