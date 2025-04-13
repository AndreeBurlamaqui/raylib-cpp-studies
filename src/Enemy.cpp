#include "Entities.h"

Enemy::Enemy(Entity* startTarget, float angleOrbit, float startSpeed, float startSize, Color startTint) : Entity(getRandomOutsidePosition(), startSpeed, startSize, startTint), target(startTarget) , orbitPos(angleOrbit)
{
    hitsToBlock = GetRandomValue(2, 10); // Set new threshold
    guardTimer = 0;

    setStrength(5);
}

void Enemy::Tick(const float& deltaTime) {
    if (!isAlive()) {
        return;
    }

    if (isAttacking()) {
        onHitTimer = GetRandomValue(1, 3);
    }

    if (isOnGuard()) {
        guardTimer -= deltaTime;

        if (guardTimer <= 0) {
            consecutiveHits = 0;
        }
    }

    if (isOnKnockback()) {
        onDamagedTimer = onHitCooldown;
    }
    else {
        Vector2 target = getTarget()->getPosition();;
        int leftOrRight = consecutiveHits % 2 == 0 ? 1 : -1; // 1 for clockwise, -1 for counterclockwise
        orbitPos = Wrap(orbitPos + (deltaTime * 90 * leftOrRight), 0, 360);

        float angleRadians = orbitPos * DEG2RAD;  // Convert degrees to radians
        Vector2 nextPosition = {
            target.x + (getAttackDistance() * static_cast<float>(cos(angleRadians))) * leftOrRight ,
            target.y + (getAttackDistance() * static_cast<float>(sin(angleRadians))) * leftOrRight
        };

        Vector2 direction = Vector2Subtract(nextPosition, getPosition());

        if (onDamagedTimer <= 0 && onHitTimer <= 0) {
            // Based on the target, go towards it
            if (Vector2Length(direction) > getAttackDistance()) {
                setMotion(direction.x, direction.y);
            }
            else {
                // Keep rotating while keeping distance

                setMotion(-direction.x, -direction.y); // Keep distance
            }

            hasBlocked = false;
        }
        else {
            onDamagedTimer -= deltaTime; // Rest a bit after receiving a punch
            onHitTimer -= deltaTime; // Rest after punching
            if (Vector2Length(direction) < getAttackDistance() * 0.75f) {
                // To close, go away
                setMotion(-direction.x, -direction.y);
            }
            else {
                setMotion(0, 0); // Keep distance
            }
        }
    }

	Entity::Tick(deltaTime);

    //DrawText(TextFormat("%i / %i", consecutiveHits, hitsToBlock), getPosition().x, getPosition().y + (getSize() * 2), 20, BLACK);
}

void Enemy::Reset() {
    Vector2 randomPos = getRandomOutsidePosition();
    setPosition(randomPos.x, randomPos.y);

    Entity::Reset();
}

Vector2 Enemy::getRandomOutsidePosition() {
    const int margin = 50;
    int side = GetRandomValue(0, 3); // 0 = left, 1 = right, 2 = top, 3 = bottom
    Vector2 pos{ 0,0 };

    switch (side) {
    case 0: // Left
        pos.x = -margin;
        pos.y = GetRandomValue(0, GetScreenHeight());
        break;
    case 1: // Right
        pos.x = GetScreenWidth() + margin;
        pos.y = GetRandomValue(0, GetScreenHeight());
        break;
    case 2: // Top
        pos.x = GetRandomValue(0, GetScreenWidth());
        pos.y = -margin;
        break;
    case 3: // Bottom
        pos.x = GetRandomValue(0, GetScreenWidth());
        pos.y = GetScreenHeight() + margin;
        break;
    }
    return pos;
}

bool Enemy::IsTryingToAttack() {
    return
        onHitTimer <= 0 && // Has recently attacked?
        onDamagedTimer <= 0 && // Is it on damaged stun?
        Vector2Length(Vector2Subtract(getTarget()->getPosition(), getPosition())) <= (getAttackDistance() + getAttackRadius()); // Is near player to attack
}

bool Enemy::IsTryingToDefend() {
    return guardTimer > 0; // Check if getting hit and defend
}

Vector2 Enemy::GetTargetPosition() {
    if (isAttacking()) {
        return Entity::GetTargetPosition(); // It'll return the current attack pos, so that the enemy attack one "line" per punch
    }

    return getTarget()->getPosition();
}

bool Enemy::TryGetDamagedBy(Entity& attacker, int damage) {
    if (hasBlocked) {
        return false; // Block only once per knockback. Don't stack false hits
    }

    if (Entity::TryGetDamagedBy(attacker, damage)) {
        // In case it was hit, add to consecutive hits
        consecutiveHits++;
        if (consecutiveHits > hitsToBlock) {
            guardTimer = guardMaxTime;
        }
        return true;
    }

    if (isOnGuard()) {
        hasBlocked = true;
        hitsToBlock = GetRandomValue(2, 10); // Set new threshold
        consecutiveHits /= 2; // Each block, decrease by half until 0
        if (consecutiveHits <= 0) {
            guardTimer = 0;
        }
    }

    return false;
}

void Enemy::CheckHit(Entity& other) {
    if (target == nullptr || target != &other) {
        return;
    }

    Entity::CheckHit(other);
}
